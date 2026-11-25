/*!
 *  @file Vertex_MPU6050.h
 *
 *  Standalone MPU6050 driver with an Adafruit_MPU6050-compatible API.
 *
 *  The public interface (method signatures, enums, constants, units and
 *  sensors_event_t output) mirrors Adafruit_MPU6050 v2.2.x, so sketches
 *  written for the Adafruit library only need two changes:
 *
 *      #include <Adafruit_MPU6050.h>   ->   #include <Vertex_MPU6050.h>
 *      Adafruit_MPU6050 mpu;           ->   Vertex_MPU6050 mpu;
 *
 *  No other libraries are required: the Adafruit Unified Sensor types are
 *  bundled (Vertex_MPU6050_Sensor.h) and I2C goes straight through Wire,
 *  which ships with every Arduino board core.
 *
 *  ---------------------------------------------------------------------------
 *  Derived from Adafruit_MPU6050 v2.2.9 (https://github.com/adafruit/Adafruit_MPU6050).
 *  Modified: classes renamed to Vertex_MPU6050*, register access rewritten
 *  to use Wire directly (no Adafruit BusIO), lenient clone chip-ID check.
 *  Vertex_MPU6050 is not affiliated with or endorsed by Adafruit Industries.
 *
 *  Software License Agreement (BSD License)
 *
 *  Copyright (c) 2019 Bryan Siepert for Adafruit Industries
 *  Copyright (c) 2026 Tommy Guo
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holders nor the
 *  names of its contributors may be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Chip-ID list partly from Electronic Cats MPU6050 / i2cdevlib (MIT);
 *  see THIRD_PARTY_NOTICES.md.
 */

#ifndef _VERTEX_MPU6050_H
#define _VERTEX_MPU6050_H

#include "Arduino.h"
#include "Vertex_MPU6050_Sensor.h"
#include <Wire.h>

#define MPU6050_I2CADDR_DEFAULT 0x68 ///< MPU6050 default i2c address w/ AD0 low
#define MPU6050_DEVICE_ID 0x68       ///< The genuine MPU6050_WHO_AM_I value

#define MPU6050_SELF_TEST_X 0x0D ///< Self test factory calibrated values register
#define MPU6050_SELF_TEST_Y 0x0E ///< Self test factory calibrated values register
#define MPU6050_SELF_TEST_Z 0x0F ///< Self test factory calibrated values register
#define MPU6050_SELF_TEST_A 0x10 ///< Self test factory calibrated values register
#define MPU6050_SMPLRT_DIV 0x19  ///< sample rate divisor register
#define MPU6050_CONFIG 0x1A      ///< General configuration register
#define MPU6050_GYRO_CONFIG 0x1B ///< Gyro specfic configuration register
#define MPU6050_ACCEL_CONFIG 0x1C ///< Accelerometer specific configration register
#define MPU6050_INT_PIN_CONFIG 0x37 ///< Interrupt pin configuration register
#define MPU6050_INT_ENABLE 0x38     ///< Interrupt enable configuration register
#define MPU6050_INT_STATUS 0x3A     ///< Interrupt status register
#define MPU6050_WHO_AM_I 0x75       ///< Divice ID register
#define MPU6050_SIGNAL_PATH_RESET 0x68 ///< Signal path reset register
#define MPU6050_USER_CTRL 0x6A         ///< FIFO and I2C Master control register
#define MPU6050_PWR_MGMT_1 0x6B        ///< Primary power/sleep control register
#define MPU6050_PWR_MGMT_2 0x6C ///< Secondary power/sleep control register
#define MPU6050_TEMP_H 0x41     ///< Temperature data high byte register
#define MPU6050_TEMP_L 0x42     ///< Temperature data low byte register
#define MPU6050_ACCEL_OUT 0x3B  ///< base address for sensor data reads
#define MPU6050_MOT_THR 0x1F    ///< Motion detection threshold bits [7:0]
#define MPU6050_MOT_DUR 0x20 ///< Duration counter threshold for motion int. 1 kHz rate, LSB = 1 ms

/** FSYNC output values */
typedef enum fsync_out {
  MPU6050_FSYNC_OUT_DISABLED,
  MPU6050_FSYNC_OUT_TEMP,
  MPU6050_FSYNC_OUT_GYROX,
  MPU6050_FSYNC_OUT_GYROY,
  MPU6050_FSYNC_OUT_GYROZ,
  MPU6050_FSYNC_OUT_ACCELX,
  MPU6050_FSYNC_OUT_ACCELY,
  MPU6050_FSYNC_OUT_ACCEL_Z,
} mpu6050_fsync_out_t;

