/*
      .....  .......  .......  .......
     /..../ /....../ /....../ /....../
    /.......  .......  .......  .......
   /../----/ /------/ /------/ /------/
  /../--.....  .......  .......  .......
  ``/--/====/ /======/ /======/ /======/
   /--/==.....  .......  .......  .......
   ``/==/$$$$/ /$$$$$$/ /$$$$$$/ /$$$$$$/
    /==/$$/   /$$//$$/ /$$//$$/ /$$//$$/
    ``/$$/   /$$//$$/ /$$$$$\  /$$$$$$/
     /$$$$/ /$$$$$$/ /$$/\$$/ /$$/
     `````  ```````  ```  ``  ```  systems
  
  
  
  //--------------------------------//
  BEWARE, OLD AND RETARDED CODE BELOW
  BEWARE, OLD AND RETARDED CODE BELOW
  BEWARE, OLD AND RETARDED CODE BELOW
  //--------------------------------//
  
  MCU - STM32F103C8
   
*/

#include <MCP7940_swi2c.h>
MCP7940_Class MCP7940;

#include <EEPROM.h>

#include <low_power.h>
#include <STM32LowPower.h>

#include <Arduino.h>
#include <U8g2lib.h>

U8G2_ST7565_KS0713_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ PB3, /* data=*/ PB4, /* cs=*/ PB5, /* dc=*/ PB6, /* reset=*/ PB7);    // KS0713 controller

#include "xbm.h"

#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
RHSoftwareSPI spi;
RH_RF95 rf95(PA4, PA3, spi); //NSS, DIO0

char messagebuf1[30], messagebuf2[30], messagebuf3[30], messagebuf4[30], messagebuf5[30], messagebuf6[30], messagebuf7[30], messagebuf8[30];

#define one_(state) if(digitalRead(PB12) == LOW && digitalRead(PB12) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PB12);
#define two_(state) if(digitalRead(PB13) == LOW && digitalRead(PB13) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PB13);
#define three_(state) if(digitalRead(PB14) == LOW && digitalRead(PB14) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PB14);
#define left_(state) if(digitalRead(PB15) == LOW && digitalRead(PB15) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PB15);
#define right_(state) if(digitalRead(PA8) == LOW && digitalRead(PA8) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PA8);
#define up_(state) if(digitalRead(PA11) == LOW && digitalRead(PA11) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PA11);
#define down_(state) if(digitalRead(PA12) == LOW && digitalRead(PA12) != state) { statebuffer = 1; } else { statebuffer = 0; }  state = digitalRead(PA12);

static const unsigned char unreadicon[] U8X8_PROGMEM = { //12x7
  0xFF, 0x0F, 0x03, 0x0C, 0x05, 0x0A, 0x09, 0x09, 0xF1, 0x08, 0x01, 0x08, 
  0xFF, 0x0F, };

static const unsigned char jamicon[] U8X8_PROGMEM = { //16x8
  0x20, 0x7F, 0x18, 0x49, 0x0C, 0x2A, 0x7E, 0x1C, 0x3F, 0x08, 0x18, 0xA8, 
  0x0C, 0x48, 0x02, 0xA8, };

static const unsigned char batticon[] U8X8_PROGMEM = { //11x8
  0x8E, 0x03, 0xFB, 0x06, 0x01, 0x04, 0x81, 0x04, 0xCD, 0x05, 0x81, 0x04, 
  0x01, 0x04, 0xFF, 0x07, };

static const unsigned char markericon[] U8X8_PROGMEM = { //5x9
  0x11, 0x1B, 0x15, 0x11, 0x00, 0x1F, 0x11, 0x0A, 0x04, };

bool keystate[7];
bool statebuffer;

bool last1state;
bool last2state;
bool last3state;
bool lastleftstate;
bool lastrightstate;
bool lastupstate;
bool lastdownstate;

bool one;
bool two;
bool three;
bool left;
bool right;
bool up;
bool down;

uint32_t currentmillisheld;
bool globalbuttonsheld;
int buttonsheldtimer;
bool buttonsheldtick;
byte lastbuttonpressed; //1 - one, 2 - two, 3 - three, 4 - left, 5 - right, 6 - up, 7 - down

bool globalbuttons;
bool updatescreen = 1; //show desktop at startup

bool vibration;
uint32_t currentmillisvibro;

bool enablebacklight;
bool lastenablebl;

byte optionselect = 1;
byte lineselect = 1;
byte movelines = 1;

byte keybrowpos = 1;

int holdtimer;

byte menuselect; //0 - desktop, 1 - msgview, 2 - prefs //3 - writemsg, 4 - displaysettings, 5 - rfsettings, 6 - feedback, 7 - sleepsettings, 8 - batterylevel, 9 - spectrum analyzer, 10 - debug menu, 11 - save settings, 12 - clock settings
byte lastmenu;
byte menulength[13] = {0, 0, 10, 5, 2, 6, 2, 2, 0, 0, 0, 0, 4}; //number of lines of a menu
                     //0  1  2   3  4  5  6  7  8  9  10 11 12

byte backlightmode = 1; //1 - timeout, 2 - no timeout, 3 - on startup
byte contrastlevel = 130;

float frequency = 860.01; //.01 sx1276 correction
byte spreadingfactor = 9; //default
byte txbandwidth = 5; //default (31250)
uint32_t txbw;
byte txpower = 20; //default
int preamble = 32; //default
byte digitcursor;
byte freqadjust;

byte feedbackmode = 1;
byte msgalert = 2; //1 - silent, 2 - beep, 3 - beep & vibro, 4 - vibro, 5 - single beep
byte bfeedback = 4; //1 - silent, 2 - click, 3 - click & vibro, 4 - vibro
bool sleepenable = 1;
bool sleepreceiving = 1;
bool sleeping;
byte currentimage = 1;
bool contmode;

uint32_t currentmillisbl;
uint32_t currentmillissleep;

byte msgcount;
byte msgnum;
uint32_t currentmillisalert;
byte beepcount;
int beepseq[9] = {4000,2000,4000,2000,4000,2000,4000,2000};
bool vibroseq[9] = {1,0,1,0,1,0,1,0};

byte msgnav = 1;

int batterylevel;

float freqstart = 850.0;
float freqend = 870.0;
float freqrange;
float freqstep;
float freqsweep;
float freqmid;
byte spectrumanalyzerinfo = 1; //0 - no info //1 - bottom row //2 - marker info
byte spectrumanalyzerparam = 3; //0 - none //1 - starting freq //2 - ending freq //3 - vertical zoom
byte verticalzoom;


String writemsgbuffer_;
char writemsgbuffer[44];
bool caps;
char lettersrow1[23] = {'1','2','3','4','5','6','7','8','9','0','-',    '!','@','#','$','%','^','&','*','(',')','_'}; //(11*2)+1
char lettersrow2[21] = {  'q','w','e','r','t','y','u','i','o','p',        'Q','W','E','R','T','Y','U','I','O','P'}; //(10*2)+1
char lettersrow3[21] = {    'a','s','d','f','g','h','j','k','l',';',        'A','S','D','F','G','H','J','K','L',':'}; //(10*2)+1
char lettersrow4[21] = {      'z','x','c','v','b','n','m',',','.','/',         'Z','X','C','V','B','N','M','<','>','?'}; //(10*2)+1
bool transmit;

uint8_t debugdata[8];
byte debugreg;
byte debugbytecount;
char hexbuffer[3];

uint8_t setyear;
uint8_t setmonth;
uint8_t setday;
uint8_t sethour;
uint8_t setminute;
uint8_t setalarmhour;
uint8_t setalarmminute;
byte timevalselect;
bool timevaladjust;
bool alarmenable;

