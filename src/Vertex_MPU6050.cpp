/*!
 *  @file Vertex_MPU6050.cpp
 *
 *  Standalone implementation of the Adafruit_MPU6050 API. Register access is
 *  done directly over Wire with the same semantics as Adafruit BusIO
 *  (register-address write with repeated start, then read; bit fields are
 *  read-modify-write). Init sequence, scaling and event output replicate
 *  Adafruit_MPU6050 v2.2.x exactly.
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

#include "Vertex_MPU6050.h"
#include <string.h>

/******************* Low-level register access *******************/

bool Vertex_MPU6050::_readRegs(uint8_t reg, uint8_t *buffer, uint8_t len) {
  if (!_wire)
    return false;
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) // repeated start, like BusIO
    return false;
  if (_wire->requestFrom((uint8_t)_addr, (uint8_t)len) != len)
    return false;
  for (uint8_t i = 0; i < len; i++)
    buffer[i] = _wire->read();
  return true;
}

bool Vertex_MPU6050::_writeReg(uint8_t reg, uint8_t value) {
  if (!_wire)
    return false;
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}

uint8_t Vertex_MPU6050::_readBits(uint8_t reg, uint8_t bits, uint8_t shift) {
  uint8_t value = 0;
  _readRegs(reg, &value, 1);
  return (value >> shift) & ((1 << bits) - 1);
}

bool Vertex_MPU6050::_writeBits(uint8_t reg, uint8_t bits, uint8_t shift,
                                  uint8_t value) {
  uint8_t current;
  if (!_readRegs(reg, &current, 1))
    return false;
  uint8_t mask = ((1 << bits) - 1) << shift;
  current = (current & ~mask) | ((value << shift) & mask);
  return _writeReg(reg, current);
}

/******************* Setup *******************/

Vertex_MPU6050::Vertex_MPU6050(void) {}

Vertex_MPU6050::~Vertex_MPU6050(void) {
  if (temp_sensor)
    delete temp_sensor;
  if (accel_sensor)
    delete accel_sensor;
  if (gyro_sensor)
    delete gyro_sensor;
}

/*!
 *  @brief  Sets up the hardware and initializes I2C
 *  @param  i2c_address The I2C address to be used.
 *  @param  wire The Wire object to be used for I2C connections.
 *  @param  sensor_id The unique ID to differentiate the sensors from others
 *  @return True if initialization was successful, otherwise false.
 */
bool Vertex_MPU6050::begin(uint8_t i2c_address, TwoWire *wire,
                             int32_t sensor_id) {
  _addr = i2c_address;
  _wire = wire;

  // Adafruit_I2CDevice::begin() starts the bus itself, so sketches written
  // for Adafruit never call Wire.begin(). Do the same here.
  _wire->begin();

  // For boards with I2C bus power control, may need to delay to allow
  // MPU6050 to come up after initial power.
  bool mpu_found = false;
  for (uint8_t tries = 0; tries < 5; tries++) {
    _wire->beginTransmission(_addr);
    mpu_found = (_wire->endTransmission() == 0);
    if (mpu_found)
      break;
    delay(10);
  }
  if (!mpu_found)
    return false;

  // Lenient chip-ID check. Accepts:
  //  - Electronic Cats testConnection() IDs: WHO_AM_I bits [6:1] of 0x34
  //    (genuine MPU6050, 0x68), 0x0C or 0x3A (common clones)
  //  - WHO_AM_I == 0x72: MPU6050 clone with an MPU6050-compatible register
  //    map (PWR_MGMT_1 resets to 0x40), verified on hardware
  uint8_t who_am_i;
  if (!_readRegs(MPU6050_WHO_AM_I, &who_am_i, 1))
    return false;
  uint8_t id = (who_am_i >> 1) & 0x3F;
  if (id != 0x34 && id != 0x0C && id != 0x3A && who_am_i != 0x72)
    return false;

  return _init(sensor_id);
}

/*!  @brief Initialization code shared by all begin variants
 *   @param sensor_id Base sensor ID
 *   @return true on success */
