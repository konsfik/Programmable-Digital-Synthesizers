/*
  Github repo: Programmable Digital Synthesizers
  Project name: Monk
  Author: Kostas Sfikas
  Date: April 2017

  ************************************************
  Necessary Parts: 
  - (1x) Arduino Uno
  - (6x) Linear Potentiometer 10KOhm
  - (1x) Usb cable
  - (1x) Computer that runs Puredata
  
  ************************************************
  Hardware Setup: 
  The hardware setup of this project is quite simple:
  - The six potentiometers must be connected to the arduino's analog pins.
  - The arduino must be connected to your pc via the usb cable
  
  ************************************************
  General Description:
  This arduino programme reads the values of six potentiometers That are connected to it. 
  Then the programme sends the data through usb-serial using a custom communication protocol.
  The data is decoded in the puredta patch, and can be used in any way you like.
  
  ************************************************
  About the data transfer protocol:
  When the arduino reads the value from a potentiometer, you get back a value from 0 to 1023
  This number will not fit in a Byte. One byte represents values from 0 to 255.
  1023 needs 10 bits to represent, so it will need to decompose in two separate Bytes
  and then compose them back (in PureData) to make the full number again.
  Here is a "table" of bits and values for reference:
  | Bits  | Max num  |  Calculation
  | 1     | 1        |  2 to the power of 1 (2) minus 1     -> 1
  | 2     | 3        |  2 to the power of 2 (4) minus 1     -> 3
  | 3     | 7        |  2 to the power of 3 (8) minus 1     -> 7
  | 4     | 15       |  2 to the power of 4 (16) minus 1    -> 15
  | 5     | 31       |  2 to the power of 5 (32) minus 1    -> 31
  | 6     | 63       |  2 to the power of 6 (64) minus 1    -> 63
  | 7     | 127      |  2 to the power of 7 (128) minus 1   -> 127
  | 8     | 255      |  2 to the power of 8 (256) minus 1   -> 255
  | 9     | 511      |  2 to the power of 9 (512) minus 1   -> 511
  | 10    | 1023     |  2 to the power of 10 (1024) minus 1 -> 1023
  
*/

// declare the array to hold the analog pins values
byte myArray[13];

const int numReadings = 10;  //number of readings
int readings[numReadings];   //array containing all the readings
int readIndex = 0;            //index of current reading           
int readAverage = 0;

void setup()
{
  Serial.begin(115200);
  for(int thisReading = 0; thisReading < numReadings; thisReading++){
    readings[thisReading] = 0;
  }
}

void loop(){
  // The main programme loop
  int byteIndex = 0;
  
  //set the first Byte to 0xc0 (this will be recognized as the start character of the message)
  myArray[byteIndex] = 0xc0; // start character (binary: 1100 0000) (decimal:192) (hex: c0)
  
  // go through the analog pins one by one
  for(int i = 0; i < 6; i++) {  //cycles through potentiometers 0 to 5 -> A0 to A5
    unsigned int knob = getSmoothReading(i);// analogRead(i);
    
    byte knobByte1 = knob & 0x007f; //^ keeps the last 7 digits of a number... (007f -> binary: 0111 1111)
    byte knobByte2 = knob >> 7; // moves the bits by 7 positions to the right...
    
    byteIndex++;  //advance index (index-values at this position: 1, 3, 5, 7, 9, 11)
    myArray[byteIndex] = knobByte1;  // calculated above
    byteIndex++;  //advance index  (ndex-values at this position: 2, 4, 6, 8, 10, 12)
    myArray[byteIndex] = knobByte2; // calculated above
  }
  Serial.write(myArray, 13);
}

int getSmoothReading(int potIndex){
  /* This function smooths out the arduino's analog readings (eliminates some of the "noise").
  It does so by reading multiple times from the same potentiometer and then calculating the average value. */
  
  int addedValues = 0;                     // will hold the added values of all readings
  
  for(int i = 0; i < numReadings; i++){    // loop for reading multiple values from the same potentiometer
    readings[i] = analogRead(potIndex);    // put the reading in the array, at the current index
    delayMicroseconds(500);                // a very small delay to ensure correct readings
  }
  
  for(int i = 0; i < numReadings; i++){    //loop for adding the values together
    addedValues += readings[i];            
  }
  
  readAverage = addedValues / numReadings;  //calculate the average value 
  return readAverage;
}


