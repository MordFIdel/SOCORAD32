/*
 * Arduino Walkie Talkie 
 * v2.24
 * 07/19/2019
 * 
 * MIT License
 * Copyright (c) 2019 Jiulong Zhao (dragonbtv@gmail.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * Special thanks to:
 * Matthias Hertel's library "RotaryEncoder", GitHub page: https://github.com/mathertel/RotaryEncoder
 * Bill Greiman's library "SSD1306Ascii", GitHub page: https://github.com/greiman/SSD1306Ascii
 * James Sleeman's library "MCP41_Simple", GitHub page: https://github.com/sleemanj/MCP41_Simple
 * 
 * KM6WZM
 * Jiulong Zhao
 * dragonBTV
 * Los Angeles, California
 * dragonbtv@gmail.com
 * 
 * In order to make the hardware working properly, please make sure you are using the latest 
 * version of software. You can add your email address to the comment area or simply send me an email.
 */

#include <EEPROM.h>               //be sure to update it to the latest version
#include <Wire.h>                 //for IIC controled OLED with SH1106 chip, be sure to update it to the latest version
#include "SSD1306Ascii.h"         //for OLED with SH1106 chip
#include "SSD1306AsciiWire.h"     //for OLED with SH1106 chip
#include <SoftwareSerial.h>       //be sure to update it to the latest version
#include <MCP41_Simple.h>         //Digital Potentiometer chip MCP41010

SoftwareSerial rfSerial(9, 8);    //RX, TX//additional soft serial ports for RF module only
MCP41_Simple MyPot;               //MCP41010 chip and 11,12,13 pins are ocupried by the SPI interface also, do NOT re-use pin 12
#define I2C_ADDRESS 0x3C          //0x3C or 0x3D //address settings of OLED screen, check with your PCB board
SSD1306AsciiWire oled;            //I2C device, we are using SH1106 chip, NOT SD1306 chipset, buy OLED module with SH1106 chip

struct dataCH {         //all our data of each channel in EEPROM
  unsigned long fRX;    //receiving fregency, long int version 446.500 = 44650000, total 8 digits
  unsigned long fTX;    //transmiting frequency, long int version 446.500 = 44650000, total 8 digits
  unsigned int CTCSS;   //CTCSS * 10 into max 4 digis int, e.g. 102.7Hz * 10 = 1027; 97.4Hz * 10 = 974  
  byte powerOut;        //the output power level, 0 = high; 1 = low, 2 = 3 = middle
  char nameCH[12];      //name of the channel, max 12 chars, as the char[12] = '\0', so you only have 11 chars
};

struct allSettings {    //all our settings in EEPROM
  byte busyWN;          //0 = unlock,wide; 1 = lock,wide; 2 = unlock,narrow; 3 = lock,narrow ("lock" means block transmiting if there is a signal)
  byte audioLevel;      //1-8; default = 8
  byte vox;             //0-8; 0 = disable
  byte SQL;             //squelch level, 0-9, 0 = monitor
  byte autoOFF;         //0-15, auto cut transmiting after minutes, 0 = disable
  byte micBoost;        //0-7, mic gain setting
  byte scramble;        //0-7, sound scramble, 7 kinds, 0 = disable
  bool autoIdle;        //boolen, automatic idle for power saving
  bool compress;        //boolen, audio preemphasis
  char callSign[12];    //callsign, max 12 chars
};

bool updown = true;               //the direction of scan channel jumping
bool CHLock = false;              //the channel lock mode, long press back button to enter
bool acceptSMS = false;           //we accept SMS, default false
bool BTlock = false;              //the power of BlueTooth, true = bluetooth on
bool SMSnow = false;              //we've got SMS and it is displaying on screen
bool RCTCSS = false;              //we do NOT use receiving CTCSS by default, active it if you need
bool modeLock = false;            //another kind of button debounce
bool backLock = false;            //another kind of button debounce
bool squelching = false;          //when true, SQL=0, we are monitoring the channel
bool transmiting = false;         //true = TX; false = RX now
bool scanning = false;            //scanning right now
bool scanLock = false;            //the lock of the toggled scan button (the center press button on your rotary senser)
byte oldSQL = 1;                  //remember the last SQ setting before squelch key pressed
byte currentMode = 1;             //cycle switching: 1 = normal receiving; 2 = USB/bluetooth setting; 3 = short message; 4 = direct control
byte volume = 128;                //the audio volume (0-255), set to middle at beginning
unsigned long debounce = 0;       //debounce button use
unsigned int batwait = 0;         //we only check battery voltage once every 1000 loop calls on volume changing mode
const byte maxCH = 40;            //total channels, safe for 40 in total, channel 0 was reserved for further use (direct remote)
const byte sizeCH = 24;           //24 bytes will be used for each channel in EEPROM
const byte settingAddress = 247;  //address = 992, save all other user settings, must*4 at all time
const byte lastVolume = 255;      //address = 1020, save the last audio volume setting, must*4 at all time
const byte lastCH = 256;          //address = 1024, position to save the last channel, must*4 at all time
const long bounce = 100;          //100 ms for debounce
const long scanTimer = 5000;      //listen 5s before jumping to the next single channel

//for rotary encoder use
const byte plate[] = {0, -1,  1,  0, 1,  0,  0, -1, -1,  0,  0,  1, 0,  1, -1,  0};
byte oldS = 3;
byte p1 = 0;         
byte p2 = 0;
byte nowP = 0;

//const byte modePin = 6;       //input, switching between modes
//const byte squelchPin = 15;   //input, this is actually A1, but we are to use it for squelch button, SQL = 0

/*
//these are all the pins, after PCB released, all pins are fixed, it is unecessary keep using name, changed to unmbers
//const byte pinA = 3;          //input, digital pin 3, (pinB,pinA) clockwise turning as increase. swap them if you prefer counter clockwise
//const byte pinB = 2;          //input, digital pin 2, (pinB,pinA) both pins are pulled up, no need for resistors
//const byte scanPin = 4;       //input, center button of rotary sensor, pulled up HIGH to switch to/from scan mode, active LOW 
//const byte PTTPin = 5;        //input, used to switch TX/RX, connect to RF module PTT pin & PTT button, active LOW
//const byte modePin = 6;       //input, switching between modes
//const byte backPin = 7;       //input, pin to go back from menu & jump back to normal receiving mode
//8 = TX; 9 = RX                //used by rfSerial for communication with RF module 
//const uint8_t  CS_PIN = 10;   //CS enable pin for the Digital Potentiometer chip MCP41010, can be changed to anyone except pin 12; 
//11,12,13 SPI pins             //occupied by the SPI interface although 12 was NOT physically used, cannot make use of it
//const byte beepPin = 14;      //beepPin; OUTPUT, this is is actually A0 but we are to use it for the beeper as pin 13 was used by SPI in this case
//const byte squelchPin = 15;   //input, this is actually A1, but we are to use it for squelch button, SQL = 0
//const byte signalPin = 16;    //input, this is actually A2, detecting signal received from RF module, active LOW
//const byte btpowerPin = 17;   //OUTPUT,this is actually A3, use it to power switch bluetooth module with an PNP transistor, active LOW 
//A4 =18=SDA; A5 =19=SCL        //used by the IIC OLED display
//A6;                           //the only analog pins left 
//const byte voltagePin = A7;   //used on monitoring the voltage of the battery
*/

