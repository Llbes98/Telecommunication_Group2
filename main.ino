/*
  This is the main code for course 34338 Group 2's final project.
  The code is written for an Arduino Mega and it contains 6 puzzles along with code
  allowing the Arduino Mega to communicate with an ESP 8266 and Arduino UNO via Serial Communication.
  
*/

// -------------------- Load libraries --------------------

#include <AccelStepper.h>


// -------------------- Common Variables --------------------
bool gameActive = false;  // This is true whenever a game is currently playing.
int heartsFull = 3;       // The player has 3 lives. This will be reduced by one every time a mistake has been made.
int hearts = 3;           // The amount of hearts the player has left in this game.
int statusTimer = 0;      // Main status-updating timer
int previousStatusTimer = 0;


char gameSerial[9] = { '\0' }; //Init serial variable
int Wires = 0; // Give all games an ID to refer to later.
int Sequence = 1;
int Maintenance = 2;
int Toggles = 3;
int Ultrasonic = 4;
int Motor = 5;



// Side-mounted LEDs
int sideLEDs[3] = { 0, 0, 0 };  // check whether to turn them on or off.
int pinsSideLEDs[3] = {};       // Pins for side-mounted LEDs


// Names of the puzzles.
String puzzles[] = { "Wires",
                     "Sequence",
                     "Maintenance",
                     "Toggles",
                     "Ultrasonic",
                     "Motor" };

// gameCheck LEDs. These light ON when the game is finished. We also use these to check when the game has been completed.
int checkLEDs[6] = { 0, 0, 1, 0, 0, 0 };
int pinsCheckLEDs[6] = { 4, 53, 0, 33, 9, A0 };
int pinsCheckBtns[6] = { 5, 34, 26, 27, 8, A1 };
int prevStateBtn[6] = { 1, 1, 1, 1, 1, 1 };

bool debugArray[6] = { 0, 0, 0, 0, 0, 0 };


//

// -------------------- Puzzle Variables --------------------

// Puzzle 1 (Wires) variables:
bool wiresTargetConfig[] = { 0, 0, 0, 0 };
bool wiresCurrentConfig[] = { 0, 0, 0, 0 };

// Pins
int wirePins[4] = { 25, 24, 23, 22};
//int btnWires;

// Puzzle 2  (Sequence) variables:

// Pins
int sequencePinCLK = 37;
int sequencePinDT = 36;
int sequencePinSW = 35;

// Vars
int sequenceCounter = 6900000;
int sequenceSelectedPin = 0;
int sequenceCurrentStateA;
int sequenceLastStateA;
int sequencePrevStateSW = 1;
bool sequenceSelectorActive = false;

// Timers
int sequenceBlinkTimerPrev = millis();
int sequenceBlinkTimer = millis();


// Arrays to keep track of stuffs

bool sequenceEnableArray[5] = { 0, 0, 0, 0, 0 }; // Whether to turn on the LED or not

int sequencePinArray[5][3] = { { 52, 51, 50 }, // Pins of the R/G/B leds
                               { 49, 48, 47 },
                               { 46, 45, 44 },
                               { 43, 42, 41 },
                               { 40, 39, 38 } };

// Current and target color IDs 
int sequenceCurrentColor[5] = { 0, 0, 0, 0, 0 };
int sequenceTargetColor[5] = { 0, 0, 0, 0, 0 };

// Color sets that are ID'd above
int sequencePossibleColors[4][3] = { { 255, 0, 0 },
                                     { 0, 255, 0 },
                                     { 0, 0, 255 },
                                     { 228, 0, 200 } };

// Puzzle 3 (Maintenance) variables:
// Game settings
int maintenance = 100;
int lifeLossPerTick = 1;
int lifeGainPerTick = 7;

// Timers
int maintenanceDeathTimer = 0;
int maintenanceTimer = 0;
int maintenanceBlinkerTimer = 0;
int maintenancePreviousTimer = 0;
int maintenancePrevBlinkTimer = 0;
int maintenanceNotDeadTimer = 0;

// Pins
int maintenancePinsLED[2] = { 2, 3 };  // PWM~ pins for controlling the RGB LED. We dont need a pin for the blue channel.

