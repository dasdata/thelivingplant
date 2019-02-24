#include <SDHCI.h>
#include <Audio.h>
// audio stuff
SDClass theSD;
AudioClass *theAudio;
File myFile,subFile;
bool ErrEnd = false;


 

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

void setup() {  
   // start audio system
  theAudio = AudioClass::getInstance(); 
  theAudio->begin(audio_attention_cb);  
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL); 
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT); 
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);
  
  /* Open file placed on SD card */
  myFile = theSD.open("rainforest_ambience.mp3");
  subFile = theSD.open("sirifillwater.mp3");  

  //serial & relay
  Serial.begin(115200);
  pinMode(RELAY1, OUTPUT);  
  digitalWrite(RELAY1,0);          // Turns Relay Off
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);   
   }

void loop() { 
  // get humidity
   output_value= analogRead(sensor_pin);
   // set percentage 
   output_value = map(output_value,550,0,0,100);
   
 // setup conditions
 if (output_value < 80 && output_value > 40) {
    setColor(0, 255, 0);  // green 
   closeValve(); 
  }
  if (output_value < 40 && output_value > 30 ) {
     setColor(0, 0, 255);  // blue
    }
 if (output_value < 30 && output_value > 5 ) {
        setColor(255, 0, 0);  // red  
      openValve(); 
      playMe(myFile); 
      } 
   if (output_value < 5 && output_value > -10 ) {
      setColor(100, 100, 100);  // white
      closeValve(); 
      } 
      
   // show some values   
   Serial.print(output_value); 
   Serial.print("%");   
   Serial.print(" ");
   Serial.println(valve);
    
   delay(1000); 
   
   
   }

void openValve() {
   digitalWrite(RELAY1,1);           // Turns ON Relays 1
   valve=1; 
   // playMe(subFile);
  }

void closeValve() {
    digitalWrite(RELAY1,0);    // Turns OFF  Relays 1
    valve=0; 
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

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
 // puts("Attention!"); 
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}


void playMe(File file) {
  puts("loop!!"); 
  
  /* Send new frames to decode in a loop until file ends */
  int err = theAudio->writeFrames(AudioClass::Player0, file);
  
  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
  {
    printf("File Read Error! =%d\n", err);
    file.close();
    exit(1);
  } 
  
  /* Main volume set to -16.0 dB */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);

  
  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND)
  {
    printf("Main player File End!\n");
  }

  /* Show error code from player and stop */
  if (err)
  {
    printf("Main player error code: %d\n", err);
    goto stop_player;
  }

  if (ErrEnd)
  {
    printf("Error End\n");
    goto stop_player;
  }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */

  usleep(40000);


  /* Don't go further and continue play */
  return;

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  file.close();
  exit(1);
 
  }
