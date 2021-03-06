/*
 * Based off of Electronoobs radio controller
 * http://www.electronoobs.com/eng_robotica_tut5_2_1.php
 */

////////////////////// PPM CONFIGURATION//////////////////////////
#define channel_number 6  //set the number of channels
#define sigPin 2  //set PPM signal output pin on the arduino
#define PPM_FrLen 27000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 400  //set the pulse length
//////////////////////////////////////////////////////////////////

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

// Last time we received data
unsigned long lastRecvTime = 0;

// PPM values
int ppm[channel_number];

/***********************************************/
void updatePPMVals()
{
  ppm[0] = map(data.throttle, 0, 255, 1000, 2000);
  ppm[1] = map(data.yaw,      0, 255, 1000, 2000);
  ppm[2] = map(data.pitch,    0, 255, 1000, 2000);
  ppm[3] = map(data.roll,     0, 255, 1000, 2000);
  ppm[4] = map(data.AUX1,     0, 1, 1000, 2000);
  ppm[5] = map(data.AUX2,      0, 1, 1000, 2000);
}

void setupPPM() {
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, 0);  //set the PPM signal pin to the default state (off)

  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;  // compare match register (not very important, sets the timeout for the first interrupt)
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}
/***********************************************/
  
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
  // Init serial connection (to HC-12 radio)
  Serial.begin(9600);
  
  // Set movement data to default vals
  resetData();

  // Set up PPM
  setupPPM();
}

// Read data from radio
void readData() {
  // Write read data onto temporary buffer
  MoveData tempData;
  byte *dataPtr = (byte *) &tempData;
  
  // Read movement data byte by byte from radio
  // Populate into temp buffer
  unsigned int bytesRead = 0;

  // Continuously read until start value
  bool start = false;
  while (Serial.available() && !start) {
    start = Serial.read() == 3;
  }

  while (Serial.available() && bytesRead < sizeof(MoveData)) {
    dataPtr[bytesRead] = Serial.read();
    bytesRead++;
  }

  // Continuously read until termination value
  // in order to keep sync
  while (Serial.available()) {
    if (Serial.read() == 2) {
      break;
    }
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
  if (now - lastRecvTime > 3000) {
    resetData();
  }
  
  updatePPMVals();
  delay(45);
}

/**************************************************/
#define clockMultiplier 2

// Write PPM vals out
ISR(TIMER1_COMPA_vect){
  static boolean state = true;
  
  // Use this to disable serial and continue waiting
  static long remaining_wait = 0;

  TCNT1 = 0;

  if (remaining_wait != 0) {
    // We interrupted early to end serial
    // To ensure next PPM interrupt not interrupted by Serial
    Serial.end();
    OCR1A = remaining_wait;
    remaining_wait = 0;
  }
  else if (state) {
    //end pulse
    PORTD = PORTD & ~B00000100; // turn pin 2 off. Could also use: digitalWrite(sigPin,0)
    OCR1A = PPM_PulseLen * clockMultiplier;
    state = false;
  }
  else {
    //start pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    PORTD = PORTD | B00000100; // turn pin 2 on. Could also use: digitalWrite(sigPin,1)
    state = true;

    long wait;
    if (cur_chan_numb >= channel_number) {
      cur_chan_numb = 0;
      calc_rest += PPM_PulseLen;
      wait = (PPM_FrLen - calc_rest) * clockMultiplier;
      calc_rest = 0;
    }
    else {
      wait = (ppm[cur_chan_numb] - PPM_PulseLen) * clockMultiplier;
      calc_rest += ppm[cur_chan_numb];
      cur_chan_numb++;
    }

    // Accept serial data for 0.95 of wait time
    remaining_wait = 0.05 * wait;
    OCR1A = wait - remaining_wait;
    Serial.begin(9600);
  }
}