/** Clock source options */
typedef enum clock_select {
  MPU6050_INTR_8MHz,
  MPU6050_PLL_GYROX,
  MPU6050_PLL_GYROY,
  MPU6050_PLL_GYROZ,
  MPU6050_PLL_EXT_32K,
  MPU6050_PLL_EXT_19MHz,
  MPU6050_STOP = 7,
} mpu6050_clock_select_t;

/** Accelerometer range options */
typedef enum {
  MPU6050_RANGE_2_G = 0b00,  ///< +/- 2g (default value)
  MPU6050_RANGE_4_G = 0b01,  ///< +/- 4g
  MPU6050_RANGE_8_G = 0b10,  ///< +/- 8g
  MPU6050_RANGE_16_G = 0b11, ///< +/- 16g
} mpu6050_accel_range_t;

/** Gyroscope range options */
typedef enum {
  MPU6050_RANGE_250_DEG,  ///< +/- 250 deg/s (default value)
  MPU6050_RANGE_500_DEG,  ///< +/- 500 deg/s
  MPU6050_RANGE_1000_DEG, ///< +/- 1000 deg/s
  MPU6050_RANGE_2000_DEG, ///< +/- 2000 deg/s
} mpu6050_gyro_range_t;

/** Digital low pass filter bandwidth options */
typedef enum {
  MPU6050_BAND_260_HZ, ///< Docs imply this disables the filter
  MPU6050_BAND_184_HZ, ///< 184 Hz
  MPU6050_BAND_94_HZ,  ///< 94 Hz
  MPU6050_BAND_44_HZ,  ///< 44 Hz
  MPU6050_BAND_21_HZ,  ///< 21 Hz
  MPU6050_BAND_10_HZ,  ///< 10 Hz
  MPU6050_BAND_5_HZ,   ///< 5 Hz
} mpu6050_bandwidth_t;

/** Accelerometer high pass filter options */
typedef enum {
  MPU6050_HIGHPASS_DISABLE,
  MPU6050_HIGHPASS_5_HZ,
  MPU6050_HIGHPASS_2_5_HZ,
  MPU6050_HIGHPASS_1_25_HZ,
  MPU6050_HIGHPASS_0_63_HZ,
  MPU6050_HIGHPASS_UNUSED,
  MPU6050_HIGHPASS_HOLD,
} mpu6050_highpass_t;

/** Periodic measurement options */
typedef enum {
  MPU6050_CYCLE_1_25_HZ, ///< 1.25 Hz
  MPU6050_CYCLE_5_HZ,    ///< 5 Hz
  MPU6050_CYCLE_20_HZ,   ///< 20 Hz
  MPU6050_CYCLE_40_HZ,   ///< 40 Hz
} mpu6050_cycle_rate_t;

class Vertex_MPU6050;

/** Adafruit Unified Sensor interface for temperature component of MPU6050 */
class Vertex_MPU6050_Temp : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the temp sensor
      @param parent A pointer to the MPU6050 class */
  Vertex_MPU6050_Temp(Vertex_MPU6050 *parent) { _theMPU6050 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x650;
  Vertex_MPU6050 *_theMPU6050 = NULL;
};

/** Adafruit Unified Sensor interface for accelerometer component of MPU6050 */
class Vertex_MPU6050_Accelerometer : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the accelerometer
      @param parent A pointer to the MPU6050 class */
  Vertex_MPU6050_Accelerometer(Vertex_MPU6050 *parent) {
    _theMPU6050 = parent;
  }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x651;
  Vertex_MPU6050 *_theMPU6050 = NULL;
};

/** Adafruit Unified Sensor interface for gyro component of MPU6050 */
class Vertex_MPU6050_Gyro : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the gyro sensor
      @param parent A pointer to the MPU6050 class */
  Vertex_MPU6050_Gyro(Vertex_MPU6050 *parent) { _theMPU6050 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x652;
  Vertex_MPU6050 *_theMPU6050 = NULL;
};

/*!
 *  @brief  Standalone MPU6050 driver (Adafruit_MPU6050-compatible API)
 */
