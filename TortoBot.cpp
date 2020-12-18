#include <EEPROM.h>
#include "TortoBot.h"

//comment below to if setting trim in TortoBot()
#define __LOAD_TRIM_FROM_EEPROM__

#define EEPROM_MAGIC  0xabcd
#define EEPROM_OFFSET 2   //eeprom starting offset to store trim[]
#define START_PIN 2

#define FRONT_RIGHT_LEG 0
#define FRONT_LEFT_LEG 1
#define REAR_RIGHT_LEG 2
#define REAR_LEFT_LEG 3

TortoBot::TortoBot() {
  //for calibrating servo deviation, put extra value to adjust the angle
  trim[FRONT_RIGHT_LEG] = 0; trim[FRONT_LEFT_LEG] = 0;
  trim[REAR_RIGHT_LEG] = 0;  trim[REAR_LEFT_LEG] = 0;
}



void TortoBot::init()
{
  servo[FRONT_RIGHT_LEG].attach(START_PIN + FRONT_RIGHT_LEG);
  servo[FRONT_LEFT_LEG].attach(START_PIN + FRONT_LEFT_LEG);
  servo[REAR_RIGHT_LEG].attach(START_PIN + REAR_RIGHT_LEG);
  servo[REAR_LEFT_LEG].attach(START_PIN + REAR_LEFT_LEG);

#ifdef __LOAD_TRIM_FROM_EEPROM__
  int val = EEPROMReadWord(0);
  if (val != EEPROM_MAGIC) {
    EEPROMWriteWord(0, EEPROM_MAGIC);
    storeTrim();
  }
#endif

  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    oscillator[i].start();
#ifdef __LOAD_TRIM_FROM_EEPROM__
    int val = EEPROMReadWord(i * 2 + EEPROM_OFFSET);
    if (val >= -90 && val <= 90) {
      trim[i] = val;
    }
#endif
  }
}

void TortoBot::home()
{
  int position[] = {90, 90, 90, 90};
  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    if (position[i] + trim[i] <= 180 && position[i] + trim[i] > 0) {
      servo[i].write(position[i] + trim[i]);
    }
  }
}


void TortoBot::stand(float T) {
  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    oscillator[i].stop();
  }
}

void  TortoBot::refresh() {
  float angle[MAX_LEG_SERVOS];
  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    angle[i] = oscillator[i].refresh();
  }
  if (1) {
  if (!oscillator[0].isStop())
    servo[0].write(angle[FRONT_RIGHT_LEG] + trim[FRONT_RIGHT_LEG]);
  if (!oscillator[1].isStop())
    servo[1].write(angle[FRONT_LEFT_LEG] + trim[FRONT_LEFT_LEG]);
  if (!oscillator[2].isStop())
    servo[2].write(180 - angle[REAR_RIGHT_LEG] + trim[REAR_RIGHT_LEG]);
  if (!oscillator[3].isStop())
    servo[3].write(180 - angle[REAR_LEFT_LEG] + trim[REAR_LEFT_LEG]);
  }
}

void  TortoBot::walk(bool rev, float T) {
  const int A = 20;
  float period[] = {T, T, T, T};
  int amplitude[] = {A, A, A, A};
  int offset[] = {90, 90, 90, 90};
  int forward_phase[] = {0, 0, -90, -90};
  int backwoard_phase[] = { -90, -90, 0, 0};
  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    oscillator[i].setOffset(offset[i]);
    oscillator[i].setPeriod(period[i]);
    oscillator[i].setAmplitude(amplitude[i]);
    if (!rev) {
      oscillator[i].setPhase(forward_phase[i]);
    } else {
      oscillator[i].setPhase(backwoard_phase[i]);
    }
    oscillator[i].start();
  }
}


void  TortoBot::turn(bool right, float T) {

  float period[] = {T, T, T, T};
  int amplitude_left[] = {50, 10, 50, 10};
  int amplitude_right[] = {10, 50, 10, 60};
  int offset[] = {90, 90, 90, 90};
  int phase[] = {0, 0, -90, -90};

  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    oscillator[i].setOffset(offset[i]);
    oscillator[i].setPeriod(period[i]);
    if (right) {
      oscillator[i].setAmplitude(amplitude_right[i]);
    } else {
      oscillator[i].setAmplitude(amplitude_left[i]);
    }
    oscillator[i].setPhase(phase[i]);
    oscillator[i].start();
  }
}

void  TortoBot::turnRight(float T) {
  turn(true, T);
}

void  TortoBot::turnLeft(float T) {
  turn(false, T);
}

void TortoBot::storeTrim() {
  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
    EEPROMWriteWord(i * 2 + EEPROM_OFFSET, trim[i]);
    delay(100);
  }
}

void TortoBot::loadTrim() {
  for (int i = 0; i < 8; i++) {
    Serial.write(EEPROM.read(i + EEPROM_OFFSET));
  }
}

int TortoBot::EEPROMReadWord(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void TortoBot::EEPROMWriteWord(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}


//void  TortoBot::jumpLeft(float T) {
//  float period[] = {T, T, T, T};
//  int amplitude[] = {40, 40, 40, 40};
//  int offset[] = {90, 90, 90, 90};
//  int phase[] = { -270, -90, -180, 0};
//  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
//    oscillator[i].setOffset(offset[i]);
//    oscillator[i].setPeriod(period[i]);
//    oscillator[i].setAmplitude(amplitude[i]);
//    oscillator[i].setPhase(phase[i]);
//  }
//}

//void  TortoBot::jumpRight(float T) {
//  float period[] = {T, T, T, T};
//  int amplitude[] = {40, 40, 40, 40};
//  int offset[] = {90, 90, 90, 90};
//  int phase[] = {0, -180, -90, -270};
//
//  for (int i = 0; i < MAX_LEG_SERVOS; i++) {
//    oscillator[i].setOffset(offset[i]);
//    oscillator[i].setPeriod(period[i]);
//    oscillator[i].setAmplitude(amplitude[i]);
//    oscillator[i].setPhase(phase[i]);
//  }
//}


