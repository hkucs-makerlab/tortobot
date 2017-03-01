#ifndef tortobot_h
#define tortobot_h
#include <Servo.h>
#include "Octosnake.h"

#define MAX_LEG_SERVOS 4


#define FORWARD 'f'
#define LEFT 'l'
#define STAND 's'
#define RIGHT 'r'
#define BACKWARD 'b'

class TortoBot {
  public:
    TortoBot();
    void init();

    void home();
    void stand(float T = 1000);
    void forward(float T = 1000) {
      walk(false, T);
    }
    void backward(float T = 1000) {
      walk(true, T);
    }
    void walk(bool rev = false, float T = 1000);
    void turn(bool right = true, float T = 1000);
    void turnRight(float T = 1000);
    void turnLeft(float T = 1000);
    void setTrim(int index, int value) {
      trim[index] = value;
    }
    void storeTrim();
    void loadTrim();
    void refresh();
  private:
    int EEPROMReadWord(int p_address);
    void EEPROMWriteWord(int p_address, int p_value);
    Servo servo[MAX_LEG_SERVOS];
    Oscillator oscillator[MAX_LEG_SERVOS];
    int trim[MAX_LEG_SERVOS]; //for calibrating servo deviation

};

#endif

