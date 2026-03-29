#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include "pressure.h"

#define ADC_VREF_MV     600
#define ADC_GAIN_FACTOR 6
#define ADC_FULL_SCALE  4095
#define SENSOR_VMIN_MV  200
#define SENSOR_VMAX_MV  3300
#define SENSOR_PMAX_KPA 700U

static const struct device *adc_dev;

static const struct adc_channel_cfg ch_cfg[4] = {
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 0,
        .input_positive   = NRF_SAADC_AIN0,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 1,
        .input_positive   = NRF_SAADC_AIN1,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 2,
        .input_positive   = NRF_SAADC_AIN2,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 3,
        .input_positive   = NRF_SAADC_AIN3,
    },
};

int pressure_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }
    for (int i = 0; i < 4; i++) {
        if (adc_channel_setup(adc_dev, &ch_cfg[i])) {
            return -EIO;
        }
    }
    printk("[PRES] 4-zone matrix ready\n");
    return 0;
}

static uint32_t raw_to_kpa(int32_t raw)
{
    int32_t vref_mv = ADC_VREF_MV * ADC_GAIN_FACTOR;
    int32_t mv      = (raw * vref_mv) / ADC_FULL_SCALE;

    if (mv <= SENSOR_VMIN_MV) {
        return 0U;
    }
    if (mv >= SENSOR_VMAX_MV) {
        return SENSOR_PMAX_KPA;
    }
    return (uint32_t)((mv - SENSOR_VMIN_MV) * (int32_t)SENSOR_PMAX_KPA /
                      (SENSOR_VMAX_MV - SENSOR_VMIN_MV));
}

int pressure_read(struct pressure_data *out)
{
    int16_t sample = 0;
    struct adc_sequence seq = {
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };
    uint32_t results[4];

    for (int i = 0; i < 4; i++) {
        sample       = 0;
        seq.channels = BIT(i);
        if (adc_read(adc_dev, &seq)) {
            out->valid = false;
            return -EIO;
        }
        results[i] = raw_to_kpa((int32_t)sample);
    }

    out->fore_kpa    = results[0];
    out->heel_kpa    = results[1];
    out->medial_kpa  = results[2];
    out->lateral_kpa = results[3];
    out->valid       = true;
    return 0;
}
