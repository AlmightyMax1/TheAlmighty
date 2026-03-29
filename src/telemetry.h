#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <zephyr/sys/util.h>

#define OMNI_FLAG_HEATING_ACTIVE   BIT(0)
#define OMNI_FLAG_SPRINT_MODE      BIT(1)
#define OMNI_FLAG_STF_LOCKED       BIT(2)
#define OMNI_FLAG_PCM_ACTIVE       BIT(3)
#define OMNI_FLAG_HARVEST_ACTIVE   BIT(4)
#define OMNI_FLAG_IMU_VALID        BIT(5)
#define OMNI_FLAG_PRESSURE_VALID   BIT(6)
#define OMNI_FLAG_FAULT            BIT(7)
#define OMNI_FLAG_GPS_VALID        BIT(8)
#define OMNI_FLAG_SPO2_VALID       BIT(9)
#define OMNI_FLAG_EMG_VALID        BIT(10)
#define OMNI_FLAG_HAPTIC_ACTIVE    BIT(11)

struct __attribute__((packed)) omni_telemetry {
    int16_t  accel_x_mg;
    int16_t  accel_y_mg;
    int16_t  accel_z_mg;
    int16_t  gyro_x_mdps;
    int16_t  gyro_y_mdps;
    int16_t  gyro_z_mdps;

    uint16_t pressure_fore_kpa;
    uint16_t pressure_heel_kpa;
    uint16_t pressure_medial_kpa;
    uint16_t pressure_lateral_kpa;

    int16_t  temp_zone0_c_x100;
    int16_t  temp_zone1_c_x100;
    int16_t  temp_zone2_c_x100;
    int16_t  temp_zone3_c_x100;
    uint8_t  heat_duty_zone0;
    uint8_t  heat_duty_zone1;
    uint8_t  heat_duty_zone2;
    uint8_t  heat_duty_zone3;

    uint16_t battery_mv;
    uint8_t  battery_soc_pct;
    uint16_t teng_harvest_mw_x10;
    uint16_t em_harvest_mw_x10;
    uint16_t heating_runtime_min;

    uint8_t  spring_mode;
    uint8_t  stf_state;
    uint8_t  pcm_state;

    uint8_t  spo2_pct;
    uint8_t  heart_rate_bpm;
    int16_t  collar_temp_c_x100;
    uint16_t gsr_kohm;

    int16_t  emg_mv[4];

    int32_t  gps_lat_deg_x1e6;
    int32_t  gps_lon_deg_x1e6;
    uint16_t gps_speed_kmh_x10;
    uint16_t gps_course_deg;

    uint8_t  terrain_class;
    uint8_t  haptic_pattern;

    uint16_t flags;
    uint32_t tick_ms;
};

struct __attribute__((packed)) omni_control {
    uint8_t  spring_mode;
    uint8_t  heat_target_zone0;
    uint8_t  heat_target_zone1;
    uint8_t  heat_target_zone2;
    uint8_t  heat_target_zone3;
    uint8_t  stf_threshold_g;
    uint8_t  pcm_valve_open;
    uint8_t  haptic_pattern;
    uint8_t  haptic_trigger;
    uint8_t  beacon_activate;
    uint8_t  reserved[2];
};

#endif /* TELEMETRY_H */
