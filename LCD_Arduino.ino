/*
  Communication between Arduino and Arduino Mega
*/

#include <LiquidCrystal.h>
#include <FastLED.h>

// Variable to store data sent from Arduino Mega
String msg;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int RedLed[] = {8,9,10};
int SideLEDs[] = {0,0,0};


byte topL[8] = {
  0b00000,
  0b00001,
  0b00011,
  0b01111,
  0b01111,
  0b11110,
  0b11110,
  0b11000
};

byte topM[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00100,
  0b00100,
  0b00100
};

byte topR[8] = {
  0b00000,
  0b10000,
  0b11000,
  0b11110,
  0b11110,
  0b01111,
  0b01111,
  0b00011
};

byte botL[8] = {
  0b11000,
  0b11111,
  0b11111,
  0b01111,
  0b00111,
  0b00011,
  0b00011,
  0b00011
};

byte botM[8] = {
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b11111,
  0b01110,
  0b01110,
  0b01110
};

byte botR[8] = {
  0b00011,
  0b11111,
  0b11111,
  0b11110,
  0b11100,
  0b11000,
  0b11000,
  0b11000
};


// Countdown timer variables
bool countdownActive = false;
int timeRemaining;
char timeRemainingFormatted[6];
int countdownTimer = 0;
int countdownTimerPrevious = 0;
int standbyTimer = 0;
int standbyTimerPrev = 0;
String gameSerial = "";

// LED Strip variables
#define NUM_LEDS 23
#define DATA_PIN 6


CRGB leds[NUM_LEDS];

bool isRed = 1;
int Green_channel = 0;
long lastChange;
long lastBlink;

int blinkCount = 0;
int lightMode = 0;
int endDelay = 150;
bool blink;

void gameStart() {
  timeRemaining = 300;  // Set game length here.
  countdownActive = true;
}

void standby() {
  standbyTimer = millis();
  if (standbyTimer - standbyTimerPrev > 1000) {
    standbyTimerPrev = standbyTimer;

      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("DONT");
      lcd.setCursor(5, 1);
      lcd.print("EXPLODE"); 
  }
}

void gameOver(bool deathByTime) {
  countdownActive = false;

  if (deathByTime) {
    Serial.print("\ngameover");  // Send gameover command for the MEGA to read.
  }

      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("YOU");
      lcd.setCursor(6, 1);
      lcd.print("DIED");

      lcd.setCursor(12, 0);
      lcd.write(byte(0));

      lcd.setCursor(13, 0);
      lcd.write(byte(1));

      lcd.setCursor(14, 0);
      lcd.write(byte(2));

      lcd.setCursor(12, 1);
      lcd.write(byte(3));

      lcd.setCursor(13, 1);
      lcd.write(byte(4));

      lcd.setCursor(14, 1);
      lcd.write(byte(5));

      ////////

      lcd.setCursor(1, 0);
      lcd.write(byte(0));

      lcd.setCursor(2, 0);
      lcd.write(byte(1));

      lcd.setCursor(3, 0);
      lcd.write(byte(2));

      lcd.setCursor(1, 1);
      lcd.write(byte(3));

      lcd.setCursor(2, 1);
      lcd.write(byte(4));

      lcd.setCursor(3, 1);
      lcd.write(byte(5));

      delay(5000);
  }


