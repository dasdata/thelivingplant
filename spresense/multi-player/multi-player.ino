#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;
File myFile;
bool ErrEnd = false;
int currentSound = 1;

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

String getSoundFile(int sound) {
  switch(sound) {
    case 1: return "slurping.mp3";
    case 2: return "laughing_birds.mp3";
    case 3: return "rainforest_ambience.mp3";
    case 4: return "howler-monkeys.mp3";
    case 5: return "wateringlass.mp3";
    case 6: return "alienship.mp3";
    case 7: return "birdsound.mp3";
    case 8: return "blop.mp3";
    default: return "firetruck.mp3";
  }
}

void playSoundFile(String fileName) {

  if (ErrEnd) 
    {
      theAudio->startPlayer(AudioClass::Player0);
      ErrEnd = false;
    }
  
  /* Close file if open */
  if (myFile)
    {
      myFile.close();
    }
  
  /* Open file placed on SD card */
  myFile = theSD.open(fileName);

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %d\n",myFile);
  printf(myFile.name());

  /* Send first frames to be decoded */
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }
    
  

  ////////////////// Error handling
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
  //theAudio->stopPlayer(AudioClass::Player0);
    
    /* Don't go further and continue play */
  return;
stop_player:
  //sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  delay(500);
  //exit(1);
}

/**
* @brief Setup audio player to play mp3 file
*
* Set clock mode to normal <br>
* Set output device to speaker <br>
* Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
* System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file)
* Open "Sound.mp3" file <br>
* Set master volume to -16.0 dB
*/
void setup()
{
  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  myFile = theSD.open("rainforest_ambience.mp3");

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %d\n",myFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");
  /* Main volume set to -16.0 dB */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);

}

/**
* @brief Play stream
*
* Send new frames to decode in a loop until file ends
*/
void loop()
{
  puts("loop!!");

  delay(5000);

  /* Send new frames to decode in a loop until file ends */
 // int err = theAudio->writeFrames(AudioClass::Player0, myFile);
  currentSound = (currentSound + 1) % 3;
  playSoundFile(getSoundFile(currentSound));
 // playSoundFile(getSoundFile(3));
  

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */

  usleep(40000);

}
 
 