//********** this subfunction will only be used to fill your EMPTY EEPROM, use it once for each new Arduino board
//********** your board should be re-programmed with this subfunction blocked
void testWriteSettings(){  //to push in demo settings for test 
  allSettings pushIn;  
  pushIn.busyWN = 0;                  //0 = unlock,wide; 1 = lock,wide; 2 = unlock,narrow; 3 = lock,narrow 
  pushIn.audioLevel = 8;              //1-9; default = 8
  pushIn.vox = 0;                     //0-8; 0 = disable
  pushIn.SQL = 2;                     //squelch level, 0-9, 0 = monitor
  pushIn.autoOFF = 0;                 //0-15, auto cut transmiting after minutes, 0 = disable
  pushIn.micBoost = 7;                //0-7, maic gain setting
  pushIn.scramble = 0;                //0-7, sound scramble, 7 kinds, 0 = disable 
  pushIn.autoIdle = false;            //boolen, automatic idle for power saving
  pushIn.compress = false;            //boolen, audio preemphasis
  strcpy(pushIn.callSign, " KM6WZM");  //change it to your call sign 
  EEPROM.put(settingAddress*4, pushIn);
  delay(10);
}

//********** this subfunction will only be used to fill your EMPTY EEPROM, use it once for each new Arduino board
//********** your board should be re-programmed with this subfunction blocked
void testWriteChannels() {  //debug use on pushing data of demo channels
  writeChannel(0,51997500,13000000,255,0,"Control");
  writeChannel(1,44970000,44470000,1318,0,"N6CIZ");
  writeChannel(2,22428000,22268000,1072,0,"St.Anita RG");
  writeChannel(3,44548000,44048000,1318,0,"Arcadia");
  writeChannel(4,44550000,44050000,854,0,"N6EX_local");
  writeChannel(5,44556000,44056000,1000,0,"W6KAT");
  writeChannel(6,44564000,44064000,1567,0,"Altadena");
  writeChannel(7,14667000,14607000,1928,0,"Mt.Lukens");
  writeChannel(8,22492000,22332000,948,0,"Glendale");
  writeChannel(9,44044500,44044500,1230,0,"SMS_test");
  writeChannel(10,44592000,44092000,1862,0,"Latino");
  writeChannel(11,44680000,44180000,1318,0,"San Dimas");
  writeChannel(12,47053750,47053750,255,1,"LAPD_rsq");
  writeChannel(13,47151250,47151250,255,1,"LAPD");
  writeChannel(14,47239375,47239375,255,1,"LAPD");
  writeChannel(15,48298750,48298750,255,1,"LAPD.tempo");
  writeChannel(16,45072500,45072500,255,1,"KFI640");
  writeChannel(17,45070000,45070000,255,1,"KNX1070");
  writeChannel(18,14608500,14668500,1109,0,"Duarte_6AMR");
  writeChannel(19,48341250,48341250,255,1,"LAPD_disp");
  writeChannel(20,45023000,45023000,255,1,"ABC-7");
  writeChannel(21,50775000,50775000,255,0,"LAPD");
  writeChannel(22,44646000,44146000,1000,0,"Mt.Oat");
  writeChannel(23,44514000,44014000,1273,0,"Papa08");
  writeChannel(24,44834000,44334000,1148,0,"K6OES");
  writeChannel(25,47133125,47133125,255,1,"LAPD.rescue");
  writeChannel(26,44676000,44176000,1273,0,"PAPA03");
  writeChannel(27,44758000,44258000,1000,0,"K6JSI");
  writeChannel(28,22408000,22248000,1567,0,"Pasadena");
  writeChannel(29,22384000,22224000,1514,0,"Covina");
  writeChannel(30,44806000,44306000,1000,0,"Lake Forest");
  writeChannel(31,44988000,44488000,1462,0,"Sunset RG");
  writeChannel(32,44640000,44140000,1035,0,"W6AFCH");
  writeChannel(33,22494000,22334000,948,0,"Mt.Wilson");
  writeChannel(34,16255000,16255000,255,1,"NOAA_weathe");
  writeChannel(35,47239375,47239375,255,1,"LAFD_local");
  writeChannel(36,44610000,44110000,1000,0,"NCLXX");
  writeChannel(37,44624000,44124000,1000,0,"K6VGP_rept");
  writeChannel(38,45028750,45028750,255,1,"TV_reporter");
  writeChannel(39,14682000,14622000,255,0,"Johnstone");
  writeChannel(40,44842000,44342000,854,0,"W6ATN");
  delay(10);
  Serial.println(F("done EEPROM writting, please block this line of code and re-compiling again, do NOT reset your Arduino"));
}

void beep(bool bp) { //you can mute the beep totally from here. by default, we need beep sound on key pressing except PTT
  if (bp == true) {
    digitalWrite(14, HIGH); //start the beep
  }else{
    digitalWrite(14, LOW); //stop the beep
  }
}

void writeChannel(byte nCH, unsigned long r, unsigned long t, unsigned int ctc, byte pp, char nm[12]){  //push channel data into EEPROM
  beep(true);
  dataCH tempCH;
  //read back channel data from EEPROM first as some input maybe NULL //0，0，4444，9，NULL
  EEPROM.get(nCH*sizeCH, tempCH);
  delay(5);
  if (r != 0){ tempCH.fRX = r; }
  if (t != 0){ tempCH.fTX = t; }
  if (ctc != 4444){ tempCH.CTCSS = ctc; }
  if (pp != 9){ tempCH.powerOut = pp; }
  if (nm != NULL){ 
    strcpy(tempCH.nameCH, nm);  
  }
  //Serial.println(sizeof(tempCH)); //monitor the real size of each channel
  EEPROM.put(nCH*sizeCH, tempCH);
  delay(10);
  beep(false); //stop the beep
}

String ctcssToHex(unsigned int decValue, int section){  //use to convert the CTCSS reading which RF module needed
  if(decValue == 255){
    return hexToString("FF");
  }
  String d1d0 = String(decValue);
  if (decValue < 1000){
    d1d0 = "0" + d1d0;
  }
  if (section == 1) {
    return hexToString(d1d0.substring(2,4));
  }else{
    return hexToString(d1d0.substring(0,2));
  }

  //other method on getting HEX string to int
  //String tt = "0x"+ d1d0.substring(2,4);
  //char test[5];
  //tt.toCharArray(test, 5); 
  //(int)strtol(test, NULL, 0));
}

String hexToString(String hex) {  //for String to HEX conversion
  String text;    
  for(int k=0;k< hex.length();k++) {
    if(k%2!=0) {
      char temp[3];
      sprintf(temp,"%c%c",hex[k-1],hex[k]);
      int number = (int)strtol(temp, NULL, 16);
      text+=char(number);
    }
  }  
  return text;
}

//frequency range
//130 - 520MHz//actually 590MHz
bool testFreq(long ff) {  //refusing typo 
  //if ((ff < 40000000)||(ff > 47000000)) { return false; } //guaranteed transmitting frequency range
  if ((ff < 13000000)||(ff >= 52000000)) { return false; }  //actual receiving range, transmitting on the risk of your own
  if ((ff % 250 != 0)&&(ff % 625 != 0)) { return false; }   //this is a must, only tuned to 5.0k and 6.25k steps
  return true;
}

