#include "esp_camera.h"
#include "app_httpd.h"
#include <WiFi.h>

// ESP32-specific file system includes
#define FS_NO_GLOBALS
#include <FS.h>
#include <SD_MMC.h>

// Firebase includes
#include <Firebase_ESP_Client.h>

// Your camera configuration
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char* ssid = "AYAM2.4G_BOLELAH";
const char* password = "fawwaznakayam";

// Firebase configuration
#define API_KEY "AIzaSyCxTNJxuuVkOOdDmqmO6TMcS3EJ4eWDNLg"
#define DATABASE_URL "https://database-for-fyp-hafiy-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // Uncomment if using authentication
  // auth.user.email = "user@email.com";
  // auth.user.password = "password";
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Your existing camera configuration
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
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_QVGA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  static unsigned long t = 0;
  if(millis() - t > 1000){
    t = millis();
    const char *name = app_get_last_matched_name();
    int id = app_get_last_matched_id();
    Serial.printf("Last matched id=%d name=%s\n", id, name);

    if(Firebase.ready()) {
      if(id == 0) {
        String recognizedName1 = String(id);
        Firebase.RTDB.setString(&fbdo, "/recognized_name", recognizedName1);
      } else if(id == 1) {
        String recognizedName2 = String(id);
        Firebase.RTDB.setString(&fbdo, "/recognized_name", recognizedName2);
      } else {
        Firebase.RTDB.setString(&fbdo, "/recognized_name", "-");
        Serial.println("Reset recognized_name");
      }
    }
  }
  delay(10000);
}