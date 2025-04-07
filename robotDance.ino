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

// instruction structure
struct instruction {
  char position[3];
  unsigned long time;
};

// Initialize variables
const int MAX_INSTRUCTIONS = 15;
instruction choreography[] = {
    {"A1N", 0},
    {"E1", 15000},
    {"B2", 35000},
    {"3A", 45000},
    {"4C", 56700},
    {"D2", 70000},
  };
int speedSet = 30;
int move = 0, pin = 1, buttonpressed=0;
int totalInstructions = sizeof(choreography) / sizeof(choreography[0]);
int currentInstruction = 1;
int currentRow = isDigit(choreography[0].position[0]) ? choreography[0].position[0] - '0' : choreography[0].position[1] - '0';
char currentColumn = isAlpha(choreography[0].position[0]) ? choreography[0].position[0] : choreography[0].position[1];
char* targetPosition;
char directions[] = {'N', 'E', 'S', 'O'};
char orientation = choreography[0].position[2];
unsigned long startTime = 0;

// pre-set choreography


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
    static constexpr int delta = 2;
};

motor leftMotor, rightMotor;

int left2 = digitalRead(L2SENSOR);
int left1 = digitalRead(L1SENSOR);
int central = digitalRead(CSENSOR);
int right1 = digitalRead(R1SENSOR);
int right2 = digitalRead(R2SENSOR);

void updateSensors() {
  left2 = digitalRead(L2SENSOR);
  left1 = digitalRead(L1SENSOR);
  central = digitalRead(CSENSOR);
  right1 = digitalRead(R1SENSOR);
  right2 = digitalRead(R2SENSOR);
}

void checkButton() {
  pin = digitalRead(BUTTON);
  if (pin == 0) {
    while (pin == 0) pin = digitalRead(BUTTON);
    move = move == 1 ? 0 : 1;
    buttonpressed++;
    startTime = millis();
  }
}

bool facingOutGrid(int row, char col, char orientation) {
  return ( (row == 1 && orientation == 'S') || (row == 5 && orientation == 'N') || (col == 'A' && orientation == 'O') || (col == 'E' && orientation == 'E') );
}

void rightTurn() {
  leftMotor.go(speedSet);
  rightMotor.go(-speedSet/2);
}

void leftTurn() {
  leftMotor.go(-speedSet/2);
  rightMotor.go(speedSet);
}

void moveForward() {
  leftMotor.go(speedSet);
  rightMotor.go(speedSet);
}

void stop() {
  leftMotor.go(0);
  rightMotor.go(0);
}

void lineFollowing() {
  if (central == BLACK) moveForward();
  else if (left1 == BLACK) leftTurn();
  else if (right1 == BLACK) rightTurn();
}

void moveForwardDir(int linesToCross) {
  int linesCrossed = 0;
  bool lineDetected = false;
  
  bool first= true;
  if(first){
    while((left2 == BLACK || right2 == BLACK)){
      lineFollowing();
      updateSensors();
    }
    first = false;
  }
  while (linesCrossed < linesToCross || (linesCrossed==linesToCross && lineDetected)) {
    updateSensors();
    lineFollowing();
    
    if ((left2 == BLACK || right2 == BLACK) && !lineDetected) {
      linesCrossed++;
      lineDetected = true;
    }

    // Check if line was passed
    if (left2 == WHITE && right2 == WHITE) {
      lineDetected = false;
    }
  }
  stop();
}

void rightTurnDir() {
  int linesCrossed = 0;
  int linesToCross = facingOutGrid(currentRow, currentColumn, orientation) ? 1 : 2;
  bool lineDetected = false;
  
  while (linesCrossed < linesToCross) {
    rightTurn();
    updateSensors();
    
    if ((left2 == BLACK || right2 == BLACK) && !lineDetected) {
      linesCrossed++;
      lineDetected = true;
    }
  
    if (left2 == WHITE && right2 == WHITE) {
      lineDetected = false;
    }

    if(linesCrossed==2){
      // until robot on the line
      while(central != BLACK){
        rightTurn();
        updateSensors();
      } 
    }
  }
}

void leftTurnDir() {
  int linesCrossed = 0;
  int linesToCross = facingOutGrid(currentRow, currentColumn, orientation) ? 1 : 2;
  bool lineDetected = false;
  
  while (linesCrossed < linesToCross) {
    updateSensors();
    leftTurn();

    if ((left2 == BLACK || right2 == BLACK) && !lineDetected) {
      linesCrossed++;
      lineDetected = true;
    }

    if (left2 == WHITE && right2 == WHITE) {
      lineDetected = false;
    } 

    if(linesCrossed==2){
      while(central!= BLACK){
        leftTurn();
        updateSensors();
      } 
    }
  }
}

void turnAroundDir() {
  int linesCrossed = 0;
  int linesToCross = facingOutGrid(currentRow, currentColumn, orientation) ? 1 : 3; // passes 3 lines when turning around inside the grid
  bool lineDetected = false;

  while (linesCrossed < linesToCross) {
    updateSensors();
    leftMotor.go(speedSet);
    rightMotor.go(-speedSet);
    
    if ((left2 == BLACK || right2 == BLACK) && !lineDetected) {
      linesCrossed++;
      lineDetected = true;
    }

    if (left2 == WHITE && right2 == WHITE) {
      lineDetected = false;
    }
  }
}

int getIndexDir(char dir) {
  for (int i = 0; i < 4; i++) {
    if (directions[i] == dir) return i;
  }
  return -1;
}

