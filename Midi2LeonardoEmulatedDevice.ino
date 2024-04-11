/*
MIDI device to emulated keyboard/gamepad using Arduino Leonardo
by: ADHSoft

*/
#include <Joystick.h>
#define KEY_UP_ARROW   0x52
#define KEY_DOWN_ARROW   0x51
#define KEY_LEFT_ARROW   0x50
#define KEY_RIGHT_ARROW   0x4F
#define KEY_TAB 0x2B
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  12, 0,                  // Button Count, Hat Switch Count
  false, false, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

//---------------------------------------------------------
//                           Setup
//---------------------------------------------------------

  byte commandByte;
  byte noteByte;
  byte velocityByte;
  unsigned int watchdogSerial=0;
  bool newData=false;
bool checkMIDI(){
  unsigned char i,a;
  static char lastCommandByte=0x89;

  if (newData) return true;
  if (Serial1.available()==0) return false;  

  a=Serial1.peek();
  if (a<0x80 ){
    noteByte=Serial1.read();
    velocityByte=Serial1.read();
    commandByte=lastCommandByte;
    if (commandByte==0x89 || commandByte==0x99) {
      newData=true;
    } else {
      Serial.print("lc");
      Serial.print(lastCommandByte,HEX);
      Serial.print(noteByte,HEX);
      Serial.print(velocityByte,HEX);
      Serial.print(" ");
      newData=false;
    }
    return newData;
  }
  lastCommandByte=a;
  if (a==0x89 || a==0x99){
    commandByte=Serial1.read();
    noteByte=Serial1.read();
    velocityByte=Serial1.read();
    newData=true;
    return newData;
  }
  if (a>=0xFE) {
    Serial1.read();
    newData=false;
    return newData;
  }
  if (a>0x80) {
    Serial.print(Serial1.peek(),HEX);
    Serial1.read();
    Serial.print(Serial1.peek(),HEX);
    Serial1.read();
    Serial.print(Serial1.peek(),HEX);
    Serial1.read();
    Serial.print(" ");
    newData=false;
    return newData;
  }

  return newData;
  //END
}

void setup() {
pinMode(3,INPUT_PULLUP);  // sets pin 3 to input & pulls it high w/ internal resistor
Serial.begin(9600);       // begin serial comms for debugging
Serial1.begin(31250); //midi
Joystick.begin();

}

void loop() {
  int button=0;
  #define BUTTON_COUNT 10
  static unsigned int buttonTimer[BUTTON_COUNT] = {0,0,0,0,0,0,0,0,0,0};
  char i;
  if (checkMIDI()){    
    newData=false;
    
    switch (noteByte){
      case 0x2a: //hh semiopen
      case 0x2e: //hh
      case 0x2c: //hhp
        button=1;
        break;
      case 0x24: //pedal
        button=3;
        break;
      case 0x2b: //ft
        button=5;
        break;
      case 0x26: //sn
        button=2;
        break;
      case 0x2d: //ht
      case 0x30: //lt--
        button=4;
        break;
      case 0x33: //cyr
      case 0x31: //cyl
        button=6;
        break;
      case 0x01: //l
      case 0x02: //start button
      case 0x03: //r
      case 0x04: //arcade coin mechanism
        button=6+noteByte;
        break;
      default:
        button=0;
        break;
    }
    if (button>0 && velocityByte>4){
      if (commandByte==0x99) {//(note on) ch10
        if (buttonTimer[button-1]!=0) {
          Joystick.setButton(button-1, false);
        }
        Joystick.setButton(button-1, true);
        buttonTimer[button-1]=50;
      }
      // if (commandByte==0x89) {//(note off) ch10
      //   Joystick.setButton(button-1, false);
      // }
    }
  }

  for (i=0;i<BUTTON_COUNT;i++){
    if (buttonTimer[i]!=0){
      //Serial.print("Test 123");
      //Serial.print(buttonTimer[i],HEX);
      buttonTimer[i]-=1;
      if (buttonTimer[i]==0){
        //Serial.print("b");
        Joystick.setButton(i, false);
      }
    }    
  }
  
  //Joystick.setButton(9, !digitalRead(3));
  if (!digitalRead(3)){
    Serial.print("reseted ");
    newData=false;
  }

  delay(1);

}