struct settings_ {
  byte backlightmode;
  byte contrastlevel;
  float frequency;
  byte spreadingfactor;
  byte txbandwidth;
  byte txpower;
  int preamble;
  byte msgalert;
  byte bfeedback;
  bool sleepenable;
  bool sleepreceiving;
  float freqstart;
  float freqend;
} settings;

void setup() {
  pinMode(PA0, OUTPUT); //backlight
//  digitalWrite(PA0, HIGH);

  pinMode(PA1, OUTPUT); //beeper
  digitalWrite(PA2, HIGH);

  pinMode(PA2, OUTPUT); //vibrator
  digitalWrite(PA2, HIGH);

  pinMode(PB12, INPUT_PULLUP); //1
  pinMode(PB13, INPUT_PULLUP); //2
  pinMode(PB14, INPUT_PULLUP); //3
  pinMode(PB15, INPUT_PULLUP); //left
  pinMode(PA8, INPUT_PULLUP); //right
  pinMode(PA11, INPUT_PULLUP); //up
  pinMode(PA12, INPUT_PULLUP); //down

  pinMode(PB0, INPUT); //measure battery
  
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.setFlipMode(1);
  u8g2.setContrast(contrastlevel);

  EEPROM.get(0, settings); //get settings at address 0
  backlightmode = settings.backlightmode;
  contrastlevel = settings.contrastlevel;
  frequency = settings.frequency;
  spreadingfactor = settings.spreadingfactor;
  txbandwidth = settings.txbandwidth;
  txpower = settings.txpower;
  preamble = settings.preamble;
  msgalert = settings.msgalert;
  bfeedback = settings.bfeedback;
  sleepenable = settings.sleepenable;
  sleepreceiving = settings.sleepreceiving;
  freqstart = settings.freqstart;
  freqend = settings.freqend;

  spi.setPins(A7, A6, A5); //MISO, MOSI, SCK
//  initlora(); //somehow this breaks rfm95 initialization
  bool rfm95status = rf95.init();
  rf95.setFrequency(frequency);
  rf95.setTxPower(txpower, false);
  rf95.setPreambleLength(preamble);
  rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);

  if(!digitalRead(PA8)) { //jam mode //hold right at startup
    rf95.setSpreadingFactor(11); //6-12 //saw frequency
    rf95.setSignalBandwidth(500000); //0.5mhz bandwidth
    rf95.setPreambleLength(0xffff); //max
    rf95.setTxMode(1); //continuous tx
    rf95.setModeTx(); //start transmitting
    delay(200);
    u8g2.clearBuffer();
    u8g2.drawXBMP(0,16, 132, 16, footer); //footer
    u8g2.drawXBMP(113,0, 16, 8, jamicon); //jam icon
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 7, "Jam mode ON");
    u8g2.setCursor(66, 15);
    u8g2.print("BW: "); u8g2.print(0.5); u8g2.print("MHz");
    u8g2.setFont(u8g2_font_6x13O_tr);
    u8g2.setCursor(0, 17);
    u8g2.print(frequency, 3); u8g2.print("MHz");
    u8g2.sendBuffer();
    while(true) { //infinite loop
    }
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tr);
  if(rfm95status) {
    u8g2.drawStr(0, 5, "rfm95 init success!");
  } else {
    u8g2.drawStr(0, 5, "rfm95 init fail!");
  }
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.sendBuffer();
  delay(200);

  LowPower.begin();
  LowPower.attachInterruptWakeup(PB14, wakeup, FALLING, SLEEP_MODE);

  MCP7940.begin(); //PA10 - sda, PA9 - scl
//  while (!MCP7940.deviceStatus()) {
//    bool deviceStatus = MCP7940.deviceStart();  // Start oscillator and return state
//    if (!deviceStatus) {                        // If it didn't start
//      delay(500);
//    }
//  }
  MCP7940.deviceStart();
  MCP7940.setBattery(1); //enable battery backup

