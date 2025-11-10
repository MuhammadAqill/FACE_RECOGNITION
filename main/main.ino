#include <WiFi.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ================= Wi-Fi =================
const char* ssid     = "AYAM2.4G_BOLELAH";
const char* password = "fawwaznakayam";

// UART dari ESP32-CAM
HardwareSerial CamSerial(2);

// =============== Firebase RTDB ===============
#define API_KEY ""
#define DATABASE_URL ""

// =============== Pin setup ===============
#define BUTTON_PIN 4
#define LED_PIN    2

// Status strings
const char* STATUS_WAITING = "Waiting";
const char* STATUS_ARRIVE  = "Someone_Arrive";

// Firebase objects
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

// ====== Debounce Button ======
bool lastStableState = HIGH;
bool lastReading = HIGH;
unsigned long lastChangeMs = 0;
const unsigned long debounceMs = 40;

// ======== UART RX parser ========
char rxBuf[16];
size_t rxPos = 0;
int    lastFaceId   = -999;
String lastFaceName = "";

// ======== LED TIMER 15s (bila ditekan) ========
unsigned long ledOnTimestamp = 0;
bool ledOnCountdownActive = false;

// ---------------- CONNECT WIFI ----------------
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  Serial.printf("\nWiFi Connected. IP: %s\n", WiFi.localIP().toString().c_str());
}

// ---------------- INIT FIREBASE ----------------
void startFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Serial.println("Firebase anonymous sign-up...");
  if (!Firebase.signUp(&config, &auth, "", "")) {
    Serial.printf("SignUp failed: %s\n", config.signer.signupError.message.c_str());
  } else {
    Serial.println("SignUp OK");
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Nilai permulaan
  Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_WAITING);
  Firebase.RTDB.setString(&fbdo, "/recognized_name", "-");
  Firebase.RTDB.setInt(&fbdo, "/face_id", -1);
  Firebase.RTDB.setString(&fbdo, "/face_name", "-");
}

// ---------------- MAP ID → NAMA ----------------
static inline const char* mapIdToName(int id) {
  if (id == 0) return "MUHAMMAD_AQIL_BIN_MUHAMMAD_SHAHRIL";
  if (id == 1) return "MUHAMMAD_RUSYAIDI_HAFIY_BIN_HASBULLAH";
  if (id == 2) return "MUHAMMAD_HAIKAL_NIZAM_BIN_RIO_SARIFFUL_NIZAM";
  return "-";
}

// ---------------- HANDLE Wajah ----------------
void handleNewFaceId(int id) {
  const char* name = mapIdToName(id);

  if (id != lastFaceId) {
    lastFaceId   = id;
    lastFaceName = name;

    Serial.print("[RX] Face ID: ");
    Serial.print(id);
    Serial.print(" -> Name: ");
    Serial.println(name);

    // Firebase Updates
    Firebase.RTDB.setInt(&fbdo, "/face_id", id);
    Firebase.RTDB.setString(&fbdo, "/face_name", name);
    Firebase.RTDB.setString(&fbdo, "/recognized_name", name);
  }
}

// ------------- PARSE UART dari CAM -------------
void readFromCameraUart() {
  while (CamSerial.available()) {
    char c = CamSerial.read();

    if (c == '\n' || c == '\r' || c == ' ' || c == ',') {
      if (rxPos > 0) {
        rxBuf[rxPos] = '\0';
        int id = atoi(rxBuf);
        handleNewFaceId(id);
        rxPos = 0;
      }
      continue;
    }

    if ((c >= '0' && c <= '9') || c == '-') {
      if (rxPos < sizeof(rxBuf) - 1) {
        rxBuf[rxPos++] = c;
      } else {
        rxPos = 0;
      }
    }
  }
}

// ---------------- ARDUINO SETUP ----------------
void setup() {
  Serial.begin(115200);
  CamSerial.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Receiver ready.");

  connectWiFi();
  startFirebase();
}

// ---------------- ARDUINO LOOP ----------------
void loop() {
  // Baca data dari ESP32-CAM
  readFromCameraUart();

  // ============================================
  //     LED AUTO OFF 15s selepas ditekan
  // ============================================
  if (ledOnCountdownActive) {
    if (millis() - ledOnTimestamp >= 15000UL) {
      digitalWrite(LED_PIN, LOW);
      ledOnCountdownActive = false;
      Serial.println("LED OFF AUTOMATIC (15 seconds passed)");
    }
  }

  // ============================================
  //     BUTTON DEBOUNCE
  // ============================================
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if (millis() - lastChangeMs > debounceMs) {

    // FALLING EDGE -> BUTANG DITEKAN
    if (lastStableState == HIGH && reading == LOW) {

      Serial.println("Button Pressed → LED ON for 15s");
      digitalWrite(LED_PIN, HIGH);

      // Start 15s timer
      ledOnTimestamp = millis();
      ledOnCountdownActive = true;

      // Arrival status
      Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_ARRIVE);
      delay(8000);
    }



    // RISING EDGE -> BUTANG DILEPAS
    if (lastStableState == LOW && reading == HIGH) {
      Serial.println("Button Released (status only)");
      Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_WAITING);
    }

    lastStableState = reading;
  }
}
