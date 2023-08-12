#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <SD.h>
#include "ESP-FTP-Server-Lib.h"
#include "FTPFilesystem.h"
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char *ssid = "TP-Link_99QM";
const char *password = "546823398lol9190";
const char *hostName = "ACCEL-ST";

#define FTP_USER "ftp"
#define FTP_PASSWORD "ftp"

FTPServer ftp;

WebSocketsServer webSocket(81);

void setupWiFi()
{
    WiFi.mode(WIFI_STA);

    if (WiFi.setHostname(hostName))
    {
        M5.Lcd.println("Hostname changed");
    }
    else
    {
        M5.Lcd.println("Hostname changing error");
    }

    M5.Lcd.println("Connecting to WiFi");

    WiFi.begin(ssid, password);
    int ccount = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        ccount++;
        if (ccount >= 20)
        {
            M5.Lcd.println(WiFi.status());
            WiFi.begin(ssid, password);
            ccount = 0;
        }
    }
    M5.Lcd.println();

    M5.Lcd.print("WiFi connected. Local IP: ");
    M5.Lcd.println(WiFi.localIP());

    WiFi.setAutoReconnect(true);
}

void setupFTP(){
    ftp.addUser(FTP_USER, FTP_PASSWORD);
	ftp.addFilesystem("SD", &SD);
	if (ftp.begin()){
        M5.Lcd.println("Ftp started");
    }else{
        M5.Lcd.println("Ftp error");
    }
}

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_ERROR:
    break;
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
  {
    break;
  }
  case WStype_BIN:
    break;
  case WStype_FRAGMENT_TEXT_START:
    break;
  case WStype_FRAGMENT_BIN_START:
    break;
  case WStype_FRAGMENT:
    break;
  case WStype_FRAGMENT_FIN:
    break;
  case WStype_PING:
    break;
  case WStype_PONG:
    break;

  default:
    break;
  }
}

void setupWebsocket()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void setup()
{
    M5.begin();
    M5.Speaker.mute();
    M5.Mpu6886.Init();
    
    setupWiFi();
    setupFTP();
    setupWebsocket();

    delay(1000);
    M5.Lcd.println();
    M5.Lcd.println();
}

void loop()
{
    M5.update();
    ftp.handle();
    webSocket.loop();

    float gx, gy, gz;
    float pitch, roll, yaw;
    float ax, ay, az;
    int x, y;
    M5.Mpu6886.getGyroData(&gx, &gy, &gz);
    M5.Mpu6886.getAhrsData(&pitch, &roll, &yaw);
    M5.Mpu6886.getAccelData(&ax, &ay, &az);

    x = M5.Lcd.cursor_x;
    y = M5.Lcd.cursor_y;

    M5.Lcd.printf("gx = %.2f; gy = %.2f; gz = %.2f\n", gx, gy, gz);
    M5.Lcd.printf("p = %.2f; r = %.2f; y = %.2f\n", pitch, roll, yaw);
    M5.Lcd.printf("ax = %.2f; ay = %.2f; az = %.2f\n", ax, ay, az);

    M5.Lcd.setCursor(x, y);

    String s;
    DynamicJsonDocument doc(1024);
    doc["gx"] = gx;
    doc["gy"] = gy;
    doc["gz"] = gz;
    doc["ax"] = ax;
    doc["ay"] = ay;
    doc["az"] = az;
    doc["p"] = pitch;
    doc["r"] = roll;
    doc["y"] = yaw;
    doc["gys"] = M5.Mpu6886.Gyscale;
    serializeJson(doc, s);

    webSocket.broadcastTXT(s);
}