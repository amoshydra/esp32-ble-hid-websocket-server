#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h" // For Wi-Fi credentials

#include <BleKeyboard.h> // For Bluetooth HID

// --- Web Server Setup ---
WebServer server(80);

// Create a BleKeyboard object.
// You can customize the device name, manufacturer, and initial battery level.
BleKeyboard bleKeyboard("ESP32 HID", "Espressif", 100);

void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP32 Web Server!</h1><p>This is a simple web page.</p>");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void handleApiHello() {
  server.send(200, "application/json", "{\"message\": \"Hello from API!\"}");
}

// --- NEW: API endpoint for triggering typing ---
void handleTypeTrigger() {
  if (bleKeyboard.isConnected()) {
    Serial.println("Received API request to type. Sending 'Hello API!' via BLE.");
    bleKeyboard.print("Hello API!"); // Sends "Hello API!" as if typed
    bleKeyboard.write(KEY_RETURN);    // Presses Enter after typing

    server.send(200, "text/plain", "Bluetooth HID: Typed 'Hello API!'.");
  } else {
    Serial.println("Received API request, but BLE Keyboard is not connected.");
    server.send(503, "text/plain", "Bluetooth HID not connected. Please pair the device first.");
  }
}
// --- End NEW API endpoint ---


void setup() {
  Serial.begin(115200);

  // --- Wi-Fi Connection ---
  Serial.print("Connecting to WiFi");
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());

  // --- Web Server Initialization ---
  server.on("/", handleRoot);
  server.on("/api/hello", handleApiHello);
  server.on("/type", handleTypeTrigger); // Register the new API endpoint
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  // --- Bluetooth HID Initialization ---
  Serial.println("Starting BLE Keyboard...");
  bleKeyboard.begin(); // This starts the BLE advertising
  Serial.println("BLE Keyboard started. Ready to pair.");
}


void loop() {
  server.handleClient(); // Continuously handle incoming web server requests
}
