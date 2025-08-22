#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "AYAM2.4G";
const char* password = "memensem";

// ESP32-CAM IP address
const char* camIP = "http://192.168.0.140";

#define BUTTON_PIN 14   // Pin untuk butang
#define LED_PIN 2       // Pin LED (built-in LED pada ESP32)



bool lastButtonState = HIGH;
bool camState = false;  // false = OFF, true = ON

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);   // LED output
  digitalWrite(LED_PIN, LOW); // Pastikan mula-mula LED padam

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Tunjukkan IP address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  Serial.println("Data : " + String(random(1,100)));
  delay(1000);

  // Detect button press (falling edge)
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Button pressed - LED ON");
    digitalWrite(LED_PIN, HIGH); // LED menyala
    sendRequest(String(camIP) + "/ledon");
  }

  // Optional: padamkan LED bila butang dilepaskan
  if (lastButtonState == LOW && buttonState == HIGH) {
    Serial.println("Button released - LED OFF");
    digitalWrite(LED_PIN, LOW);
    sendRequest(String(camIP) + "/ledoff");
  }

  lastButtonState = buttonState;
}

void sendRequest(String url) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpCode);
    } else {
      Serial.printf("HTTP Request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
