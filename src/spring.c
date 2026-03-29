#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "spring.h"

static const struct gpio_dt_spec sel0 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(spring_sel0), gpios);
static const struct gpio_dt_spec sel1 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(spring_sel1), gpios);

static spring_mode_t current_mode = SPRING_MODE_ENDURANCE;

int spring_init(void)
{
    if (!gpio_is_ready_dt(&sel0) || !gpio_is_ready_dt(&sel1)) {
        return -ENODEV;
    }
    gpio_pin_configure_dt(&sel0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&sel1, GPIO_OUTPUT_INACTIVE);
    spring_set_mode(SPRING_MODE_ENDURANCE);
    printk("[SPRING] 3-mode cassette ready\n");
    return 0;
}

int spring_set_mode(spring_mode_t mode)
{
    if (mode > SPRING_MODE_STABILITY) {
        return -EINVAL;
    }
    gpio_pin_set_dt(&sel0, (int)((mode & 0x01U) != 0U));
    gpio_pin_set_dt(&sel1, (int)((mode & 0x02U) != 0U));
    current_mode = mode;
    printk("[SPRING] mode=%u\n", (uint8_t)mode);
    return 0;
}

spring_mode_t spring_get_mode(void)
{
    return current_mode;
}
