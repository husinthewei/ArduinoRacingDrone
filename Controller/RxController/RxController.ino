#include <Wire.h>

#include "WiiClassicControl.h"

// 6 byte struct containing movement info
struct MoveData {
  byte start;
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
  byte term;
};

// Create instance of wii classic controller
WiiClassicControl WiiController;

// Current movement data to send to receiver
MoveData data;

// Reset movement data to defaults
void resetData() {
  data.start = 3;
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.AUX1 = 0;
  data.AUX2 = 0;
  data.term = 2;
}

void setup() {
  // Init wii classic controller
  WiiController.begin(NEUTRAL, OFF);

  // Begin serial comm (to HC-12 radio)
  Serial.begin(9600);

  // Set movement data to default vals
  resetData();
}

void loop() {
  WiiController.CollectData();

  // Read trigger buttons to increase or decrease throttle
  int throttle = data.throttle;
  if (WiiController.lzPressed()) {
    throttle -= 15;
  } 
  if (WiiController.rzPressed()) {
    throttle += 15;
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
  Serial.write((byte*) &data, sizeof(MoveData));
  delay(45);
}
