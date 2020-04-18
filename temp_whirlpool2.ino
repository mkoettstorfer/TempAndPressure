/*
 NodeMcu-Client-Aussentemperatur
 
 Projektbeschreibung und weitere Beispiele unter https://www.mikrocontroller-elektronik.de/
*/
#include <ESP8266WiFi.h>
#include <Base64.h>
#include "DHTesp.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


#define DHTTYPE DHT22    // Es handelt sich um den DHT22 Sensor
#define DHTPIN 2 //D4 //Der Sensor wird an PIN 2 angeschlossen    

const char* ssid = "muchfaster"; //Hier SSID eures WLAN Netzes eintragen
const char* password = "Flascheneu"; //Hier euer Passwort des WLAN Netzes eintragen

//ThingSpeek Server
const char* server = "api.thingspeak.com";
const int httpPort = 80;
//ThingSpeek
String apiKey = "PYYE8T3SAFU102I6"; 

WiFiClient client;

/** Initialize DHT sensor */
DHTesp dht;
//DHT dht(DHTPIN, DHTTYPE); //Der Sensor wird ab jetzt mit „dth“ angesprochen

Adafruit_BMP280 bme; // I2C
//Adafruit_BMP280 bme(BMP_CS); // hardware SPI
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);

//
// Setup Serial, Wifi, ...
//
void setup() {
 Serial.begin(115200);
 Serial.setTimeout(2000);

 while(!Serial) { }  
 Serial.println("Wake Up!");
 delay(100);
 
 // Initialize temperature sensor 
  dht.setup(DHTPIN, DHTesp::DHT22); //DHT22 Sensor starten
 delay(10);

  if (!bme.begin(0x76)) { 
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    //while (1);
  }

 Serial.println();
 Serial.println();
 Serial.print("Verbinde mich mit Netz: ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }

 Serial.println("");
 Serial.println("WiFi Verbindung aufgebaut"); 
 Serial.print("Eigene IP des ESP-Modul: ");
 Serial.println(WiFi.localIP());
}

//
//Thingspeek Connection
//
void SendThingSpeak(String temp1, String pressure, String temp2, String humidity){
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(temp1);
    postStr +="&field2=";
    postStr += String(pressure);
    postStr +="&field3=";
    postStr += String(temp2);
    postStr +="&field4=";
    postStr += String(humidity);
    postStr += "\r\n\r\n";
     
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
 
    Serial.print("Außentemperatur 1: ");
    Serial.print(temp1);
    Serial.println(" Grad Celcius");

    Serial.print("Luftdruck: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    Serial.print("Außentemperatur 2: ");
    Serial.print(temp2);
    Serial.println(" Grad Celcius");    
    
    Serial.print("Luftfeuchtigkeit: ");    
    Serial.print(humidity);
    Serial.println("%.");
    Serial.println("Sende an Thingspeak.");  
}

//Daten vom DTH22 Sensor lesen
void ReadDTH22(float &outTemp, float &outHumidity){

  // Reading temperature and humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  //Luftfeuchtigekit
  char humidityStr[6];
  dtostrf(lastValues.humidity, 2, 1, humidityStr);
  Serial.print("Luftfeuchtigkeit: "); 
  Serial.println(humidityStr); 
  outHumidity = lastValues.humidity;

  //Außen Temp
  char temperaturStr[6];
  dtostrf(lastValues.temperature, 2, 1, temperaturStr);
  Serial.print("Temperatur2: "); 
  Serial.println(temperaturStr); 
  outTemp = lastValues.temperature;
}

//Daten vom BMP Sensor  lesen
void ReadBMP(float &outTemp, float &outPressure){

  float temp = bme.readTemperature();
  char tempStr[6];
  dtostrf(temp, 2, 1, tempStr);  
  Serial.print("Temperatur1: ");
  Serial.println(tempStr);
  outTemp = temp;

  float pressure = bme.readPressure() / 100.0F; // Pa -> hPa
  char pressurStr[8];
  dtostrf(pressure, 4, 1, pressurStr);   
  Serial.print("Luftdruck = ");
  Serial.print(pressurStr);
  Serial.println(" hPa");
  outPressure = pressure;

  Serial.println();
}

//Hauptschleife
void loop() {

  Serial.println("Ermittle Daten");
  //Whirlpool Temp
  char temperaturStr1[6];
  char temperaturStr2[6];  
  char pressureStr[8];
  char humidityStr[6];
  float temp1;
  float temp2;
  float humidity;
  float pressure;

  ReadDTH22(temp2, humidity);
  ReadBMP(temp1, pressure);  

  dtostrf(temp1, 2, 1, temperaturStr1);
  dtostrf(temp2, 2, 1, temperaturStr2);
  dtostrf(pressure, 4, 1, pressureStr);
  dtostrf(humidity, 2, 1, humidityStr);      
 
  Serial.print("Verbindungsaufbau zu Server ");
  Serial.println(server);

  if (client.connect(server,80)) {   //   "184.106.153.149" or api.thingspeak.com

    SendThingSpeak(temperaturStr1, pressureStr, temperaturStr2, humidityStr);
  } else {
    Serial.println("\nVerbindung gescheitert");
  }
  client.stop();
 
  WiFi.disconnect(); 
  Serial.println("\nVerbindung beendet");
 
  Serial.println("Schlafe jetzt ...");
  ESP.deepSleep( 10*60000000); //Angabe in Minuten - hier 10
  //delay(30000); //1Min warten
}