void receivingSerialData() {  //we are receiving user settings from serial port on SETTING MODE
  boolean newData = false;
  char rc;
  const byte alowChars = 24;  //default should be 24 actually we are using less then 20
  char lineChars[alowChars];
  static byte ndx = 0;

  //receiving serial data
  while ((Serial.available() > 0)&&(newData == false)) { 
    rc = Serial.read();
    if (rc != '\n') {
      lineChars[ndx] = rc;
      ndx++;
      if (ndx >= alowChars) {
         ndx = alowChars - 1; //jump the long string and wait for theline break '\n'
      }
    }else{
      lineChars[ndx] = '\0'; //terminate the string
      ndx = 0;
      //Serial.println(lineChars);  //one line receiveded, let's display it
      newData = true;
    }
  }
  
  //processing received data
  if (newData == true) {
    allSettings newSettings;
    dataCH newCH;
    char * strtokIndx;  //init them here you need NOT to zero them again
    int gotIt;
    strtokIndx = strtok(lineChars,"="); //get the first part - the command
    //char com = strtokIndx[strlen(strtokIndx) - 1] + '\0';  //if you are prefer to switch char 'A'. 'B', 'C'...
    int com = strtokIndx[strlen(strtokIndx) - 1];

    switch (com) { //A=65, B=66, C=67, Y=89, K=75, V=86, R=82, N=78, L=76, X=88, H=72, F=70, S=83
      case 65://A:
        //using [A=channel_number,long_int_receing_frequency,power_level(one char among H M L)] 
        //e.g."A=40,4500000,1" to save a receving frequency to EEPROM
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if (gotIt <= maxCH) {
          beep(true); //start the beep
          strtokIndx = strtok(NULL, ",");
          long RF = String(strtokIndx).toInt(); //char array -> string -> integer instead of atoi() for long int
          if (testFreq(RF)) {
            newCH.fRX = RF;
            oled.print(F("> CH = "));
            oled.println(gotIt);
            oled.print(F("> RF = "));
            oled.println(newCH.fRX);
            oled.print(F("> power out = "));
            strtokIndx = strtok(NULL, ",");
            newCH.powerOut = atoi(strtokIndx);
            oled.println(newCH.powerOut);
            //modify the channel on EEPROM
            writeChannel(gotIt, newCH.fRX, 0, 4444, newCH.powerOut, NULL); //write to the EEPROM
          }else{
            oled.println(F("> RF frequency error"));
            oled.println(F("> use integer please"));
            oled.println(F("> 6.25/5.0 divisible"));
            oled.println(F("> e.g. 44650000"));
          } 
        }
        oled.println(F(">"));
        beep(false); //stop the beep
        break;

      case 66://B:
        //using [B=channel_number,long_int_trasmiting_frequency,int_version_CTCSS] e.g."B=38,45000000,2503" to save tranmiting frequency to EEPROM
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if (gotIt <= maxCH) {
          beep(true); //start the beep
          strtokIndx = strtok(NULL, ",");
          long TF = String(strtokIndx).toInt();
          if (testFreq(TF) == true) {
            newCH.fTX = TF;
            oled.print(F("> CH = "));
            oled.println(gotIt);
            oled.print(F("> TF = "));
            oled.println(newCH.fTX);
            oled.print(F("> CTCSS = "));
            //CTCSS
            strtokIndx = strtok(NULL, ",");
            newCH.CTCSS = atoi(strtokIndx);
            oled.println(newCH.CTCSS);
            oled.println(F("> (NO CTCSS = 255)"));
            //modify the channel on EEPROM
            writeChannel(gotIt, 0, newCH.fTX, newCH.CTCSS, 9, NULL); //write to the EEPROM
          }else{
            oled.println(F("> TF frequency error"));
            oled.println(F("> use integer please"));
            oled.println(F("> 6.25/5.0 divisible"));
            oled.println(F("> e.g. 44650000"));
          }
        }  
        oled.println(F(">"));
        beep(false); //stop the beep        
        break;

      case 67://C:
        //using [C=channel_number,max12_charectors_channel_name,byte_busyWN(0,1,2,3)] e.g."C=38,KM6WZM,3" to save channel name to EEPROM
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if (gotIt <= maxCH) {
          beep(true); //start the beep
          oled.print(F("> CH = "));
          oled.println(gotIt);
          oled.print(F("> Name = "));
          strtokIndx = strtok(NULL, ",");
          if (strlen(strtokIndx) > 11){  //make sure the name is 12 charectors as the max
              strtokIndx[11] = '\0';
          }   
          strcpy(newCH.nameCH, strtokIndx); //copy the char to
          oled.println(newCH.nameCH);
          //modify the channel on EEPROM
          writeChannel(gotIt, 0, 0, 4444, 9, newCH.nameCH); //write to the EEPROM
        }else{
          oled.println(F("> CH number error")); 
        }
        oled.println(F(">"));
        beep(false); //stop the beep
        break;

      case 89: //Y-saving call sign or welcome screen 
        //[Y=my_call_sign_or welcome_display_12char_max] e.g. "Y=KM6WZM"
        beep(true); //start the beep
        strtokIndx = strtok(NULL, ",");
        if (strlen(strtokIndx) > 11){  //make sure the name is 12 charectors as the max
          strtokIndx[11] = '\0';
        }  
        //readback all settings from EEPROM first
        EEPROM.get(settingAddress*4, newSettings);  
        strcpy(newSettings.callSign, strtokIndx); //copy the char
        oled.print(F("> CS = "));
        oled.println(newSettings.callSign);
        oled.println(F(">"));
        //write back to EEPROM
        EEPROM.put(settingAddress*4, newSettings);
        delay(10);
        beep(false); //stop the beep
        break;

      case 75:  //K://the busy lock setting and the band width setting //[K=byte_busyWN(0,1,2,3)] e.g. "K=0"
        //0 = unlock,wide; 1 = lock,wide; 2 = unlock,narrow; 3 = lock,narrow 
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 3)){ 
          beep(true); //start the beep
          //readback all settings from EEPROM first
          EEPROM.get(settingAddress*4, newSettings);  
          newSettings.busyWN = gotIt;
          oled.print(F("> busyWN = "));
          oled.println(gotIt);
          //write back to EEPROM
          EEPROM.put(settingAddress*4, newSettings);
          delay(10);
          beep(false); //stop the beep
        }else{
          oled.println(F("> input error (0-3)")); 
        }
        oled.println(F(">"));
        break;

      case 86:  //V://"DMOSAV":  //this is the automatic idle setting,default 0 means ON for power saving, we'd better set it to 1
        beep(true); //start the beep
        EEPROM.get(settingAddress*4, newSettings);  //readback all settings from EEPROM first

        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if (gotIt == 0) { 
          newSettings.autoIdle = false; 
        }else{ 
          newSettings.autoIdle = true; 
        } 
        oled.print(F("> autoIdle (V) = "));
        oled.println(gotIt);
        oled.println(F(">"));
        //write back to EEPROM
        EEPROM.put(settingAddress*4, newSettings);
        delay(10);
        rfSerial.print(F("AT+DMOSAV="));
        rfSerial.print(gotIt);
        rfSerial.println(F("\r\n"));
        beep(false); //stop the beep
        break;
      
      case 82:  //R://"DMOVER":  //doing nothing in this version
          //get version report of the RF module from terminal
          rfSerial.print(F("AT+DMOVER\r\n"));
          oled.println(F("> Sent to Terminal "));
          oled.println(F(">"));
          break;
        
      case 78:  //N://"DMOFUN":  
        beep(true); //start the beep
        //grab and save those 5 settings together: 
        //SQL(0-9,0=off),mic gain(0-7),auto OFF(0-15,0=off),sound scramble(0-7, 0=off),audio preemphasis(bool)
        EEPROM.get(settingAddress*4, newSettings);  //readback all settings from EEPROM first

        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 9)) {
          newSettings.SQL = gotIt;
          oled.print(F("> SQL = "));
          oled.println(newSettings.SQL);
        }else{
          oled.println(F("> SQL error (0-9)")); 
        }
          
        //0-7, mic gain setting
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 7)) {
          newSettings.micBoost = gotIt;
          oled.print(F("> micBoost = "));
          oled.println(newSettings.micBoost);
        }else{
          oled.println(F("> micBoost error 0-7")); 
        }
          
        //0-15, auto cut transmiting after minutes, 0 = disable
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 15)) {
          newSettings.autoOFF = gotIt;
          oled.print(F("> autoOFF = "));
          oled.println(newSettings.autoOFF);
        }else{
          oled.println(F("> autoOFF error 0-15")); 
        }
          
        //0-7, sound scramble, 7 kinds, 0 = disable
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 7)) {
          newSettings.scramble = gotIt;
          oled.print(F("> scramble = "));
          oled.println(newSettings.scramble);
        }else{
          oled.println(F("> scramble error 0-7")); 
        }
          
        //boolen, audio preemphasis
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if (gotIt == 0) { 
          newSettings.compress = false; 
        }else{ 
          newSettings.compress = true; 
        } 
        oled.print(F("> compress = "));
        oled.println(newSettings.compress);
        oled.println(F(">"));
        
        //write back to EEPROM
        EEPROM.put(settingAddress*4, newSettings);
        delay(10);

        //push in RF module instantly
        rfSerial.print(F("AT+DMOFUN="));
        rfSerial.print(String(newSettings.SQL));
        rfSerial.print(F(","));
        rfSerial.print(String(newSettings.micBoost));
        rfSerial.print(F(","));
        rfSerial.print(String(newSettings.autoOFF));
        rfSerial.print(F(","));
        rfSerial.print(String(newSettings.scramble));
        rfSerial.print(F(","));
        rfSerial.print(String(newSettings.compress));
        rfSerial.println(F("\r\n"));
        beep(false); //stop the beep
        break;
          
      case 76:  //L://"DMOVOL":  //sound output level, 1-9; default = 8
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if((0 < gotIt)&&(gotIt <= 9)) {
          beep(true); //start the beep
          EEPROM.get(settingAddress*4, newSettings);  //readback all settings from EEPROM first
          delay(5);
          newSettings.audioLevel = gotIt;
          oled.print(F("> audioLevel (L) = "));
          oled.println(newSettings.audioLevel);
          //write back to EEPROM
          EEPROM.put(settingAddress*4, newSettings);
          delay(10);
          //push in RF module instantly //set volume:audioLevel;    //1-8; default = 8
          rfSerial.print(F("AT+DMOVOL="));
          rfSerial.print(gotIt);
          rfSerial.println(F("\r\n"));
          beep(false); //stop the beep
        }else{
          oled.println(F("> audioLevel error"));
        }
        oled.println(F(">"));
        break;
  
      case 88:  //X://"DMOVOX":  //VOX, 0-8; 0 = disable
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);
        if ((0 <= gotIt)&&(gotIt <= 8)) {
          beep(true); //start the beep
          EEPROM.get(settingAddress*4, newSettings);  //readback all settings from EEPROM first
          newSettings.vox = gotIt;
          oled.print(F("> vox (X) = "));
          oled.println(newSettings.vox);
          //write back to EEPROM
          EEPROM.put(settingAddress*4, newSettings);
          delay(10);

          //push in RF module instantly //set vox:vox; //0-8; 0 = disable
          rfSerial.print(F("AT+DMOVOX="));
          rfSerial.print(gotIt);
          rfSerial.println(F("\r\n"));
          beep(false); //stop the beep
        }else{
          oled.println(F("> VOX error (0-8)")); 
        }
        oled.println(F(">"));
        break;

      case 72:  //H: //change the volume instantly
        beep(true); //start the beep
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);          
        if ((gotIt <= 255)&&(gotIt >= 0)){
          volume = gotIt;
          MyPot.setWiper(volume);
          EEPROM.write(lastVolume *4, volume);  //write the volume to EEPROM
          delay(10);
          oled.print(F("> Volume (H) = "));
          oled.println(gotIt);
        }else{
          oled.println(F("> H error (8-246)")); 
        }
        oled.println(F(">"));
        beep(false); //stop the beep
        break;

      case 70:  //F: //jumping to the channel directly instantly         
        beep(true); //start the beep
        strtokIndx = strtok(NULL, ",");
        gotIt = atoi(strtokIndx);          
        if ((gotIt <= 40)&&(gotIt > 0)){          
          //change the rotary sensor to your setting channel
          nowP = gotIt;
          setPosition(nowP);
          
          //force jumping to the channel
          loadCurrent();

          //remember the changed channel
          if (currentMode == 2) { //be sure we are in setting mode
            EEPROM.write(lastCH *4, nowP);  
            delay(10);
          }

          oled.print(F("> Channel (F) = "));
          oled.println(gotIt);
        }else{
          oled.println(F("> F error (1-40)")); 
        }
        oled.println(F(">"));
        beep(false); //stop the beep
        break;
        
      case 83:  //S: //"DMOMES":  //sending text message, max 60 charectors each time
        beep(true); //start the beep
        strtokIndx = strtok(NULL, ",");
        byte mesLength = strlen(strtokIndx);
          
        //push in RF module instantly 
        rfSerial.print(F("AT+DMOMES\r\n"));
        rfSerial.print(F("AT+DMOMES="));
        rfSerial.print(mesLength, HEX);
        rfSerial.print(strtokIndx);
        rfSerial.println(F("\r\n"));
        delay(5);   

        oled.print(F("> [DEC="));
        oled.print(mesLength);
        oled.print(F("; HEX="));
        oled.print(mesLength, HEX);
        oled.println(F("]"));
        oled.print(F("> "));
        oled.println(strtokIndx); 
        oled.println(F("> please use SMS mode")); 
        oled.println(F("> don't send S= here"));
        oled.println(F(">"));
          
        beep(false); //stop the beep
        break;
    }
    //new data was processed 
    newData = false;
  }
}

