#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "twilio.hpp"


#define WLAN_SSID "Invite-ESIEA"
#define WLAN_PASS "hQV86deaazEZQPu9a"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
#define AIO_USERNAME "raike"
#define AIO_KEY "aio_moWi09kG8q0daSp7l6Z4tuAL30Wq"
#define SEALEVELPRESSURE_HPA (1013.25)

  
#define uS_TO_mS_FACTOR 1000ULL
//#define TIME_TO_SLEEP 10000
#define maxTemperature 30
#define minTemperature 20
#define maxTime 60000
#define minTime 10000

static const char *account_sid = "AC3549a777358ebd77f81598c6be19bf77";
static const char *auth_token = "6c324c1ec010d46e76411eb570144a2c";
static const char *from_number = "+18156764738";
static const char *to_number = "+33675586557";

Twilio *twilio;

Adafruit_BME680 bme;
 WiFiClient client;
 Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
 Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
 Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
 void MQTT_connect();

 void setup() {
  
 Serial.begin(115200);
 delay(1000);
 
 pinMode(4, INPUT_PULLDOWN);
 Serial.println(F("Adafruit MQTT demo"));
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


twilio = new Twilio(account_sid, auth_token);


 while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
 }

 uint32_t x=0;
 void loop() {

  
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  
 MQTT_connect();
 /*Adafruit_MQTT_Subscribe *subscription;
 while ((subscription = mqtt.readSubscription(5000))) 
 {
  if (subscription == &onoffbutton) {
  Serial.print(F("Got: "));
  Serial.println((char *)onoffbutton.lastread);
  }
 }*/
 
 // Now we can publish stuff!
 Serial.print(F("\nSending humidity and temperature val "));
 Serial.print(bme.temperature);
  Serial.print(" / ");
 Serial.print(bme.humidity);
 Serial.print("...");
 /*
 if (! photocell.publish(x++)) {
 Serial.println(F("Failed"));
 } else {
 Serial.println(F("OK!"));
 }*/

if (! temperature.publish(bme.temperature)) {
 Serial.println(F("Failed"));
 } else {
 Serial.println(F("OK!"));
 }
 
 if (! humidity.publish(bme.humidity)) {
 Serial.println(F("Failed"));
 } else {
 Serial.println(F("OK!"));
 }

 if( (double)bme.temperature > 30)
 {
  char message[60];
  sprintf(message, "Temperature = %.2f and humidity = %.2f", (double)bme.temperature, (double)bme.humidity);
  String response;
  bool success = twilio->send_message(to_number, from_number, message, response);
  if (success) {
    Serial.println("Sent message successfully!");
  } else {
    Serial.println(response);
  }
 }

  int TIME_TO_SLEEP=0;

  TIME_TO_SLEEP=maxTime - (((double)bme.temperature - 20) *5) *1000;
  
  if((double)bme.temperature <= 20) {
    TIME_TO_SLEEP=maxTime;
  }
  if((double)bme.temperature >= 30) {
    TIME_TO_SLEEP=minTime;
  }

  Serial.println("Go to sleep for ");
  Serial.println(TIME_TO_SLEEP);
  Serial.println("seconds");

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_mS_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);
  esp_deep_sleep_start();
 
 }
 
 // Function to connect and reconnect as necessary to the MQTT server.
 // Should be called in the loop function and it will take care if connecting.
 void MQTT_connect() {
 int8_t ret;
 if (mqtt.connected()) {
 return;
 }
 Serial.print("Connecting to MQTT... ");
 uint8_t retries = 3;
 while ((ret = mqtt.connect()) != 0) { 
 Serial.println(mqtt.connectErrorString(ret));
 Serial.println("Retrying MQTT connection in 5 seconds...");
 mqtt.disconnect();
 delay(5000);
 retries--;
 if (retries == 0) {
 while (1);
 }
 }
 Serial.println("MQTT Connected!");
 }