// Puzzle 4 (Toggles) variables:
int togglesPins[5] = { 32, 31, 30, 29, 28 };
bool togglesCurrentConfig[5] = { 0, 0, 0, 0, 0 };
bool togglesTargetConfig[5] = { 0, 0, 0, 0, 0 };

// variables
int lastTSpress;

// configurations of switches
bool switch_config1[] = {1,1,0,0,1};
bool switch_config2[] = {1,0,1,0,1};
bool switch_config3[] = {0,1,0,0,1};
bool switch_config4[] = {1,0,1,0,0};
bool switch_config5[] = {0,0,1,1,1};

// Puzzle 5 (Ultrasonic) variables:

// Pins
int ultrasonicPinEcho = 6;  // Echo
int ultrasonicPinTrig = 7;  // Trigger

// Variables
long ultrasonicDuration, ultrasonicMM;
int ultrasonicCorrectDistance = 0;


// Puzzle 6 (Motor) variables:

int motorFullStep = 4;

// Game params
float motorTargetSegment = 3.0;
float motorTolerance = 0.5;
float motorCurrentSegment;
int motorDistanceFromHome = 0;

// Creates an instance
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper motorStepper(motorFullStep, 10, 11, 12, 13);

// -------------------- Functions --------------------

// General functions:

//Serial command help functions
// Send command to the UNO handling the LCD display.
void LCD_UNO(String command) {
  Serial1.println(command);
  Serial.println("");
  Serial.print((String) "\"" + command + "\" sent to LCD_UNO on Serial1\n");
}

// Send command to the ESP.
void ESP_CMD(String command) {
  Serial2.println(command);
  Serial.println((String) "\"" + command + "\" sent to ESP8266 on Serial2\n");
}

// Checklist of things to do whenever the game is starting.
void start() {
  generateSerial();
  generateSideLEDs();
  initializeWires();
  initializeSequence();
  initializeToggles();
  initializeUltrasonic();
  initializeMotor();

  Serial.println("Get ready to start in");
  delay(1000);
  Serial.print("3... ");
  delay(1000);
  Serial.print("2... ");
  delay(1000);
  Serial.print("1... ");
  delay(1000);

  hearts = heartsFull;  // Reset hearts.
  maintenance = 100;    // Reset maintenance back to full (100).
  gameActive = true;    // Set game to ACTIVE

  sequenceBootUp();
  LCD_UNO("start");  // Send start command to LCD_UNO
  ESP_CMD("hearts=3"); // Show 3 hearts on node red
  Serial.println("GO!");
  Serial.println("#########################");
}

// Generate random serial number. 100% original code, totally not autogenerated code by ChatGPT
void generateSerial() {
  randomSeed(analogRead(0));
  for (int i = 0; i < 8; i++) {
    char c;
    do {
      c = '0' + random(36);
      if (c > '9') {
        c += ('A' - '0' - 10);
      }
    } while (c == 'O');
    gameSerial[i] = c;
  }

  gameSerial[8] = '\0';
  Serial.println((String) "Game serial this round: " + gameSerial);
  LCD_UNO((String) "Serial=" + gameSerial);  //Send serial to LCD UNO
}

// Side LEDs randomization:
void generateSideLEDs() {
  Serial.println("");
  Serial.print("Side LEDs config: ");
  for (int i = 0; i < 3; i++) {
    sideLEDs[i] = random(2);
    Serial.print(sideLEDs[i]);
  }

  LCD_UNO((String) "SideLEDs=" + sideLEDs[0] + sideLEDs[1] + sideLEDs[2]);
}




