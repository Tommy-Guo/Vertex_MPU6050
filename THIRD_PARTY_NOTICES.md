# Third-party notices

Vertex_MPU6050 includes or is derived from the following third-party software.
Vertex_MPU6050 is not affiliated with or endorsed by Adafruit Industries,
Electronic Cats or Jeff Rowberg. Their names appear here only to give
attribution, as the licenses require.

---

## 1. Adafruit_MPU6050 (BSD 3-clause)

- Source: https://github.com/adafruit/Adafruit_MPU6050 (v2.2.9)
- Used in: `src/Vertex_MPU6050.h`, `src/Vertex_MPU6050.cpp`,
  `examples/basic_readings/basic_readings.ino`
- Changes: classes renamed from `Adafruit_MPU6050*` to `Vertex_MPU6050*`.
  Register access was rewritten to use Wire directly instead of Adafruit BusIO.
  The chip-ID check accepts additional clone IDs.

```
Software License Agreement (BSD License)

Copyright (c) 2019 Bryan Siepert for Adafruit Industries
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

---

## 2. Adafruit Unified Sensor (Apache License 2.0)

- Source: https://github.com/adafruit/Adafruit_Sensor (v1.1.15)
- Used in: `src/Vertex_MPU6050_Sensor.h` (modified copy of `Adafruit_Sensor.h`)
- Changes: added an explanatory banner. `Adafruit_Sensor::printSensorDetails()`
  was moved from `Adafruit_Sensor.cpp` into the header as an `inline` function.
  A stray `< /span>` markup typo was removed from the original license comment.
  The type definitions are otherwise unchanged.
- Full license text: [`LICENSE-APACHE-2.0.txt`](LICENSE-APACHE-2.0.txt)

```
Copyright (C) 2008 The Android Open Source Project

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Update by K. Townsend (Adafruit Industries) for lighter typedefs, and
extended sensor support to include color, voltage and current
```

---

## 3. Electronic Cats MPU6050 / i2cdevlib (MIT)

- Source: https://github.com/ElectronicCats/mpu6050 (v1.4.5), based on
  https://github.com/jrowberg/i2cdevlib
- Used in: `src/Vertex_MPU6050.cpp`. Only the list of accepted chip IDs from
  `MPU6050_Base::testConnection()` was used; no code was copied.

```
MIT License

Copyright (c) 2019 ElectronicCats
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
