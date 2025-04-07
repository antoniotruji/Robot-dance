// Antonio Trujillo Reino

#include <Servo.h>
#define LEFT 12
#define RIGHT 13
#define BUTTON 2
#define L2SENSOR A0
#define L1SENSOR A1
#define CSENSOR A2
#define R1SENSOR A3
#define R2SENSOR A4
#define BLACK 0
#define WHITE 1

class motor : public Servo
{
  public:
    motor(void) { _dir=1; _currentSpeed=0;}
    void go(int targetSpeed) {
      if (_currentSpeed < targetSpeed){
        _currentSpeed += delta;
      } else if (_currentSpeed > targetSpeed) {
        _currentSpeed -= delta;
      }
      writeMicroseconds(1500+_dir*_currentSpeed*2); // or so
    }
    void setDirection(bool there) {
      if(there)
        _dir = 1;
      else
        _dir = -1;
    }
  private:
    int _dir;
    int _currentSpeed;
    static constexpr int delta = 3;
};

motor leftMotor, rightMotor;

int move = 0, pin = 1, ignore = 1, split = 0;

// marks control
const bool follow = true; // true for standard, false for inverse
int decision = 0; //0 means left, 1 means right 

unsigned long current = 0;
unsigned long timer = 0;

// Gaps control
char instructions[] = {'f', 'r','l', 'r', 'l'};
unsigned long times[] = {2000, 800, 1500, 700, 700};

int numOfInstructions = 3, i = -1;
unsigned long instructionTime = 0, timeInCurrentGap = 0;
char currentInstruction;

int left2 = digitalRead(L2SENSOR);
int left1 = digitalRead(L1SENSOR);
int central = digitalRead(CSENSOR);
int right1 = digitalRead(R1SENSOR);
int right2 = digitalRead(R2SENSOR);

// Method to refresh sensors
void checkSensors(){
  left2 = digitalRead(L2SENSOR);
  left1 = digitalRead(L1SENSOR);
  central = digitalRead(CSENSOR);
  right1 = digitalRead(R1SENSOR);
  right2 = digitalRead(R2SENSOR);
}

// Method for the button
void checkButton(){
  pin = digitalRead(BUTTON);
  if(pin == 0){
    while(pin == 0) pin = digitalRead(BUTTON);
    if(move == 1) move = 0;
    else if(move == 0) move = 1;
  }
}

void forward(){
  leftMotor.go(12);
  rightMotor.go(12);
}

void rightturn(){
  leftMotor.go(12);
  rightMotor.go(-12);
}

void leftturn(){
  leftMotor.go(-12);
  rightMotor.go(12);
}

void stop(){
  leftMotor.go(0);
  rightMotor.go(0);
  move = 0;
}

// Methods for passing through the splits following the right path
void followLineLeft(){
  if(central == BLACK) forward();
  else if (left1 == BLACK) leftturn();
}

void followLineRight(){
  if(central == BLACK) forward();
  else if (right1 == BLACK) rightturn();
}

// Method to determine the split
void checkSplit(){
  if((right1 == BLACK) && (left1 == BLACK)){
    current = millis();
    if(current >= 1000){ 
      timer = millis();
      while(millis() - timer <= 300){// to decide when to avoid oher paths.
        if(decision == 0) followLineLeft();
         else followLineRight();
      }

      if(split) split = 0;
      else split = 1;
    }
  }
}

// Method to determine the type of mark
void checkMarks(){
  checkSensors();
  if(split == 0){
    if(right2 == BLACK){
      if(follow) decision = 1;
      else decision = 0;
    }else if(left2 == BLACK){
      if(follow) decision = 0;
      else decision = 1;
    } 
  }
}

void followLine(){
  checkSensors();
  if(right1 == BLACK) rightturn();
  else if(left1 == BLACK) leftturn();
  else if(central == BLACK) forward();

  if((left2 == BLACK) && (right2 == BLACK)){
    if(ignore == 0) stop();
    while((left2 == BLACK) && (right2 == BLACK)) checkSensors();
    ignore = 0;
  }
}

// Methods for driving during gaps
void gapForward(){
  leftMotor.go(12);
  rightMotor.go(12);
}

void gapRightturn(){
  leftMotor.go(30);
  rightMotor.go(0);
}

void gapLeftturn(){
  leftMotor.go(0);
  rightMotor.go(30);
}

// Method for restarting for possible future gaps
void restart(){
  i = -1;
  instructionTime = 0;
  timeInCurrentGap = 0;
}

// Method to manage gaps
void checkGap(){
  checkSensors();
  while((left2 == WHITE) && (left1 == WHITE) && (central == WHITE) && (right1 == WHITE) && (right2 == WHITE)){
    checkSensors();
    if(millis() - timeInCurrentGap > instructionTime){ 
      i += 1;
      if(i == numOfInstructions) i = 1;
      timeInCurrentGap = millis();
      currentInstruction = instructions[i];
      instructionTime = times[i];

    }

    if(currentInstruction == 'l') gapLeftturn();
    else if (currentInstruction == 'r') gapRightturn();
    else gapForward();
    
  }

  checkSensors();
  if((left2 == BLACK) && (left1 == WHITE) && (central == WHITE) && (right1 == WHITE)) while(central == WHITE){
    checkSensors();
    gapLeftturn();
  }

  if((right2 == BLACK) && (left1 == WHITE) && (central == WHITE) && (right1 == WHITE)) while(central == WHITE){
    checkSensors();
    gapRightturn();
  }
  restart();  
}

void setup() {
  Serial.begin(115200);
  leftMotor.attach(LEFT, 500, 2500);
  leftMotor.setDirection(true);
  rightMotor.attach(RIGHT, 500, 2500);
  rightMotor.setDirection(false);
  pinMode(L2SENSOR, INPUT);
  pinMode(L1SENSOR, INPUT);
  pinMode(CSENSOR, INPUT);
  pinMode(R1SENSOR, INPUT);
  pinMode(R2SENSOR, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  if(!follow) decision = 1;
}

void loop() {
  checkSensors();
  checkButton();

  if(move){
    checkMarks();
    checkSplit();
    checkGap();
    followLine();
  }else stop();

  delay(30);
}
