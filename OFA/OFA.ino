#include <WiFi.h>
#include <WebServer.h>
#include "PMS.h"
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>


#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing reset pin)

// Set up PMS
PMS pms(Serial2);
PMS::DATA data;

int RELAY_PIN = 23;
int BUZZER_PIN = 15;
 
// Set up SSD OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Set up Server
const String serverURL = "broker.mqtt-dashboard.com";
const int port = 1883;
const char* TOPIC_INPUT = "20127470_20127566_20127611/OFA_input";
const char* TOPIC_OUTPUT = "20127470_20127566_20127611/OFA_output";
WebServer server(80);


// Set up Wifi
const char *ssid = "Wokwi-GUEST";
const char *password = "";
WiFiClient espClient;
PubSubClient client(espClient);

// Set up PM variables
String PM1_0;
String PM2_5;
String PM10;


void wifiConnect()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("\nConnected wifi success!");
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
}
 
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
 
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
 
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
 
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.display();
  delay(100);
  display.clearDisplay();
 
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("Initializing....");
  display.display();
  delay(3000);
  
 
  Serial.println("Connecting to ");
  Serial.println(ssid);

 
  //check wi-fi is connected to wi-fi network
  wifiConnect();
 
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.print("WiFi connected..!");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.print(WiFi.localIP());
  display.display();
  delay(4000);
 
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println("HTTP server started");

  client.setServer(serverURL.c_str(), port);
  client.setCallback(callback);
}
 
void loop()
{
  if (pms.read(data))
  {
    PM1_0 = data.PM_AE_UG_1_0;
    PM2_5 = data.PM_AE_UG_2_5;
    PM10 = data.PM_AE_UG_10_0;
 
    Serial.println("Air Quality Monitor");
 
    Serial.println("PM1.0 :" + PM1_0 + "(ug/m3)");
 
    Serial.println("PM2.5 :" + PM2_5 + "(ug/m3)");
 
    Serial.println("PM10  :" + PM10 + "(ug/m3)");
 
    Serial.println("");
 
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("PM1.0: ");
    display.print(PM1_0);
    display.print(" ug/m3");
 
    display.setCursor(10, 30);
    display.print("PM2.5: ");
    display.print(PM2_5);
    display.print(" ug/m3");
 
    display.setCursor(10, 50);
    display.print("PM10 : ");
    display.print(PM10);
    display.print(" ug/m3");
 
    display.display();
 
    delay(2000);
  }
  if (PM2_5.toFloat() >= 150)
  {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  server.handleClient();
}
void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(PM1_0.toFloat(), PM2_5.toFloat(), PM10.toFloat()));
}
 
 
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
 
String SendHTML(int PM1_0, int PM2_5, int PM10)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Wireless Weather Station</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script>\n";
  ptr += "setInterval(loadDoc,1000);\n";
  ptr += "function loadDoc() {\n";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.onreadystatechange = function() {\n";
  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "document.body.innerHTML =this.responseText}\n";
  ptr += "};\n";
  ptr += "xhttp.open(\"GET\", \"/\", true);\n";
  ptr += "xhttp.send();\n";
  ptr += "}\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>Particulate Matter Monitoring</h1>\n";
 
  ptr += "<p>PM1.0: ";
  ptr += PM1_0;
  ptr += " ug/m3</p>";
 
  ptr += "<p>PM2.5: ";
  ptr += PM2_5;
  ptr += " ug/m3</p>";
 
  ptr += "<p>PM10: ";
  ptr += PM10;
  ptr += " ug/m3</p>";
 
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}