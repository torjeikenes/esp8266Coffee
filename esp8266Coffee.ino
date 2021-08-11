#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Servo.h"

#include "config.h"

#define ledPin LED_BUILTIN

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = SSID;
const char* password = PASS;
const char* mqtt_server = MQTT;
const char* mqtt_user = MQTT_U;
const char* mqtt_password = MQTT_P;

int servoPin = D7; 
int btnPin = D5;
Servo coffeeServo;

WiFiClient espClient;
PubSubClient client(espClient);
String command;
String strTopic;
String strPayload;
int btnState = 0;
int prevState = 0;


void push()
{
  coffeeServo.attach(servoPin);
  coffeeServo.write(10);
  delay(1000);
  coffeeServo.write(50);
  delay(1000);
  coffeeServo.detach();
}


void setup_wifi() {
 Serial.begin(115200);
  delay(100);
 
  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if(strTopic == "kaffe/control"){
    command = String((char*)payload);
    if(command == "start"){
        Serial.println("ON");
        push();
    }
  }
  else if(strTopic == "kaffe/state"){
    command = String((char*)payload);
    if(command == "on"){
      digitalWrite(ledPin, HIGH);
    }
    else if(command == "off"){
      digitalWrite(ledPin, LOW);
    }
  }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish("kaffe/state","oppe");
      // Once connected, publish an announcement...
      client.subscribe("kaffe/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(btnPin,INPUT);
  pinMode(ledPin,OUTPUT);

}
 
void loop()
{
  btnState = digitalRead(btnPin);

  if(btnState == HIGH && prevState == LOW){
    prevState = HIGH;
    client.publish("kaffe/state","btn");
  }
  else if(btnState == LOW){
    prevState = LOW;
  }


  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
