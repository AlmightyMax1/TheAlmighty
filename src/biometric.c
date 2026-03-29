#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include "biometric.h"
#include "mux.h"

#define ADC_VREF_MV     600
#define ADC_GAIN_FACTOR 6
#define ADC_FULL_SCALE  4095
#define GSR_REF_R_KOHM  100

static const struct device *max30102_dev;
static const struct device *adc_dev;

static const struct adc_channel_cfg mux_a_cfg = {
    .gain             = ADC_GAIN_1_6,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = MUX_A_ADC_CH,
    .input_positive   = NRF_SAADC_AIN0,
};

static uint8_t ratio_to_spo2(uint8_t r_x10)
{
    if (r_x10 < 5U)  { return 100U; }
    if (r_x10 > 33U) { return 70U;  }
    return (uint8_t)(110U - (uint32_t)r_x10 * 25U / 10U);
}

int biometric_init(void)
{
    max30102_dev = DEVICE_DT_GET_ANY(maxim_max30102);
    if (!max30102_dev || !device_is_ready(max30102_dev)) {
        printk("[BIO] MAX30102 not ready\n");
        return -ENODEV;
    }

    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }
    if (adc_channel_setup(adc_dev, &mux_a_cfg)) {
        return -EIO;
    }

    printk("[BIO] SpO2 / HR / GSR ready\n");
    return 0;
}

int biometric_read(struct biometric_data *out)
{
    struct sensor_value red_val, ir_val;

    int err = sensor_sample_fetch(max30102_dev);
    if (err) {
        out->valid = false;
        return err;
    }

    sensor_channel_get(max30102_dev, SENSOR_CHAN_RED, &red_val);
    sensor_channel_get(max30102_dev, SENSOR_CHAN_IR,  &ir_val);

    uint32_t red    = (uint32_t)red_val.val1;
    uint32_t ir     = MAX((uint32_t)ir_val.val1, 1U);
    uint8_t  r_x10  = (uint8_t)CLAMP((red * 10U) / ir, 0U, 40U);

    out->spo2_pct           = ratio_to_spo2(r_x10);
    out->heart_rate_bpm     = 0U;
    out->collar_temp_c_x100 = 3700;

    mux_select(MUX_A, MUXA_GSR);

    int16_t sample = 0;
    struct adc_sequence seq = {
        .channels    = BIT(MUX_A_ADC_CH),
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };

    err = adc_read(adc_dev, &seq);
    mux_release(MUX_A);

    if (err == 0) {
        int32_t vref_mv = ADC_VREF_MV * ADC_GAIN_FACTOR;
        int32_t v_mv    = ((int32_t)sample * vref_mv) / ADC_FULL_SCALE;
        if (v_mv > 0 && v_mv < vref_mv) {
            int32_t r_kohm = (GSR_REF_R_KOHM * v_mv) / (vref_mv - v_mv);
            out->gsr_kohm  = (uint16_t)CLAMP(r_kohm, 0, 65535);
        } else {
            out->gsr_kohm = 0U;
        }
    }

    out->valid = true;
    return 0;
}