// IMPORTANT: Runs on every game tick. Keeps track of the status of the game, updates gamecheck LEDs, listens for serial commands
void updateGameStatus() {
  statusTimer = millis();
  updateGameCheckLEDs();
  motorUpdate();


  if ((winCondition()) && (gameActive)) {
    Serial.println("Win condition passed.");
    win();
  }


  if (statusTimer - previousStatusTimer > 2000) {
    previousStatusTimer = statusTimer;

    if (!gameActive) {
      Serial.println("Send \"start\" to start a new game.");
    }
  }

  // If hearts <= 0 (all lives spent), then call gameover().
  if ((hearts <= 0) and (gameActive)) {
    gameover();
  }

  // Scans for serial cmds on the standard serial (serial monitor)
  if (Serial.available() > 0) {                 // Check serial stream for message.
    String msg = Serial.readStringUntil('\n');  // Read the serial data and store it
    msg.trim();                                 // Trims data for end line information
    Serial.println((String) "Incoming msg on standard serial: " + msg);

    if ((msg == "start") and (!gameActive)) {  // If message is "start" then start game.
      start();
    }
    if ((msg == "end") and (gameActive)) {  // If message is "end" then quit game.
      gameover();
    }
    if (msg.substring(0, 6) == "debug=") {
      debugArray[msg.substring(6, 7).toInt()] = !(debugArray[msg.substring(6, 7).toInt()]);
    }
  }

  // Checking for serial information sent from the ESP8266
  if (Serial2.available() > 0) {                 // Check serial stream for message.
    String msg = Serial2.readStringUntil('\n');  //Read the serial data and store it
    msg.trim();                                  // Trims data for end line information
    Serial.println(msg);
    if (msg.substring(0, 11) == "From MQTT: ") {
      msg.remove(0, 11);
      Serial.println((String) "Incoming msg on Serial2: " + msg);

      // Changing the difficulty.
      if (msg.substring(0, 7) == "hearts=") {
        heartsFull = msg.substring(7, 8).toInt();
        Serial.println((String) "Hearts set to " + heartsFull + ".");
      }

      // Start the game
      if ((msg == "start") and (!gameActive)) {  // If message is "start" then start game.
        start();
      }

      // Stop the game
      if ((msg == "gameover") and (gameActive)) {  // If message is "gameover" then stop game.
        gameover();
      }
    }
  }
}

// Sets LEDs ON/OFF wrt. puzzle completion.
void updateGameCheckLEDs() {
  for (int i = 0; i < 6; i++) {
    if (i != 2) {  //dont check maintenance LED (it doesnt exist)
      if (checkLEDs[i]) {
        digitalWrite(pinsCheckLEDs[i], HIGH);
      } else {
        digitalWrite(pinsCheckLEDs[i], LOW);
      }
    }
  }
}

// Function that flags puzzle as completed.
void puzzleFinished(int puzzleID) {
  Serial.println((String) "Completed " + puzzles[puzzleID] + ".");
  checkLEDs[puzzleID] = true;
  ESP_CMD((String) "complete=" + puzzleID);
  LCD_UNO("modulesolved");
}





// Checks whether all puzzles have been completed.
bool winCondition() {
  for (int i = 0; i < 6; i++) {
    if (!checkLEDs[i]) {
      return false;
    }
  }
  return true;
}

// Is called when winCondition is passed
void win() {
  Serial.println("YOU WON THE GAME!");
  LCD_UNO("win");
  ESP_CMD("win");
  gameActive = false;
  delay(3000);
}

// Function to be called whenever a heart is lost.
void loseHeart(int puzzleID) {
  hearts--;
  ESP_CMD((String) "hearts=" + hearts);
  LCD_UNO("lifelost");
  Serial.println((String) "Mistake made in " + puzzles[puzzleID] + ". Lives remaining: " + hearts);
}

// Things to do when the game is lost or otherwise ended.
void gameover() {
  gameActive = false;
  ESP_CMD("gameover");
  resetCheckPins();
  sequenceShutdown();
  maintenanceShutdown();


  int checkLEDs[6] = { 0, 0, 1, 0, 0, 0 };
  LCD_UNO("gameover");  // Send game over command to LCD UNO.
  Serial2.println("gameover");

  Serial.println("Game over.");
  delay(1000);
  Serial.println("#########################");
  delay(3000);
}

void resetCheckPins() {
  for (int i = 0; i < 6; i++) {
    checkLEDs[i] = 0;
  }
  checkLEDs[2] = 1;  //Set maintenance status to completed.
}


// Puzzle 1 (Wires) functions:

