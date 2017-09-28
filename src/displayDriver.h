/*
* led_display_driver.c:
*      7-segment LED driver
*
* 2017 Wibaut Pierre-Henri. <wibautph@gmail.com>
***********************************************************************
*/
#ifndef _DISPLAY_DRIVER // if this class hasn't been defined, the program can define it
#define _DISPLAY_DRIVER // by using this if statement you prevent the class to be called more than once which would confuse the compiler

#pragma once

#include <string.h>
#include <stdio.h>
#include <wiringPi.h>
#include <sr595.h>
#include <math.h>
static const int _digits[11] = {0x77, 0x14, 0xb3, 0xb6, 0xd4,0xe6,0xe7,0x34,0xf7,0xf6,0x08};

class DisplayDriver {

public:

  DisplayDriver(){
    digits =_digits;
  }

  void setup(int _display_id,int _data_pin, int _clock_pin, int _latch_pin){
    ofLog(OF_LOG_NOTICE, "Setting up the display #"+ofToString(_display_id));
    pins_address=100+100*_display_id;
    sr595Setup (pins_address, 32, _data_pin, _clock_pin, _latch_pin);
  }

  void displayNumber (int integer_to_display) {
    int digit_position=0;
    while(digit_position<4){
      int bit_values=digits[integer_to_display%10]; //extract the digit from the int
      // set the 8 segments with the binary signature found in digits array
      int bit;
      for (bit = digit_position*8; bit < (digit_position+1)*8 ; ++bit){
        digitalWrite (pins_address + bit, 0x1 & bit_values) ;
        bit_values >>= 1;
      }
      integer_to_display /=10;
      digit_position++;
    }
  }
  void clearDisplay(){
    int i;
    for (i=0; i < 32 ; ++i){
      digitalWrite (pins_address + i, 0x0) ;
    }
  }
  void AllOn(){
    int i;
    for (i=0; i < 32 ; ++i){
      digitalWrite (pins_address + i, 0x1) ;
    }
  }

  //3 blinks for led testing
  void testLed(){
    for(int i=0; i<3;i++){
      AllOn();
      printf("All on\n");
      sleep(1);
      clearDisplay();
      printf("All off\n");
      sleep(1);
    }
  }

private:
  //binary mapping between array position number and corresponding led status
  /*
   _         7
  |_|      6 5 8
  |_|. ->  1 2 3  4
  */
  const int *digits;
  //= {0xe7, 0x84, 0xd3, 0xd6, 0xb4,0x76,0x77,0xc4,0xf7,0xf6,0x08}
  int pins_address;
  int display_id;

  //parsing format: "0.34.2", "0.1.2.3" etc
  /*unsigned int parse_message(char *message){
  unsigned int bits_signature = 0;
  int i = 0;
  for (i = 0; i < 4 && message[i] != '\0'; i++) {
    int position=atoi(message[i]);
    position <<= 8*i; //shifting by 8
    if(message[i+1]=='.'){
      |= 0x8;//switch on the dot
      i++; //go to next digit
    }
    bits_signature = bits_signature | position;
  }
  printf("send bit pattern: %d \n",bits_signature);
}*/

};
#endif
