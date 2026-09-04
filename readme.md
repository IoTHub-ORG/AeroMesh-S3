## AeroMesh S3

An open-source, autonomous 3D aerial mapping drone controlled directly from a smartphone. AeroMesh S3 eliminates costly proprietary radio transmitters by implementing an onboard ESP32-S3 Wi-Fi MAVLink telementry bridge, pairing autonomous waypoint navigation with a laser-synchronized downward photogrammetry payload for under $170(₹14,100).

## 1. Project Overview

* **The Problem:** Enterprise aerial surveying systems(such as Wingtra or senseFly) coset upwards of $5k to $ 10k , making photogrammetric mapping inaccessible to students, independent researchers, and small-scale agricultirist.

* **The Limitation of Hobby Drones:** Standard DIY quadcopters require an expensive $80+ dedicated radio controller, use upward-tilted cameras suited only for manual racing, and capture uncalibrated video clips rather than structure, geo-referenced spatial data.

* **The Solution:** AeroMesh S3 is a sub-$170 mapping platform that executes automated lawnmower survey grids without a physical RC transmitter. It uses an onboard ESP32-S3 microcontroller to create a local Wi-Fi telemetry bridge, allowing any smartphone running **QGroundControl** to dispatch missons, monitor live metrics, and trigger fail-safes.

## 2. Heterogeneous Dual-Brain Architecture

Running flight stabilization loops alaongside camera and Wi-Fi streaming on a single microcontroller introduces RTOS thread latency and task contention. AeroMesh S3 separates flight stabilization from mission compute across two dedicated microcontrollers:

```text 
[Smartphone/ Ground Control Station]
          |
          |
          |
          ▼
[Mission Computer: ESP32-S3]
          |────MicroSD Card (High-speed SPI: Stores JPEG frames and CSV spatial logs)
          |────VL53L0X Laser Rangefinder (I2C: Measures real-time ground clearance)
          |
          |────Hardware UART(115200 Baud- bi-directional MAVLink Bridge)
          ▼
[Flight Controller: STM32F411 AIO]
          |────UBLOX NEO-M8N GPS + Compass(Autonomous waypoint navigation)
          |────ICM42688 IMU( 400Hz real-time attitude stabilization via iNav)
          |
          └──── 25A 4-in-1 ESC + 4X 2205 Brushless Motors
```


* **The Real-Time Brain (STM32F411):** Runs iNav flight firmware, dedicating all compute cycles to sensor filtering, attitude stabilization, and motor drive via DShat protocols.
* **The Mission Brain (ESP32-S3):** Broadcasts a local Wi-Fi Access Point, routes MAVLink telemetry packets to QGroundControl, triggers the downward camera, and logs distance data from the laser sensor.
-------

## 3. Ground Control and Smartphone Connection

1. **Standalone Wi-Fi Hotspot:** The ESP32-S3 chip broadcasts a local Wi-Fi network after boot. There is no need for any router, mobile internet, or cellular towers.

2. **QGroundControl Integration:** You have to connect your mobile device or laptop to the drone's Wi-Fi and open the open-source **QGroundControl** app. It creates a connection over UDP port 14550.

3. **Autonomous Grid Survey**