void initializeWires() {
  //Start by finding the correct sequence.
  // First wire is about finding two numbers in the serial. Connect if yes.
  wiresTargetConfig[0] = containsTwoNumbers(gameSerial);

  // Second wire is checking if there's just 1 lit side LED.
  int sumSideLEDs = 0;
  for (int i = 0; i < 3; i++) {
    if (sideLEDs[i]) {
      sumSideLEDs++;
    }
  }
  if (sumSideLEDs == 1) {
    wiresTargetConfig[1] = 1;
  }

  // Third wire is XOR-ing the first two.
  wiresTargetConfig[2] = (wiresTargetConfig[0] ^ wiresTargetConfig[1]);

  // The fourth wire is connected if there is an even number of connected cables above.
  if (((wiresTargetConfig[0]) + (wiresTargetConfig[1]) + (wiresTargetConfig[2])) % 2 == 0) {
    wiresTargetConfig[3] = 1;
  }

  Serial.println("");
  Serial.print("Target wire config: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(wiresTargetConfig[i]);
  }
}

bool wiresCheckConfig() {
  for (int i = 0; i < 4; i++) {
    if (wiresCurrentConfig[i] != wiresTargetConfig[i]) {
      return false;
    }
  }
  return true;
}

void wiresUpdateCurrentConfig() {
  for (int i = 0; i < 4; i++) {
    wiresCurrentConfig[i] = !digitalRead(wirePins[i]);
  }
}


// Puzzle 2 (Sequence) functions:

bool containsTwoNumbers(char* arr) {
  int count = 0;
  for (int i = 0; i < 8; i++) {
    if (arr[i] >= '0' && arr[i] <= '9') {
      count++;
      if (count >= 2)
        return true;
    }
  }
  return false;
}

// Set all current colors to red and set targetColor.
void initializeSequence() {
  for (int i = 0; i < 5; ++i) {
    sequenceCurrentColor[i] = 0;
    sequenceTargetColor[i] = random(4);  // change this to be an actual rule
  }

  Serial.println("");
  Serial.print("Sequence target colors: ");
  for (int i = 0; i < 5; ++i) {
    Serial.print(sequenceTargetColor[i]);
  }
}

void sequenceBootUp() {
  sequenceSelectorActive = false;
  //delay(1000);
  for (int i = 0; i < 5; ++i) {
    sequenceEnableArray[i] = true;
    sequenceUpdateLEDs();
    delay(100);
  }
}

void sequenceShutdown() {
  sequenceSelectorActive = false;

  for (int i = 0; i < 5; ++i) {
    sequenceEnableArray[i] = false;
    sequenceUpdateLEDs();
    delay(100);
  }
}

// Continously run to update the LEDs.
void sequenceUpdateLEDs() {
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (sequenceEnableArray[i]) {
        digitalWrite(sequencePinArray[i][j], sequencePossibleColors[sequenceCurrentColor[i]][j]); //this sucked to write 
      } else {
        digitalWrite(sequencePinArray[i][j], 0);
      }
    }
  }
}

// To be run when the check btn is pressed. 
bool sequenceCheckConfig() {

  for (int i = 0; i < 5; ++i) {
    if (sequenceCurrentColor[i] != sequenceTargetColor[i]) {

      Serial.println((String) "Sequence config wrong at index=" + i);
      return false;
    }
  }

  return true;
}

// Code to read rotation/press of the rotary encoder. 
void sequenceUpdateEncoder() {
  sequenceCurrentStateA = digitalRead(sequencePinCLK);
  if (sequenceCurrentStateA != sequenceLastStateA && sequenceCurrentStateA == 1) {
    sequenceSelectorActive = true;
    for (int i = 0; i < 5; ++i) {
      sequenceEnableArray[i] = true;
    }
    if (digitalRead(sequencePinDT) != sequenceCurrentStateA) {
      sequenceCounter++;
    } else {
      sequenceCounter--;
    }
    sequenceSelectedPin = sequenceCounter % 5;
  }
  sequenceLastStateA = sequenceCurrentStateA;
}

// Blink the currently selected LED
void sequenceCheckEnable() {
  sequenceBlinkTimer = millis();
  if (sequenceSelectorActive) {
    if (sequenceBlinkTimer - sequenceBlinkTimerPrev > 100) {
      sequenceBlinkTimerPrev = sequenceBlinkTimer;
      sequenceEnableArray[sequenceSelectedPin] = !sequenceEnableArray[sequenceSelectedPin];
    }
  }
}

// Cycle through the available colors on click
void sequenceSwapColor(int sequenceSelectedPin) {
  sequenceCurrentColor[sequenceSelectedPin] = (sequenceCurrentColor[sequenceSelectedPin] + 1) % 4;
}

