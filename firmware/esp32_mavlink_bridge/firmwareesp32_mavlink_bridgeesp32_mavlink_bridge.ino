#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi Access Point Configuration
const char *ssid = "AeroMesh-UAV";
const char *password = "aeromesh2026";

// MAVLink UDP Port (Standard QGroundControl port)
const unsigned int localUdpPort = 14550;
IPAddress remoteIP;
unsigned int remotePort = 0;
bool clientConnected = false;

WiFiUDP udp;

// Hardware Serial2 for STM32F411 Flight Controller connection
#define FC_SERIAL Serial2
#define RX_PIN 44 // Connect to FC UART1 TX
#define TX_PIN 43 // Connect to FC UART1 RX

void setup() {
  Serial.begin(115200);
  FC_SERIAL.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Set up standalone SoftAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.println("\n--- AeroMesh S3 Telemetry Bridge Online ---");
  Serial.print("Access Point SSID: ");
  Serial.println(ssid);
  Serial.print("Host IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(localUdpPort);
  Serial.printf("Listening on UDP port %d\n", localUdpPort);
}

void loop() {
  // 1. Read incoming MAVLink packets from QGroundControl over UDP and forward to FC
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    remoteIP = udp.remoteIP();
    remotePort = udp.remotePort();
    clientConnected = true;

    uint8_t udpBuffer[512];
    int len = udp.read(udpBuffer, sizeof(udpBuffer));
    if (len > 0) {
      FC_SERIAL.write(udpBuffer, len);
    }
  }

  // 2. Read telemetry stream from Flight Controller and pipe back to QGroundControl
  if (FC_SERIAL.available() && clientConnected) {
    uint8_t serialBuffer[512];
    int bytesRead = 0;
    while (FC_SERIAL.available() && bytesRead < sizeof(serialBuffer)) {
      serialBuffer[bytesRead++] = FC_SERIAL.read();
    }
    udp.beginPacket(remoteIP, remotePort);
    udp.write(serialBuffer, bytesRead);
    udp.endPacket();
  }
}