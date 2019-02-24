#include "arduino_setup.h" 
#include "DHT.h"
#include "Air_Quality_Sensor.h"

// air quality
AirQualitySensor airqsensor(A1);
int airq, airqval;
String airqstr;
  
//hum sensor
#define DHTPIN 6
#define DHTTYPE DHT11   // DHT 11
int sensor_pin = A0;
int output_value ;

//relay
#define RELAY1  7   
int valve = 0;

// rgb 
int redPin = 8;
int greenPin =9;
int bluePin = 10;

DHT dht(DHTPIN, DHTTYPE);
float temp_out,humid_out;

unsigned long startMillis;  
unsigned long currentMillis;
 
#define LED_PIN LED0

#define REPLYBUFFSIZ 0xFFFF
char replybuffer[REPLYBUFFSIZ];
uint8_t getReply(char *send, uint16_t timeout = 500, boolean echo = true);
uint8_t espreadline(uint16_t timeout = 500, boolean multiline = false);
boolean sendCheckReply(char *send, char *reply, uint16_t timeout = 500);

enum {WIFI_ERROR_NONE=0, WIFI_ERROR_AT, WIFI_ERROR_RST, WIFI_ERROR_SSIDPWD, WIFI_ERROR_SERVER, WIFI_ERROR_UNKNOWN};

void setup() {  
  Serial.begin(115200);
  pinMode(RELAY1, OUTPUT);  
  digitalWrite(RELAY1,0);          // Turns Relay Off
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
  dht.begin();

  //Open software serial for chatting to ESP
  Serial2.begin(115200);

  //connect to the wifi
  byte err = setupWiFi();

  if (err) {
    // error, print error code
    Serial.print("setup error:");  Serial.println((int)err);
  //  debugLoop();
  }

  // success, print IP
   getIP();
   startMillis = millis();  //initial start time
  //set TCP server timeout
  //sendCheckReply("AT+CIPSTO=0", "OK");  , char *page
    if (airqsensor.init()) {
    Serial.println("Sensor ready.");
  }
  else {
    Serial.println("Sensor ERROR!");
  }
   }

boolean ESP_GETpage(char *host, uint16_t port) {
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += host;
  cmd += "\",";
  cmd += port;
  cmd.toCharArray(replybuffer, REPLYBUFFSIZ);

  getReply(replybuffer);

  if (strcmp(replybuffer, "OK") != 0) {
    while (true) {
      espreadline(500);  // this is the 'echo' from the data
      Serial.print("<--- "); Serial.println(replybuffer);
      if (strstr(replybuffer, "OK"))
      break;
    }
  }

//  delay(500);
 
  String data = ""; 
  data += "/a/?d=";
  data += output_value;
  data += "|";
  data += temp_out;
  data += "|";
  data += humid_out;
  data += "|";
  data += airqval;
  data += "|";
  data += airqstr;
  data += "&s=";
  data += DSTOKEN;
  data += "&a=";
  data += ATOKEN;
  
  String request = "GET ";
  request += data;
  request += " HTTP/1.1\r\nHost: ";
  request += host;
  request += "\r\n\r\n";

  cmd = "AT+CIPSEND=";
  cmd += request.length();
  cmd.toCharArray(replybuffer, REPLYBUFFSIZ);
  sendCheckReply(replybuffer, ">");

  Serial.print("Sending: "); Serial.println(request.length());
  Serial.println(F("*********SENDING*********"));
  Serial.print(request);
  Serial.println(F("*************************"));

  request.toCharArray(replybuffer, REPLYBUFFSIZ);

  Serial2.println(request);

  while (true) {
    espreadline(3000);  // this is the 'echo' from the data
    Serial.print(">"); Serial.println(replybuffer); // probably the 'busy s...'

    // LOOK AT ALL THESE POSSIBLE ARBITRARY RESPONSES!!!
    if (strstr(replybuffer, "wrong syntax"))
      continue;
    else if (strstr(replybuffer, "ERROR"))
      continue;
    else if (strstr(replybuffer, "busy s..."))
      continue;
    else if (strstr(replybuffer, "OK"))
      break;
//    else break;
  }

  if (! strstr(replybuffer, "OK") ) return false;

  espreadline(50);
  Serial.print("3>"); Serial.println(replybuffer);
  if(char *s = strstr(replybuffer, "+IPD,")){
    uint16_t len = atoi(s+5);
    Serial.print(len); Serial.println(" bytes total");
  }

  unsigned long i = 0;
  while (1) {
    char c;
    if(Serial2.available()){
      c = Serial2.read(); //UDR0 = c;
//      Serial.write(c);
      replybuffer[i] = c;
      i++;
      delay(1);
      if(!Serial2.available()){
        return true;
      }
    }
  }
  //while (1) {
  //  if (esp.available()) UDR0 = esp.read();
  //}
}


void loop() {
   getEnvData(); 
 
 if (output_value < 80 && output_value > 40) {
    setColor(0, 255, 0);  // green 
    digitalWrite(RELAY1,0);          // Turns Relay Off
    valve=0; 
  }
 if (output_value < 40 && output_value > 30 ) {
     setColor(0, 0, 255);  // blue
    }
 if (output_value < 30 && output_value > 5 ) {
    setColor(255, 0, 0);  // red  
    digitalWrite(RELAY1,1);           // Turns ON Relays 1
    valve=1; 
   } 
 if (output_value < 5 && output_value > -10 ) {
    setColor(100, 100, 100);  // white
    digitalWrite(RELAY1,0);  
    valve=0; 
   } 
 
      
   Serial.print(output_value); 
   Serial.print("%");   
   Serial.print(" ");
   Serial.print(temp_out);  
   Serial.print("'C ");
   Serial.print(humid_out);  
   Serial.print("% ");
   Serial.println(valve);

 currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= PERIOD)  //test whether the period has elapsed
  {
    ESP_GETpage(HOST, PORT);  //, WEBPAGE
   Serial.println(F("**********REPLY***********"));
   Serial.println(replybuffer);
   Serial.println(F("**************************"));

   sendCheckReply("AT+CIPCLOSE", "OK");   
   startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state. 
  // debugLoop();

  }
   
    
   delay(2000); 
   
   
   }