// Puzzle 3 (Maintenance) functions:

void maintenanceShutdown() {
  digitalWrite(maintenancePinsLED[0], LOW);
  digitalWrite(maintenancePinsLED[1], LOW);
}

// Puzzle 4 (Toggles) functions:

/*void initializeToggles() {
  Serial.println("");
  Serial.print("Target Toggle positions: ");
  for (int i = 0; i < 5; i++) {
    togglesTargetConfig[i] = random(2);
    Serial.print(togglesTargetConfig[i]);
  }
} */

// Continuously run to update the state of the toggles 
void togglesUpdateSwitchConfig() {
  for (int i = 0; i < 5; i++) {
    togglesCurrentConfig[i] = !digitalRead(togglesPins[i]);
  }
}

// To be run when the check btn is pressed. 
bool togglesCheckConfig() {
  for (int i = 0; i < 5; i++) {
    if (togglesCurrentConfig[i] != togglesTargetConfig[i]) {
      return false;
    }
  }
  return true;
}

bool containsLetter(char* arr, char* letter) {
  for (int i = 0; i < 8; i++) {
    if (arr[i] == letter) {
      return true;
    }
  }
  return false;
}

// Finds highest num in char array (Lucas)
int highestNum(char* arr) {
  int x = 0;
  for (int i = 0; i < 8; i++) {
    if (arr[i] == '1') {
        if (x < 1) { 
          x = 1;
        }
    } else if (arr[i] == '2') {
        if (x < 2) { 
          x = 2;
        }
    } else if (arr[i] == '3') {
        if (x < 3) { 
          x = 3;
        }
    } else if (arr[i] == '4') {
        if (x < 4) { 
          x = 4;
        }
    } else if (arr[i] == '5') {
        if (x < 5) { 
          x = 5;
        }
    } else if (arr[i] == '6') {
        if (x < 6) { 
          x = 6;
        }
    } else if (arr[i] == '7') {
        if (x < 7) { 
          x = 7;
        }
    } else if (arr[i] == '8') {
        if (x < 8) { 
          x = 8;
        }
    } else if (arr[i] == '9') {
        if (x < 9) { 
          x = 9;
        }
    } 
  } 
  return x;
}

// Finds count of numbers in array (Lucas)
int numberOfChars(char* arr) {
  int x;
  for (int i = 0; i < 8; i++) {
    if (arr[i] == '1' || arr[i] == '2' || arr[i] == '3' || arr[i] == '4' || arr[i] == '5' 
    || arr[i] == '6' || arr[i] == '7' || arr[i] == '8' || arr[i] == '9' || arr[i] == '0') {
      x = x;
    } else {
      x++;
    }
  }
  return x;
}

// Function to return # of lit side LEDs.
int checkSideLEDs() {
  int x;
  for (int i = 0; i < 3; i++) {
    if (sideLEDs[i] == 1){
      x++;
    }
  }
  return x;
}

// Set target toggles positions based on rules (Lucas)
void initializeToggles() {
    if (containsLetter(gameSerial, 'T') || containsLetter(gameSerial, 'S')) {
      for (int i = 0; i < 5; i++) {
        togglesTargetConfig[i] = switch_config1[i];
      }
    }
    else if (checkSideLEDs() == 3) {
      for (int i = 0; i < 5; i++) {
        togglesTargetConfig[i] = switch_config2[i];
      }
    }
    else if (checkSideLEDs() != 0 && highestNum > 4) {
      for (int i = 0; i < 5; i++) {
        togglesTargetConfig[i] = switch_config3[i];
      }
    }
    else if (numberOfChars(gameSerial) == 4) {
      for (int i = 0; i < 5; i++) {
        togglesTargetConfig[i] = switch_config4[i];
      }
    }
    else {
      for (int i = 0; i < 5; i++) {
        togglesTargetConfig[i] = switch_config5[i];
      }
    }

  Serial.println("");
  Serial.print("Target Toggle positions: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(togglesTargetConfig[i]);
  }
}

// Puzzle 5 (Ultrasonic) functions:

bool containsLetterE(char* arr) {
  for (int i = 0; i < 8; i++) {
    if (arr[i] == 'E') {
      return true;
    }
  }
  return false;
}