class Vertex_MPU6050 final {
public:
  Vertex_MPU6050();
  ~Vertex_MPU6050();

  bool begin(uint8_t i2c_addr = MPU6050_I2CADDR_DEFAULT, TwoWire *wire = &Wire,
             int32_t sensorID = 0);

  // Adafruit_Sensor API/Interface
  bool getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                sensors_event_t *temp);

  mpu6050_accel_range_t getAccelerometerRange(void);
  void setAccelerometerRange(mpu6050_accel_range_t);

  mpu6050_gyro_range_t getGyroRange(void);
  void setGyroRange(mpu6050_gyro_range_t);

  void setInterruptPinPolarity(bool active_low);
  void setInterruptPinLatch(bool held);
  void setFsyncSampleOutput(mpu6050_fsync_out_t fsync_output);

  mpu6050_highpass_t getHighPassFilter(void);
  void setHighPassFilter(mpu6050_highpass_t bandwidth);

  void setMotionInterrupt(bool active);
  void setMotionDetectionThreshold(uint8_t thr);
  void setMotionDetectionDuration(uint8_t dur);
  bool getMotionInterruptStatus(void);

  mpu6050_fsync_out_t getFsyncSampleOutput(void);
  void setI2CBypass(bool bypass);

  void setClock(mpu6050_clock_select_t);
  mpu6050_clock_select_t getClock(void);

  void setFilterBandwidth(mpu6050_bandwidth_t bandwidth);
  mpu6050_bandwidth_t getFilterBandwidth(void);

  void setSampleRateDivisor(uint8_t);
  uint8_t getSampleRateDivisor(void);

  bool enableSleep(bool enable);
  bool enableCycle(bool enable);

  void setCycleRate(mpu6050_cycle_rate_t rate);
  mpu6050_cycle_rate_t getCycleRate(void);

  bool setGyroStandby(bool xAxisStandby, bool yAxisStandby, bool zAxisStandby);
  bool setAccelerometerStandby(bool xAxisStandby, bool yAxisStandby,
                               bool zAxisStandby);
  bool setTemperatureStandby(bool enable);

  void reset(void);

  Adafruit_Sensor *getTemperatureSensor(void);
  Adafruit_Sensor *getAccelerometerSensor(void);
  Adafruit_Sensor *getGyroSensor(void);

protected:
  float temperature, ///< Last reading's temperature (C)
      accX,          ///< Last reading's accelerometer X axis (g)
      accY,          ///< Last reading's accelerometer Y axis (g)
      accZ,          ///< Last reading's accelerometer Z axis (g)
      gyroX,         ///< Last reading's gyro X axis (deg/s)
      gyroY,         ///< Last reading's gyro Y axis (deg/s)
      gyroZ;         ///< Last reading's gyro Z axis (deg/s)

  TwoWire *_wire = NULL;                   ///< I2C bus in use
  uint8_t _addr = MPU6050_I2CADDR_DEFAULT; ///< I2C address in use

  // Low-level register access (same semantics as Adafruit BusIO)
  bool _readRegs(uint8_t reg, uint8_t *buffer, uint8_t len);
  bool _writeReg(uint8_t reg, uint8_t value);
  uint8_t _readBits(uint8_t reg, uint8_t bits, uint8_t shift);
  bool _writeBits(uint8_t reg, uint8_t bits, uint8_t shift, uint8_t value);

  Vertex_MPU6050_Temp *temp_sensor = NULL; ///< Temp sensor data object
  Vertex_MPU6050_Accelerometer *accel_sensor =
      NULL;                                  ///< Accelerometer data object
  Vertex_MPU6050_Gyro *gyro_sensor = NULL; ///< Gyro data object

  uint16_t _sensorid_accel, ///< ID number for accelerometer
      _sensorid_gyro,       ///< ID number for gyro
      _sensorid_temp;       ///< ID number for temperature

  void _read(void);
  bool _init(int32_t sensor_id);

private:
  friend class Vertex_MPU6050_Temp;
  friend class Vertex_MPU6050_Accelerometer;
  friend class Vertex_MPU6050_Gyro;

  int16_t rawAccX, rawAccY, rawAccZ, rawTemp, rawGyroX, rawGyroY, rawGyroZ;

  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
  void fillAccelEvent(sensors_event_t *accel, uint32_t timestamp);
  void fillGyroEvent(sensors_event_t *gyro, uint32_t timestamp);
};

#endif
