// /*
//           Rotary_Encoder_LED_Ring_Example
//           www.mayhewlabs.com/products/rotary-encoder-led-ring 
//           Copyright (c) 2011 Mayhew Labs.
//           Written by Mark Mayhew
          
// This example shows 3 sequences that are possible on the ring of LEDs around the encoder based on rotation of an encoder.  The 3 sequences are 
// selected by entering 1, 2, or 3 in the serial command prompt.  The serial window shows the current rotary encoder count.  As the encoder is turned, 
// the serial display shows a raw count of the rotary encoder's value and the LEDs show a scaled version of the value.  If the button on the rotary 
// encoder is pushed, the bottom LED will come on.  Each section of code below discusses the process required to do this. 

// A note on setting the output sequence:
// Think of each LED as a single bit in a 16-bit long binary string.  If a bit is 1, the LED is on, if a bit is 0, the LED is off.  
// By making a string of ones and zeros, we choose which LEDs to have on and off, and then send this string to the shift register to display it.
// For example 1000000000000001 binary (0x8001 in hex) will have the fist and last LEDs on, the rest off.  

// CREDIT:
// Reading the rotary encoder is performed with Oleg's example code:a
// http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
// */

// //These are the pins that will talk to the shift register through SPI
// #define SDI    2    //Data
// #define CLK    3    //Clock
// #define LE     4    //Latch

// //These are the rotary encoder pins A, B, and switch
// #define ENC_A    8
// #define ENC_B    9
// #define ENC_SW   10
// #define ENC_PORT PINB  //The port that the rotary encoder is on (see http://www.arduino.cc/en/Reference/PortManipulation)

// // Global variables
// int scaledCounter = 0;  //The LED output is based on a scaled veryson of the rotary encoder counter
// int sequenceNumber=0;   //The output sequence for the LEDs
// int incomingByte = 0;   //Serial input to select LED output sequence


// /*This is a 2 dimensional array with 3 LED sequences.  The outer array is the sequence; 
//   the inner arrays are the values to output at each step of each sequence.  The output values
//   are 16 bit hex values (hex math is actually easier here!).  An LED will be on if its 
//   corresponding binary bit is a one, for example: 0x7 = 0000000000000111 and the first 3 LEDs 
//   will be on.
  
//   The data type must be 'unsigned int' if the sequence uses the bottom LED since it's value is 0x8000 (out of range for signed int).
// */
// unsigned int sequence[3][16] = {{0x0,0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000}, // normal progression1 to 16?
//                                 {0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff},
//                                 {0x0,0x7fff,0x3ffe,0x1ffc,0xff8,0x7f0,0x3e0,0x1c0,0x80,0x1c0,0x3e0,0x7f0,0xff8,0x1ffC,0x3ffe,0x7fff},
//                                };

// void setup()
// {
//   //Set SPI pins to output
//   pinMode(SDI, OUTPUT);
//   pinMode(CLK, OUTPUT);
//   pinMode(LE,OUTPUT);
//   //Set encoder pins to input, turn internal pull-ups on
//   pinMode(ENC_A, INPUT);
//   digitalWrite(ENC_A, HIGH);
//   pinMode(ENC_B, INPUT);
//   digitalWrite(ENC_B, HIGH);
//   pinMode(ENC_SW, INPUT);
//   digitalWrite(ENC_SW, HIGH);
//   //Set serial rate, prompt for desired sequence
//   Serial.begin(115200);
//   Serial.println("Enter 1, 2, or 3 to change the LED sequence");
// }


// void loop()
// { 
//   //Local Variables
//   static uint8_t counter = 0;      //this variable will be changed by encoder input
//   int8_t tmpdata;
  
//   //Get any serial input
//   if (Serial.available() > 0) 
//   {
//     incomingByte = Serial.read();
//   }
  
//   //if the serial input is valid, set the LED output sequence appropriately 
//   if (incomingByte == '1')
//     sequenceNumber=0;
//   if (incomingByte == '2')
//     sequenceNumber=1;
//   if (incomingByte == '3')
//     sequenceNumber=2;
    
//   //Call read_encoder() function    
//   tmpdata = read_encoder();
  
//   //if the encoder has moved
//   if(tmpdata) 
//     {
//       //Print out the counter value
//       Serial.print("Counter value: ");
//       Serial.println(counter, DEC);
//       //Set the new counter value of the encoder
//       counter += tmpdata;      
      
//       //Scale the counter value for referencing the LED sequence
//       //***NOTE: Change the map() function to suit the limits of your rotary encoder application.
//       //         The first two numbers are the lower, upper limit of the rotary encoder, the 
//       //         second two numbers 0 and 14 are limits of LED sequence arrays.  This is done
//       //         so that the LEDs can use a different scaling than the encoder value. 
//       scaledCounter = map(counter,0,100,0,15);
      
//       //Send the LED output to the shift register 
//       digitalWrite(LE,LOW);
//       shiftOut(SDI,CLK,MSBFIRST,(sequence[sequenceNumber][scaledCounter] >> 8));    //High byte first
//       shiftOut(SDI,CLK,MSBFIRST,sequence[sequenceNumber][scaledCounter]);           //Low byte second
//       digitalWrite(LE,HIGH);
//     }
  
//   //If the encoder switch is pushed, this will turn on the bottom LED.  The bottom LED is turned
//   //on by 'OR-ing' the current display with 0x8000 (1000000000000000 in binary)
//   if (!digitalRead(ENC_SW))
//   {
//     digitalWrite(LE,LOW);  
//     shiftOut(SDI,CLK,MSBFIRST,((sequence[sequenceNumber][scaledCounter]|0x8000) >> 8));
//     shiftOut(SDI,CLK,MSBFIRST,sequence[sequenceNumber][scaledCounter]);              
//     digitalWrite(LE,HIGH);   
//   }
// }



// /*************************************************************************
// *    read_encoder() function as provided by Oleg:                        *
// *    http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino *
// *                                                                        *
// *    Returns change in encoder state (-1,0,1)                            *
// ************************************************************************ */
// int8_t read_encoder()
// {
//   int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
//   static uint8_t old_AB = 0;
//   /**/
//   old_AB <<= 2;                   //remember previous state
//   old_AB |= ( ENC_PORT & 0x03 );  //add current state
//   return ( enc_states[( old_AB & 0x0f )]);
// }