// Set target distance (Sarah)
void initializeUltrasonic() {
  //1. If the serial number incudes the letter “E”. The desired distance is 3 cm.
  if (containsLetterE(gameSerial)) {
    ultrasonicCorrectDistance = 30;

    // 2. If not and there are two numbers in the serial number. The desired distance is 6 cm
  } else if (containsTwoNumbers(gameSerial)) {
    ultrasonicCorrectDistance = 60;

    //3. If the other 2 don’t apply and the serial number has a length of 8, The desired distance is 9 cm.
  } else {
    ultrasonicCorrectDistance = 90;
  }
  Serial.println("");
  Serial.println((String) "Target ultrasonic distance: " + ultrasonicCorrectDistance + "mm");
}

// Puzzle 6 (Motor) functions:

void initializeMotor() {
  motorTargetSegment = float(random(7) + 2.0);
  Serial.println((String) "Motor target segment: " + motorTargetSegment);
}

// To be run continuously when gameActive
void motorUpdate() {

  motorCurrentSegment = (float((motorDistanceFromHome) / 2038.0)) * 10 + 1;

  if (!gameActive) {
    if (motorDistanceFromHome != 0) {
      motorStepper.run();
    }
  }


  if (gameActive) {
    motorStepper.move(69420);
    motorStepper.run();
  }

  // Finds the num of steps past the home pos. Used to return to home on game end.
  motorDistanceFromHome = motorStepper.currentPosition() % 2038;

  if (debugArray[Motor] || true) {
    Serial3.println((String) "Segment: " + motorCurrentSegment + "/" + motorTargetSegment + "    Pos: " + motorStepper.currentPosition() + "    motorDistanceFromHome: " + motorDistanceFromHome + "    gameActive: " + gameActive);
  }
}


void setup() {
  // General setup:
  Serial.begin(115200);   // Serial is for output on the Serial monitor
  Serial2.begin(115200);  // Serial2 communicates with the ESP8266 on TX2 and RX2 (pin 16 and 17)
  Serial1.begin(115200);  // Serial1 communicates with the LCD UNO on TX1 (pin 18)
  Serial3.begin(115200);  // Serial3 communicates with the AUXILIARY UNO on TX3 (pin 14)

  Serial.println("");
  Serial.println("####################################");
  Serial.println("#           DON'T EXPLODE          #");
  Serial.println("####################################");

  // Set check buttons as INPUT_PULLUP
  for (int i = 0; i < 6; i++) {
    pinMode(pinsCheckBtns[i], INPUT_PULLUP);
  }

  // Set check LEDs as OUTPUT
  for (int i = 0; i < 6; i++) {
    pinMode(pinsCheckLEDs[i], OUTPUT);
  }

  //Init all prevBtnState variables.
  for (int i = 0; i < 6; i++) {
    prevStateBtn[i] = digitalRead(pinsCheckBtns[i]);
  }


  // Puzzle 1 (Wires) Setup:
  for (int i = 0; i < 4; i++) {
    pinMode(wirePins[i], INPUT_PULLUP);
  }


  // Puzzle 2 (Sequence) Setup:

  pinMode(sequencePinCLK, INPUT);
  pinMode(sequencePinDT, INPUT);
  pinMode(sequencePinSW, INPUT_PULLUP);
  sequenceLastStateA = digitalRead(sequencePinCLK);  // Initialize encoder

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      pinMode(sequencePinArray[i][j], OUTPUT);
    }
  }

  // Puzzle 3 (Maintenance) Setup:

  pinMode(maintenancePinsLED[0], OUTPUT);
  pinMode(maintenancePinsLED[1], OUTPUT);

  // Puzzle 4 (Toggles) Setup:

  for (int i = 0; i < 5; ++i) {
    pinMode(togglesPins[i], INPUT_PULLUP);
  }


  // Puzzle 5 (Ultrasonic) Setup:

  pinMode(ultrasonicPinTrig, OUTPUT);
  pinMode(ultrasonicPinEcho, INPUT);


  // Puzzle 6 Setup:

  motorStepper.setMaxSpeed(1000.0);
  motorStepper.setAcceleration(20000.0);
  motorStepper.setSpeed(4000);
  motorStepper.setCurrentPosition(0);

  //
}