void getEnvData(){
   // air quality data
   airq = airqsensor.slope();
   airqval = airqsensor.getValue()/10;

  if (airq == AirQualitySensor::FORCE_SIGNAL) {
    Serial.print("High pollution! Force signal active. ");
    airqstr = "high high";
  }
  else if (airq == AirQualitySensor::HIGH_POLLUTION) {
    Serial.print("High pollution! ");
     airqstr = "high";
  }
  else if (airq == AirQualitySensor::LOW_POLLUTION) {
    Serial.print("Low pollution! ");
     airqstr = "low";
  }
  else if (airq == AirQualitySensor::FRESH_AIR) {
    Serial.print("Fresh air. ");
     airqstr = "fresh";
  }
   
   // Read humid from soil
  output_value= analogRead(sensor_pin); 
  output_value = map(output_value,550,0,0,100);
  // Read temperature as Celsius (the default)
  temp_out = dht.readTemperature();
  humid_out = dht.readHumidity(); 

  }
   
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

boolean ESPconnectAP(char *s, char *p) {

  getReply("AT+CWMODE=1", 500, true);
  if (! (strstr(replybuffer, "OK") || strstr(replybuffer, "no change")) )
    return false;

  String connectStr = "AT+CWJAP=\"";
  connectStr += SSID;
  connectStr += "\",\"";
  connectStr += PASS;
  connectStr += "\"";
  connectStr.toCharArray(replybuffer, REPLYBUFFSIZ);
  getReply(replybuffer, 200, true);

  while (true) {
    espreadline(200);  // this is the 'echo' from the data
    if((String)replybuffer == ""){
      Serial.print(".");
    }else{
      Serial.println("");
      Serial.print("<--- "); Serial.println(replybuffer);
    }
    if (strstr(replybuffer, "OK"))
    break;
  }

  return true;
}


byte setupWiFi() { 
 
  delay(500);

  Serial.println(F("Checking for ESP AT response"));
  if (!sendCheckReply("AT", "OK"))
    return WIFI_ERROR_AT;
 
  Serial.print(F("Connecting to ")); Serial.println(SSID);
  if (!ESPconnectAP(SSID, PASS))
    return WIFI_ERROR_SSIDPWD;

  Serial.println(F("Single Client Mode"));

  if (!sendCheckReply("AT+CIPMUX=0", "OK"))
        return WIFI_ERROR_SERVER;

  return WIFI_ERROR_NONE;
}

boolean getIP() {
  getReply("AT+CIFSR", 100, true);
  while (true) {
    espreadline(50);  // this is the 'echo' from the data
    Serial.print("<--- "); Serial.println(replybuffer);
    if (strstr(replybuffer, "OK"))
    break;
  }

  delay(100);

  return true;
}




/************************/
uint8_t espreadline(uint16_t timeout, boolean multiline) {
  uint16_t replyidx = 0;

  while (timeout--) {
    if (replyidx > REPLYBUFFSIZ-1) break;

    while(Serial2.available()) {
      char c =  Serial2.read();
      if (c == '\r') continue;
      if (c == 0xA) {
        if (replyidx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiline) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
      }
      replybuffer[replyidx] = c;
      // Serial.print(c, HEX); Serial.print("#"); Serial.println(c);
      replyidx++;
    }

    if (timeout == 0) break;
    delay(1);
  }
  replybuffer[replyidx] = 0;  // null term
  return replyidx;
}

uint8_t getReply(char *send, uint16_t timeout, boolean echo) {
  // flush input
  while(Serial2.available()) {
     Serial2.read();
     delay(1);
  }

  if (echo) {
    Serial.print("---> "); Serial.println(send);
  }
  Serial2.println(send);

  // eat first reply sentence (echo)
  uint8_t readlen = espreadline(timeout);

  //Serial.print("echo? "); Serial.print(readlen); Serial.print(" vs "); Serial.println(strlen(send));

  if (strncmp(send, replybuffer, readlen) == 0) {
    // its an echo, read another line!
    readlen = espreadline();
  }

  if (echo) {
    Serial.print ("<--- "); Serial.println(replybuffer);
  }
  return readlen;
}

boolean sendCheckReply(char *send, char *reply, uint16_t timeout) {

  getReply(send, timeout, true);

 /*
  for (uint8_t i=0; i<strlen(replybuffer); i++) {
    Serial.print(replybuffer[i], HEX); Serial.print(" ");
  }
  Serial.println();
  for (uint8_t i=0; i<strlen(reply); i++) {
    Serial.print(reply[i], HEX); Serial.print(" ");
  }
  Serial.println();
*/
  return (strcmp(replybuffer, reply) == 0);
}

void debugLoop() {
  Serial.println("========================");
  //serial loop mode for diag
  while(1) {
    if (Serial.available()) {
      Serial2.write(Serial.read());
//      delay(1);
    }
    if (Serial2.available()) {
      Serial.write(Serial2.read());
//      delay(1);
    }
  }
}
