#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include "heating.h"

#define ADC_VREF_MV     600
#define ADC_GAIN_FACTOR 6
#define ADC_FULL_SCALE  4095
#define NTC_REF_R       10000
#define PID_KP_X100     120
#define PID_KI_X100      10
#define PID_KD_X100       5

static const struct device *adc_dev;
static const struct device *pwm_dev;
static int32_t pid_integral[4];
static int32_t pid_prev_err[4];

static const int32_t NTC_LUT_R[16] = {
    32650, 25390, 19900, 15710, 12490, 10000, 8057, 6531,
     5327,  4369,  3603,  2986,  2488,  2083, 1752, 1481
};
static const int32_t NTC_LUT_T_X100[16] = {
       0,  250,  500,  750, 1000, 1250, 1500, 1750,
    2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750
};

static const struct adc_channel_cfg therm_cfg[4] = {
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 4,
        .input_positive   = NRF_SAADC_AIN4,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 5,
        .input_positive   = NRF_SAADC_AIN5,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 6,
        .input_positive   = NRF_SAADC_AIN6,
    },
    {
        .gain             = ADC_GAIN_1_6,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = 7,
        .input_positive   = NRF_SAADC_AIN7,
    },
};

static int32_t raw_to_temp_x100(int32_t raw)
{
    int32_t vref_mv = ADC_VREF_MV * ADC_GAIN_FACTOR;
    int32_t v_mv    = (raw * vref_mv) / ADC_FULL_SCALE;

    if (v_mv <= 0 || v_mv >= vref_mv) {
        return 2500;
    }

    int32_t r_ntc = (NTC_REF_R * v_mv) / (vref_mv - v_mv);

    for (int i = 0; i < 15; i++) {
        if (r_ntc <= NTC_LUT_R[i] && r_ntc > NTC_LUT_R[i + 1]) {
            int32_t r_span = NTC_LUT_R[i]          - NTC_LUT_R[i + 1];
            int32_t r_off  = NTC_LUT_R[i]          - r_ntc;
            int32_t t_span = NTC_LUT_T_X100[i + 1] - NTC_LUT_T_X100[i];
            return NTC_LUT_T_X100[i] + (r_off * t_span / r_span);
        }
    }

    return (r_ntc >= NTC_LUT_R[0]) ? NTC_LUT_T_X100[0] : NTC_LUT_T_X100[15];
}

static uint8_t pid_compute(int z, int32_t sp_x100, int32_t pv_x100)
{
    int32_t err       = sp_x100 - pv_x100;
    pid_integral[z]  += err;
    pid_integral[z]   = CLAMP(pid_integral[z], -10000, 10000);
    int32_t deriv     = err - pid_prev_err[z];
    pid_prev_err[z]   = err;
    int32_t out = (PID_KP_X100 * err +
                   PID_KI_X100 * pid_integral[z] +
                   PID_KD_X100 * deriv) / 100;
    return (uint8_t)CLAMP(out, 0, 100);
}

int heating_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        return -ENODEV;
    }

    pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm0));
    if (!device_is_ready(pwm_dev)) {
        return -ENODEV;
    }

    for (int z = 0; z < 4; z++) {
        if (adc_channel_setup(adc_dev, &therm_cfg[z])) {
            return -EIO;
        }
        pid_integral[z] = 0;
        pid_prev_err[z] = 0;
        pwm_set(pwm_dev, z, PWM_HZ(5000), 0U, PWM_POLARITY_NORMAL);
    }

    printk("[HEAT] 4-zone heating ready\n");
    return 0;
}

int heating_update(const uint8_t targets_c[4], struct heating_state *state)
{
    int16_t sample = 0;
    struct adc_sequence seq = {
        .resolution  = 12,
        .buffer      = &sample,
        .buffer_size = sizeof(sample),
    };

    for (int z = 0; z < 4; z++) {
        sample       = 0;
        seq.channels = BIT(4 + z);

        if (adc_read(adc_dev, &seq)) {
            state->duty_pct[z] = 0U;
            pwm_set(pwm_dev, z, PWM_HZ(5000), 0U, PWM_POLARITY_NORMAL);
            continue;
        }

        state->temp_c_x100[z] = raw_to_temp_x100((int32_t)sample);

        uint8_t target_capped = MIN(targets_c[z], HEAT_TEMP_MAX);
        uint8_t duty = pid_compute(z,
                                   (int32_t)target_capped * 100,
                                   state->temp_c_x100[z]);
        state->duty_pct[z] = duty;

        uint32_t period = PWM_HZ(5000);
        uint32_t pulse  = (period / 100U) * (uint32_t)duty;
        pwm_set(pwm_dev, z, period, pulse, PWM_POLARITY_NORMAL);
    }

    return 0;
}
