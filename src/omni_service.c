#include <errno.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "omni_service.h"

static struct bt_uuid_128 omni_svc_uuid   = BT_UUID_INIT_128(OMNI_SVC_UUID_VAL);
static struct bt_uuid_128 omni_telem_uuid = BT_UUID_INIT_128(OMNI_TELEM_UUID_VAL);
static struct bt_uuid_128 omni_ctrl_uuid  = BT_UUID_INIT_128(OMNI_CTRL_UUID_VAL);

static struct bt_conn    *active_conn;
static bool               notify_enabled;
static struct omni_telemetry cur_telem;

K_MUTEX_DEFINE(ctrl_mutex);
static struct omni_control cur_ctrl = {
    .spring_mode       = 0U,
    .heat_target_zone0 = 22U,
    .heat_target_zone1 = 22U,
    .heat_target_zone2 = 22U,
    .heat_target_zone3 = 22U,
    .stf_threshold_g   = 30U,
    .pcm_valve_open    = 0U,
    .haptic_pattern    = 0U,
    .haptic_trigger    = 0U,
    .beacon_activate   = 0U,
};

static ssize_t telem_read(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr,
                           void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &cur_telem, sizeof(cur_telem));
}

static ssize_t ctrl_read(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset)
{
    struct omni_control snap;
    k_mutex_lock(&ctrl_mutex, K_FOREVER);
    snap = cur_ctrl;
    k_mutex_unlock(&ctrl_mutex);
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &snap, sizeof(snap));
}

static ssize_t ctrl_write(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr,
                           const void *buf, uint16_t len,
                           uint16_t offset, uint8_t flags)
{
    struct omni_control incoming;
    ARG_UNUSED(conn);
    ARG_UNUSED(attr);
    ARG_UNUSED(flags);

    if (offset != 0U) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    if (len != sizeof(incoming)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    memcpy(&incoming, buf, sizeof(incoming));

    if (incoming.spring_mode > 2U) {
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }
    incoming.heat_target_zone0 = MIN(incoming.heat_target_zone0, 45U);
    incoming.heat_target_zone1 = MIN(incoming.heat_target_zone1, 45U);
    incoming.heat_target_zone2 = MIN(incoming.heat_target_zone2, 45U);
    incoming.heat_target_zone3 = MIN(incoming.heat_target_zone3, 45U);

    k_mutex_lock(&ctrl_mutex, K_FOREVER);
    cur_ctrl = incoming;
    k_mutex_unlock(&ctrl_mutex);

    printk("[CTRL] spring=%u heat=[%u %u %u %u] stf=%ug haptic=%u beacon=%u\n",
           incoming.spring_mode,
           incoming.heat_target_zone0, incoming.heat_target_zone1,
           incoming.heat_target_zone2, incoming.heat_target_zone3,
           incoming.stf_threshold_g,   incoming.haptic_pattern,
           incoming.beacon_activate);

    return (ssize_t)len;
}

static void telem_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    notify_enabled = (value == BT_GATT_CCC_NOTIFY);
    printk("[BLE] notify %s\n", notify_enabled ? "ON" : "OFF");
}

BT_GATT_SERVICE_DEFINE(omni_svc,
    BT_GATT_PRIMARY_SERVICE(&omni_svc_uuid.uuid),

    BT_GATT_CHARACTERISTIC(&omni_telem_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        telem_read, NULL, &cur_telem),
    BT_GATT_CCC(telem_ccc_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    BT_GATT_CHARACTERISTIC(&omni_ctrl_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
            BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        ctrl_read, ctrl_write, &cur_ctrl),
);

int omni_service_init(void)
{
    memset(&cur_telem, 0, sizeof(cur_telem));
    return 0;
}

void omni_service_connected(struct bt_conn *conn)
{
    if (active_conn) {
        bt_conn_unref(active_conn);
    }
    active_conn = bt_conn_ref(conn);
}

void omni_service_disconnected(void)
{
    if (active_conn) {
        bt_conn_unref(active_conn);
        active_conn = NULL;
    }
    notify_enabled = false;
}

void omni_service_update(const struct omni_telemetry *t)
{
    cur_telem = *t;
}

int omni_service_notify(void)
{
    if (!active_conn || !notify_enabled) {
        return -ENOTCONN;
    }
    return bt_gatt_notify(active_conn, &omni_svc.attrs[2],
                          &cur_telem, sizeof(cur_telem));
}

void omni_service_get_control_copy(struct omni_control *out)
{
    k_mutex_lock(&ctrl_mutex, K_FOREVER);
    *out = cur_ctrl;
    k_mutex_unlock(&ctrl_mutex);
}
