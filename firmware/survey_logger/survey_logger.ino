#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include<Wire.h>
#include <Adafruit_VL53L0X.h>


#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      10
#define SIOD_GPIO_NUM      21
#defince SIOC_GPIO_NUM     14

#define Y9_GPIO_NUM       11
#defince Y8_GPIO_NUM       9
#define Y7_GPIO_NUM        8
#define Y6_GPIO_NUM        6
#define Y5_GPIO_NUM        4
#define Y4_GPIO_NUM        5
#defince Y3_GPIO_NUM       3
#define Y2_GPIO_NUM       13

#define VSYNC_GPIO_NUM    12
#define HREF_GPIO_NUM      7
#define PCLK_GPIO_NUM     15

#define I2C_SDA 1
#define I2C_SCL 2

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

unsigned long imageIndex = 0;
unsigned long lastCapturemillis = 0;
const unsigned long captureIntervalMillis = 15000; // Capture image every 1.5 seconds

bool sdAvailable = false;
bool sensorAvailable = false;


void initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;


    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        
    } else {
        Serial.println("Camera initialized successfully");
    }
}
void initMicroSD() {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("[ERROR] MicroSD card mount failed.");
    sdAvailable = false;
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("[ERROR] No MicroSD card attached.");
    sdAvailable = false;
    return;
  }

  sdAvailable = true;
  Serial.printf("[OK] MicroSD card mounted. Capacity: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));

  // Initialize CSV metadata header if log file doesn't exist
  if (!SD_MMC.exists("/survey_log.csv")) {
    File logFile = SD_MMC.open("/survey_log.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("Frame_ID,Timestamp_ms,Laser_AGL_mm,Laser_Status");
      logFile.close();
      Serial.println("[OK] Created new survey_log.csv header.");
    }
  }
}

void initLaserSensor() {
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if (!lox.begin(VL53L0X_I2C_ADDR, false, &Wire)) {
    Serial.println("[WARN] VL53L0X micro-LiDAR not detected on I2C bus.");
    sensorAvailable = false;
  } else {
    sensorAvailable = true;
    // Set high accuracy mode for ground truth measurement
    lox.setMeasurementTimingBudgetMicroSeconds(30000);
    Serial.println("[OK] VL53L0X micro-LiDAR initialized.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- AeroMesh S3 Survey Payload Initializing ---");

  initLaserSensor();
  initMicroSD();
  initCamera();

  Serial.println("--- Setup Complete. Starting Autonomous Capture Loop ---\n");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastCaptureMillis >= captureIntervalMs) {
    lastCaptureMillis = currentMillis;

  
    uint16_t groundDistanceMm = 0;
    uint8_t rangeStatus = 4; // 4 = out of range / error

    if (sensorAvailable) {
      VL53L0X_RangingMeasurementData_t measure;
      lox.rangingTest(&measure, false);
      rangeStatus = measure.RangeStatus;
      if (rangeStatus != 4) {
        groundDistanceMm = measure.RangeMilliMeter;
      }
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[ERROR] Failed to capture camera frame buffer.");
      return;
    }

    char fileName[32];
    sprintf(fileName, "/IMG_%04lu.JPG", ++imageIndex);

    if (sdAvailable) {
      File imgFile = SD_MMC.open(fileName, FILE_WRITE);
      if (imgFile) {
        imgFile.write(fb->buf, fb->len);
        imgFile.close();
        Serial.printf("[SAVED] %s (%u bytes) | Laser AGL: %u mm\n", fileName, fb->len, groundDistanceMm);
      } else {
        Serial.printf("[ERROR] Failed to open %s on MicroSD for write.\n", fileName);
      }

      File logFile = SD_MMC.open("/survey_log.csv", FILE_APPEND);
      if (logFile) {
        logFile.printf("%s,%lu,%u,%u\n", fileName, currentMillis, groundDistanceMm, rangeStatus);
        logFile.close();
      }
    } else {
      Serial.printf("[DRY-RUN] Captured %s (No SD card present) | Laser: %u mm\n", fileName, groundDistanceMm);
    }

    // Return the frame buffer back to the camera driver pool
    esp_camera_fb_return(fb);
  }
}

}