void sendingSMS() {  //we are receiving user SMS from serial port on SMS MODE
  boolean newData = false;
  char rc;
  const byte alowChars = 108;  //6 lines in total
  char lineChars[alowChars];
  static byte ndx = 0;

  //receiving serial data
  while ((Serial.available() > 0)&&(newData == false)) { 
    rc = Serial.read();
    if (rc != '\n') {
      lineChars[ndx] = rc;
      ndx++;
      if (ndx >= alowChars) {
         ndx = alowChars - 1; //jump the long string and wait for theline break '\n'
      }
    }else{
      lineChars[ndx] = '\0'; //terminate the string, we have to
      ndx = 0;
      //debug use
      //Serial.println(lineChars);  //one line receiveded, let's display it
      newData = true;
    }
  }
  
  //processing received data
  if (newData == true) {
    beep(true); //start the beep

    byte mesLength = strlen(lineChars) - 1;
    byte tt = int(mesLength / 18);
    byte lt = mesLength % 18;

    //prepare the oled
    oled.println(F("SMS sent:"));

    //deal with these magazines
    if (tt >= 1) {
      for (byte h = 0; h < tt; h++) {
        beep(false); //stop the beep
        rfSerial.print(F("AT+DMOMES="));
        rfSerial.write(0x12);
        for (byte i = 0; i < 18; i++) {
          rfSerial.write(lineChars[h*18 + i]);
          oled.print(lineChars[h*18 + i]);
        }
        rfSerial.write('\r');
        rfSerial.write('\n'); 
        rfSerial.flush();
        oled.println();

        unsigned long waitTime = millis();
        while(millis() - waitTime < 2000) { rfSerial.flush(); };
        beep(true); //start the beep
      }
    }

    //finish the last shot
    if (lt != 0) {
      rfSerial.print(F("AT+DMOMES="));
      rfSerial.write(0x12);
      for (byte i = 0; i < lt; i++) {
        rfSerial.write(lineChars[tt*18 + i]);
        oled.print(lineChars[tt*18 + i]);
      }
      for (byte i = 0; i < 18 - lt; i++) {
        rfSerial.write(0x20); //the blank space
      }
      rfSerial.write('\r');
      rfSerial.write('\n'); 
      rfSerial.flush();
      oled.println();
    }
    
    beep(false); //stop the beep
    newData = false; //new data was processed    
  }
}