void LEDStrip() {
  // lightmode before gamestart
  if (lightMode == 0) { 
    if(millis() - lastChange > 100) {
      lastChange = millis();
      if (isRed == 1) {
        Green_channel++;
        for (int j = 0; j < NUM_LEDS; j++) {
          leds[j].setRGB(255 - Green_channel, Green_channel, 0);
          if (Green_channel == 255) {
            isRed = 0;
          }
        }
      }
      else {
        Green_channel--;
        for (int j = 0; j < NUM_LEDS; j++) {
          leds[j].setRGB(255 - Green_channel, Green_channel, 0);
          if (Green_channel == 0) {
            isRed = 1;
          }
        }
      }
    }
  }

  // Ligtmode when the game is running normally
  if (lightMode == 1) { 
    if(millis() - lastChange > 100) {
      lastChange = millis();
      if (isRed == 1) {
        Green_channel++;
        for (int j = 0; j < NUM_LEDS; j++) {
          leds[j].setRGB(255, Green_channel, 0);
          if (Green_channel == 50) {
            isRed = 0;
          }
        }
      }
      else {
        Green_channel--;
        for (int j = 0; j < NUM_LEDS; j++) {
          leds[j].setRGB(255, Green_channel, 0);
          if (Green_channel == 0) {
            isRed = 1;
          }
        }
      }
    }
  }

  //lightmode for when a mistake is made
  if (lightMode == 2) {
    if (millis() - lastBlink > 150) {
      lastBlink = millis();
      blinkCount++;
      if (blinkCount % 2 == 1) {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(0, 0, 0);
        }
      }
      else {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(255, 0, 0);
        }
      }
    }
    if (blinkCount == 11) {
      blinkCount = 0;
      lightMode = 1;
      isRed = 1;
      Green_channel = 0;
    }
  }

  // lightmode when module is solved
  if (lightMode == 3) { 
    if (millis() - lastBlink > 150) {
      lastBlink = millis();
      blinkCount++;
      if (blinkCount % 2 == 1) {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(0, 0, 0);
        }
      }
      else {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(0, 255, 0);
        }
      }
    }
    if (blinkCount == 11) {
      blinkCount = 0;
      lightMode = 1;
      isRed = 1;
      Green_channel = 0;
    }
  }

  // lightmode when game is lost
  if (lightMode == 4) {
    if (millis() - lastBlink > endDelay) {
      if (endDelay > 50) {
        endDelay = endDelay - 5;
      }
      lastBlink = millis();
      blinkCount++;
      if (blinkCount % 2 == 1) {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(0, 0, 0);
        }
      }
      else {
        for (int j = 0; j < NUM_LEDS; j++) {
        leds[j].setRGB(255, 0, 0);
        }
      }
    }
    if (blinkCount == 30) {
      blinkCount = 0;
      lightMode = 
      isRed = 1;
      Green_channel = 0;
    }
  }

  // lightmode when game is won
  if (lightMode == 5) {
    if (millis() - lastChange > 150) {
      lastChange = millis();
      for (int i = 0; i <= 255; i++) {
        for (int j = 0; j < NUM_LEDS; j++) {
            leds[j].setRGB(255 - i , i, 0);
        }
        if (i == 255) {
          blink = 1;
        }
      }
      if (blink == 1) {
            blinkCount++;
          if (blinkCount % 2 == 1) {
            for (int j = 0; j < NUM_LEDS; j++) {
            leds[j].setRGB(0, 0, 0);
            }
          }
          else {
            for (int j = 0; j < NUM_LEDS; j++) {
            leds[j].setRGB(0, 255, 0);
            }
          }
        if (blinkCount == 11) {
          blinkCount = 0;
          lightMode = 7;
          isRed = 1;
          Green_channel = 255;
        }
      }
    }
  }

  // lightmode for red light
  if (lightMode == 6) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j].setRGB(255, 0, 0);
    }
    lightMode = 8;
  }

  // lightmode for green light
  if (lightMode == 7) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j].setRGB(0, 255, 0);
    }
    lightMode = 8;
  }

  FastLED.show();
}



void setup() {
  lcd.begin(16, 2);  // Enable once the LCD has been installed.
  lcd.createChar(0, topL);
  lcd.createChar(1, topM);
  lcd.createChar(2, topR);
  lcd.createChar(3, botL);
  lcd.createChar(4, botM);
  lcd.createChar(5, botR);

  for (int i = 0; i < 3; i++) {
    pinMode(RedLed[i], OUTPUT);
  }

  // LED setup
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  // Start serial as usual, make sure baudRate is the same for both uno and the Mega serial used for communication
  // since Serial is used for communication it cannot be used to output information on Serial Monitor
  Serial.begin(115200);  // Connect RX pin on uno to the chosen TX pin on Mega
}

void loop() {
  // Checks if information is sent by Arduino mega
  if (Serial.available() > 0) {
    msg = Serial.readStringUntil('\n');  // Stores data sent
    msg.trim();                          // trims the data for end of line or other invisible data
    //Serial.print("\nIncoming msg: " + msg);
  } else {
    msg = "";  // Resets msg so that if statement doesnt run continously
  }

  // Run if message from ESP = "startCountdown"
  if (msg == "start") {
    gameStart();
    // set variables for LED strip
    isRed = 1;
    Green_channel = 0;
    lightMode = 1;

    for (int i = 0; i < 3; i++) {
      digitalWrite(RedLed[i], SideLEDs[i]);
    }
  }

  if (msg == "lifelost") {
    lightMode = 2;
  }

  if (msg == "modulesolved") {
    lightMode = 3;
  }

  if (msg == "gameover") {
    lightMode = 4;
    gameOver(false);
    for (int i = 0; i < 3; i++) {
      digitalWrite(RedLed[i], 0);      
    }
  }

  if (msg == "win") {
    countdownActive = false;
    lightMode = 5;
  }

  // checks for red Led configuration
  if (msg.substring(0,9) == "SideLEDs=") {
    for (int i = 0; i < 3; i++) {
      String LEDon = msg.substring((9+i),(10+i));
      SideLEDs[i] = LEDon.toInt();
    }  
  }

  // Checks for serial number
  if (msg.substring(0, 7) == "Serial=") {
    gameSerial = msg.substring(7, 15);
  }


  // Countdown loop
  if (countdownActive) {

    countdownTimer = millis();
    if (countdownTimer - countdownTimerPrevious >= 1000) {
      countdownTimerPrevious = countdownTimer;



      int minutes = int(timeRemaining / 60);
      int seconds = timeRemaining - (60 * minutes);


      if (timeRemaining == -1) {

        gameOver(true);

      } else {

        sprintf(timeRemainingFormatted, "%01d:%02d", minutes, seconds);
        Serial.print((String) "\n" + timeRemainingFormatted);

        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print(timeRemainingFormatted);
        lcd.setCursor(4, 1);
        lcd.print(gameSerial);

        timeRemaining--;
      }
    }
  } else {
    standby();
  }

  LEDStrip();
  delay(10);
}
