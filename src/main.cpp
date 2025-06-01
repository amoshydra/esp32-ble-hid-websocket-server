#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Include your secrets file
#include "secrets.h" // This line brings in SECRET_SSID and SECRET_PASSWORD

WebServer server(80); // Create a web server on port 80

void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP32 Web Server!</h1><p>This is a simple web page.</p>");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void handleApiHello() {
  server.send(200, "application/json", "{\"message\": \"Hello from API!\"}");
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi using credentials from secrets.h
  Serial.print("Connecting to WiFi");
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/api/hello", handleApiHello);
  server.onNotFound(handleNotFound); // Handle unknown paths

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}