bool Vertex_MPU6050::_init(int32_t sensor_id) {
  _sensorid_accel = sensor_id;
  _sensorid_gyro = sensor_id + 1;
  _sensorid_temp = sensor_id + 2;

  reset();

  setSampleRateDivisor(0);
  setFilterBandwidth(MPU6050_BAND_260_HZ);
  setGyroRange(MPU6050_RANGE_500_DEG);
  setAccelerometerRange(MPU6050_RANGE_2_G); // already the default

  _writeReg(MPU6050_PWR_MGMT_1, 0x01); // clock: PLL with Gyro X reference
  delay(100);

  if (temp_sensor)
    delete temp_sensor;
  if (accel_sensor)
    delete accel_sensor;
  if (gyro_sensor)
    delete gyro_sensor;

  temp_sensor = new Vertex_MPU6050_Temp(this);
  accel_sensor = new Vertex_MPU6050_Accelerometer(this);
  gyro_sensor = new Vertex_MPU6050_Gyro(this);

  return true;
}

/*!
 *   @brief Resets registers to their initial value and resets the sensors'
 *   analog and digital signal paths.
 */
void Vertex_MPU6050::reset(void) {
  _writeBits(MPU6050_PWR_MGMT_1, 1, 7, 1); // device reset
  while (_readBits(MPU6050_PWR_MGMT_1, 1, 7) == 1) { // wait for reset to clear
    delay(1);
  }
  delay(100);

  _writeReg(MPU6050_SIGNAL_PATH_RESET, 0x7);
  delay(100);
}

/******************* Configuration *******************/

uint8_t Vertex_MPU6050::getSampleRateDivisor(void) {
  uint8_t value = 0;
  _readRegs(MPU6050_SMPLRT_DIV, &value, 1);
  return value;
}

void Vertex_MPU6050::setSampleRateDivisor(uint8_t divisor) {
  _writeReg(MPU6050_SMPLRT_DIV, divisor);
}

mpu6050_accel_range_t Vertex_MPU6050::getAccelerometerRange(void) {
  return (mpu6050_accel_range_t)_readBits(MPU6050_ACCEL_CONFIG, 2, 3);
}

void Vertex_MPU6050::setAccelerometerRange(mpu6050_accel_range_t new_range) {
  _writeBits(MPU6050_ACCEL_CONFIG, 2, 3, new_range);
}

mpu6050_gyro_range_t Vertex_MPU6050::getGyroRange(void) {
  return (mpu6050_gyro_range_t)_readBits(MPU6050_GYRO_CONFIG, 2, 3);
}

void Vertex_MPU6050::setGyroRange(mpu6050_gyro_range_t new_range) {
  _writeBits(MPU6050_GYRO_CONFIG, 2, 3, new_range);
}

void Vertex_MPU6050::setClock(mpu6050_clock_select_t new_clock) {
  _writeBits(MPU6050_PWR_MGMT_1, 3, 0, new_clock);
}

mpu6050_clock_select_t Vertex_MPU6050::getClock(void) {
  return (mpu6050_clock_select_t)_readBits(MPU6050_PWR_MGMT_1, 3, 0);
}

mpu6050_fsync_out_t Vertex_MPU6050::getFsyncSampleOutput(void) {
  return (mpu6050_fsync_out_t)_readBits(MPU6050_CONFIG, 3, 3);
}

void Vertex_MPU6050::setFsyncSampleOutput(mpu6050_fsync_out_t fsync_output) {
  _writeBits(MPU6050_CONFIG, 3, 3, fsync_output);
}

mpu6050_bandwidth_t Vertex_MPU6050::getFilterBandwidth(void) {
  return (mpu6050_bandwidth_t)_readBits(MPU6050_CONFIG, 3, 0);
}

void Vertex_MPU6050::setFilterBandwidth(mpu6050_bandwidth_t bandwidth) {
  _writeBits(MPU6050_CONFIG, 3, 0, bandwidth);
}

mpu6050_highpass_t Vertex_MPU6050::getHighPassFilter(void) {
  return (mpu6050_highpass_t)_readBits(MPU6050_ACCEL_CONFIG, 3, 0);
}

void Vertex_MPU6050::setHighPassFilter(mpu6050_highpass_t bandwidth) {
  _writeBits(MPU6050_ACCEL_CONFIG, 3, 0, bandwidth);
}

void Vertex_MPU6050::setInterruptPinPolarity(bool active_low) {
  _writeBits(MPU6050_INT_PIN_CONFIG, 1, 7, active_low);
}

void Vertex_MPU6050::setInterruptPinLatch(bool held) {
  _writeBits(MPU6050_INT_PIN_CONFIG, 1, 5, held);
}

void Vertex_MPU6050::setMotionInterrupt(bool active) {
  _writeBits(MPU6050_INT_ENABLE, 1, 6, active);
}