void loadCurrent() { //switching channels BUT do not change display  
  dataCH outputCH; //read back all data of the current channel
  EEPROM.get(nowP*sizeCH, outputCH);  //read back channel data from EEPROM directly 
  delay(5);

  //prepare those strings
  String tempS = String(outputCH.fRX);
  String fR = tempS.substring(0, 3) + "." + tempS.substring(3,8);
  tempS = String(outputCH.fTX);
  String fT = tempS.substring(0, 3) + "." + tempS.substring(3,8);

  //making and send the AT command to RF module
  rfSerial.print(F("AT+DMOGRP="));
  rfSerial.print(fR);
  rfSerial.print(F(","));
  rfSerial.print(fT);
  rfSerial.print(F(","));
  if (RCTCSS == true) {
    if (outputCH.CTCSS == 255) {
      rfSerial.write(255);
      rfSerial.write(255);
    }else{
      rfSerial.print(ctcssToHex(outputCH.CTCSS, 1));
      rfSerial.print(ctcssToHex(outputCH.CTCSS, 2));
    }
  }else{
    rfSerial.write(255);
    rfSerial.write(255);
  }
  rfSerial.print(F(","));
  if (outputCH.CTCSS == 255) {
    rfSerial.write(255);
    rfSerial.write(255);
  }else{
    rfSerial.print(ctcssToHex(outputCH.CTCSS, 1));
    rfSerial.print(ctcssToHex(outputCH.CTCSS, 2));
  }
  rfSerial.print(F(",0,"));
  rfSerial.print(outputCH.powerOut);
  rfSerial.println(F("\r\n"));
}

void displayCurrent(){  //switching channels
  allSettings mySetting;
  EEPROM.get(settingAddress*4, mySetting);  //readback all settings from EEPROM for use
  delay(5);
  
  dataCH outputCH; //read back all data of the current channel
  EEPROM.get(nowP*sizeCH, outputCH);  //read back channel data from EEPROM directly 
  delay(5);

  //prepare those strings
  String tempS = String(outputCH.fRX);
  String fR = tempS.substring(0, 3) + "." + tempS.substring(3,8);
  tempS = String(outputCH.fTX);
  String fT = tempS.substring(0, 3) + "." + tempS.substring(3,8);

  //String utilityS = F("SQ3 VX0 WB"); 
  byte band = mySetting.busyWN;
  if ((band >3)||(band <0)){ band = 0;}
  if (mySetting.SQL > 9) { mySetting.SQL = 2; };
  if (mySetting.vox > 8) { mySetting.vox = 0; };
  
  //making and send the AT command to RF module
  rfSerial.print(F("AT+DMOGRP="));
  rfSerial.print(fR);
  rfSerial.print(F(","));
  rfSerial.print(fT);
  rfSerial.print(F(","));
  if (RCTCSS == true) {
    if (outputCH.CTCSS == 255) {
      rfSerial.write(255);
      rfSerial.write(255);
    }else{
      rfSerial.print(ctcssToHex(outputCH.CTCSS, 1));
      rfSerial.print(ctcssToHex(outputCH.CTCSS, 2));
    }
  }else{
    rfSerial.write(255);
    rfSerial.write(255);
  }
  rfSerial.print(F(","));
  if (outputCH.CTCSS == 255) {
    rfSerial.write(255);
    rfSerial.write(255);
  }else{
    rfSerial.print(ctcssToHex(outputCH.CTCSS, 1));
    rfSerial.print(ctcssToHex(outputCH.CTCSS, 2));
  }
  rfSerial.print(F(","));
  rfSerial.print(String(band));
  rfSerial.print(F(","));
  rfSerial.print(outputCH.powerOut);
  rfSerial.println(F("\r\n"));
  
  //show current channel on OLED display
  //we have to clear the display just in case we were receiving SMS
  if (SMSnow == true) { oled.clear(); }else{ oled.home(); } 
  
  //these lines are for "CH:39          KM6WZM"  
  String fullName = String(outputCH.nameCH);
  fullName.trim();
  String firstBlock ="CH:" + String(nowP,DEC);
  oled.print(firstBlock);
  for(int i = firstBlock.length(); i<21-fullName.length();i++) {
    oled.print(F(" "));
  }
  oled.print(fullName);
  oled.println(F(" "));
  oled.println(F("---------------------")); 

  //these lines are for "TX:444.88250  PO:High"
  oled.print(F("TX:"));
  oled.print(fT);
  switch (outputCH.powerOut){
    case 0:
      oled.println(F("  PO:High"));
      break;
    case 1:
      oled.println(F("   PO:Low"));
      break;
    case 2:
      oled.println(F("   PO:Mid"));
      break;
    case 3:
      oled.println(F("   PO:Mid"));
      break;
    default:
      oled.println(F("  PO:High"));
      break;
  }

  //these for large receiving frequency
  oled.setCursor(24,4);
  oled.set2X();
  oled.print(fR.substring(0, 7));
  oled.set1X();
  oled.println(fR.substring(7,9));
  
  //these lines are for "SQ3 VX0 WB CTS:107.2 "));
  oled.setCursor(0,7);
  oled.print(F("SQ"));
  oled.print(mySetting.SQL);
  oled.print(F(" VX"));
  oled.print(mySetting.vox);

  if(band <= 1) {
    oled.print(F(" WB")); 
  }else{ 
    oled.print(F(" NB")); 
  }
  
  String ctcss = String(outputCH.CTCSS);

  if(outputCH.CTCSS == 255) {
      oled.println(F("   CTS:None"));
  }else{
    if (outputCH.CTCSS < 1000) {
      oled.print(F("   CTS:"));
      oled.print(ctcss.substring(0, 2));
      oled.print(F("."));
      oled.println(ctcss.substring(2));
    }else{
      oled.print(F("  CTS:"));
      oled.print(ctcss.substring(0, 3));
      oled.print(F("."));
      oled.println(ctcss.substring(3));
    }
  }
}

void setPosition(byte newP) {   //reloading saved rotary encoder position
  p1 = ((newP<<2) | (p1 & 0x03L));
  p2 = newP;
} 

void positionCheck() {    //switching position of rotary encoder 
  byte nowS = digitalRead(3) | (digitalRead(2) << 1); //pinA=3; pinB=2;
  if (oldS != nowS) 
  {
    p1 += plate[nowS | (oldS<<2)];
    if (nowS == 3) {p2 = p1 >> 2;}
    oldS = nowS;
  } 
  byte currentP = p2;
  if (currentP != nowP)
  {
    beep(true); //start the beep
    //find the direction up or down
    bool goingUP; 
    if (currentP > nowP) { goingUP = true; }else{ goingUP = false; }  

    //limit the range of the rotary encoder
    if (currentP > maxCH) {setPosition(1);}
    if (currentP < 1) {setPosition(maxCH);}
    
    //got the current position
    nowP = p2;
    
    if (modeLock == true) {  //we are to deal with the volume changing now
      oled.set2X();
      oled.home(); 
      oled.setCursor(0,5);
      oled.println(F("            "));
      oled.setCursor(0,5);
      if (goingUP) {
        if (volume >= 246) {volume = 246;}
        volume = volume + 8;
        MyPot.setWiper(volume);
        oled.print(F("  >>> "));
        oled.println(volume/8);        
      }else{
        if (volume <= 8) {volume = 8;}
        volume = volume - 8;
        MyPot.setWiper(volume);
        oled.print(F("  "));
        oled.print(volume/8);
        oled.println(F(" <<< "));
      }
    }else if (currentMode == 1) displayCurrent();   
    beep(false); //stop the beep
  }else if(modeLock == true) batteryMonitor();
  //we are to check battery votage only in idle time and the battery votage monitor function only aviliable during volume adjusting mode
} 

