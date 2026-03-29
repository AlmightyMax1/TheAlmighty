#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include "imu.h"

static const struct device *imu_dev;

int imu_init(void)
{
    imu_dev = DEVICE_DT_GET_ANY(invensense_icm42688);
    if (!imu_dev || !device_is_ready(imu_dev)) {
        printk("[IMU] ICM-42688-P not ready\n");
        return -ENODEV;
    }
    printk("[IMU] ICM-42688-P ready\n");
    return 0;
}

int imu_read(struct imu_data *out)
{
    struct sensor_value a[3], g[3];

    if (!imu_dev) {
        out->valid = false;
        return -ENODEV;
    }

    int err = sensor_sample_fetch(imu_dev);
    if (err) {
        out->valid = false;
        return err;
    }

    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, a);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ,  g);

    out->accel_x_mg  = (int32_t)(sensor_value_to_double(&a[0]) * 1000.0 / 9.80665);
    out->accel_y_mg  = (int32_t)(sensor_value_to_double(&a[1]) * 1000.0 / 9.80665);
    out->accel_z_mg  = (int32_t)(sensor_value_to_double(&a[2]) * 1000.0 / 9.80665);
    out->gyro_x_mdps = (int32_t)(sensor_value_to_double(&g[0]) * 57295.7796);
    out->gyro_y_mdps = (int32_t)(sensor_value_to_double(&g[1]) * 57295.7796);
    out->gyro_z_mdps = (int32_t)(sensor_value_to_double(&g[2]) * 57295.7796);
    out->valid       = true;
    return 0;
}
