#define RX_PIN 5
#define TX_PIN 6

#define LED_PIN 13

#include "GParser.h"
#include <stdlib.h>
#include <string.h>
#include "SoftwareSerial.h"

SoftwareSerial BTSerial(RX_PIN, TX_PIN);

void setup() {
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  BTSerial.begin(9600);
}

char BTBuffer[64];
bool isBTTransmitted = false;

void loop() {

  // RECEIVING BL DATA

  while(BTSerial.available()) {

    int byte = BTSerial.read();
    BTBuffer[strlen(BTBuffer)] = byte;

    if(byte == 10) {
      isBTTransmitted = true;
      break;      
    }
  }

  // LOGIC BY PARSED BL DATA

  if(isBTTransmitted) {

    GParser data(BTBuffer, ',');
    int dataAmout = data.split();

    for(int i = 0; i < dataAmout; i++) {

      if(strcmp(data[i], "l13") == 0) {
        int val = atoi(data[i+1]);

        if(val == 0) digitalWrite(LED_PIN, LOW);
        else if(val == 1) digitalWrite(LED_PIN, HIGH);
      }
    }

    isBTTransmitted = false;
    memset(BTBuffer, 0, sizeof BTBuffer);
  }

  // OTHER PROGRAM
}