// handle the turns
void turnToDir(char currentOrientation, char targetOrientation) {
  int currentIndex = getIndexDir(currentOrientation);
  int targetIndex = getIndexDir(targetOrientation);

  int diff = (targetIndex - currentIndex+4) % 4;

  if (diff == 1) {
    rightTurnDir();
  } else if (diff == 3) {
    leftTurnDir();
  } else if (diff == 2) {
    turnAroundDir();
  }

  // update current orientation of the robot
  orientation = targetOrientation;
} 

void dance() {
  // Load target coordinates
  targetPosition = choreography[currentInstruction].position;
  int targetRow = isDigit(targetPosition[0]) ? targetPosition[0] - '0': targetPosition[1] - '0'; 
  char targetColumn = isAlpha(targetPosition[0]) ? targetPosition[0] : targetPosition[1];
  
  if (isDigit(targetPosition[0])) {
    // First to row
    int linesToCross = abs(targetRow - currentRow);
    
    if (targetRow > currentRow) turnToDir(orientation, 'N');
    else if (targetRow < currentRow) turnToDir(orientation, 'S');
    moveForwardDir(linesToCross);
    currentRow = targetRow;

    // Then to column
    linesToCross = abs(targetColumn - currentColumn);
    if (targetColumn > currentColumn) turnToDir(orientation, 'E');
    else if (targetColumn < currentColumn) turnToDir(orientation, 'O');
    moveForwardDir(linesToCross);
    currentColumn = targetColumn;

  } else {
    // First to column
    int linesToCross = abs(targetColumn - currentColumn);
    if (targetColumn > currentColumn) turnToDir(orientation, 'E');
    else if (targetColumn < currentColumn) turnToDir(orientation, 'O');
    moveForwardDir(linesToCross);
    currentColumn = targetColumn;

    // Then to row
    linesToCross = abs(targetRow - currentRow);
    if (targetRow > currentRow) turnToDir(orientation, 'N');
    else if (targetRow < currentRow) turnToDir(orientation, 'S');
    moveForwardDir(linesToCross);
    currentRow = targetRow;
  }
}

bool parseChoreographyInput(String input) {
  instruction newChoreography[MAX_INSTRUCTIONS];
  totalInstructions = 0;
  char *token = strtok((char *)input.c_str(), " ,;\t\r\n");
  if (strlen(token) < 2 || strlen(token) > 3) return false;
  strcpy(newChoreography[totalInstructions].position, token);
  Serial.println(newChoreography[totalInstructions].position);
  if (newChoreography[0].position[0] < 'A' || newChoreography[0].position[0] > 'E') return false;
  if(newChoreography[0].position[1] < '1' || newChoreography[0].position[1] > '5') return false;
  if (newChoreography[0].position[2] != 'N' && newChoreography[0].position[2] != 'S' && newChoreography[0].position[2] != 'O' && newChoreography[0].position[2] != 'E') return false;
  newChoreography[totalInstructions].time = 0;

  totalInstructions++;
  token = strtok(NULL, "T ,;\t\r\n");
  //Serial.println(MAX_INSTRUCTIONS);
  
  while (token != NULL && totalInstructions < MAX_INSTRUCTIONS) {
    if (strlen(token) < 2 || strlen(token) > 3) return false;
    strcpy(newChoreography[totalInstructions].position, token);
    Serial.println(newChoreography[totalInstructions].position);

    if (isAlpha(newChoreography[totalInstructions].position[0])){
      if (newChoreography[totalInstructions].position[0] < 'A' || newChoreography[totalInstructions].position[0] > 'E') return false;
      else if (newChoreography[totalInstructions].position[1] < '1' || newChoreography[totalInstructions].position[1] > '5') return false;
    } else {
      if (newChoreography[totalInstructions].position[1] < 'A' || newChoreography[totalInstructions].position[1] > 'E') return false;
      else if (newChoreography[totalInstructions].position[1] < '1' || newChoreography[totalInstructions].position[0] > '5') return false;
    }

    token = strtok(NULL, "T ,;\t\r\n");
    if (token == NULL) return false;
    newChoreography[totalInstructions].time = atol(token)*100;
    Serial.println(newChoreography[totalInstructions].time);
    token = strtok(NULL, "T ,;\t\r\n");
    totalInstructions++;
    //delay(10);
  }
  if (totalInstructions>0){
    memcpy(choreography, newChoreography, totalInstructions*sizeof(instruction));
    Serial.println("hola");
    for (int i=0; i<totalInstructions; i++){
      Serial.println(choreography[i].position);
      Serial.println(choreography[i].time);
    }

    
  }
  return totalInstructions > 0;
}

void checkSerialInput() {
  if (Serial.available()) {
    String input = "";
    while (Serial.available()){
      delay(4);
      char c = Serial.read();
      input+=c;
    }
    input.toUpperCase();

    if (parseChoreographyInput(input)) {
      Serial.println("Choreography saved!");
    } else {
      Serial.println("Invalid choreography format!");
    }
  }
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
  Serial.println("Ready for input!");

}

void loop() {
  checkSerialInput();

  updateSensors();
  checkButton();

  if (move) {
    unsigned long currentTime = millis() - startTime;

    if (currentInstruction < totalInstructions) {
      stop();
      if (currentTime >= choreography[currentInstruction-1].time) {
        dance();
        currentInstruction++;
      }
    } else {
      stop();
    }
  } else if((buttonpressed%2)==0){
      currentInstruction = 0;
      dance();
      turnToDir(orientation, choreography[0].position[2]);
  } else {
    stop();
  }

  delay(10);
}

//a1n c3t150 4bt225 5ct350
