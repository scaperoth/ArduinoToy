/*####################################################################
 * FILE: sensational_toy.ino 
 * AUTHORS: Matt Scaperoth, Niyi Odumosu, Joseph Burns
 * VERSION: 1.0
 * PURPOSE: various sensors attached to Arduino Leonardo
 *               with EEPROM recording humidity data that can
 *               be dumped to a Java program. 
 * LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 * HISTORY:
#######################################################################*/

/***************************
 * INCLUDED FILES
 ***************************/
#include "Arduino.h"
#include "EEPROM.h"
#include "SFEbarGraph.h"
#include "SPI.h"
#include "dht22.h"
//Soft serial library used to send serial commands on pin 2 instead of regular serial pin.
#include <SoftwareSerial.h>

/*
 * This file includes all functions and 
 * variables used in the program. 
 * These declarations were moved to a .h file
 * for readability.
 */
#include "R24U.h"

/*********************************
 *         BEGIN PROGRAM
 *********************************/
void setup(){
  //init bargraph
  BG.begin();
  
  //set address_val to currently saved value
  address_val = EEPROM.read(ADDRESSBIT);
  
  Serial.begin(9600);
  
  /*SETUP FUNCTIONS*/
  setup_voicebox();
  setup_leds();
  setup_ranger();
  DHT22.attach(DHTPIN);
  
  //rest for a sec before diving in to loop
  delay(1000);
}

void loop(){
  //check humidity and update bargraph
  activate_bargraph();
  get_C0_value();
  //check range sensor values
  find_range();
  //set the control value to whatever is stored in the control mem address
  control_val = EEPROM.read(CONTROLBIT);
  /*
   * If the serial monitor is closed, and the control value says to write
   * and the timer is at the chosen interval (see timing variables)
   * then write humidity data to memory
   */
  if(!Serial && control_val==WRITE && timer%rom_delay==0){
     mem_write();
   }
   /*
   * else if the control mem address stores a read value
   * or the serial monitor is open, then output data.
   *
   * the serial monitor option is there in case
   * someone would like to read data before all 510s
   * memory address have been filled up, which may take
   * a long time. 
   */
   else if(control_val == READ || Serial){
     mem_read();
   }
   
  increment_timer();
  delay(global_delay);
}









