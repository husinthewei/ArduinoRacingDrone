#include <SoftwareSerial.h>

// 6 byte struct containing movement info
struct MoveData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
};

// Current movement data to send to receiver
MoveData data;

// Create instance of radio transceiver
SoftwareSerial HC12(10, 11);

// Last time we received data
unsigned long lastRecvTime = 0;

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
  // Begin serial comm
  Serial.begin(9600);

  // Init radio receiver
  HC12.begin(9600);

  // Set movement data to default vals
  resetData();
}

// Read data from radio
void readData() {
  // Write read data onto temporary buffer
  MoveData tempData;
  byte *dataPtr = (byte *) &tempData;
  
  // Read movement data byte by byte from radio
  // Populate into temp buffer
  unsigned int bytesRead = 0;
  while (HC12.available() && bytesRead < sizeof(MoveData)) {
    dataPtr[bytesRead] = HC12.read();
    bytesRead++;
  }

  // Set last received time if success
  // Also copy data from buffer to stored movement data
  if (bytesRead == sizeof(MoveData)) {
    memcpy((byte *) &data, dataPtr, sizeof(MoveData));
    lastRecvTime = millis();
  }
}

void loop() {
  readData();

  // Reset data if signal lost (1 second since last data)
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    resetData();
  }
  
  // Print current movement data
//  Serial.println("Throttle: " + String(data.throttle));
  Serial.println("Yaw: " + String(data.yaw));
//  Serial.println("Pitch: " + String(data.pitch));
//  Serial.println("Roll: " + String(data.roll));
//  Serial.println("Aux1: " + String(data.AUX1));
//  Serial.println("Aux2: " + String(data.AUX2));

  delay(45);
}
