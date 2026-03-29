#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "pcm.h"

static const struct gpio_dt_spec valve_pin =
    GPIO_DT_SPEC_GET(DT_NODELABEL(pcm_valve), gpios);

static pcm_state_t cur_state = PCM_STATE_INACTIVE;

#define PCM_MELT_TEMP_X100  3700
#define PCM_STORE_TEMP_X100 3800
#define PCM_HYST_X100         50

int pcm_init(void)
{
    if (!gpio_is_ready_dt(&valve_pin)) {
        return -ENODEV;
    }
    gpio_pin_configure_dt(&valve_pin, GPIO_OUTPUT_INACTIVE);
    printk("[PCM] thermal buffer ready (37-38C)\n");
    return 0;
}

pcm_state_t pcm_update(int32_t avg_temp_c_x100, bool valve_open_req)
{
    if (valve_open_req) {
        gpio_pin_set_dt(&valve_pin, 1);
        cur_state = PCM_STATE_MELTING;
        return cur_state;
    }

    if (avg_temp_c_x100 <= PCM_MELT_TEMP_X100) {
        gpio_pin_set_dt(&valve_pin, 1);
        cur_state = PCM_STATE_MELTING;
    } else if (avg_temp_c_x100 >= PCM_STORE_TEMP_X100) {
        gpio_pin_set_dt(&valve_pin, 0);
        cur_state = PCM_STATE_STORING;
    } else if (cur_state == PCM_STATE_MELTING &&
               avg_temp_c_x100 >= (PCM_MELT_TEMP_X100 + PCM_HYST_X100)) {
        gpio_pin_set_dt(&valve_pin, 0);
        cur_state = PCM_STATE_INACTIVE;
    }

    return cur_state;
}

pcm_state_t pcm_get_state(void)
{
    return cur_state;
}
