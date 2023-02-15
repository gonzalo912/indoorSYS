#include <time.h>
#include <Arduino.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FS.h>
#include <NewPing.h>

#include "index.h"

#define DHTPIN 14
#define DHTTYPE DHT11

#define HC_SR04_TRIGGER 10
#define HC_SR04_ECHO  9
#define MAX_DISTANCE 400

#define BOMBA_MIXER 13
#define BOMBA_RIEGO 12

#define DIAMETRO  33
#define ALTURA  46

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#ifndef STASSID
#define STASSID "GONZALO2"
#define STAPSK  "00441665963"
#endif

/***** DEFINICION OBJETOS *****/

DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

NewPing hc_sr04(HC_SR04_TRIGGER, HC_SR04_ECHO, MAX_DISTANCE);

IPAddress ip(192, 168, 0, 26); // the IP address you want to use

IPAddress gateway(192, 168, 0, 0); // your gateway IP address

IPAddress subnet(255, 255, 255, 0); // your subnet mask

/******************************/

const int led = 2;

/******* VARIABLES DE CONTROL *********/

String wm = "0"; //water minutes
String mm = "0"; //mix minutes
String wtm = "0"; //wait to mix
String t, h; //temperatura y humedad de la carpa
String d; //distancia de la tapa al agua.

float v; //volumen deposito agua
float v_actual; //volumen de agua en el deposito

const char *ssid = STASSID;
const char *password = STAPSK;

unsigned long TIME;
unsigned char HORAS;
unsigned char MINUTOS;

bool WATERING = false;
bool MIXING = false;
bool STOP = false;

/***************************************/

void handleRoot() {
//  String path = "/home/gonzalo/Desktop/indoorSYS/esp8266/data/index.html";
//  if(SPIFFS.exists(path)){
//    Serial.print("EXISTE EL ARCHIVO.");
//  }
//  File file = SPIFFS.open(path, "r");
//  if (!file) {
//    server.send(404, "text/plain", "File not found");
//    return;
//  }
//  server.streamFile(file, "text/html");
//  file.close();

  String s = MAIN_page;
  String ledState = "";
  server.send(200, "text/html", s);
  server.send(200, "text/plain", "volumen actual del deposito: "+ String(v_actual));
  server.send(200, "text/plain", "volumen del deposito: "+ String(v));
  if(digitalRead(led)){
    ledState = "LED ON";
  }
  else{
    ledState = "LED OFF";
  }
  server.send(200, "text/plain", ledState);
}

void handleWaterPump (){
  String s = water_page;
  server.send(200, "text/html", s);
  wm = server.arg("waterMins");
  server.send(200, "text/plain", "RIEGO SETEADO.");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleStirPump(){
  String s = stir_page;
  server.send(200, "text/html", s);
  mm = server.arg("mixMins");
  wtm = server.arg("waitToMixMins");
  server.send(200, "text/plain", "MIXER SETEADO.");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void handleTogglePump(void){
  digitalWrite(BOMBA_RIEGO, !digitalRead(BOMBA_RIEGO));
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}


void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}


void ESP8266_init(void){
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  int contador = 0;
  while (WiFi.status() != WL_CONNECTED) {
    contador++;
    display.print(".");
    display.display();
    delay(250);
    if (contador == 5) {
      contador = 0;
      display.setCursor(0, 0);
      display.clearDisplay();
      display.display();
    }
  }

  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);
  display.println("IP: ");
  display.print(WiFi.localIP());
  display.println("SSID: ");
  display.print(ssid);
  display.display();
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.on("/water", handleWaterPump);
  server.on("/stir", handleStirPump);
  server.on("/togglePump", handleTogglePump);
  server.begin();
  display.println("SERVER STARTED.");
  display.display();
  delay(5000);
  display.clearDisplay();
}

void display_init (void){
  display.begin(i2c_Address, true);
  display.setContrast(0);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.display();
}

void hc_sr04_init(void){
  return;
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  pinMode(BOMBA_MIXER, OUTPUT);
  pinMode(BOMBA_RIEGO, OUTPUT);

  pinMode(HC_SR04_TRIGGER, OUTPUT);
  pinMode(HC_SR04_ECHO, INPUT);

  digitalWrite(HC_SR04_TRIGGER, LOW);
  digitalWrite(HC_SR04_ECHO, LOW);

  digitalWrite(BOMBA_MIXER, LOW);
  digitalWrite(BOMBA_RIEGO, LOW);

  
  /* DISPLAY SETUP */
  display_init();
  /* WIFI SETUP */
  ESP8266_init();

  hc_sr04_init();
  
  dht.begin();
}

void dht_control(void){
  float t_dht, h_dht;
  t_dht = dht.readHumidity();
  h_dht = dht.readTemperature();
  if (isnan(h_dht) || isnan(t_dht )) {
    t = "--";
    h = "--";
    return;
  }
  t = String(int(t_dht));
  h = String(int(h_dht));
}

void hc_sr04_control(void){
  unsigned int d_hc = hc_sr04.ping_cm();
  float r;
  d = String(d_hc);
  r = DIAMETRO/2;
  v = r * r * 3.14 * ALTURA;
  v_actual = r * r * 3.14 * (ALTURA - d_hc); 
  v_actual = v_actual/1000;
}

void control_water_pump(void){
  unsigned int wm_int = wm.toInt();
  // control de la bomba de agua usando la variable wm
  unsigned long water_mins = wm_int * 60000;
  unsigned long TIME_ACTUAL;
  if (HORAS == 24){
    WATERING = true;
    TIME_ACTUAL = TIME;
    digitalWrite(BOMBA_RIEGO, HIGH);
    HORAS = 0;
  }
  if(v_actual < 10 || TIME - TIME_ACTUAL >= water_mins){
    digitalWrite(BOMBA_RIEGO, LOW);
    return;
  }

  
}

void control_stir_pump(void){
  unsigned int mm_int = mm.toInt();
  unsigned int wtm_int = wtm.toInt();
  // control de la bomba revolvedora usando las variables wtm y mm
  unsigned long mix_milisecs = mm_int * 60000;
  unsigned long wtm_milisecs = wtm_int * 60000;
  

}

void handleTime(void){
  TIME = millis();
  if(TIME%60000 >= 0){
    MINUTOS++;
    if(MINUTOS == 60){
      MINUTOS = 0;
      HORAS++;
    }
  }
}


void display_vars(void) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("T:" + t + "C");
  display.println("H:" + h + "%");
  display.setTextSize(1);
  display.setCursor(0, 32);
  display.print("water: " + wm);
  display.setCursor(0, 40);
  display.print("mix:" + mm);
  display.print("    cada:" + wtm);
  display.setCursor(0, 48);
  display.print("d:" + d + "cm" + " v:" + String(v_actual) + "L");
  display.setCursor(0, 56);
  display.print("IP: ");
  display.print(WiFi.localIP());
  display.display();
}

void loop(void) {
  server.handleClient();
  dht_control();
  hc_sr04_control();
  display_vars();
}
