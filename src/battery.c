#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include "battery.h"
#include "mux.h"

#define ADC_VREF_MV     600
#define ADC_GAIN_FACTOR 6
#define ADC_FULL_SCALE  4095
#define BATT_VDIV_R1    100000U
#define BATT_VDIV_R2    100000U
#define BATT_VMAX_MV    4200U
#define BATT_VMIN_MV    3000U

static const struct device *adc_dev;

static const struct adc_channel_cfg mux_a_cfg = {
    .gain             = ADC_GAIN_1_6,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = MUX_A_ADC_CH,
    .input_positive   = NRF_SAADC_AIN0,
};

int battery_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }
    if (adc_channel_setup(adc_dev, &mux_a_cfg)) {
        return -EIO;
    }
    printk("[BATT] Li-ion monitor via MUX_A ch2 ready\n");
    return 0;
}

int battery_read(struct battery_data *out)
{
    mux_select(MUX_A, MUXA_BATTERY);

    int16_t sample = 0;
    struct adc_sequence seq = {
        .channels    = BIT(MUX_A_ADC_CH),
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };

    int err = adc_read(adc_dev, &seq);
    mux_release(MUX_A);

    if (err) {
        return -EIO;
    }

    int32_t  vref_mv  = ADC_VREF_MV * ADC_GAIN_FACTOR;
    uint32_t vadc_mv  = (uint32_t)(((int32_t)sample * vref_mv) / ADC_FULL_SCALE);
    uint32_t vbatt_mv = vadc_mv * (BATT_VDIV_R1 + BATT_VDIV_R2) / BATT_VDIV_R2;

    out->voltage_mv = vbatt_mv;

    if (vbatt_mv >= BATT_VMAX_MV) {
        out->soc_pct = 100U;
    } else if (vbatt_mv <= BATT_VMIN_MV) {
        out->soc_pct = 0U;
    } else {
        out->soc_pct = (uint8_t)((vbatt_mv - BATT_VMIN_MV) * 100U /
                                  (BATT_VMAX_MV - BATT_VMIN_MV));
    }

    return 0;
}