//  playstartupsplash();
  u8g2.clearBuffer();
  u8g2.drawXBMP(0,0, 132, 32, frame9);
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  /*button reading*/
  one_(keystate[0]) if(statebuffer) {
    globalbuttons = 1;
    one = 1;
    lastbuttonpressed = 1;
  }
  if(!digitalRead(PB12)) { //hold one, turn on/off backlight
    holdtimer++;
    if(holdtimer == 1200) {
      enablebacklight = !enablebacklight;
      digitalWrite(PA0, enablebacklight);
    }
    delay(1);
  } else {
    holdtimer = 0;
  }
  two_(keystate[1]) if(statebuffer) {
    globalbuttons = 1;
    two = 1;
    lastbuttonpressed = 2;
  }
  three_(keystate[2]) if(statebuffer) {
    globalbuttons = 1;
    three = 1;
    lastbuttonpressed = 3;
  }
  left_(keystate[3]) if(statebuffer) {
    globalbuttons = 1;
    left = 1;
    lastbuttonpressed = 4;
  }
  right_(keystate[4]) if(statebuffer) {
    globalbuttons = 1;
    right = 1;
    lastbuttonpressed = 5;
  }
  up_(keystate[5]) if(statebuffer) {
    globalbuttons = 1;
    up = 1;
    lastbuttonpressed = 6;
  }
  down_(keystate[6]) if(statebuffer) {
    globalbuttons = 1;
    down = 1;
    lastbuttonpressed = 7;
  }
  if(!digitalRead(PB12) && !digitalRead(PB14) && !digitalRead(PA8)) { //play startup splash with button combo (one, three, right)
    digitalWrite(PA2, HIGH); //turn off vibrator
//    playstartupsplash();
  }

  /*global button functions*/
  if(globalbuttons) {
    buttonfeedback();
    updatescreen = 1;
    currentmillisvibro = millis();
    currentmillissleep = millis(); //reset sleep timer
    currentmillisbl = millis(); //reset backlight timer
    currentmillisheld = millis(); //reset button hold timer
    beepcount = 0; //stop alerting
    batterylevel = analogRead(PB0); //read battery level
  }

  if(digitalRead(PB15) == LOW || digitalRead(PA8) == LOW || digitalRead(PA11) == LOW || digitalRead(PA12) == LOW) {
    if(millis() >= currentmillisheld + 800) { //amount of time for nav buttons to be held
      globalbuttonsheld = 1;
    }
  } else {
    globalbuttonsheld = 0;
  }

  if(globalbuttonsheld) {
//    delay(1); //slow down the program
    buttonsheldtimer++;
    if(buttonsheldtimer == 2) { //speed of repetition
      buttonsheldtimer = 0; //reset timer
      buttonsheldtick = 1; //tick for incrementing values while button is held
    }
  }

  if(buttonsheldtick) {
    switch(lastbuttonpressed) { //repeat last button pressed
      case 1:
        one = 1;
        break;
      case 2:
        two = 1;
        break;
      case 3:
        three = 1;
        break;
      case 4:
        left = 1;
        break;
      case 5:
        right = 1;
        break;
      case 6:
        up = 1;
        break;
      case 7:
        down = 1;
        break;
    }
    
//    buttonfeedback();

    if(bfeedback == 2 || bfeedback == 3) {
      tone(PA1, 4000, 20);
    }
    updatescreen = 1;
    currentmillissleep = millis(); //reset sleep timer
  }

  /*button logic*/
  if(up && !timevaladjust) {
    optionselect--;
    if(optionselect == 0) {
      optionselect = 1; //no less than 1
    }
    lineselect--;
    if(lineselect == 0) {
      lineselect = 1; //no less than 1
      movelines--;
      if(movelines == 0) {
        movelines = 1;
      }
    }
  }
  if(down && !timevaladjust) {
    optionselect++;
    if(optionselect >= menulength[menuselect]+1) { //adapt to menu size
      optionselect = menulength[menuselect];      
    }
//    if(optionselect >= 5) { //start moving after the 4th line
//      movelines++;
//      if(movelines >= menulength[menuselect]-3) {
//        movelines = menulength[menuselect]-4; //stop moving at the last 4 lines
//      }
//    }
    lineselect++;
    if(lineselect >= menulength[menuselect]+1) {
      lineselect = menulength[menuselect]; //no more than menu size
    }
    if(menuselect == 3) { //only in msg view
      if(lineselect > 2) { //start moving after the 2th line
        movelines++;
        if(movelines >= menulength[menuselect]) {
          movelines = menulength[menuselect]-1;
        }
      }
    } else {
      if(lineselect > 4) { //start moving after the 4th line
        movelines++;
        if(movelines >= menulength[menuselect]-2) {
          movelines = menulength[menuselect]-3; //stop moving at the last 4 lines
        }
      }
    }
    if(menuselect == 3) { //only in msg view
      if(lineselect >= 3) {
        lineselect = 2; //no more than 2
      }
    } else {
      if(lineselect >= 5) {
        lineselect = 4; //no more than 4
      }
    }
  }
  if(freqadjust) { //block selector when adjusting frequency in rf settings
    optionselect = 1;
  }

  if(one) { //open prefs from desktop
    if(menuselect == 0) {
      menuselect = 2;
      one = 0; //so it doesnt open a menu when entering
    }
  }
  if(two) { //view msgs
    if(menuselect == 0) {
      menuselect = 1;
    }
  }
  if(three) { //exit out of menus
    if(menuselect == 1 || menuselect == 2) { //main menus
      menuselect = 0;
    }
    if(menuselect >= 3 && menuselect <= 12) { //submenus
      if(menuselect == 9) { //only when exiting spectrum analyzer
//        rf95.sleep(); //modulation can only be changed while in sleep
//        rf95.setModulation(1); //fsk mode
        initlora();
      }
      menuselect = 2;
      freqadjust = 0; //make sure freq adjustment is disabled outside of rf settings
      timevaladjust = 0; //make sure time adjustment is disabled outside of time and date
    }
  }
  if(left || right || up || down) { //open prefs with nav keys from desktop
    if(menuselect == 0) {
      menuselect = 2;
    }
  }

  /*menu navigation*/
  if(menuselect == 1) { //in msg view
    if(left) {
      msgnav--;
      if(msgnav == 0) {
        msgnav = msgnum; //loop around
      }
    }
    if(right) {
      msgnav++;
      if(msgnav >= msgnum+1) {
        msgnav = 1; //loop around
      }
    }
  }
  if(menuselect == 2) { //in prefs menu
    if(one) {
      switch(optionselect) {
        case 1:
          menuselect = 3; //send message
          keybrowpos = 1; //reset keyboard position
          one = 0; //dont type letter when opening
          break;
        case 2:
          menuselect = 4; //display settings
          break;
        case 3:
          menuselect = 5; //rf settings
          break;
        case 4:
          menuselect = 6; //feedback
          break;
        case 5:
          menuselect = 7; //sleep settings
          break;
        case 6:
          menuselect = 8; //battery level
          break;
        case 7:
          menuselect = 9; //spectrum analyzer
          initspectrumanalyzer();
          break;
        case 8:
          menuselect = 10; //debug menu
          break;
        case 9:
//          menuselect = 11; //save settings
          savesettings();
          break;
        case 10:
          menuselect = 12; //clock settings
          timevaladjust = 1; //fix adjusting when entering menu
          DateTime now = MCP7940.now();
          setyear = now.year() - 2000;
          setmonth = now.month();
          setday = now.day();
          sethour = now.hour();
          setminute = now.minute();
//          setalarmhour = sethour;
//          setalarmminute = setminute;
          break;
      }
    }
  }
  if(menuselect == 3) { //in write msg
    byte maxrowlength;
    switch(optionselect) { //selected row
      case 1: //11 keys
        maxrowlength = 11;
        break;
      case 2: //10 keys
        maxrowlength = 10;
        break;
      case 3: //10 keys
        maxrowlength = 10;
        break;
      case 4: //11 keys
        maxrowlength = 11;
        break;
      case 5: //3 keys
        maxrowlength = 3;
        break;
    }
    if(left) { //navigate horizontally
      keybrowpos--;
      if(keybrowpos == 0) { //adapt to row length
        keybrowpos = maxrowlength; //loop around
      }
    }
    if(right) { //navigate horizontally
      keybrowpos++;
      if(keybrowpos >= maxrowlength+1) { //adapt to row length
        keybrowpos = 1; //loop around
      }
    }
    if(one) { //select key
      switch(optionselect) { //selected row
      case 1:
        if(caps) {
          writemsgbuffer_+=lettersrow1[(keybrowpos+11)-1];  //row 1 //caps on
        } else {
          writemsgbuffer_+=lettersrow1[keybrowpos-1];  //row 1
        }
        break;
      case 2:
        if(caps) {
          writemsgbuffer_+=lettersrow2[(keybrowpos+10)-1];  //row 2 //caps on
        } else {
          writemsgbuffer_+=lettersrow2[keybrowpos-1];  //row 2
        }
        break;
      case 3:
        if(caps) {
          writemsgbuffer_+=lettersrow3[(keybrowpos+10)-1];  //row 3 //caps on
        } else {
          writemsgbuffer_+=lettersrow3[keybrowpos-1];  //row 3
        }
        break;
      case 4:
        if(caps) {
          writemsgbuffer_+=lettersrow4[(keybrowpos+10)-2];  //row 4 //caps on
        } else {
          writemsgbuffer_+=lettersrow4[keybrowpos-2];  //row 4
        }
        if(keybrowpos == 1) { //toggle caps lock
          caps = !caps;
          int str_len = writemsgbuffer_.length() - 1; //bug fix
          writemsgbuffer_.remove(str_len,1);
        }
        break;
      case 5:
        switch(keybrowpos) {  //row 5
          case 1: //send
            transmit = 1;
            break;
          case 2: //clear whole buffer
            writemsgbuffer_ = "";
            break;
          case 3: //backspace (delete last char)
            int str_len = writemsgbuffer_.length() - 1;
            writemsgbuffer_.remove(str_len,1);
            break;
        }
        break;
      }
    }
    if(two) { //space key
      writemsgbuffer_+=" ";
    }
  }
  if(menuselect == 4) { //in display settings
    if(optionselect == 1) { //backlight mode
      if(left) {
        backlightmode--;
        if(backlightmode == 0) {
          backlightmode = 3; //loop around
        }
      }
      if(right) {
        backlightmode++;
        if(backlightmode >= 4) {
          backlightmode = 1; //loop around
        }
      }
    }
    if(optionselect == 2) { //contrast level
      if(left) {
        contrastlevel -= 10;
        if(contrastlevel <= 9) {
          contrastlevel = 10; //no less than 10
        }
        u8g2.setContrast(contrastlevel);
      }
      if(right) {
        contrastlevel += 10;
        if(contrastlevel >= 241) {
          contrastlevel = 240; //no more than 250
        }
        u8g2.setContrast(contrastlevel);
      }
    }
  }
  if(menuselect == 5) { //in rf settings
    if(optionselect == 1) { //set frequency
//      if(left) {
//        if(globalbuttonsheld) {
//          frequency -= 1.0; //coarse adjust when held
//        } else {
//          frequency -= 0.01;
//        }
//      }
//      if(right) {
//        if(globalbuttonsheld) {
//          frequency += 1.0; //coarse adjust when held
//        } else {
//          frequency += 0.01;
//        }
//      }
      if(one) {
        freqadjust = !freqadjust; //toggle freq adjustment
      }
      if(freqadjust) {
        if(left) {
          digitcursor--;
          if(digitcursor == 255) {
            digitcursor = 5; //loop around
          }
        }
        if(right) {
          digitcursor++;
          if(digitcursor >= 6) {
            digitcursor = 0; //loop around
          }
        }
        if(up) {
          switch(digitcursor) {
          case 0:
            frequency += 100.0;
            break;
          case 1:
            frequency += 10.0;
            break;
          case 2:
            frequency += 1.0;
            break;
          case 3:
            frequency += 0.1;
            break;
          case 4:
            frequency += 0.01;
            break;
          case 5:
            frequency += 0.001;
            break;
          }
        }
        if(down) {
          switch(digitcursor) {
          case 0:
            frequency -= 100.0;
            break;
          case 1:
            frequency -= 10.0;
            break;
          case 2:
            frequency -= 1.0;
            break;
          case 3:
            frequency -= 0.1;
            break;
          case 4:
            frequency -= 0.01;
            break;
          case 5:
            frequency -= 0.001;
            break;
          }
        }
      }
      rf95.setFrequency(frequency);
    }
    if(optionselect == 2) { //contmode on/off
      if(left || right) {
        contmode = !contmode;
        rf95.setTxMode(contmode);
        if(contmode) {
          rf95.setModeTx();
        } else {
          rf95.setModeIdle();
        }
      }
    }
    if(optionselect == 3) { //set spreading factor
      if(left) {
        spreadingfactor--;
        if(spreadingfactor < 6) {
          spreadingfactor = 6;
        }
      }
      if(right) {
        spreadingfactor++;
        if(spreadingfactor > 12) {
          spreadingfactor = 12;
        }
      }
    }
    rf95.setSpreadingFactor(spreadingfactor);
    if(optionselect == 4) { //set tx bandwidth
      if(left) {
        txbandwidth--;
        if(txbandwidth < 1) {
          txbandwidth = 1;
        }
      }
      if(right) {
        txbandwidth++;
        if(txbandwidth > 10) {
          txbandwidth = 10;
        }
      }
    }
    if(optionselect == 5) { //set tx power
      if(up) {
        lineselect = 5;
      }
      if(left) {
        txpower--;
        if(txpower <= 1) {
          txpower = 20;
        }
      }
      if(right) {
        txpower++;
        if(txpower >= 21) {
          txpower = 2;
        }
      }
      rf95.setTxPower(txpower, false);
    }
    if(optionselect == 6) { //set preamble
      if(left) {
        preamble--;
        if(preamble == 0) {
          preamble = 1;
        }
      }
      if(right) {
        preamble++;
      }
      rf95.setPreambleLength(preamble);
    }
    switch(txbandwidth) {
      case 1:
        txbw = 7800;
        break;
      case 2:
        txbw = 10400;
        break;
      case 3:
        txbw = 15600;
        break;
      case 4:
        txbw = 20800;
        break;
      case 5:
        txbw = 31250;
        break;
      case 6:
        txbw = 41700;
        break;
      case 7:
        txbw = 62500;
        break;
      case 8:
        txbw = 125000;
        break;
      case 9:
        txbw = 250000;
        break;
      case 10:
        txbw = 500000;
        break;
    }
    rf95.setSignalBandwidth(txbw);
  }
  if(menuselect == 6) { //in feedback
    if(optionselect == 1) { //msg alert
      if(left) {
        msgalert--;
        if(msgalert == 0) {
          msgalert = 5; //loop around
        }
      }
      if(right) {
        msgalert++;
        if(msgalert >= 6) {
          msgalert = 1; //loop around
        }
      }
    }
    if(optionselect == 2) { //button feedback
      if(left) {
        bfeedback--;
        if(bfeedback == 0) {
          bfeedback = 4; //loop around
        }
      }
      if(right) {
        bfeedback++;
        if(bfeedback >= 5) {
          bfeedback = 1; //loop around
        }
      }
    }
  }
  if(menuselect == 7) { //in sleep settings
    if(optionselect == 1) { //sleep on/off
      if(left) {
        sleepenable = !sleepenable;
      }
      if(right) {
        sleepenable = !sleepenable;
      }
    }
    if(optionselect == 2) { //receiving on/off
      if(left) {
        sleepreceiving = !sleepreceiving;
      }
      if(right) {
        sleepreceiving = !sleepreceiving;
      }
    }
  }
  if(menuselect == 8) { //in battery level
    
  }
  if(menuselect == 9) { //in spectrum analyzer
    freqrange = freqend - freqstart; //calculate difference
    freqstep = freqrange / 132.0; //calculate step
    freqsweep = freqstart; //set starting point
    freqmid = freqstart + (freqrange / 2); //frequency in center
    updatescreen = 1; //constantly update the screen

    if(two) { //change what info is shown
      spectrumanalyzerinfo++;
      if(spectrumanalyzerinfo >= 3) {
        spectrumanalyzerinfo = 0; //loop around
      }
    }
    if(one) { //select parameter
      spectrumanalyzerparam++;
      if(spectrumanalyzerparam >= 4) {
        spectrumanalyzerparam = 0; //loop around
      }
    }
    switch(spectrumanalyzerparam) { //change parameters
      case 0: //move window horizontally
        if(up) {
          freqstart += 1.0;
          freqend += 1.0;
        }
        if(down) {
          freqstart -= 1.0;
          freqend -= 1.0;
        }
        if(left) {
          freqstart -= 10.0;
          freqend -= 10.0;
        }
        if(right) {
          freqstart += 10.0;
          freqend += 10.0;
        }
        break;
      case 1: //freq start
        if(up) {
          freqstart += 1.0;
        }
        if(down) {
          freqstart -= 1.0;
        }
        if(left) {
          freqstart -= 5.0;
        }
        if(right) {
          freqstart += 5.0;
        }
        break;
      case 2: //freq end
        if(up) {
          freqend += 1.0;
        }
        if(down) {
          freqend -= 1.0;
        }
        if(left) {
          freqend -= 5.0;
        }
        if(right) {
          freqend += 5.0;
        }
        break;
      case 3: //vertical zoom
        if(up) {
          verticalzoom++;
          if(verticalzoom >= 4) {
            verticalzoom = 3; //cap
          }
        }
        if(down) {
          verticalzoom--;
          if(verticalzoom == 255) {
            verticalzoom = 0; //cap
          }
        }
        break;
    }
  }
  if(menuselect == 10) { //in debug menu
    if(up) {
      debugbytecount++;
      if(debugbytecount >= 8) {
        debugbytecount = 0; //loop around
      }
    }
    if(down) {
      debugbytecount--;
      if(debugbytecount == 255) {
        debugbytecount = 7; //loop around
      }
    }
    if(left) {
      debugreg -= 8;
    }
    if(right) {
      debugreg += 8;
    }
    
    rf95.spiBurstRead(debugreg, debugdata, 8);
    updatescreen = 1; //constantly update the screen
  }
