#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "mux.h"

#define MUX_SETTLE_US 10U

static const struct gpio_dt_spec muxa_a0 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxa_a0), gpios);
static const struct gpio_dt_spec muxa_a1 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxa_a1), gpios);
static const struct gpio_dt_spec muxa_a2 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxa_a2), gpios);

static const struct gpio_dt_spec muxb_a0 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxb_a0), gpios);
static const struct gpio_dt_spec muxb_a1 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxb_a1), gpios);
static const struct gpio_dt_spec muxb_a2 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(muxb_a2), gpios);

static K_MUTEX_DEFINE(muxa_mutex);
static K_MUTEX_DEFINE(muxb_mutex);

int mux_init(void)
{
    const struct gpio_dt_spec *pins[6] = {
        &muxa_a0, &muxa_a1, &muxa_a2,
        &muxb_a0, &muxb_a1, &muxb_a2,
    };

    for (int i = 0; i < 6; i++) {
        if (!gpio_is_ready_dt(pins[i])) {
            printk("[MUX] pin %d not ready\n", i);
            return -ENODEV;
        }
        gpio_pin_configure_dt(pins[i], GPIO_OUTPUT_INACTIVE);
    }

    printk("[MUX] ADG1608 MUX_A + MUX_B ready\n");
    return 0;
}

int mux_select(mux_id_t mux, uint8_t ch)
{
    if (ch > 7U) {
        return -EINVAL;
    }

    const struct gpio_dt_spec *a0, *a1, *a2;
    struct k_mutex            *mtx;

    if (mux == MUX_A) {
        a0  = &muxa_a0;
        a1  = &muxa_a1;
        a2  = &muxa_a2;
        mtx = &muxa_mutex;
    } else {
        a0  = &muxb_a0;
        a1  = &muxb_a1;
        a2  = &muxb_a2;
        mtx = &muxb_mutex;
    }

    k_mutex_lock(mtx, K_FOREVER);

    gpio_pin_set_dt(a0, (int)((ch & 0x01U) != 0U));
    gpio_pin_set_dt(a1, (int)((ch & 0x02U) != 0U));
    gpio_pin_set_dt(a2, (int)((ch & 0x04U) != 0U));

    k_busy_wait(MUX_SETTLE_US);
    return 0;
}

void mux_release(mux_id_t mux)
{
    if (mux == MUX_A) {
        k_mutex_unlock(&muxa_mutex);
    } else {
        k_mutex_unlock(&muxb_mutex);
    }
}
