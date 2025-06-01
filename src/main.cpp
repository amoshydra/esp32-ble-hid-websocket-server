// Standard library includes
#include <Arduino.h>

// ESP-IDF and Arduino framework includes
#include <WiFi.h>
#include <WebServer.h>

// Project-specific includes
#include "secrets.h" // For Wi-Fi credentials

// Third-party libraries
#include <BleKeyboard.h>  // For Bluetooth HID
#include <WebSocketsServer.h>

// Global objects
WebSocketsServer webSocket = WebSocketsServer(81, "*");

// Create a BleKeyboard object.
// You can customize the device name, manufacturer, and initial battery level.
BleKeyboard bleKeyboard("ESP32 HID", "Espressif", 100);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  digitalWrite(LED_BUILTIN, HIGH);

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      webSocket.sendTXT(num, "Connected");
      break;
    }
    case WStype_BIN: {
      if (length < 2) {
        Serial.printf("BINARY:: Payload too short!\n");
        return;
      }
      char eventType = payload[0];
      // Declare message variable outside the switch-case construct
      char* message = nullptr;

      if (eventType == '1') {
        if (length > 1) {
          message = new char[length]; // Allocate memory for the message
          memcpy(message, payload + 1, length - 1); // Copy all characters except the first one
          message[length - 1] = '\0'; // Null-terminate the string

          printf("Sending message: %s\n", message);
          bleKeyboard.print(message);

          delete[] message; // Free allocated memory for the message
        }
      // write don't use
      } else {
        uint8_t hidKeyCode = payload[1]; // Ensure it's interpreted as an integer
        printf("%c: HID Key Code: %d\n", eventType, hidKeyCode);
        
        if (eventType == '2') {
          bleKeyboard.write(hidKeyCode);
        } else if (eventType == '3') {
          bleKeyboard.press(hidKeyCode);
        } else if (eventType == '4') {
          bleKeyboard.release(hidKeyCode);
        }
      }
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
}

// Web server setup
WebServer server(80);

void handleRoot() {
  server.send(200, "text/html",
              "<h1>Hello from ESP32 Web Server!</h1>"
              "<p>This is a simple web page.</p>");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// API endpoint for triggering typing
void handleTypeTrigger() {
  if (bleKeyboard.isConnected()) {
    Serial.println("Received API request to type. Sending 'Hello API!' via BLE.");
    bleKeyboard.print("Hello API!"); // Sends "Hello API!" as if typed
    bleKeyboard.write(KEY_RETURN);   // Presses Enter after typing

    server.send(200, "text/plain", "Bluetooth HID: Typed 'Hello API!'. ");
  } else {
    Serial.println("Received API request, but BLE Keyboard is not connected.");
    server.send(503, "text/plain", "Bluetooth HID not connected. Please pair the device first.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Wi-Fi Connection
  Serial.print("Connecting to WiFi");
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());

  // Web Server Initialization
  server.on("/", handleRoot);
  server.on("/type", handleTypeTrigger); // Register the new API endpoint
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  // Bluetooth HID Initialization
  Serial.println("Starting BLE Keyboard...");
  bleKeyboard.begin(); // This starts the BLE advertising
  Serial.println("BLE Keyboard started. Ready to pair.");

  // WebSocket setup
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient(); // Continuously handle incoming web server requests
  webSocket.loop();
}
