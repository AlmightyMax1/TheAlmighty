#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include "emg.h"
#include "mux.h"

#define ADC_VREF_MV     600
#define ADC_GAIN_FACTOR 6
#define ADC_FULL_SCALE  4095
#define INA_GAIN        100

static const struct device *adc_dev;

static const struct adc_channel_cfg mux_b_cfg = {
    .gain             = ADC_GAIN_1_6,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = MUX_B_ADC_CH,
    .input_positive   = NRF_SAADC_AIN1,
};

int emg_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }
    if (adc_channel_setup(adc_dev, &mux_b_cfg)) {
        return -EIO;
    }
    printk("[EMG] 4-site muscle monitoring via MUX_B ready\n");
    return 0;
}

int emg_read(struct emg_data *out)
{
    int16_t sample = 0;
    struct adc_sequence seq = {
        .channels    = BIT(MUX_B_ADC_CH),
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };

    for (int i = 0; i < EMG_NUM_SITES; i++) {
        sample = 0;
        mux_select(MUX_B, (uint8_t)i);
        int err = adc_read(adc_dev, &seq);
        mux_release(MUX_B);

        if (err) {
            out->valid = false;
            return -EIO;
        }

        int32_t vref_mv = ADC_VREF_MV * ADC_GAIN_FACTOR;
        int32_t v_mv    = ((int32_t)sample * vref_mv) / ADC_FULL_SCALE;
        out->mv[i]      = (int16_t)(v_mv / INA_GAIN);
    }

    out->valid = true;
    return 0;
}