//====================================================================================================== main start
void setup()
{
  Serial.begin(9600);
  
  //only run ONCE at the very first time when you are programming your fresh Arduino board!  
  //testWriteSettings();  //push in default settings into EEPROM
  //testWriteChannels();  //push in 40 demo channels into EEPROM + control channel 0
  
  pinMode(14, OUTPUT); //beepPin; be sure to init the beep PIN as we are dealing with an analogo pin
  beep(true); //start the beep
  
  //#if INCLUDE_SCROLLING == 0 //make sure to modify h file for text scrolling display
  //#error INCLUDE_SCROLLING must be non-zero.  Edit SSD1306Ascii.h
  //#endif 

  allSettings saved;
  EEPROM.get(settingAddress*4, saved);  //readback my call sign or welcome screen from EEPROM
  delay(5);
  
  //start OLED screen
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&SH1106_128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.setContrast(255);  
  //oled.displayRemap(true); //flip the screen //not used on current PCB version v2.12 in case you need it
  oled.setScrollMode(SCROLL_MODE_OFF);  //preset to no scroll mode
  oled.clear();
  oled.println(F("Arduino Walkie Talkie"));
  oled.println(F("---------------------")); 
  oled.setCursor(0,3);
  oled.set2X();
  oled.print(F(" ")); 
  oled.println(String(saved.callSign));
  oled.set1X();
  oled.setCursor(0,7);
  //oled.println(F("------------- Welcome"));
  oled.println(F("--------------- v2.23"));
  
  //read back last CH from EEPROM
  nowP = EEPROM.read(lastCH *4);
  delay(5);
  if ((nowP > maxCH) || (nowP < 1)){
    nowP = 1;
  }
  setPosition(nowP); //read back last position and set to rotary encoder

  //stop the first beep
  beep(false); //stop the beep
  
  //start the potentiometer
  MyPot.begin(10); //CS_PIN
  //read back wiper position from EEPROM
  volume = EEPROM.read(lastVolume * 4);
  if ((volume > 254) || (volume < 0)){
    volume = 128;
  }
  MyPot.setWiper(volume);
  
  //start soft serial
  rfSerial.begin(9600); 
  
  delay (500); //just for welcome screen display
  oled.clear();

  //active all pins and buttons
  pinMode(3, INPUT_PULLUP);           //pinA; set pinA as an input, pulled HIGH, rotary encoder
  pinMode(2, INPUT_PULLUP);           //pinB; set pinB as an input, pulled HIGH, rotary encoder
  pinMode(7, INPUT_PULLUP);           //backPin; set as an input, pulled HIGH, jump back to normal mode and back from menu 
  pinMode(6, INPUT_PULLUP);           //modePin; cycle switching mode, pulled HIGH
  pinMode(4, INPUT_PULLUP);           //scanPin; in/out of scan mode
  pinMode(16, INPUT_PULLUP);          //signalPin; detecting signal received from RF module, pulled HIGH, active LOW
  pinMode(15, INPUT_PULLUP);          //squelchPin; setup SQ switching for instant squelch control to the rf module
  pinMode(5, INPUT_PULLUP);           //PTTPin; PTT button for transfer, pulled HIGH
  pinMode(17, OUTPUT);                //btpowerPin; we have to init it as it is an analog pin
  digitalWrite(17, LOW);              //btpowerPin; active HIGH, power switch for bluetooth module, drive a 3.3v regulator now

  //launch the current channel
  displayCurrent(); 
  
  //force RF module to output the default highest audio at anytime
  rfSerial.print(F("AT+DMOVOL=8\r\n"));

  //force RF module to NOT power saving mode (for SMS mode)
  rfSerial.print(F("AT+DMOSAV=1\r\n"));

  //make sure we are using 18 charector per frame on SMS
  rfSerial.print(F("AT+DMOMES="));
  rfSerial.write(0x12);
  rfSerial.write('\r');
  rfSerial.write('\n');
}

//================================================================================================== main loop

void loop() {
  
  //you can NOT entering other modes when on scannning mode,so does the channel lock state
  if ((scanning == false)&&(CHLock == false)&&(transmiting == false)&&(squelching == false)) {
    modeButton();  //deal with mode button
  }
  
  //PTT and scan button detect, only response on normal receiving mode
  if (currentMode == 1) { 
    if ((CHLock == false)&&(transmiting == false)&&(squelching == false)) scanButton();   //scan button can NOT be pressed during channel lock
    if ((scanning == false)&&(squelching == false)&&(modeLock == false)&&(backLock == false)) PTTButton();  //don NOT accept PTT button during scanning mode
  
    //check and entering channel lock mode by long pressing back button
    if((digitalRead(7) == LOW)&&(backLock == true)&&(CHLock == false)&&(millis() - debounce > scanTimer / 2)) { 
      //back button pressed too long, 2.5s
      beep(true);
      CHLock = true;
      lockScreen();
      beep(false);
    }
  }

  //squelch button can be pressed at anytime and short click swithing on/off the Bluetooth
  if (transmiting == false) squelchButton(); //deal with SQ button

  //back button can be pressed any mode 
  if ((transmiting == false)&&(squelching == false)) backButton(); 
  
  //rotary encoder can be detected during normal receiving mode and volume mode
  if ((currentMode == 1)||(modeLock == true)) 
  {
    if ((scanning == false)&&(CHLock == false)&&(transmiting == false)&&(squelching == false)) { 
      positionCheck();  //check the status of the rotary encoder
      //we are handing rotary sensor in scanning mode seperatlly
    }
  }

  //if we accept SMS
  if (acceptSMS == true) {
    if (currentMode == 1) { 
      //only accept SMS in normal mode, you can add other modes
      //processingSMS();
    }
  }else{
    //forward all RF serial data to serial port at anytime except scanning mode
    if (scanning == false) {
      if (rfSerial.available()) {
        Serial.write(rfSerial.read()); //these are received SMS
      }
    }
  }

  //only accept serial control on setting mode 
  if (currentMode == 2) {  //we are in setting mode 
    receivingSerialData();  //receiving data from coming serials
  }

  //only accept serial message on short message mode
  if (currentMode == 3) {  //we are in SMS mode  
    sendingSMS();  //prepare SMS from coming serials
  }

  //transparent mode talk to RF module directly
  if (currentMode == 4) {
    if (rfSerial.available()) {
      Serial.write(rfSerial.read()); 
    }
    if (Serial.available()) {
      rfSerial.write(Serial.read()); 
    }
  }
}

//================================================================================================== end of main loop

void batteryMonitor() {
  batwait = batwait + 1; //only check once every 1000 calls
  if (batwait >= 1000) {
    batwait = 0;
    //check the voltage of the battery
    int sum = 0;
    float voltage = 0.0;
    for (int i = 0; i <= 10; i++) {
      sum += analogRead(A7); //voltagePin
      delay(5);
    }
    
    //how did we get these numbers:
    //please mesure vcc from your board on full battery, instead of 3.30v, use the read. e.g.3.27 here
    //if the divider resistors are 120k + 200k, the rate is 120+200/200 = 1.600
    //if the divider resisters are 100k + 220k, the rate is 100+220/220 = 1.455
    //actually we had masured the voltage for 11 time on the for loop, so sum should devide 11
    voltage = (sum / 11 * 3.27) / 1024.0 * 1.455;
      
    //displaying the voltage on OLED
    oled.set1X();
    oled.home(); 
    oled.setCursor(90,1);
    oled.print(voltage);
    oled.println(F("v"));
    oled.setCursor(0,5);
  }
}