//  if(menuselect == 11) { //save settings
//    
//  }
  if(menuselect == 12) { //clock settings
//    if(up) {
//      timerowselect--;
//      if(timerowselect == 255) {
//        timerowselect = 2;
//      }
//    }
//    if(down) {
//      timerowselect++;
//      if(timerowselect >= 3) {
//        timerowselect = 0;
//      }
//    }
    if(one) {
      timevaladjust = !timevaladjust;
    }
    
    byte optioncount;
    switch(optionselect) {
      case 1: //clock
        optioncount = 2;
        break;
      case 2: //date
        optioncount = 3;
        break;
      case 3: //alarm
        optioncount = 3;
        break;
    }

    if(timevaladjust) {
      if(right) {
        timevalselect++;
        if(timevalselect >= optioncount) {
          timevalselect = 0; //loop around
        }
      }
      if(left) {
        timevalselect--;
        if(timevalselect == 255) {
          timevalselect = optioncount - 1; //loop around
        }
      }
      switch(optionselect) {
        case 1: //clock
          switch(timevalselect) {
            case 0: //hour
              if(up) {
                sethour++;
              }
              if(down) {
                sethour--;
              }
              break;
            case 1: //minute
              if(up) {
                setminute++;
              }
              if(down) {
                setminute--;
              }
              break;
          }
          break;
        case 2: //date
          switch(timevalselect) {
            case 0: //day
              if(up) {
                setday++;
              }
              if(down) {
                setday--;
              }
              break;
            case 1: //month
              if(up) {
                setmonth++;
              }
              if(down) {
                setmonth--;
              }
              break;
            case 2: //year
              if(up) {
                setyear++;
              }
              if(down) {
                setyear--;
              }
              break;
          }
          break;
        case 3: //alarm
          switch(timevalselect) {
            case 0: //alarm hour
              if(up) {
                setalarmhour++;
              }
              if(down) {
                setalarmhour--;
              }
              break;
            case 1: //alarm minute
              if(up) {
                setalarmminute++;
              }
              if(down) {
                setalarmminute--;
              }
              break;
            case 2: //alarm enable
              if(up) {
                alarmenable = !alarmenable;
              }
              if(down) {
                alarmenable = !alarmenable;
              }
              break;
          }
          break;
        case 4: //apply
          if(one) {
            MCP7940.adjust(DateTime(setyear + 2000, setmonth, setday, sethour, setminute, 0));  // Adjust the RTC date/time
            u8g2.clearBuffer();
            u8g2.drawStr(23, 15, "Time / date set");
            u8g2.sendBuffer();
            delay(2000);
            timevaladjust = 0; //make sure time adjustment is disabled outside of time and date
            menuselect = 2;
          }
          break;
      }
    } else {
      timevalselect = 0; //reset value selection when changing or exiting rows
    }
  }
  if(lastmenu != menuselect) { //reset selectors when menus change
    lineselect = 1; //reset line selector
    optionselect = 1; //reset option selector
    movelines = 1;
  }
  lastmenu = menuselect;




  

  /*update the display*/
  if(updatescreen) {
    u8g2.clearBuffer();
    if(contmode) {
      u8g2.drawXBMP(113,0, 16, 8, jamicon); //jam icon
    }
    if(menuselect == 0) { //desktop
//      u8g2.drawStr(0, 7,  "QWERTYUIOPASDFGHJKLZXCVBNM");
//      u8g2.drawStr(0, 15, "TEXT TEST 123 / text test 123");
//      u8g2.drawStr(0, 23, "TEXT TEST 123 / text test 123");
//      u8g2.drawStr(0, 31, "TEXT TEST 123 / text test 123");
//    //  u8g2.drawFrame(0, 0, 132, 32);
//    
//      u8g2.setDrawColor(2);
//      u8g2.drawBox(0, (lineselect*8)-8, 132, 8);
//      u8g2.setDrawColor(1);

      showstatusicons();
      u8g2.drawXBMP(0,25, 12, 7, unreadicon); //mail icon
      u8g2.setCursor(13, 32);
      u8g2.print(msgnum);
      if(batterylevel <= 440) { //indicate low battery
        batterylowindicator();
      }
      DateTime now = MCP7940.now();
      if(now.hour() > 9) {
        u8g2.setCursor(20,32);
      } else {
        u8g2.setCursor(26,32);
      }
      u8g2.print(now.hour()); u8g2.print(':');
      u8g2.setCursor(37,32);
      u8g2.print(now.minute());
      u8g2.setCursor(52,32);
      u8g2.print(now.day()); u8g2.print('/'); u8g2.print(now.month()); u8g2.print('/'); u8g2.print(now.year() - 2000);
    }
    if(menuselect == 1) { //view msg
//      u8g2.drawStr(0, 7,  "!!!!!!!!!!!!!!!!!!!!!!"); //22chars max
//      u8g2.drawStr(0, 15, "1234567890-          >");
//      u8g2.drawStr(0, 23, "QWERTYUIOPASDFGHJ    >");
//      u8g2.drawStr(0, 31, "qwertyuiopasdfghj    >");
      u8g2.setCursor(0, 7);
      u8g2.print("Message: ");
      u8g2.print(msgnav);
      u8g2.print("-");
      u8g2.print(msgnum);
      byte buffernav = ((((8 - 1) + msgcount) - (msgnav - 1)) % 8) + 1; //message navigation
      switch(buffernav) {
        case 1:
          u8g2.drawStr(0, 15, messagebuf1);
          break;
        case 2:
          u8g2.drawStr(0, 15, messagebuf2);
          break;
        case 3:
          u8g2.drawStr(0, 15, messagebuf3);
          break;
        case 4:
          u8g2.drawStr(0, 15, messagebuf4);
          break;
        case 5:
          u8g2.drawStr(0, 15, messagebuf5);
          break;
        case 6:
          u8g2.drawStr(0, 15, messagebuf6);
          break;
        case 7:
          u8g2.drawStr(0, 15, messagebuf7);
          break;
        case 8:
          u8g2.drawStr(0, 15, messagebuf8);
          break;
      }
    }
    if(menuselect == 2) { //preferences
      if(movelines >= 1 && movelines <= 4) {
      u8g2.drawStr(0, (8*(2-movelines))-1,  "Send message"); //22chars max
      }
      if(movelines >= 1 && movelines <= 5) {
      u8g2.drawStr(0, (8*(3-movelines))-1, "Display settings");
      }
      if(movelines >= 1 && movelines <= 6) {
      u8g2.drawStr(0, (8*(4-movelines))-1, "RF settings");
      }
      if(movelines >= 1 && movelines <= 7) {
      u8g2.drawStr(0, (8*(5-movelines))-1, "Feedback");
      }
      if(movelines >= 2 && movelines <= 8) {
      u8g2.drawStr(0, (8*(6-movelines))-1, "Sleep settings");
      }
      if(movelines >= 3 && movelines <= 9) {
      u8g2.drawStr(0, (8*(7-movelines))-1, "Battery level");
      }
      if(movelines >= 4 && movelines <= 10) {
      u8g2.drawStr(0, (8*(8-movelines))-1, "Spectrum analyzer");
      }
      if(movelines >= 5 && movelines <= 11) {
      u8g2.drawStr(0, (8*(9-movelines))-1, "Registers debug");
      }
      if(movelines >= 6 && movelines <= 12) {
      u8g2.drawStr(0, (8*(10-movelines))-1, "Save settings");
      }
      if(movelines >= 7 && movelines <= 13) {
      u8g2.drawStr(0, (8*(11-movelines))-1, "Time and date");
      }

      u8g2.setDrawColor(2);
      u8g2.drawBox(0, (lineselect*8)-8, 132, 8); //selection box
      u8g2.setDrawColor(1);
    }
    if(menuselect == 3) { //send message
      if(caps) {
        if(movelines >= 1 && movelines <= 1) {
          u8g2.drawStr(0, (8*(4-movelines))-1,  "! @ # $ % ^ & * ( ) _"); //row 1
        }
        if(movelines >= 1 && movelines <= 2) {
          u8g2.drawStr(0, (8*(5-movelines))-1, " Q W E R T Y U I O P"); //row 2
        }
        if(movelines >= 2 && movelines <= 3) {
          u8g2.drawStr(0, (8*(6-movelines))-1, "  A S D F G H J K L :"); //row 3
        }
        if(movelines >= 3 && movelines <= 4) {
          u8g2.drawStr(0, (8*(7-movelines))-1, "^  Z X C V B N M < > ?"); //row 4
        }
      } else {
        if(movelines >= 1 && movelines <= 1) {
          u8g2.drawStr(0, (8*(4-movelines))-1,  "1 2 3 4 5 6 7 8 9 0 -"); //row 1
        }
        if(movelines >= 1 && movelines <= 2) {
          u8g2.drawStr(0, (8*(5-movelines))-1, " q w e r t y u i o p"); //row 2
        }
        if(movelines >= 2 && movelines <= 3) {
          u8g2.drawStr(0, (8*(6-movelines))-1, "  a s d f g h j k l ;"); //row 3
        }
        if(movelines >= 3 && movelines <= 4) {
          u8g2.drawStr(0, (8*(7-movelines))-1, "^  z x c v b n m , . /"); //row 4
        }
      }
      if(movelines >= 4 && movelines <= 5) {
        u8g2.drawStr(0, (8*(8-movelines))-1, "SEND    CLEAR    <---"); //row 5
      }

      strcpy(writemsgbuffer, (writemsgbuffer_.substring(0, 22)).c_str()); //convert string to char
      u8g2.drawStr(0, 7,  writemsgbuffer); //show buffer 1st line
      
      strcpy(writemsgbuffer, (writemsgbuffer_.substring(22, 44)).c_str()); //convert string to char
      u8g2.drawStr(0, 15,  writemsgbuffer); //show buffer 2nd line

      if(writemsgbuffer_.length() <= 22) {
        u8g2.drawGlyph(6*writemsgbuffer_.length(), 7, 95); //text cursor
      } else {
        u8g2.drawGlyph(6*(writemsgbuffer_.length()-22), 15, 95); //text cursor
      }

      strcpy(writemsgbuffer, writemsgbuffer_.c_str()); //convert all string to char

      u8g2.setDrawColor(2);
      switch(optionselect) {
        case 1: //row 1
          u8g2.drawBox(12*(keybrowpos-1), (lineselect*8)+8, 6, 8); //selection box
          break;
        case 2: //row 2 //6 offset
          u8g2.drawBox((12*keybrowpos)-6, (lineselect*8)+8, 6, 8); //selection box
          break;
        case 3: //row 3 //12 offset
          u8g2.drawBox(12*keybrowpos, (lineselect*8)+8, 6, 8); //selection box
          break;
        case 4: //row 4 //18 offset
          if(keybrowpos == 1) { //if caps lock
            u8g2.drawBox(0, (lineselect*8)+6, 6, 8); //caps lock selection box
          } else {
            u8g2.drawBox((12*(keybrowpos-1))+6, (lineselect*8)+8, 6, 8); //selection box
          }
          break;
        case 5: //row 5
          byte keynav;
          switch(keybrowpos) {
            case 1:
              keynav = 0;
              break;
            case 2:
              keynav = 47;
              break;
            case 3:
              keynav = 98;
              break;
          }
          u8g2.drawBox(keynav, (lineselect*8)+8, 31, 8); //selection box
          break;
      }
      u8g2.setDrawColor(1);
    }
    if(menuselect == 4) { //display settings
      u8g2.drawStr(24, 7,  "Backlight mode");
      switch(backlightmode) {
        case 1:
          u8g2.drawStr(48, 15, "Timeout");
          break;
        case 2:
          u8g2.drawStr(36, 15, "No timeout");
          break;
        case 3:
          u8g2.drawStr(36, 15, "On startup");
          break;
      }
      
      u8g2.drawStr(42, 23, "Contrast");
      u8g2.setCursor(60, 31);
      u8g2.print(contrastlevel);

      u8g2.setDrawColor(2);
      u8g2.drawBox(0, (lineselect*16)-8, 132, 8); //selection box
      u8g2.setDrawColor(1);
    }
    if(menuselect == 5) { //rf settings
      if(optionselect <= 4) { //first half of menu
        u8g2.setCursor(0, 7);
        u8g2.print("Frequency: "); u8g2.print(frequency, 3);
        u8g2.setCursor(0, 15);
        u8g2.print("Continuous TX: ");
        if(contmode) {
          u8g2.print("Enabled");
        } else {
          u8g2.print("Disabled");
        }
        u8g2.setCursor(0, 23);
        u8g2.print("Spreading Factor: ");
        u8g2.print(spreadingfactor);
        u8g2.setCursor(0, 31);
        u8g2.print("TX Bandwidth: ");
        u8g2.print(txbw);
  
        u8g2.setDrawColor(2);
        if(freqadjust) {
          byte cursorpos;
          switch(digitcursor) {
            case 0:
              cursorpos = 0;
              break;
            case 1:
              cursorpos = 6;
              break;
            case 2:
              cursorpos = 12;
              break;
            case 3:
              cursorpos = 24;
              break;
            case 4:
              cursorpos = 30;
              break;
            case 5:
              cursorpos = 36;
              break;
          }
          if(frequency >= 1000.0) {
            cursorpos += 6; //offset cursor when value is above 1000
          }
          u8g2.drawBox(66+cursorpos, 0, 6, 8); //selection box
        } else {
          u8g2.drawBox(0, (lineselect*8)-8, 132, 8); //selection box
        }
        u8g2.setDrawColor(1);
      } else {
        u8g2.setCursor(0, 7);
        u8g2.print("TX Power: "); u8g2.print(txpower);
        u8g2.setCursor(0, 15);
        u8g2.print("Preamble: "); u8g2.print(preamble);

        u8g2.setDrawColor(2);
        u8g2.drawBox(0, ((optionselect-4)*8)-8, 132, 8); //selection box
        u8g2.setDrawColor(1);

        u8g2.drawXBMP(0,16, 132, 16, footer); //footer
      }
    }
    if(menuselect == 6) { //feedback
      u8g2.drawStr(30, 7,  "Message alert");
      switch(msgalert) {
        case 1:
          u8g2.drawStr(48, 15, "Silent");
          break;
        case 2:
          u8g2.drawStr(54, 15, "Beep");
          break;
        case 3:
          u8g2.drawStr(30, 15, "Beep & vibro");
          break;
        case 4:
          u8g2.drawStr(54, 15, "Vibro");
          break;
        case 5:
          u8g2.drawStr(36, 15, "Single beep");
          break;
      }
      u8g2.drawStr(24, 23, "Button feedback");
      switch(bfeedback) {
        case 1:
          u8g2.drawStr(48, 31, "Silent");
          break;
        case 2:
          u8g2.drawStr(54, 31, "Click");
          break;
        case 3:
          u8g2.drawStr(30, 31, "Click & vibro");
          break;
        case 4:
          u8g2.drawStr(54, 31, "Vibro");
          break;
      }

      u8g2.setDrawColor(2);
      u8g2.drawBox(0, (lineselect*16)-8, 132, 8); //selection box
      u8g2.setDrawColor(1);
    }
    if(menuselect == 7) { //sleep settings
      u8g2.drawStr(48, 7,  "Sleep");
      if(sleepenable) {
        u8g2.drawStr(48, 15,  "Enabled");
      } else {
        u8g2.drawStr(42, 15,  "Disabled");
      }
      u8g2.drawStr(42, 23, "Receiving");
      if(sleepreceiving) {
        u8g2.drawStr(48, 31,  "Enabled");
      } else {
        u8g2.drawStr(42, 31,  "Disabled");
      }

      u8g2.setDrawColor(2);
      u8g2.drawBox(0, (lineselect*16)-8, 132, 8); //selection box
      u8g2.setDrawColor(1);
    }
    if(menuselect == 8) { //battery level
      float batteryvoltage = map(batterylevel, 435, 522, 350, 420) / 100.0;
      u8g2.drawStr(28, 15, "Battery Level");
      u8g2.setCursor(54, 23);
      u8g2.print(batteryvoltage);
      u8g2.drawFrame(18, 5, 96, 21);
    }
    if(menuselect == 9) { //spectrum analyzer
      float centerrssi;
      float centerfreq;
      for(byte i = 0; i <= 131; i++) { //increment position, draw buffer
        freqsweep += freqstep; //increment frequencies based on step size
        rf95.setFrequency(freqsweep);
        float rssi = rf95.getRSSI();
        int num = map(rssi, -128, -30, 0, 12 * (verticalzoom + 1)); //remap values to screen height
        if(i == 64) {
          centerrssi = rssi;
          centerfreq = freqsweep;
          if(spectrumanalyzerinfo >= 2) {
            u8g2.drawXBMP(64,13+(num*-1), 5, 9, markericon); //marker icon
          }
        }
        u8g2.drawVLine(i, 25-num, num);  
      }
      if(spectrumanalyzerinfo >= 1) {
        u8g2.setCursor(0, 32); u8g2.print('<'); u8g2.print((uint16_t)freqstart); //show starting freq
        u8g2.setCursor(64, 33); u8g2.print('^'); u8g2.setCursor(70, 32); u8g2.print((uint16_t)freqmid); //show center freq
        u8g2.setCursor(108, 32); u8g2.print((uint16_t)freqend); u8g2.print('>'); //show stopping freq
      }
      if(spectrumanalyzerinfo == 2) {
        u8g2.setCursor(0, 7);
        u8g2.print('M');
        u8g2.print('|');
        u8g2.print(centerrssi);
      }
      u8g2.setDrawColor(2);
      switch(spectrumanalyzerparam) {
        case 1: //starting freq
          if(spectrumanalyzerinfo >=1) {
          u8g2.drawBox(0, 25, 30, 7); //selection box
          }
          break;
        case 2: //ending freq
          if(spectrumanalyzerinfo >=1) {
          u8g2.drawBox(102, 25, 30, 7); //selection box
          }
          break;
        case 3: //vertical zoom
          u8g2.setCursor(90, 8);
          u8g2.print(verticalzoom+1);
          u8g2.print("x zoom");
          break;
      }
      u8g2.setDrawColor(1);
    }
    if(menuselect == 10) { //in debug menu
      u8g2.setCursor(0, 7);
      sprintf(hexbuffer, "%02X", debugreg); u8g2.print(hexbuffer); //show page address
      u8g2.drawHLine(0, 8, 132);
      u8g2.setCursor(0, 18);
      for(byte i = 0; i < 4; i++) { //bytes row one
        sprintf(hexbuffer, "%02X", debugdata[i]);
        u8g2.print(hexbuffer);
        u8g2.print('|');
      }
      for(byte i = 0; i < 4; i++) { //addresses row one
        sprintf(hexbuffer, "%02x", debugreg+i);
        u8g2.print(hexbuffer);
//        u8g2.print(' ');
      }
      u8g2.setCursor(0, 26);
      for(byte i = 0; i < 4; i++) { //bytes row two
        sprintf(hexbuffer, "%02X", debugdata[i+4]);
        u8g2.print(hexbuffer);
        u8g2.print('|');
      }
      for(byte i = 0; i < 4; i++) { //addresses row two
        sprintf(hexbuffer, "%02x", debugreg+(i+4));
        u8g2.print(hexbuffer);
//        u8g2.print(' ');
      }
      u8g2.setCursor(80, 7);
      for(byte i = 8; i > 0; i--) { //selected byte in binary form
        u8g2.print(bitread(debugdata[debugbytecount], i-1));
      }
      u8g2.setFont(u8g2_font_4x6_tr);
      u8g2.drawStr(0, 32, "values            addresses");
      u8g2.setFont(u8g2_font_6x12_tr);
      u8g2.setDrawColor(2);
      if(debugbytecount <= 3) {
        u8g2.drawBox(18*(debugbytecount), 10, 12, 8); //byte selection box
        u8g2.drawBox(72+(12*(debugbytecount)), 10, 12, 8); //address selection box
      } else {
        u8g2.drawBox(18*(debugbytecount-4), 18, 12, 8); //byte selection box
        u8g2.drawBox(72+(12*(debugbytecount-4)), 18, 12, 8); //address selection box
      }
      u8g2.setDrawColor(1);
    }
//    if(menuselect == 11) { //save settings
//    
//    }
    if(menuselect == 12) { //clock settings
//      u8g2.drawStr(14, 7, "Set time and date");
      u8g2.setCursor(0, 7);
      u8g2.print("Time: "); u8g2.print(sethour); u8g2.setCursor(48, 7); u8g2.print(':'); u8g2.print(setminute);
      u8g2.setCursor(0, 15);
      u8g2.print("Date: "); u8g2.print(setday); u8g2.setCursor(48, 15); u8g2.print('/'); u8g2.print(setmonth); u8g2.setCursor(66, 15); u8g2.print('/'); u8g2.print(setyear);
      u8g2.setCursor(0, 23);
      u8g2.print("Alarm: "); u8g2.print(setalarmhour); u8g2.setCursor(54, 23); u8g2.print(':'); u8g2.print(setalarmminute);
      u8g2.setCursor(78, 23);
      if(alarmenable) {
        u8g2.print("ON");
      } else {
        u8g2.print("OFF");
      }
      u8g2.drawStr(23, 31, "Set time / date");

      u8g2.setDrawColor(2);
      if(!timevaladjust) {
        u8g2.drawBox(0, (lineselect*8)-8, 132, 8); //selection box
      } else {
        switch(optionselect) { //selection boxes
          case 1: //clock
            switch(timevalselect) {
              case 0: //hour
                u8g2.drawBox(36, 0, 12, 8);
                break;
              case 1: //minute
                u8g2.drawBox(54, 0, 12, 8);
                break;
            }
            break;
          case 2: //date
            switch(timevalselect) {
              case 0: //day
                u8g2.drawBox(36, 8, 12, 8);
                break;
              case 1: //month
                u8g2.drawBox(54, 8, 12, 8);
                break;
              case 2: //year
                u8g2.drawBox(72, 8, 12, 8);
                break;
            }
            break;
          case 3: //alarm
            switch(timevalselect) {
              case 0: //alarm hour
                u8g2.drawBox(42, 16, 12, 8);
                break;
              case 1: //alarm minute
                u8g2.drawBox(60, 16, 12, 8);
                break;
              case 2: //alarm enable
                u8g2.drawBox(78, 16, 18, 8);
                break;
            }
            break;
        }
      }
      u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer();
  }

//  digitalWrite(PA0, enablebacklight);

  if(vibration) {
    digitalWrite(PA2, LOW); //start vibration
    if(millis() >= currentmillisvibro + 140) {
      digitalWrite(PA2, HIGH); //end vibration
      vibration = 0;
    }
  }

  if(enablebacklight != lastenablebl) { //generate a tick to get currentmillisbl once
    currentmillisbl = millis();
  }
  lastenablebl = enablebacklight;
  if(enablebacklight){
    switch(backlightmode) {
      case 1: //timeout
        if(millis() >= currentmillisbl + 8000) {
          enablebacklight = 0;
          digitalWrite(PA0, enablebacklight);
        }
        break;
      case 2: //no timeout
        
        break;
      case 3: //on startup
        
        break;
    }
  }

  if(backlightmode != 2 && sleepenable) {
    if(millis() >= currentmillissleep + 20000) { //go to sleep
      while(true) { //forever loop
        u8g2.clearBuffer();
        batterylevel = analogRead(PB0); //read battery level
        if(batterylevel <= 440) {
          tone(PA1, 2000, 100);
        }
        showstatusicons();
        u8g2.sendBuffer();
  
        enablebacklight = 0;
        digitalWrite(PA0, LOW); //turn off backlight
        digitalWrite(PA2, HIGH); //turn off vibrator

        sleeping = 1; //sleep status

        if(sleepreceiving) {
          rfmreceive(); //constantly receive
        }

        if(alarmenable) {
          DateTime now = MCP7940.now();
          if(now.hour() == setalarmhour && now.minute() == setalarmminute) {
            beepcount = 1; //start alert
            sleeping = 0; //sleep status
          }
        }
    
        rf95.sleep();
  
        
    
  //      currentmillissleep = millis(); //reset timer
        
        if(sleepreceiving) {
          LowPower.deepSleep(5000); //wake up at 5s intervals to receive
        } else {
          LowPower.deepSleep(60000); //wake up at 30s intervals
        }
  
//        LowPower.deepSleep(30000); //wake up at 30s intervals
        
        if(digitalRead(PB14) == LOW || sleeping == 0) {
          currentmillissleep = millis(); //reset timer
          sleeping = 0; //sleep status
          break; //exit out of forever loop
        }
      }
    }
  }

  if(transmit) {
    rf95.send((uint8_t* )writemsgbuffer, writemsgbuffer_.length()+1); //send the message
    transmit = 0; //so it doesnt transmit again
    tone(PA1, 2000, 40);
  }
  
  resetstates(); //reset states at loop end

  if(beepcount >= 1) {
    alert();
    if(beepcount >= 9) {
      beepcount = 0; //stop alert
    }
  }

  rfmreceive(); //constantly receive
}

