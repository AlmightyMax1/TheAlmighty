#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include <stdbool.h>

struct imu_data {
    int32_t accel_x_mg;
    int32_t accel_y_mg;
    int32_t accel_z_mg;
    int32_t gyro_x_mdps;
    int32_t gyro_y_mdps;
    int32_t gyro_z_mdps;
    bool    valid;
};

int imu_init(void);
int imu_read(struct imu_data *out);

#endif /* IMU_H */
