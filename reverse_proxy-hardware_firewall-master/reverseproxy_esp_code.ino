#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#define MAX_STRING_LENGTH 50 // Maximum length of the input string

const char* ssid = "a";
const char* password = "1234567890";
const char* apSSID = "NodeMCU_AP";
const char* apPassword = "AP_Password";
String flaskServerAddress ;

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
ESP8266WebServer server(80);

void handleRequest() {
  if (server.method() == HTTP_GET || server.method() == HTTP_POST) {
    // Forward request to Flask server
        WiFiClient client;
    HTTPClient http;
    http.begin(client, flaskServerAddress + server.uri());
    http.addHeader("Content-Type", server.header("Content-Type"));
    int httpResponseCode = 0;
    if (server.method() == HTTP_GET) {
      httpResponseCode = http.GET();
    } else if (server.method() == HTTP_POST) {
      httpResponseCode = http.POST(server.arg("plain"));
    }

    // Relay response back to client
    server.send(httpResponseCode, "text/html", http.getString());
    http.end();
  }
}


String readStringFromSerial() {
  String inputString = "";
  while (true) {
    if (Serial.available()) {
      char incomingChar = Serial.read();
      if (incomingChar == '\n') {
        break; // Exit the loop if newline is received
      }
      inputString += incomingChar;
      if (inputString.length() >= MAX_STRING_LENGTH) {
        break; // Exit the loop if maximum length is reached
      }
    }
  }
  return inputString;
}

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  while (!Serial) {
    ; // Wait for the serial port to connect. Needed for native USB port only
  }

  flaskServerAddress = readStringFromSerial();
  WiFi.softAP(apSSID, apPassword);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.println("Access Point Started");

  // Start DNS server
  dnsServer.start(DNS_PORT, "*", apIP);

  // Set up HTTP server
  server.onNotFound(handleRequest);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