void resetstates() {
  one = 0;
  two = 0;
  three = 0;
  left = 0;
  right = 0;
  up = 0;
  down = 0;
  globalbuttons = 0;
  updatescreen = 0;
  buttonsheldtick = 0;
}

void showstatusicons() {
  u8g2.setFont(u8g2_font_6x13O_tr);
  u8g2.setCursor(34, 10);
  u8g2.print("CORP PAGING");
  u8g2.setFont(u8g2_font_6x12_tr);
  if(batterylevel <= 440) { //indicate low battery
    batterylowindicator();
  }
  u8g2.drawXBMP(121,24, 11, 8, batticon); //battery icon
  u8g2.drawFrame(102, 25, 18, 7); //battery level
  uint8_t bar = max((int)map(batterylevel, 435, 522, 0, 16), 0);
  u8g2.drawBox(103, 26, bar, 5); //battery level bar
  if(contmode) {
    u8g2.drawXBMP(113,0, 16, 8, jamicon); //jam icon
  }
//  u8g2.setCursor(40, 32);
//  u8g2.print(millis()); //ms elapsed since power on
//  u8g2.print("ms");
}

void buttonfeedback() {
  if(bfeedback == 2 || bfeedback == 3) {
    tone(PA1, 4000, 35);
  }
  if(bfeedback == 3 || bfeedback == 4) {
    currentmillisvibro = millis(); //set currentmillisvibro before vibration
    vibration = 1;
  }
}