bool Vertex_MPU6050::getMotionInterruptStatus(void) {
  return (bool)_readBits(MPU6050_INT_STATUS, 1, 6);
}

void Vertex_MPU6050::setMotionDetectionThreshold(uint8_t thr) {
  _writeReg(MPU6050_MOT_THR, thr);
}

void Vertex_MPU6050::setMotionDetectionDuration(uint8_t dur) {
  _writeReg(MPU6050_MOT_DUR, dur);
}

void Vertex_MPU6050::setI2CBypass(bool bypass) {
  _writeBits(MPU6050_INT_PIN_CONFIG, 1, 1, bypass);
  _writeBits(MPU6050_USER_CTRL, 1, 5, !bypass);
}

bool Vertex_MPU6050::enableSleep(bool enable) {
  return _writeBits(MPU6050_PWR_MGMT_1, 1, 6, enable);
}

bool Vertex_MPU6050::enableCycle(bool enable) {
  return _writeBits(MPU6050_PWR_MGMT_1, 1, 5, enable);
}

mpu6050_cycle_rate_t Vertex_MPU6050::getCycleRate(void) {
  return (mpu6050_cycle_rate_t)_readBits(MPU6050_PWR_MGMT_2, 2, 6);
}

void Vertex_MPU6050::setCycleRate(mpu6050_cycle_rate_t rate) {
  _writeBits(MPU6050_PWR_MGMT_2, 2, 6, rate);
}

bool Vertex_MPU6050::setGyroStandby(bool xAxisStandby, bool yAxisStandby,
                                      bool zAxisStandby) {
  return _writeBits(MPU6050_PWR_MGMT_2, 3, 0,
                    xAxisStandby << 2 | yAxisStandby << 1 | zAxisStandby);
}

bool Vertex_MPU6050::setAccelerometerStandby(bool xAxisStandby,
                                               bool yAxisStandby,
                                               bool zAxisStandby) {
  return _writeBits(MPU6050_PWR_MGMT_2, 3, 3,
                    xAxisStandby << 2 | yAxisStandby << 1 | zAxisStandby);
}

bool Vertex_MPU6050::setTemperatureStandby(bool enable) {
  return _writeBits(MPU6050_PWR_MGMT_1, 1, 3, enable);
}

/******************* Data reading *******************/

void Vertex_MPU6050::_read(void) {
  // single 14-byte burst read: accel(6) temp(2) gyro(6)
  uint8_t buffer[14];
  _readRegs(MPU6050_ACCEL_OUT, buffer, 14);

  rawAccX = buffer[0] << 8 | buffer[1];
  rawAccY = buffer[2] << 8 | buffer[3];
  rawAccZ = buffer[4] << 8 | buffer[5];

  rawTemp = buffer[6] << 8 | buffer[7];

  rawGyroX = buffer[8] << 8 | buffer[9];
  rawGyroY = buffer[10] << 8 | buffer[11];
  rawGyroZ = buffer[12] << 8 | buffer[13];

  temperature = (rawTemp / 340.0) + 36.53;

  mpu6050_accel_range_t accel_range = getAccelerometerRange();

  float accel_scale = 1;
  if (accel_range == MPU6050_RANGE_16_G)
    accel_scale = 2048;
  if (accel_range == MPU6050_RANGE_8_G)
    accel_scale = 4096;
  if (accel_range == MPU6050_RANGE_4_G)
    accel_scale = 8192;
  if (accel_range == MPU6050_RANGE_2_G)
    accel_scale = 16384;

  // setup range dependant scaling
  accX = ((float)rawAccX) / accel_scale;
  accY = ((float)rawAccY) / accel_scale;
  accZ = ((float)rawAccZ) / accel_scale;

  mpu6050_gyro_range_t gyro_range = getGyroRange();

  float gyro_scale = 1;
  if (gyro_range == MPU6050_RANGE_250_DEG)
    gyro_scale = 131;
  if (gyro_range == MPU6050_RANGE_500_DEG)
    gyro_scale = 65.5;
  if (gyro_range == MPU6050_RANGE_1000_DEG)
    gyro_scale = 32.8;
  if (gyro_range == MPU6050_RANGE_2000_DEG)
    gyro_scale = 16.4;

  gyroX = ((float)rawGyroX) / gyro_scale;
  gyroY = ((float)rawGyroY) / gyro_scale;
  gyroZ = ((float)rawGyroZ) / gyro_scale;
}

