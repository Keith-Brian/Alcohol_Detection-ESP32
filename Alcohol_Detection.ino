#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include<PubSubClient.h> // Mqtt Header files

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


#define WIFI_SSID "Sierra"  // 
#define WIFI_PASS "Sierra@13"           //

String apiKey = "8437196"; //8408457 //2209090
String mobileNumber = "254722209026";            //254702333332

WiFiClient Client;

// defining Mqtt
const char* mqtt_server = "192.168.137.229";
PubSubClient mqttClient(Client);


void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
      // Subscribing to a topic for the actuators
      
      mqttClient.subscribe("alert");
      
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

/*******************************************************************************/

// Creating a callback function to check incoming messages from the broker 
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="alert"){
      Serial.print("Incoming Alert: ");
      if(messageTemp == "drunk"){

    String realTime = String(gps.time.hour()+3) +":" +String(gps.time.minute())+ " on: " + String(gps.date.day())+"/"+String(gps.date.month())+"/"+String(gps.date.year());
    String message = "Subject Info: "+ String(gps.location.lat(), 6)+","+String(gps.location.lng(), 6) +" at: " + realTime;
    sendWhatsapp(message);

    delay(2000);
    
      }
      else if(messageTemp == "OFF"){
      
      }
  }else if(topic=="onHour"){
   // onHour = messageTemp.toInt();
    }else if(topic=="onMin"){
   //   onMin = messageTemp.toInt();
      }else{
        Serial.println("Message arrived on unknown topic");
        }
}


// should be called in the loop section
void runMqtt(){

    if (!mqttClient.connected()) {
    reconnect();
  }
  if(!mqttClient.loop())
    mqttClient.connect("ESP8266Client");
  
  }


void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud);

  setupWiFi();

  mqttClient.setServer(mqtt_server,1883);
  mqttClient.setCallback(callback);
  Serial.println();
}

void loop()
{
  runMqtt();
  
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0){
    if (gps.encode(ss.read()))
      displayInfo();
      
      }
      
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
    sendWhatsapp("Location Cannot be Found!");
   
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());

  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
    
  }

  Serial.println();
}


void setupWiFi(){

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting...");
  WiFi.begin(WIFI_SSID,WIFI_PASS);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print("...");
    delay(500);
    }

    Serial.println();
    Serial.println("Connected!");
 
  }

void sendWhatsapp(String message){
  
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + mobileNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message); 

  //String url = "https://api.whatabot.net/whatsapp/sendMessage?apikey=" +apiKey + "&text="+urlEncode(message)+"&phone="+mobileNumber;
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int responseCode = http.POST(url);
  
  if(responseCode ==200){
    Serial.println();
    Serial.println("DM sent successfully!");
     Serial.println(responseCode);
    }else{
      Serial.print("Error sending the message: ");
      Serial.println(responseCode);
      }
      
  http.end();
  }
