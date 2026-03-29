#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "harvest.h"
#include "mux.h"

#define ADC_VREF_MV      600
#define ADC_GAIN_FACTOR  6
#define ADC_FULL_SCALE   4095
#define HARVEST_LOAD_OHM 100U
#define HARVEST_VMIN_MV  2800U

static const struct device       *adc_dev;
static const struct gpio_dt_spec  gate_pin =
    GPIO_DT_SPEC_GET(DT_NODELABEL(harvest_gate), gpios);

static const struct adc_channel_cfg mux_a_adc_cfg = {
    .gain             = ADC_GAIN_1_6,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = MUX_A_ADC_CH,
    .input_positive   = NRF_SAADC_AIN0,
};

int harvest_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }
    if (adc_channel_setup(adc_dev, &mux_a_adc_cfg)) {
        return -EIO;
    }
    if (!gpio_is_ready_dt(&gate_pin)) {
        return -ENODEV;
    }
    gpio_pin_configure_dt(&gate_pin, GPIO_OUTPUT_INACTIVE);
    printk("[HARV] TENG + EM harvest via MUX_A ready\n");
    return 0;
}

static uint32_t read_mv_via_mux(uint8_t mux_ch)
{
    mux_select(MUX_A, mux_ch);

    int16_t sample = 0;
    struct adc_sequence seq = {
        .channels    = BIT(MUX_A_ADC_CH),
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };

    adc_read(adc_dev, &seq);
    mux_release(MUX_A);

    int32_t vref_mv = ADC_VREF_MV * ADC_GAIN_FACTOR;
    int32_t v       = ((int32_t)sample * vref_mv) / ADC_FULL_SCALE;
    return (uint32_t)MAX(v, 0);
}

static uint32_t mv_to_power_mw_x10(uint32_t v_mv)
{
    uint32_t i_ua  = (v_mv * 1000U) / HARVEST_LOAD_OHM;
    uint32_t p_uw  = (v_mv * i_ua)  / 1000U;
    return p_uw / 100U;
}

int harvest_read(struct harvest_data *out)
{
    out->teng_voltage_mv   = read_mv_via_mux(MUXA_TENG);
    out->teng_power_mw_x10 = mv_to_power_mw_x10(out->teng_voltage_mv);

    out->em_voltage_mv   = read_mv_via_mux(MUXA_EM_GEN);
    out->em_power_mw_x10 = mv_to_power_mw_x10(out->em_voltage_mv);

    bool should_open = (out->teng_voltage_mv >= HARVEST_VMIN_MV ||
                        out->em_voltage_mv   >= HARVEST_VMIN_MV);
    harvest_set_gate(should_open);
    out->gate_open = should_open;
    return 0;
}

void harvest_set_gate(bool open)
{
    gpio_pin_set_dt(&gate_pin, open ? 1 : 0);
}