/*!
 *  @brief  Gets the most recent sensor event, Adafruit Unified Sensor format
 *  @return True on successful read
 */
bool Vertex_MPU6050::getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                                sensors_event_t *temp) {
  uint32_t timestamp = millis();
  _read();

  fillTempEvent(temp, timestamp);
  fillAccelEvent(accel, timestamp);
  fillGyroEvent(gyro, timestamp);

  return true;
}

void Vertex_MPU6050::fillTempEvent(sensors_event_t *temp,
                                     uint32_t timestamp) {
  memset(temp, 0, sizeof(sensors_event_t));
  temp->version = sizeof(sensors_event_t);
  temp->sensor_id = _sensorid_temp;
  temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  temp->timestamp = timestamp;
  temp->temperature = temperature;
}

void Vertex_MPU6050::fillAccelEvent(sensors_event_t *accel,
                                      uint32_t timestamp) {
  memset(accel, 0, sizeof(sensors_event_t));
  accel->version = 1;
  accel->sensor_id = _sensorid_accel;
  accel->type = SENSOR_TYPE_ACCELEROMETER;
  accel->timestamp = timestamp;
  accel->acceleration.x = accX * SENSORS_GRAVITY_STANDARD;
  accel->acceleration.y = accY * SENSORS_GRAVITY_STANDARD;
  accel->acceleration.z = accZ * SENSORS_GRAVITY_STANDARD;
}

void Vertex_MPU6050::fillGyroEvent(sensors_event_t *gyro,
                                     uint32_t timestamp) {
  memset(gyro, 0, sizeof(sensors_event_t));
  gyro->version = 1;
  gyro->sensor_id = _sensorid_gyro;
  gyro->type = SENSOR_TYPE_GYROSCOPE;
  gyro->timestamp = timestamp;
  gyro->gyro.x = gyroX * SENSORS_DPS_TO_RADS;
  gyro->gyro.y = gyroY * SENSORS_DPS_TO_RADS;
  gyro->gyro.z = gyroZ * SENSORS_DPS_TO_RADS;
}

Adafruit_Sensor *Vertex_MPU6050::getTemperatureSensor(void) {
  return temp_sensor;
}

Adafruit_Sensor *Vertex_MPU6050::getAccelerometerSensor(void) {
  return accel_sensor;
}

Adafruit_Sensor *Vertex_MPU6050::getGyroSensor(void) { return gyro_sensor; }

/**************************************************************************/
/*  Unified Sensor sub-objects                                            */
/**************************************************************************/

void Vertex_MPU6050_Gyro::getSensor(sensor_t *sensor) {
  memset(sensor, 0, sizeof(sensor_t));
  strncpy(sensor->name, "MPU6050_G", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_GYROSCOPE;
  sensor->min_delay = 0;
  sensor->min_value = -34.91; /* -2000 dps -> rad/s (radians per second) */
  sensor->max_value = +34.91;
  sensor->resolution = 1.332e-4; /* 131.5 LSB/DPS */
}

bool Vertex_MPU6050_Gyro::getEvent(sensors_event_t *event) {
  _theMPU6050->_read();
  _theMPU6050->fillGyroEvent(event, millis());
  return true;
}

void Vertex_MPU6050_Accelerometer::getSensor(sensor_t *sensor) {
  memset(sensor, 0, sizeof(sensor_t));
  strncpy(sensor->name, "MPU6050_A", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_ACCELEROMETER;
  sensor->min_delay = 0;
  sensor->min_value = -156.9064F; /*  -16g = 156.9064 m/s^2  */
  sensor->max_value = 156.9064F;  /* 16g = 156.9064 m/s^2  */
  sensor->resolution = 0.061;     /* 0.061 mg/LSB at +-2g */
}

bool Vertex_MPU6050_Accelerometer::getEvent(sensors_event_t *event) {
  _theMPU6050->_read();
  _theMPU6050->fillAccelEvent(event, millis());
  return true;
}

void Vertex_MPU6050_Temp::getSensor(sensor_t *sensor) {
  memset(sensor, 0, sizeof(sensor_t));
  strncpy(sensor->name, "MPU6050_T", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -40;
  sensor->max_value = 105;
  sensor->resolution = 0.00294; /* 340 LSB/C => 1/340 C/LSB */
}

bool Vertex_MPU6050_Temp::getEvent(sensors_event_t *event) {
  _theMPU6050->_read();
  _theMPU6050->fillTempEvent(event, millis());
  return true;
}
