
#include "GoBLE.hpp"
#include "Octosnake.h"
#include "TortoBot.h"

//#define __DEBUG__

//#define __SOFTWARE_SERIAL__
#ifdef __SOFTWARE_SERIAL__
#include <SoftwareSerial.h>
#define BT_RX_PIN 10
#define BT_TX_PIN 11
SoftwareSerial BlueTooth(BT_RX_PIN, BT_TX_PIN);
#define Console Serial
_GoBLE<SoftwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#else
#define BlueTooth Serial
#define Console Serial
_GoBLE<HardwareSerial, HardwareSerial> Goble(BlueTooth, Console);
#endif
#define BAUD_RATE 115200
//
#define TIME_INTERVAL 5000
#define SERIAL_DATA_PERIOD 200
#define CAL_TRIGGER_PIN 12
#define LED_PIN A0
#define  AUTO_MODE_PIN A1
//
#define FORWARD 'f'
#define LEFT 'l'
#define STAND 's'
#define RIGHT 'r'
#define BACKWARD 'b'
#define GO 'g'
#define RIGHT_FRONT 'c'
#define LEFT_FRONT 'd'
#define RIGHT_BACK 'e'
#define LEFT_BACK 'h'

typedef void (TortoBot::*walk_t) (float); // method function pointer type
walk_t walk; // method function pointer
walk_t prev_walk = nullptr;


TortoBot tortobot;
walk_t gaits[] = { &tortobot.forward,
                   &tortobot.turnLeft,
                   &tortobot.backward,
                   &tortobot.turnRight
                 };
int gaits_len;
boolean auto_mode = false;
unsigned long cur_time, prev_time;
boolean random_walk = false;
bool led_state = true;

void setup() {
  Goble.begin(BAUD_RATE);
#ifdef __DEBUG__
#ifdef __SOFTWARE_SERIAL__
  Console.begin(115200);
#endif
  Console.println("in debugging mode");
#endif
  randomSeed(analogRead(A7));
  gaits_len = sizeof(gaits) / sizeof(walk_t);
  tortobot.init();
  tortobot.home();
  delay(2000);

  //begin: triggering delay for servo calibrating
  pinMode(CAL_TRIGGER_PIN, OUTPUT);
  digitalWrite(CAL_TRIGGER_PIN, 0);
  pinMode(CAL_TRIGGER_PIN, INPUT);
  while (digitalRead(CAL_TRIGGER_PIN)) {
    analogWrite(LED_PIN, 128 * led_state); // on calibarting indication LED
    delay(1000);
    led_state = !led_state;
  }
  analogWrite(LED_PIN, 0); // off calibarting indication LED
  //
  pinMode(AUTO_MODE_PIN, OUTPUT);
  digitalWrite(AUTO_MODE_PIN, 0);
  pinMode(AUTO_MODE_PIN, INPUT);
  if (digitalRead(AUTO_MODE_PIN)) {
    auto_mode = true;
    Console.println("auto walking on!");
  }

  prev_walk = nullptr;
  walk = gaits[0];
  (tortobot.*(walk_t)walk)(1000);
  //
  cur_time = millis();
  prev_time = cur_time ;
}

void loop() {
  static char cmd = STAND, prev_cmd = FORWARD;

  if (Goble.available()) {
    int joystickX, joystickY;
    joystickY = Goble.readJoystickY();
    joystickX = Goble.readJoystickX();

    if (joystickY > 180) cmd = FORWARD;
    else if (joystickY < 90) cmd = BACKWARD;
    else if (joystickX > 180) cmd = RIGHT;
    else if (joystickX < 90) cmd = LEFT;
    else {
      prev_cmd = 'a';
      cmd = STAND;
    }

#ifdef __DEBUG__
    //    Console.print("Joystick Value: ");
    //    Console.print(joystickX);
    //    Console.print("  ");
    //    Console.println(joystickY);

    Console.print("recevied cmd ");
    Console.println(cmd);
#endif

    if (Goble.readSwitchSelect() == PRESSED) {
      auto_mode = !auto_mode;
      if (!auto_mode) {
          cmd=STAND;
      }
    }
    if (Goble.readSwitchStart() == PRESSED) {
      random_walk = !random_walk;
    }
  }

  if (auto_mode) {
    if (random_walk)
      walkPatterns(0);
    else
      walkPatterns(1);
    goto __loop_end;
  }

  //if (cmd != prev_cmd && !auto_mode) {
  if (cmd != prev_cmd) {
#ifdef __DEBUG__
    Console.print("prev_cmd ");
    Console.print(prev_cmd);
    Console.print(", cmd ");
    Console.println(cmd);
#endif
    switch (cmd) {
      case GO:
      case FORWARD:
        walk = gaits[0];
        prev_cmd = cmd;
        break;
      case LEFT_FRONT:
      case LEFT:
        walk = gaits[1];
        prev_cmd = cmd;
        break;
      case RIGHT_FRONT:
      case RIGHT:
        walk = gaits[3];
        prev_cmd = cmd;
        break;
      case RIGHT_BACK:
      case LEFT_BACK:
      case BACKWARD:
        walk = gaits[2];
        prev_cmd = cmd;
        break;
      case STAND:
        tortobot.home();
        walk = &tortobot.stand;
        prev_cmd = cmd;
        break;
    }  // end of switch
    (tortobot.*(walk_t)walk)(800);
  }

__loop_end:
  tortobot.refresh();
}

void walkPatterns(int pattern) {
  static int c = 0;
  cur_time = millis();
  if (cur_time - prev_time >= TIME_INTERVAL) {
    walk_t new_walk = prev_walk;
    prev_time = cur_time;
    do {
      switch (pattern) {
        case 0: c = (int)random(0, gaits_len + 1);
          new_walk = gaits[c];
          break;
        case 1:  c = c % gaits_len;
          new_walk = gaits[c++];
          break;
        default:
          pattern = 0;
      }
    } while (new_walk == prev_walk);
    prev_walk = walk;
    walk = new_walk;
    (tortobot.*(walk_t)walk)(1000);
  }
}