void loop() {

  // -------------------- Common loops --------------------

  if (gameActive) {  // only loop if game has been started. This is changed in the start() function.

    // Everything puzzle-related should come below this point:

    // -------------------- Puzzle 1 (Wires) loops --------------------

    // Read current config of wires:
    if (debugArray[Wires]) {
      Serial.println("Current Wires config: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(wiresCurrentConfig[i]);
      }
      Serial.print(" ---- Target Wires config: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(wiresTargetConfig[i]);
      }
    }

    wiresUpdateCurrentConfig();


    // When button is pressed, check if wire config is the same as wiresTargetConfig.
    if ((digitalRead(pinsCheckBtns[Wires]) == 0) && (digitalRead(pinsCheckBtns[Wires]) != prevStateBtn[Wires])) {

      if (wiresCheckConfig()) {
        puzzleFinished(Wires);

      } else {
        loseHeart(Wires);
      }
    }
    prevStateBtn[Wires] = digitalRead(pinsCheckBtns[Wires]);



    // -------------------- Puzzle 2 (Sequence) loops --------------------

    sequenceCheckEnable();
    sequenceUpdateLEDs();
    sequenceUpdateEncoder();

    if (debugArray[Sequence]) {
      Serial.println("Current Sequence color config: ");
      for (int i = 0; i < 5; i++) {
        Serial.print((String)sequenceCurrentColor[i] + " ");
      }
      Serial.print("----- Sequence target color config: ");
      for (int i = 0; i < 5; i++) {
        Serial.print((String)sequenceTargetColor[i] + " ");
      }
    }


    // Rotary btn
    if ((digitalRead(sequencePinSW) == 0) && (digitalRead(sequencePinSW) != sequencePrevStateSW)) {
      sequenceSelectorActive = true;
      sequenceSwapColor(sequenceSelectedPin);
    }
    sequencePrevStateSW = digitalRead(sequencePinSW);

    // Check btn
    if ((digitalRead(pinsCheckBtns[Sequence]) == 0) and (digitalRead(pinsCheckBtns[Sequence]) != prevStateBtn[Sequence])) {

      if (sequenceCheckConfig()) {
        sequenceSelectorActive = false;
        sequenceEnableArray[sequenceSelectedPin] = true;
        puzzleFinished(Sequence);
        sequenceShutdown();


      } else {
        loseHeart(Sequence);
      }
    }
    prevStateBtn[Sequence] = digitalRead(pinsCheckBtns[Sequence]);


    // -------------------- Puzzle 3 (maintenance) loops --------------------

    // Display maintenance status
    if (debugArray[Maintenance]) {
      Serial.println((String) "Maintenance: " + maintenance + "%");
    }

    // Decrease maintenance every x ms.
    maintenanceTimer = millis();
    if (maintenanceTimer - maintenancePreviousTimer > 200) {
      maintenancePreviousTimer = maintenanceTimer;

      // If button pushed: increase, otherwise decrease.
      if (!digitalRead(pinsCheckBtns[Maintenance])) {
        maintenance += lifeGainPerTick;
      } else {
        maintenance -= lifeLossPerTick;
      }
      maintenance = constrain(maintenance, 0, 100);
    }


    // Control RGB LED based on % remaining.
    int colorMidPoint = 60;
    if ((maintenance <= 100) and (maintenance > colorMidPoint)) {
      // Som maintenance går fra 100 til 51 skal R gå fra 0 til 200
      // Som maintenance går fra 100 til 51 skal G gå fra 255 til 50 osv...
      analogWrite(maintenancePinsLED[0], map(maintenance, 100, colorMidPoint + 1, 0, 200));
      analogWrite(maintenancePinsLED[1], map(maintenance, 100, colorMidPoint + 1, 255, 50));
    }
    if ((maintenance <= colorMidPoint) and (maintenance > 0)) {
      analogWrite(maintenancePinsLED[0], map(maintenance, colorMidPoint, 0, 200, 255));
      analogWrite(maintenancePinsLED[1], map(maintenance, colorMidPoint, 0, 50, 0));
    }


    // If maintenance depleted, blink LED and start deathcounter.
    if (maintenance == 0) {
      analogWrite(maintenancePinsLED[1], 0);
      //maintenanceSetLED(1, 0);
      maintenanceBlinkerTimer = millis();
      maintenanceDeathTimer = millis();

      // Making blinks happen
      if (maintenanceBlinkerTimer - maintenancePrevBlinkTimer > 100) {
        maintenancePrevBlinkTimer = maintenanceBlinkerTimer;

        if (digitalRead(maintenancePinsLED[0])) {
          digitalWrite(maintenancePinsLED[0], LOW);
        } else {
          digitalWrite(maintenancePinsLED[0], HIGH);
        }
      }

      // Check if maintenance has been at 0% for longer than 3 secs. If yes, deduct a heart.
      if (maintenanceDeathTimer - maintenanceNotDeadTimer > 3000) {
        maintenanceNotDeadTimer = maintenanceDeathTimer;
        loseHeart(Maintenance);
      }
    } else {
      maintenanceNotDeadTimer = millis();
    }

    // -------------------- Puzzle 4 (Toggles) loops --------------------

    if (debugArray[Toggles]) {
      Serial.println("Current Toggles state: ");
      for (int i = 0; i < 5; i++) {
        Serial.print(togglesCurrentConfig[i]);
      }
      Serial.print(" ---- Toggles target config: ");
      for (int i = 0; i < 5; i++) {
        Serial.print(togglesTargetConfig[i]);
      }
    }

    togglesUpdateSwitchConfig();

    if ((digitalRead(pinsCheckBtns[Toggles]) == 0) and (digitalRead(pinsCheckBtns[Toggles]) != prevStateBtn[Toggles])) {

      if (togglesCheckConfig()) {
        puzzleFinished(Toggles);
      } else {
        loseHeart(Toggles);
      }
    }
    prevStateBtn[Toggles] = digitalRead(pinsCheckBtns[Toggles]); //
    

    // -------------------- Puzzle 5 (Ultrasonic) loops --------------------

    if ((digitalRead(pinsCheckBtns[Ultrasonic]) == 0) and (digitalRead(pinsCheckBtns[Ultrasonic]) != prevStateBtn[Ultrasonic])) {  // Read the distance when the button is hit
      digitalWrite(ultrasonicPinTrig, LOW);                                                                                        // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
      delayMicroseconds(5);
      digitalWrite(ultrasonicPinTrig, HIGH);
      delayMicroseconds(10);
      digitalWrite(ultrasonicPinTrig, LOW);

      ultrasonicDuration = pulseIn(ultrasonicPinEcho, HIGH);  // ultrasonicDuration is the time (in microseconds) from the sending
      ultrasonicMM = (ultrasonicDuration / 2) * 0.343;        // Convert the time into a distance

      if (debugArray[Ultrasonic]) {
        Serial.print((String) "Current Ultrasonic distance (mm): " + ultrasonicMM);
      }

      if ((ultrasonicCorrectDistance == 30) and (ultrasonicMM <= 45)) {
        puzzleFinished(Ultrasonic);
      }

      else if ((ultrasonicCorrectDistance == 60) and (ultrasonicMM > 45) and (ultrasonicMM < 75)) {
        puzzleFinished(Ultrasonic);
      }

      else if ((ultrasonicCorrectDistance == 90) and (ultrasonicMM >= 75)) {
        puzzleFinished(Ultrasonic);

      } else {  //take one life off
        loseHeart(Ultrasonic);
      }
    }
    prevStateBtn[Ultrasonic] = digitalRead(pinsCheckBtns[Ultrasonic]);

    // -------------------- Puzzle 6 loops --------------------

    if (gameActive) {



      if ((digitalRead(pinsCheckBtns[Motor]) == 0) and (digitalRead(pinsCheckBtns[Motor]) != prevStateBtn[Motor])) {  // Read the distance when the button is hit
        if ((motorCurrentSegment <= motorTargetSegment + motorTolerance) && (motorCurrentSegment >= motorTargetSegment - motorTolerance)) {
          puzzleFinished(Motor);
        } else {
          loseHeart(Motor);
        }
      }
      prevStateBtn[Motor] = digitalRead(pinsCheckBtns[Motor]);
    }
  }

  updateGameStatus();
  //delay(1);  // Main delay.
}