void backButton() { 
  if((digitalRead(7) == LOW)&&(backLock == false)&&(millis() - debounce > bounce)) { //backPin
    backLock = true;
    beep(true); //start the beep
    debounce = millis();

    //try to push settings to EEPROM if user was in setting mode
    if (currentMode == 2) { //transfer all settings to RF module
      //transferRFsettings(); not necessary as we've save it during setting
    }

    //deal with the scan mode if it is right in scan mode for just in case (almost impossible)
    if (scanning == true) {
      scanning = false; //disable the scan  
    }

    //dealing SMS displaying and channel lock
    if (currentMode == 1) {
      if ((SMSnow == true)||(CHLock == true)) { 
        oled.setScrollMode(SCROLL_MODE_OFF);  //prepare OLED to NO scrolling mode
        oled.clear();
        oled.set1X();
        displayCurrent(); //re-load the last channel for OLED display
      }
    }else{      
      //we are backing normal mode    
      currentMode = 1;
      oled.setScrollMode(SCROLL_MODE_OFF);  //prepare OLED to NO scrolling mode
      oled.clear();
      oled.set1X();
      nowP = EEPROM.read(lastCH *4);  //read back last CH from EEPROM, all nowP changed related to channel setting were pushed to EEPROM
      if ((nowP > maxCH) || (nowP < 1)) { nowP = 1; }
      setPosition(nowP); //read back last position and set to rotary encoder
      displayCurrent(); //re-load the last channel for OLED display
    }
    
    //reset these state
    SMSnow = false;
    CHLock = false;
    
    beep(false); //stop the beep 
  }

  //releasing the toggle back button lock if it is locked 
  if ((backLock == true)&&(digitalRead(7) == HIGH)) { //backPin
      backLock = false;
  }  
}

void modeButton() { 
  unsigned int oldVolume;
  if((digitalRead(6) == LOW)&&(modeLock == false)&&(millis() - debounce > bounce)) { //modePin
    modeLock = true;
    beep(true); //start the beep
    debounce = millis();

    //we are to remember the current volume to see if we are to change mode or volume
    oldVolume = volume;

    //try to remember last channel
    if (currentMode == 1) { //write the current chaneal to EEPROM
      EEPROM.write(lastCH *4, nowP);  
      delay(10);
    }

    //showup the volume control
    volumeScreen();
    beep(false); //stop the beep 
  }

  //releasing the mode button lock if it is locked
  if ((modeLock == true)&&(digitalRead(6) == HIGH))   //modePin
  { 
    modeLock = false;
    if (volume == oldVolume) {
      //switching to the next mode, as we did not change volume we are to switch mode
      currentMode = currentMode + 1;
      if (currentMode > 4) currentMode = 1;
    }else{
      //we are coming for volume control. try to remember last volume write the last volume to EEPROM
      EEPROM.write(lastVolume * 4, volume); 
      delay(10);
    }

    //clear oled
    oled.clear();
    oled.set1X();

    switch (currentMode) {
      case 1:
        //we are backing normal mode     
        oled.setScrollMode(SCROLL_MODE_OFF);  //prepare OLED to NO scrolling mode
        nowP = EEPROM.read(lastCH *4);  //read back last CH from EEPROM, it seems nowP was NOT changed during ANY setting
        delay(5);
        if ((nowP > maxCH) || (nowP < 1)) { nowP = 1; }
        setPosition(nowP); //read back last position and set to rotary encoder
        displayCurrent(); //re-load the last channel for OLED display
        break;
      case 2: //setting mode
        settingScreen();
        break;
      case 3: //Short Message mode
        messageScreen();
        break;
      case 4: //transparent mode
        transparentScreen();
        break;
    }
  }  
}

void lockScreen() {
  oled.setScrollMode(SCROLL_MODE_OFF);  //prepare OLED to NO scrolling mode
  oled.clear();
  oled.set1X();
  dataCH outputCH; //read back all data of the current channel
  EEPROM.get(nowP*sizeCH, outputCH);  //read back channel data from EEPROM directly 
  delay(5);
  
  //prepare those strings
  String tempS = String(outputCH.fRX);
  String fR = tempS.substring(0, 3) + "." + tempS.substring(3,8);
  tempS = String(outputCH.fTX);
  String fT = tempS.substring(0, 3) + "." + tempS.substring(3,8);

  //these for large receiving frequency
  oled.println(F("---------------------")); 
  oled.setCursor(0,1);
  oled.println(F("RX:"));
  oled.setCursor(26,1);
  oled.set2X();
  oled.print(fR.substring(0, 7));
  oled.set1X();
  oled.println(fR.substring(7,9)); 
  oled.setCursor(0,3); 
  oled.println(F("---------------------")); 

  //these lines are for "CH:39          KM6WZM"  
  String fullName = String(outputCH.nameCH);
  fullName.trim();
  String firstBlock ="CH:" + String(nowP,DEC);
  oled.print(firstBlock);
  for(int i = firstBlock.length(); i<21-fullName.length();i++) {
    oled.print(F(" "));
  }
  oled.print(fullName);
  oled.println(F(" "));
  oled.println(F("---------------------")); 
  
  //these lines are for "TX:444.88250 CT:100.0"
  oled.print(F("TX:"));
  oled.print(fT);
  String ctcss = String(outputCH.CTCSS);
  if(outputCH.CTCSS == 255) {
      oled.println(F(" CTS:None"));
  }else{
    if (outputCH.CTCSS < 1000) {
      oled.print(F(" CTS:"));
      oled.print(ctcss.substring(0, 2));
      oled.print(F("."));
      oled.println(ctcss.substring(2));
    }else{
      oled.print(F(" CS:"));
      oled.print(ctcss.substring(0, 3));
      oled.print(F("."));
      oled.println(ctcss.substring(3));
    }
  }
  oled.println(F("---------------------")); 
}

void volumeScreen() { //displaying volume tip
  oled.setScrollMode(SCROLL_MODE_OFF); 
  oled.clear();
  oled.set1X();
  oled.setCursor(0,1);
  oled.println(F(" AUDIO VOLUME:"));
  oled.println(F(" -------------------"));
  oled.println(F(" 32 steps in total"));
  oled.setCursor(0,5);
  oled.set2X();
  oled.print(F(" >> "));
  oled.print(volume / 8); 
  oled.println(F(" <<")); 
}

void settingScreen(){  //init for the OLED display of setting mode
  oled.setScrollMode(SCROLL_MODE_AUTO); //Set auto scrolling of OLED screen
  oled.clear();
  oled.setInvertMode(true);
  oled.println(F(" Setting Mode "));
  oled.setInvertMode(false);
  oled.println(F("---------------------")); //the "?" was modifried in System5x7.h to 0x02,0x00,0x02,0x00,0x02
  oled.println(F("please connect phone"));
  oled.println(F("var Bluetooth or USB"));
  oled.println(F("to your computer."));
  oled.println(F(" "));
  oled.println(F("> waitting..."));
}

void messageScreen(){  //init for the OLED display of short message mode
  oled.setScrollMode(SCROLL_MODE_AUTO); //Set auto scrolling of OLED screen
  oled.clear();
  oled.setInvertMode(true);
  oled.println(F(" Short Message Mode "));
  oled.setInvertMode(false);
  oled.println(F("---------------------")); 
  oled.println(F("please send SMS from"));
  oled.println(F("phone or computer ver"));
  oled.println(F("Bluetooth or USB"));
  oled.println(F(" "));
  oled.println(F(">message sent:"));
}

