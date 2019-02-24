#include "DHT.h"

#define DHTPIN 6
#define DHTTYPE DHT11   // DHT 11
//hum sensor
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

void setup() {  
  Serial.begin(115200);
  pinMode(RELAY1, OUTPUT);  
  digitalWrite(RELAY1,0);          // Turns Relay Off
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
  dht.begin();
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
    
   delay(1000); 
   
   
   }

void getEnvData(){
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
