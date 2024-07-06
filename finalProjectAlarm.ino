#include "SevSeg.h"
#include "SR04.h"
#include <IRremote.h>

// 4-DIGIT 7-SEGMENT
const uint8_t SEGMENT_A = 5;
const uint8_t SEGMENT_B = 6;
const uint8_t SEGMENT_C = 7;
const uint8_t SEGMENT_D = 8;
const uint8_t SEGMENT_E = 9;
const uint8_t SEGMENT_F = 10;
const uint8_t SEGMENT_G = 11;
const uint8_t SEGMENT_D1 = A1;
const uint8_t SEGMENT_D2 = A2;
const uint8_t SEGMENT_D3 = A3;
const uint8_t SEGMENT_D4 = A4;

// LEDS
const uint8_t LED_GREEN = 4;
const uint8_t LED_YELLOW = 3;
const uint8_t LED_RED = A5;

// BUZZER
const uint8_t BUZZER = 2;

// CONTROL REMOTO
const uint8_t IR = A0;

// ULTRASONIDO
const uint8_t TRIG = 13;
const uint8_t ECHO = 12;

// EL CÓDIGO PARA DESACTIVAR LA ALARMA
const uint16_t CODE = 905;

// Configuración display de 7 segmentos
SevSeg sevseg;
byte numDigits = 4;
byte digitPins[] = { SEGMENT_D1, SEGMENT_D2, SEGMENT_D3, SEGMENT_D4 };
// Display segment pins A,B,C,D,E,F,G,DP (DP por el momento se pone en un pin cualquiera)
byte segmentPins[] = { SEGMENT_A, SEGMENT_B, SEGMENT_C, SEGMENT_D, SEGMENT_E, SEGMENT_F, SEGMENT_G };
// Dropping resistors used
bool resistorsOnSegments = true;  // 'false' means resistors are on digit pins
// Display type
byte hardwareConfig = COMMON_CATHODE;
bool updateWithDelays = false;  // Default 'false' is Recommended
bool leadingZeros = false;      // Use 'true' if you'd like to keep the leading zeros
bool disableDecPoint = true;    // Use 'true' if your decimal point doesn't exist or isn't connected

// Configuración sensor ultrasónico
SR04 sr04 = SR04(ECHO, TRIG);
long ultrasonicDistance;
// Controlar cada cuanto se comprueba el sensor
long timer = millis();

// Comprueba si se ha activado el sensor de proximidad, y el número de la cuenta atrás actual
bool countDown = false;
uint8_t countDownNum = 0;

// El número que se mostrará en el display
uint16_t numberToDisplay = 0;
int16_t actualCode = -1;
// El código para desactivar la alarma
uint16_t alarmCode = 896;

// Configuración inicial control remoto
IRrecv irrecv(IR);  // create instance of 'irrecv'
//vairable uses to store the last decodedRawData
uint32_t last_decodedRawData = 0;

// Se comprueba si la alarma está activada o no
bool alarmOn = true;


void setup() {
  Serial.begin(9600);
  delay(1000);
  // Start display object
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  // Set brightness
  sevseg.setBrightness(90);
  irrecv.enableIRIn();  // Start the receiver
}

void loop() {
  if (alarmOn) {
    if (!countDown) {
      if (millis() - timer >= 1000) {
        ultrasonicDistance = sr04.Distance();
        if (ultrasonicDistance < 50) {
          countDown = true;
          countDownNum = 10;
          numberToDisplay = countDownNum;
          digitalWrite(LED_YELLOW, HIGH);
        }
        timer = millis();
      }
    }

    else {
      if (countDownNum == 0) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        tone(BUZZER, 440);
        sevseg.blank();
        numberToDisplay = 0000;
        alarmOn = false;
      }

      else if (millis() - timer >= 1000) {
        numberToDisplay--;
        countDownNum--;
        timer = millis();
      }

      if (irrecv.decode())  // have we received an IR signal?
      {
        int8_t code = translateIR();
        if (code != -2) {
          if (actualCode == -1) {
            actualCode = code;
          }

          else {
            if (code == -1 && actualCode == alarmCode) {
              digitalWrite(LED_GREEN, HIGH);
              digitalWrite(LED_YELLOW, LOW);
              sevseg.blank();
              numberToDisplay = 0000;
              alarmOn = false;
              actualCode = 000;
              countDownNum = 0;
            }

            if (actualCode < 100) {
              actualCode = concat(actualCode, code);
            }
          }
          numberToDisplay = concat(actualCode, countDownNum);
        }
        irrecv.resume();  // receive the next value
      }
    }
    // Set the number to display
    sevseg.setNumber(numberToDisplay);
    // Refresh the display
    sevseg.refreshDisplay();
  }
}

int8_t translateIR()  // takes action based on IR code received
{
  Serial.print("IR code:0x");
  Serial.println(irrecv.decodedIRData.decodedRawData, HEX);
  //map the IR code to the remote key
  switch (irrecv.decodedIRData.decodedRawData) {
    case 0xBA45FF00:
      if (actualCode > 100 && actualCode < 999) return -1;
      break;
    case 0xE916FF00: return 0; break;
    case 0xF30CFF00: return 1; break;
    case 0xE718FF00: return 2; break;
    case 0xA15EFF00: return 3; break;
    case 0xF708FF00: return 4; break;
    case 0xE31CFF00: return 5; break;
    case 0xA55AFF00: return 6; break;
    case 0xBD42FF00: return 7; break;
    case 0xAD52FF00: return 8; break;
    case 0xB54AFF00: return 9; break;
    default: return -2; break;
  }  // End Case
  //store the last decodedRawData
  last_decodedRawData = irrecv.decodedIRData.decodedRawData;
  delay(500);  // Do not get immediate repeat
}  //END translateIR

uint16_t concat(uint16_t num1, uint16_t num2) {
  if (num1 == -1) {
    return num2;
  }
  uint16_t pow = 10;
  while (num2 >= pow)
    pow *= 10;

  return (num1 * pow) + num2;
}