void transparentScreen() {
  oled.setScrollMode(SCROLL_MODE_AUTO); //Set auto scrolling of OLED screen
  oled.clear();
  oled.println(F("Transparent Terminal:"));
  oled.println(F("---------------------")); 
  oled.println(F("please connect your"));
  oled.println(F("phone or computer ver"));
  oled.println(F("Bluetooth or USB"));
  oled.println(F(" "));
  oled.println(F(">ready..."));
}

void scanButton() {
  if ((digitalRead(4) == LOW)&&(scanLock == false)&&(millis() - debounce > bounce)) { //scanPin
    scanLock = true;   
    beep(true); //start the beep
    debounce = millis();
    scanning = ! scanning;
    
    //OLED display
    if(scanning == true) { 
      oled.setCursor(0,4);
      oled.print(F(">>>")); 
    }else{
      oled.setCursor(0,4);
      oled.print(F("   ")); 
    }
  }

  //doing with the channel scan
  if (scanning == true) {
    if (updown == true) {  
      nowP = nowP + 1;
      if (nowP > 40) nowP = 1;
    }else{
      nowP = nowP - 1;
      if (nowP < 1) nowP = 40; 
    }
    setPosition(nowP);
    
    displayCurrent(); //show current channel
    
    beep(false); //stop the beep
    if(digitalRead(16) == LOW) {  //signalPin
      unsigned long startTime = millis();

      while(millis() - startTime < scanTimer) {  //signalPin -- 4
        if((digitalRead(4) == LOW)||(digitalRead(16) == HIGH)||(scanning == false)) break; //scanPin--16
        //we are dealing jumping request from rotary sensor
        byte nowS = digitalRead(3) | (digitalRead(2) << 1); //pinA=3; pinB=2;
        if (oldS != nowS) {
          p1 += plate[nowS | (oldS<<2)];
          if (nowS == 3) {p2 = p1 >> 2;}
          oldS = nowS;
        } 
        byte currentP = p2;
        if (currentP != nowP) {
          if (currentP > nowP) updown = true;
          else updown = false;
          break;
        }
      }
    }
  }   

  //releasing the toggle button lock if it is locked
  if ((scanLock == true)&&(digitalRead(4) == HIGH)) { //scanPin
    scanLock = false;
    beep(false); //stop the beep
  }
}

void PTTButton() {  
  if ((digitalRead(5) == LOW)&&(transmiting == false)&&(millis() - debounce > bounce)) { //PTTPin
    transmiting = true;
    debounce = millis();

    dataCH outputCH; //read back all data of the current channel, array one is enough
    EEPROM.get(nowP*sizeCH, outputCH);  //read back channel data from EEPROM directly 
    delay(5);

    if (CHLock == true) {
      //prepare those strings
      String tempS = String(outputCH.fRX);
      String fR = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      tempS = String(outputCH.fTX);
      String fT = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      oled.setCursor(0,1);
      oled.print(F("TX:"));
      oled.setCursor(26,1);
      oled.set2X();
      oled.print(fT.substring(0, 7));
      oled.set1X();
      oled.println(fT.substring(7,9)); 
      oled.setCursor(0,6);
      oled.print(F("RX:"));
      oled.println(fR);
    }else{
      oled.setCursor(0,2);
      oled.print(F("RX:"));
      String tempS = String(outputCH.fRX);
      tempS = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      oled.print(tempS);
      oled.setCursor(3,4);
      oled.print(F("on")); 
      oled.setCursor(24,4);
      oled.set2X();
      String rxS = String(outputCH.fTX);
      oled.print(rxS.substring(0, 3));
      oled.print(F("."));
      oled.print(rxS.substring(3, 6));
      oled.set1X();
      oled.println(rxS.substring(6,8));
      oled.print(F("air"));
    }
  }
  
  //the PTT button is released now 
  if ((transmiting == true)&&(digitalRead(5) == HIGH)) { //PTTPin
    transmiting = false;
    dataCH outputCH; //read back all data of the current channel, array one is enough
    EEPROM.get(nowP*sizeCH, outputCH);  //read back channel data from EEPROM directly 
    delay(5);

    if (CHLock == true) {
      //prepare those strings
      String tempS = String(outputCH.fRX);
      String fR = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      tempS = String(outputCH.fTX);
      String fT = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      oled.setCursor(0,1);
      oled.print(F("RX:"));
      oled.setCursor(26,1);
      oled.set2X();
      oled.print(fR.substring(0, 7));
      oled.set1X();
      oled.println(fR.substring(7,9)); 
      oled.setCursor(0,2);
      oled.setCursor(0,6);
      oled.print(F("TX:"));
      oled.println(fT);
    }else{  
      oled.setCursor(0,2);
      oled.print(F("TX:"));
      String tempS = String(outputCH.fTX);
      tempS = tempS.substring(0, 3) + "." + tempS.substring(3,8);
      oled.print(tempS);
      oled.setCursor(0,4);
      oled.print(F("    ")); //4 spaces to erase the "on"
      oled.setCursor(24,4);
      oled.set2X();
      String rxS = String(outputCH.fRX);
      oled.print(rxS.substring(0, 3));
      oled.print(F("."));
      oled.print(rxS.substring(3, 6));
      oled.set1X();
      oled.println(rxS.substring(6,8));
      oled.print(F("   ")); //4 spaces to erase the "air"
    }
  }
}

void squelchButton(){    //squelchPin;
  if((digitalRead(15) == LOW)&&(squelching == false)&&(millis() - debounce > 2 * bounce)){
    debounce = millis();
    beep(true); //start the beep

    //handle short press bluetooth switching
    if (BTlock == false) {
      BTlock = true;
      digitalWrite(17, HIGH); //btpowerPin; power on the blue tooth module
    }else{
      BTlock = false;
      digitalWrite(17, LOW); //btpowerPin; turn-off the bluetooth module
    }
    
    //read back the group setting
    allSettings mySetting;
    EEPROM.get(settingAddress*4, mySetting);  
    delay(5);

    //force RF module to switch to SQL = 0 rught now
    rfSerial.print(F("AT+DMOFUN=0,"));
    rfSerial.print(String(mySetting.micBoost));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.autoOFF));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.scramble));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.compress));
    rfSerial.println(F("\r\n"));
    delay(5);

    if ((currentMode == 1)&&(CHLock == false)) { //only modify OLED display if we are under normal receiving mode & not channel lock
      oled.setCursor(0,4);
      oled.setInvertMode(true);
      oled.println(F("mon"));
      oled.setInvertMode(false);
    }
    squelching = true;
    beep(false); //stop the beep
  }
  
  //let's handle the releasing of the button, squelchPin;
  if ((digitalRead(15) == HIGH)&&(squelching == true)){
    beep(true); //start the beep
    //read back the group setting
    allSettings mySetting;
    EEPROM.get(settingAddress*4, mySetting);  
    delay(5);
    
    rfSerial.print(F("AT+DMOFUN="));
    rfSerial.print(String(mySetting.SQL));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.micBoost));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.autoOFF));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.scramble));
    rfSerial.print(F(","));
    rfSerial.print(String(mySetting.compress));
    rfSerial.println(F("\r\n"));
    delay(5);

    if ((currentMode == 1)&&(CHLock == false)) { //only modify OLED display if we are under normal receiving mode & not channel lock
      oled.setCursor(0,4);
      oled.setInvertMode(false);
      oled.println(F("   "));
    }
    squelching = false;
    beep(false); //stop the beep
  }
}

//end 
