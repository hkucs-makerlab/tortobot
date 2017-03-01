#include "Octosnake.h"
#include "TortoBot.h"

#define TIME_INTERVAL 8000
#define CAL_TRIGGER_PIN 12
#define LED_PIN A0

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
bool state = true;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A7));

  gaits_len = sizeof(gaits) / sizeof(walk_t);
  state = auto_mode = true;

  tortobot.init();
  tortobot.home();
  delay(2000);

  //begin: triggering delay for servo calibrating
  pinMode(CAL_TRIGGER_PIN, OUTPUT);
  digitalWrite(CAL_TRIGGER_PIN, 0);
  pinMode(CAL_TRIGGER_PIN, INPUT);
  while (digitalRead(CAL_TRIGGER_PIN)) {
    analogWrite(LED_PIN, 128 * state); // on calibarting indication LED
    delay(1000);
    state = !state;
  }
  analogWrite(LED_PIN, 0); // off calibarting indication LED
  //end:

  prev_walk = nullptr;
  walk = gaits[0];
  (tortobot.*(walk_t)walk)(1000);

  cur_time = millis();
  prev_time = cur_time ;
}



void loop() {
  if (auto_mode) {
    if (random_walk)
      walkPatterns(0);
    else
      walkPatterns(1);
  }

  if (Serial.available () > 0) {
    state = !state;
    analogWrite(LED_PIN, 128 * state); //flashing LED indicating cmd received
    static char cmd = STAND, prev_cmd = 0;
    cmd = Serial.read ();
    if (1 && cmd != prev_cmd && !auto_mode) {
__begin:
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

    if (1) {
      static short receivedNumber = 0;
      static bool negative = false;
      static byte channel = 1;
      switch (cmd) {
        case 'L':
          tortobot.loadTrim();
          break;
        case 'S':
          tortobot.storeTrim();
          break;
        case  '<':  // fall through to start a new number with prefix x,y char
          receivedNumber = 0;
          channel = 0;
          negative = false;
          break;
        case 'x':
          channel = 1;
          break;
        case 'y':
          channel = 2;
          break;
        case 'z':
          channel = 3;
          break;
        case 'Z':
          channel = 4;
          break;
        case 'a':
          channel = 5;
          break;
        case 'A':
          channel = 6;
          break;
        case '0' ... '9':
          receivedNumber *= 10;
          receivedNumber += cmd - '0';
          break;
        case '-':
          negative = true;
          break;
        case '>': // end of getting a number in format of <[x,y,z,a]number>
          if (negative) receivedNumber = -receivedNumber;
          switch (channel) {
            case 1:
              if (receivedNumber >= 127) {
                state = auto_mode = false;
                analogWrite(LED_PIN, 0);
                cmd = STAND;
                goto __begin;
              }
              else {
                state = auto_mode = true;
                analogWrite(LED_PIN, 128);
              }
              break;
            case 2:
              if (receivedNumber >= 127)
                random_walk = true;
              else
                random_walk = false;
              break;
            case 3:
              if (auto_mode) break;
              tortobot.setTrim(0, receivedNumber);
              tortobot.home();
              break;
            case 4:
              if (auto_mode) break;
              tortobot.setTrim(1, receivedNumber);
              tortobot.home();
              break;
            case 5:
              if (auto_mode) break;
              tortobot.setTrim(2, receivedNumber);
              tortobot.home();
              break;
            case 6:
              if (auto_mode) break;
              tortobot.setTrim(3, receivedNumber);
              tortobot.home();
              break;
          }  // end of inner switch
          break;
      } // end of outter switch
    }
  }
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