void alert() {
  if(millis() >= currentmillisalert + 500){
    currentmillisalert = millis();
    
    if(msgalert == 2 || msgalert == 3) {
      tone(PA1, beepseq[beepcount-1], 300);
    }
    if(msgalert == 3 || msgalert == 4) {
      if(vibroseq[beepcount-1]) {
        digitalWrite(PA2, LOW);
      } else {
        digitalWrite(PA2, HIGH);
      }
    }
    beepcount++;
    if(msgalert == 5) {
      tone(PA1, 4000, 300);
      beepcount = 0; //single beep
    }
  }
}

void rfmreceive() {
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  
  if(rf95.recv(buf, &len)) { //check if a whole message is received
    msgnum++;
    if(msgnum >= 9) {
      msgnum = 8; //no more than 8
    }
    msgcount++;
    if(msgcount >= 9) {
      msgcount = 1; //loop around
    }
    switch(msgcount) { //increment thru message buffers
      case 1:
        strcpy(messagebuf1, (char*)buf);
        break;
      case 2:
        strcpy(messagebuf2, (char*)buf);
        break;
      case 3:
        strcpy(messagebuf3, (char*)buf);
        break;
      case 4:
        strcpy(messagebuf4, (char*)buf);
        break;
      case 5:
        strcpy(messagebuf5, (char*)buf);
        break;
      case 6:
        strcpy(messagebuf6, (char*)buf);
        break;
      case 7:
        strcpy(messagebuf7, (char*)buf);
        break;
      case 8:
        strcpy(messagebuf8, (char*)buf);
        break;
    }
    
    beepcount = 1; //start alert
    sleeping = 0; //sleep status
    currentmillissleep = millis(); //reset timer so it doesnt go back to sleep immediately
    updatescreen = 1;
  }
}

