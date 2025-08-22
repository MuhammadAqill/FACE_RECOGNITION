#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "AYAM2.4G";
const char* password = "memensem";

// ESP32-CAM IP address
const char* camIP = "http://192.168.0.140";

#define BUTTON_PIN 4   // Pin untuk butang
#define LED_PIN 2       // Pin LED (built-in LED pada ESP32)


bool lastButtonState = HIGH;
bool camState = false;  // false = OFF, true = ON

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);  // UART2 untuk komunikasi
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

  // Detect button press (falling edge)
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Button pressed - LED ON");
    digitalWrite(LED_PIN, HIGH); // LED menyala
  }

  // Optional: padamkan LED bila butang dilepaskan
  if (lastButtonState == LOW && buttonState == HIGH) {
    Serial.println("Button released - LED OFF");
    digitalWrite(LED_PIN, LOW);
  }

  lastButtonState = buttonState;
}
