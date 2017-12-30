#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SSID"
#define WLAN_PASS       "PASS"
bool one_time_only = 1;
/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "USERNAME"
#define AIO_KEY         "AIOKey"

#define touch A0
#define LED D2
int x = 0;
bool touch_flag = 0;
bool mqtt_flag = 0;
bool one_time_flag = 1;
int lamp_1_master = 0;
int lamp_2_master = 0;
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish lamp_brightness_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lamp_brightness"); //lamp_1_master
Adafruit_MQTT_Publish lamp_1_master_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lamp_1_master");
Adafruit_MQTT_Publish lamp_2_master_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lamp_2_master");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe lamp_brightness_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lamp_brightness");
Adafruit_MQTT_Subscribe lamp_1_master_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lamp_1_master");
Adafruit_MQTT_Subscribe lamp_2_master_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lamp_2_master");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {

  pinMode(touch, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&lamp_brightness_sub);
   mqtt.subscribe(&lamp_1_master_sub);
  mqtt.subscribe(&lamp_2_master_sub);

}

void loop() {


  int brightness_value;
  int counter_flag = 0;

  MQTT_connect();

if (one_time_only)
  {
    if (!lamp_1_master_pub.publish(0)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("LAMP 1 = 0"));
    }
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(1))) {
      Serial.println("FIRST READING");

      if (subscription == &lamp_1_master_sub) {
        Serial.print(F("Got: "));
        Serial.println((char *)lamp_1_master_sub.lastread);
        lamp_1_master = atoi((char *)lamp_1_master_sub.lastread);
      }
    }
    if (!lamp_2_master_pub.publish(0)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("LAMP 2 = 0"));
    }

    while ((subscription = mqtt.readSubscription(1))) {
      Serial.println("SECOND READING");

      if (subscription == &lamp_2_master_sub) {
        Serial.print(F("Got: "));
        Serial.println((char *)lamp_2_master_sub.lastread);
        lamp_2_master = atoi((char *)lamp_2_master_sub.lastread);
      }
    }
    Serial.print(F("\nSending brightness val "));
        Serial.print(x);
        Serial.print("...");
        if (! lamp_brightness_pub.publish(x)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Published value"));
        }
        analogWrite(LED, x);
    one_time_only = 0;
  }
  if (lamp_1_master == 0 && lamp_2_master == 0)
  {
    Serial.println("FIRST IF");
    while (analogRead(touch) < 850)
    {
      delay(1);
      if (analogRead(touch) > 850)
      {
         lamp_2_master = 1;
        if (!lamp_2_master_pub.publish(1)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("LAMP 2 = 1"));
        }
        Serial.println("lamp_2_master");
        break;
      }
      
      Adafruit_MQTT_Subscribe *subscription;
      while ((subscription = mqtt.readSubscription(1))) {
    Serial.println("FIRST READING");

    if (subscription == &lamp_1_master_sub) {
      Serial.print(F("Got: "));
      Serial.println((char *)lamp_1_master_sub.lastread);
      lamp_1_master = atoi((char *)lamp_1_master_sub.lastread);
    }
  }

  if(lamp_1_master == 1)
  break;
    }
  }

 if (lamp_1_master == 0 && lamp_2_master == 1 && counter_flag < 5000)
  {
    // Serial.println("SECOND IF");
repeat:
    // Serial.println(analogRead(touch));
    if (analogRead(touch) > 850)
    {
      Serial.println("TOUCH");
      counter_flag = 0;

      x = x + 15;
      if (x > 255)
        x = 255;
      analogWrite(LED, x);
      delay(200);
    }
    while (counter_flag < 5000)
    {

      if (analogRead(touch) > 850 && counter_flag < 5000)
      {
        delay(1);
        break;
      }
      else if (analogRead(touch) < 850 && counter_flag > 5000)      {

        delay(1);
        break;
      }
      else
      {
        counter_flag++;
        delay(1);
        goto repeat;
      }

    }

    if (counter_flag >= 5000)
    {
    
        Serial.print(F("\nSending brightness val from lamp 2"));
        Serial.print(x);
        Serial.print("...");
        if (! lamp_brightness_pub.publish(x)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Published value"));
        }
  }
  }
  if (lamp_1_master == 0 && lamp_2_master == 1 && counter_flag >= 5000)
  {
  while(lamp_2_master != 0){
     Adafruit_MQTT_Subscribe *subscription;
      while ((subscription = mqtt.readSubscription(1))) {
        Serial.println("mqtt");
        if (subscription == &lamp_brightness_sub) {
          Serial.print(F("Got: "));
          Serial.println((char *)lamp_brightness_sub.lastread);
          x = atoi((char *)lamp_brightness_sub.lastread);
          analogWrite(LED, x);
          Serial.print("Value of x = "); Serial.println(x);
        }
      }
      if(x == 0)
      {
        counter_flag = 0;
         if (!lamp_2_master_pub.publish(x)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("LAMP 2 = 0"));
      }
      lamp_2_master = 0;
      }
  }
  }

   if (lamp_1_master == 1)
  {

    while (analogRead(touch) < 850)
    {
      Adafruit_MQTT_Subscribe *subscription;
      while ((subscription = mqtt.readSubscription(1))) {
        Serial.println("mqtt");
        if (subscription == &lamp_brightness_sub) {
          Serial.print(F("Got: "));
          Serial.println((char *)lamp_brightness_sub.lastread);
          x = atoi((char *)lamp_brightness_sub.lastread);
          analogWrite(LED, x);
          Serial.print("Value of x = "); Serial.println(x);
        }
      }
    }
    Serial.println("TOUCH");
    touch_flag = 1;
    x = 0;
    analogWrite(LED, x);
    Serial.print(F("\nSending brightness val from lamp 1"));
    Serial.print(x);
    Serial.print("...");
    if (! lamp_brightness_pub.publish(x)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("brightness = 0"));
    }
    if (!lamp_1_master_pub.publish(x)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("LAMP 1 = 0"));
    }
    lamp_1_master = 0;
    if (!lamp_2_master_pub.publish(x)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("LAMP 2 = 0"));
    }
    lamp_2_master = 0;
    counter_flag = 0;

  }


 // delay(500);



  // put your main code here, to run repeatedly:

}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