void initspectrumanalyzer() {
  rf95.sleep(); //modulation can only be changed while in sleep
  rf95.setModulation(0); //fsk mode
  rf95.setModeRx();
  rf95.fastHop(); //fix frequency updating
  rf95.setFskRxBw125kHz(); //set max rx bw
}

void initlora() {
//  rf95.sleep(); //modulation can only be changed while in sleep
//  rf95.setModulation(1); //fsk mode
//  rf95.init();
//  rf95.setFrequency(frequency);
//  rf95.setTxPower(20, false);
//  rf95.setPreambleLength(32);
//  rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);

  ///           ˇregister           ˇregister ˇmask         ˇbits to change
//  rf95.spiWrite(0x44, (rf95.spiRead(0x44) & ~0b10000000) | 0b00000000); //return fasthopon to default state
//  rf95.spiWrite(0x12, (rf95.spiRead(0x12) & ~0b00011111) | 0b00010101); //return rxbw to default state
  rf95.spiWrite(0x01, (rf95.spiRead(0x01) & ~0b00000111) | 0b00000000); //sleep mode
  rf95.spiWrite(0x01, (rf95.spiRead(0x01) & ~0b10000000) | 0b10000000); //lora mode
  rf95.spiWrite(0x01, (rf95.spiRead(0x01) & ~0b00000111) | 0b00000001); //idle mode
  rf95.setFrequency(frequency); //bodge
}

