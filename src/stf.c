#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "stf.h"

static const struct gpio_dt_spec stf_pin =
    GPIO_DT_SPEC_GET(DT_NODELABEL(stf_enable), gpios);

static stf_state_t cur_state    = STF_STATE_SOFT;
static int64_t     lock_time_ms = 0;

#define STF_HYSTERESIS_MG 500
#define STF_HOLD_MS        80

int stf_init(void)
{
    if (!gpio_is_ready_dt(&stf_pin)) {
        return -ENODEV;
    }
    gpio_pin_configure_dt(&stf_pin, GPIO_OUTPUT_INACTIVE);
    printk("[STF] STF sidewall capsule ready\n");
    return 0;
}

stf_state_t stf_update(int32_t accel_mag_mg, uint8_t threshold_g)
{
    int32_t threshold_mg = (int32_t)threshold_g * 1000;
    int64_t now          = k_uptime_get();

    if (accel_mag_mg >= threshold_mg) {
        gpio_pin_set_dt(&stf_pin, 1);
        cur_state    = STF_STATE_LOCKED;
        lock_time_ms = now;
    } else if (cur_state == STF_STATE_LOCKED) {
        if ((now - lock_time_ms) >= STF_HOLD_MS &&
            accel_mag_mg < (threshold_mg - STF_HYSTERESIS_MG)) {
            gpio_pin_set_dt(&stf_pin, 0);
            cur_state = STF_STATE_SOFT;
        }
    }

    return cur_state;
}

stf_state_t stf_get_state(void)
{
    return cur_state;
}
