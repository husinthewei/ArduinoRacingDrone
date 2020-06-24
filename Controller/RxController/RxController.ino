#include <SoftwareSerial.h>
#include <Wire.h>

#include "WiiClassicControl.h"

// 6 byte struct containing movement info
struct MoveData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};

// Create instance of radio transceiver
SoftwareSerial HC12(10, 11);

// Create instance of wii classic controller
WiiClassicControl WiiController;

// Current movement data to send to receiver
MoveData data;

// Reset movement data to defaults
void resetData() {   
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
}

void setup() {
  // Init wii classic controller
  WiiController.begin(NEUTRAL, OFF);

  // Init radio receiver
  HC12.begin(9600);

  // Begin serial comm
  Serial.begin(9600);

  // Set movement data to default vals
  resetData();
}

void loop() {
  WiiController.CollectData();

  // Read trigger buttons to increase or decrease throttle
  int throttle = data.throttle;
  if (WiiController.lzPressed()) {
    throttle -= 51;
  } 
  if (WiiController.rzPressed()) {
    throttle += 51;
  }
  data.throttle = constrain(throttle, 0, 255);

  // Read joystick values and update current movement data
  data.yaw = constrain(WiiController.leftStickX(), 0, 255);
  data.pitch = constrain(WiiController.rightStickY(), 0, 255);
  data.roll = constrain(WiiController.rightStickX(), 0, 255);

  // Bumper buttons used as AUX buttons
  data.AUX1 = WiiController.leftShoulderPressed();
  data.AUX2 = WiiController.rightShoulderPressed();

  // Send data and wait some time
  HC12.write((byte*) &data, sizeof(MoveData));
  delay(45);
}
