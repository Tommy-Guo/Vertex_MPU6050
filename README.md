# Vertex_MPU6050

Standalone MPU6050 driver with the same API as **Adafruit_MPU6050**, under the
class name `Vertex_MPU6050`.

**No other libraries are needed.** It doesn't use Adafruit_MPU6050, Adafruit
Unified Sensor, Adafruit BusIO or Electronic Cats MPU6050. It talks to the chip
directly through `Wire`, which comes with every Arduino board package. The
Adafruit Unified Sensor types (`sensors_event_t`, `sensor_t`,
`Adafruit_Sensor`) are bundled in `Vertex_MPU6050_Sensor.h`.

The method signatures, enums, constants and `sensors_event_t` output are the
same as Adafruit_MPU6050 v2.2.x, and so are the init sequence, default
settings, scaling and units.

## Install
Copy this `Vertex_MPU6050` folder into your Arduino `libraries`
folder (e.g. `~/Documents/Arduino/libraries/`).

## Porting existing Adafruit code
Two changes:

```cpp
// before
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;

// after
#include <Vertex_MPU6050.h>
Vertex_MPU6050 mpu;
```

Everything else stays the same: `mpu.begin()`, `mpu.getEvent(...)`,
`sensors_event_t`, `MPU6050_RANGE_8_G`, `MPU6050_BAND_21_HZ` and so on.

| Adafruit_MPU6050 | Vertex_MPU6050 |
|---|---|
| `Adafruit_MPU6050` | `Vertex_MPU6050` |
| `Adafruit_MPU6050_Accelerometer` | `Vertex_MPU6050_Accelerometer` |
| `Adafruit_MPU6050_Gyro` | `Vertex_MPU6050_Gyro` |
| `Adafruit_MPU6050_Temp` | `Vertex_MPU6050_Temp` |

`Adafruit_Sensor`, `sensors_event_t` and `sensor_t` keep their names so the
library stays compatible with other Adafruit sensor code.

Notes:
- If you keep `#include <Adafruit_Sensor.h>`, that line still needs Adafruit
  Unified Sensor installed. Remove the line if you don't have it.
- Using Vertex_MPU6050 next to other Adafruit sensor libraries is fine. The
  bundled types share the real header's include guard, so only one copy is
  ever compiled.
- Don't include `Adafruit_MPU6050.h` and `Vertex_MPU6050.h` in the same sketch.
  Both define the same `MPU6050_*` enums and constants.

## Behaviour (identical to Adafruit)
| | |
|---|---|
| `begin()` defaults | addr 0x68, `&Wire`, sensorID 0; calls `wire->begin()` itself |
| After `begin()` | device reset + signal path reset, sample-rate divisor 0, DLPF 260 Hz, gyro ±500 °/s, accel ±2 g, clock PLL gyro-X |
| Acceleration | m/s² (raw / {16384, 8192, 4096, 2048} × 9.80665) |
| Gyro | rad/s (raw / {131, 65.5, 32.8, 16.4} × 0.017453293) |
| Temperature | °C (raw / 340 + 36.53) |

## The one intentional difference
`begin()` uses a lenient chip-ID check. It accepts genuine MPU6050s **and**
clones whose `WHO_AM_I` register is not exactly 0x68:

- the IDs accepted by Electronic Cats' `testConnection()` (0x68, 0x18, 0x74/0x75)
- **0x72**, a clone with an MPU6050-compatible register map. This is the
  chip on our boards, and it was tested working on an Arduino Uno.

Adafruit's `begin()` rejects all of these clones, so on those boards it fails
with "Failed to find MPU6050 chip".

## License

Vertex_MPU6050 is released under the **BSD 3-clause license** (see
[`LICENSE`](LICENSE)). It is derived from Adafruit_MPU6050, which uses the
same license.

- `src/Vertex_MPU6050_Sensor.h` is a modified copy of Adafruit's
  `Adafruit_Sensor.h` and stays under the **Apache License 2.0** (see
  [`LICENSE-APACHE-2.0.txt`](LICENSE-APACHE-2.0.txt)).
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) lists every third-party
  source, what was changed, and the full original notices.

**Shipping firmware built with this library:** the BSD license (clause 2)
requires the copyright notice, conditions and disclaimer from `LICENSE` to
appear in your product's documentation or other materials. Apache 2.0 also
requires a copy of its license text. Including `LICENSE`,
`LICENSE-APACHE-2.0.txt` and `THIRD_PARTY_NOTICES.md` with your product docs
(e.g. an "Open-source licenses" page in the manual or on the website) covers
all three licenses.

Vertex_MPU6050 is not affiliated with or endorsed by Adafruit Industries,
Electronic Cats or Jeff Rowberg. Adafruit names such as `Adafruit_Sensor` and
`sensors_event_t` are kept only for API compatibility.
