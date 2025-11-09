#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"   // optional (debug token)
#include "addons/RTDBHelper.h"    // optional (debug RTDB)

// ================= Wi-Fi =================
const char* ssid     = "AYAM2.4G_BOLELAH";
const char* password = "fawwaznakayam";

// =============== Firebase RTDB ===============
#define API_KEY "AIzaSyCxTNJxuuVkOOdDmqmO6TMcS3EJ4eWDNLg"
#define DATABASE_URL "https://database-for-fyp-hafiy-default-rtdb.asia-southeast1.firebasedatabase.app/"

// =============== Pin setup ===============
#define BUTTON_PIN 4
#define LED_PIN    5

// =============== Status strings ===============
const char* STATUS_WAITING = "Waiting";
const char* STATUS_ARRIVE  = "Someone_Arrive";

// Firebase objects
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

// State & debounce
bool lastStableState = HIGH;     // INPUT_PULLUP: idle HIGH
bool lastReading     = HIGH;
unsigned long lastChangeMs = 0;
const unsigned long debounceMs = 40;

// ---------------- Helpers ----------------
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  Serial.printf("\nWiFi Connected. IP: %s\n", WiFi.localIP().toString().c_str());
}

void startFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // optional

  // Anonymous sign-up (simple & works with auth rules)
  Serial.println("Firebase anonymous sign-up...");
  if (!Firebase.signUp(&config, &auth, "", "")) {
    Serial.printf("SignUp failed: %s\n", config.signer.signupError.message.c_str());
  } else {
    Serial.println("SignUp OK");
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set initial status to "Waiting" (optional)
  if (!Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_WAITING)) {
    Serial.printf("Init write failed: %s\n", fbdo.errorReason().c_str());
  }
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();
  startFirebase();
}

void loop() {
  bool reading = digitalRead(BUTTON_PIN);

  // Debounce
  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if (millis() - lastChangeMs > debounceMs) {
    // FALLING EDGE (button pressed)
    if (lastStableState == HIGH && reading == LOW) {
      Serial.println("Pressed -> LED ON, status: Someone Arrive");
      digitalWrite(LED_PIN, HIGH);
      if (!Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_ARRIVE)) {
        Serial.printf("Write failed: %s\n", fbdo.errorReason().c_str());
      }
    }

    // RISING EDGE (button released)
    if (lastStableState == LOW && reading == HIGH) {
      Serial.println("Released -> LED OFF, status: Waiting");
      digitalWrite(LED_PIN, LOW);
      if (!Firebase.RTDB.setString(&fbdo, "/arrival_status", STATUS_WAITING)) {
        Serial.printf("Write failed: %s\n", fbdo.errorReason().c_str());
      }
    }

    lastStableState = reading;

    delay(2000);
  }
}