void playstartupsplash() {
  digitalWrite(PA0, HIGH); //turn on backlight
  for(byte i = 4; i < 10; i++) { //startup splash
    u8g2.clearBuffer();
    switch(i) {
      case 0:
        u8g2.drawXBMP(0,0, 132, 32, frame0);
        break;
      case 1:
        u8g2.drawXBMP(0,0, 132, 32, frame1);
        break;
      case 2:
        u8g2.drawXBMP(0,0, 132, 32, frame2);
        break;
      case 3:
        u8g2.drawXBMP(0,0, 132, 32, frame3);
        break;
      case 4:
        u8g2.drawXBMP(0,0, 132, 32, frame4);
        break;
      case 5:
        u8g2.drawXBMP(0,0, 132, 32, frame5);
        break;
      case 6:
        u8g2.drawXBMP(0,0, 132, 32, frame6);
        break;
      case 7:
        u8g2.drawXBMP(0,0, 132, 32, frame7);
        break;
      case 8:
        u8g2.drawXBMP(0,0, 132, 32, frame8);
        break;
      case 9:
        u8g2.drawXBMP(0,0, 132, 32, frame9);
        break;
    }
    u8g2.sendBuffer();
    delay(50);
  }
  delay(1000);
  digitalWrite(PA0, LOW); //turn off backlight
}

bool bitread(byte value, byte select) {
  return (value >> select) & B00000001;
}

void batterylowindicator() {
  u8g2.drawStr(36, 23,  "Battery low");
}

void wakeup() {
  
}

void savesettings() {
  settings.backlightmode = backlightmode;
  settings.contrastlevel = contrastlevel;
  settings.frequency = frequency;
  settings.spreadingfactor = spreadingfactor;
  settings.txbandwidth = txbandwidth;
  settings.txpower = txpower;
  settings.preamble = preamble;
  settings.msgalert = msgalert;
  settings.bfeedback = bfeedback;
  settings.sleepenable = sleepenable;
  settings.sleepreceiving = sleepreceiving;
  settings.freqstart = freqstart;
  settings.freqend = freqend;
  EEPROM.put(0, settings); //save settings at address 0
  u8g2.clearBuffer();
  u8g2.drawStr(42,15, "Saved to");
  u8g2.drawStr(51,23, "FLASH");
  u8g2.sendBuffer();
  delay(2000);
}
