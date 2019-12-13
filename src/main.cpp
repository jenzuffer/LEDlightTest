#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <string>

#define LED 2
int PWM= D1;//Right side  5
int dir = D3;//Left side  4
int minimumFrequency = 414; //714
static volatile int magnetCounter = 0;
static volatile bool lowCheck = false;

//int pin6= 5;
/*
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/

//motoC
//db020dec6e3b
const char *ssid = "motoC"; // The name of the Wi-Fi network that will be created
const char *password = "db020dec6e3b";   // The password required to connect to it, leave blank for an open network
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on //IP = 192.168.43.13
char incomingPacket[255];  // buffer for incoming packets
char replyPacket[] = "Hi there packet recived";

void ICACHE_RAM_ATTR magnetCallBackD5(){
  if (!!magnetCounter){
    if (digitalRead(dir) == LOW){
      lowCheck = false;
    }else if (digitalRead(dir) == HIGH && !lowCheck) {
      Serial.println("reached D5 magnetCallback");
      lowCheck = true;
      magnetCounter--;
    }
    if (!magnetCounter){
      Serial.printf("reached magnetCounter");
      analogWrite(PWM, 0);
    }
  }
}

void ICACHE_RAM_ATTR magnetCallBackD6(){
  if (!!magnetCounter){
    if (digitalRead(dir) == LOW && !lowCheck){
      Serial.println("reached D6 magnetCallback");
      lowCheck = true;
      magnetCounter--;
    }else if (digitalRead(dir) == HIGH){
      lowCheck = false;
    }
    if (!magnetCounter){
      Serial.printf("reached magnetCounter");
      analogWrite(PWM, 0);
    }
  }
}

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  Serial.print("here begin");
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(". failed ");
  }
  Serial.println(" connected");
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(PWM, OUTPUT);
  pinMode(dir, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(D6), magnetCallBackD6, RISING);
  attachInterrupt(digitalPinToInterrupt(D5), magnetCallBackD5, RISING);
  //attachInterrupt PWM to check speed in an easy callback?
  //analogWrite(PWM, minimumFrequency);
}

/*
void defaultfunctionality(){
  //light
  digitalWrite(LED, LOW);
  digitalWrite(LED, HIGH);
  analogWrite(PWM, minimumFrequency);
  if (digitalRead(dir) == LOW){
    if (digitalRead(D6) == LOW){
      lowCheck = false;
      Serial.println("reached D6");
    }
    else if (digitalRead(D5) == LOW && !lowCheck)
    {
      lowCheck = true;
      minimumFrequency = 714;
      Serial.println("reached D5");
      magnetCounter++;
      if (magnetCounter == 2){
        analogWrite(PWM, minimumFrequency / 4);
      } else if (magnetCounter > 2){
        //digitalWrite(PWM, LOW);
        Serial.println("reached reverse");
        digitalWrite(dir, HIGH);
        analogWrite(PWM, minimumFrequency); 
        magnetCounter = 0;
      }
    }
  } else if (digitalRead(dir) == HIGH){
      if (digitalRead(D5) == LOW){
        lowCheck = false;
        Serial.println("reached D5");
      }
      else if (digitalRead(D6) == LOW && !lowCheck)
      {
        lowCheck = true;
        magnetCounter++;
        if (magnetCounter == 2){
          analogWrite(PWM, minimumFrequency / 4);
        } else if (magnetCounter > 2){
          //digitalWrite(PWM, LOW);
          Serial.println("going forwards again");
          digitalWrite(dir, LOW);
          analogWrite(PWM, minimumFrequency);
          magnetCounter = 0;
       }
      }
  }
} */

void changeToBackwards(){
  digitalWrite(dir, HIGH);
  //analogWrite(PWM, minimumFrequency);
}

void removeFrequencyVelocity(){
  analogWrite(PWM, 0);
}

void changeToForwards(){
  digitalWrite(dir, LOW);
  //analogWrite(PWM, minimumFrequency); 
}

void changeFrequencyRate(int velocity){
  analogWrite(PWM, velocity); 
}

int convert_slice(const char *s, size_t a, size_t b) {
    int val = 0;
    while (a < b) {
       val = val * 10 + s[a++] - '0';
    }
    return val;
}

void magnetCounterfunc(int magnetCounterCap){
    magnetCounter = magnetCounterCap;
    lowCheck = true;
}

void replyHelpCommand(){
  Serial.printf("Sending INFO packet back");
  char replyPacket[] = "Useable Commands:H (Help), F (Forward), B (Backward), E (Exit), S (Stop velocity), V (Velocity 100 to 999), M (MagnetCounter 01 to 99)";
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(replyPacket);
  Udp.endPacket();
}

void checkUDPPackets(){
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    Serial.printf("incomingPacket[0] %d\n", incomingPacket[0]);
    switch (incomingPacket[0]){
      case 'F':
        changeToForwards();
        Serial.printf("\ncapital F");
        break;
      case 'E':
        removeFrequencyVelocity();
        Serial.printf("finishing program");
        break;
      case 'S':
        removeFrequencyVelocity();
        Serial.printf("stopping velocity");
        break;
      case 'B':
        changeToBackwards();

        Serial.printf("\nmoving backwards");
        break;
      case 'V':
      {
        int value = convert_slice(incomingPacket, 1, 4);
        Serial.printf("value %d", value);
        changeFrequencyRate(value);
      }
      break;
      case 'M':
      {
        int value1 = convert_slice(incomingPacket, 1, 3);
        Serial.printf("Magnets counting");
        Serial.printf("value %d", value1);
        magnetCounterfunc(value1);
      }
      break;
      case 'H':

      {
        replyHelpCommand();
        break;
      } 
    }
  }
}

void loop() {
  //defaultfunctionality();
  checkUDPPackets();
}