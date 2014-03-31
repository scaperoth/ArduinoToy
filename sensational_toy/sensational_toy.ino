/**********************************************************
 * @authors: Matt Scaperoth, Niyi Odumosu, Joseph Burns
 * @description: various sensors attached to Arduino Leonardo
 *               with EEPROM recording humidity data that can
 *               be dumped to a Java program. 
 **********************************************************

/***************************
 * INCLUDED FILES
 ***************************/
#include <EEPROM.h>
#include <SFEbarGraph.h>
#include <SPI.h>
#include <dht22.h>
//Soft serial library used to send serial commands on pin 2 instead of regular serial pin.
#include <SoftwareSerial.h>

/***************************
 * SPARKJET VOICEBOX SETUP
 * GND on SPK-
 * VCC on SPK+
 ***************************/
//Define the Pin Numbers for the sketch.
#define E0  5
#define E1  6
#define E2  7
#define E3  8
#define E4  9
#define E5  10
#define E6  11
#define E7  12

#define RDY  13
#define RES  3
#define SPK  4

#define txPin  2

//Create a SoftSerial Objet
SoftwareSerial speakjet = SoftwareSerial(0, txPin);

//The message array contains the command for sounds to be sent in order to inunciate the words "All your base belong to us." Check the SpeakJet Manual for more information
//on producing words
//All              Your         Base                 Are     Belong                       to          us
char message[] = {
  20, 96, 21, 114, 22, 88, 23, 5, 8, 135, 8, 146, 5, 128, 153, 5, 170, 154, 8, 188, 5, 152, 5, 170, 8,128,146,8,135,8,144,5,8,191,162,5,8,134,187};

//The sounds array contains the commands to send robot sounds to the SpeakJet chip.
char humidity_sounds[] = {
  201};
char range_sounds[] = {
  220};
char c02_sounds[] = {
  240};
  
/***************************
 * VARIABLE DEFINITIONS
 ***************************/
//MEMORY MANIPULATION
#define READ 0
#define WRITE 1
#define CONTROLBIT 512
#define ADDRESSBIT 511

//HUMIDITY SENSOR
#define DHTPIN 6     // what pin we're connected to

//RANGER
#define trigPin 9 //trig to pin 9
#define echoPin 10 //echo to pin 10

/***************************
 * MEM CONTROL VARIABLES
 ***************************/
int control_val = 0;
int address_val = 0;

/***************************
 * TIMING VARIABLES
 ***************************/
int timer = 0;
//delay for whole program
int global_delay = 100;

//delay for various sensors and components
double rom_minutes = .1; //how often to save sensor data
double humid_speaker_seconds = 4; //how long to wait to alarm

int rom_delay = rom_minutes * (60000/global_delay); //minutes relative to global delay
int humid_speaker_delay = humid_speaker_seconds * (1000/global_delay); //seconds relative to global delay

/***************************
 * BARGRAPH SETUP
 * LAT & SIN to MOSI 
 *   - LAT behind SIN such that
 *     SIN receives current first
 * CLK to SCK
 * VCC/+5V to 5V
 * GND to GND
 ***************************/
SFEbarGraph BG;
int num_leds = 0;

/***************************
 * HUMIDITY SETUP
 * DTA to Pin 6
 ***************************/
dht22 DHT22;
int humidity_val= 0;
int chk;
//at what humidity level should the alarm go off
int humidity_alarm_value = 40; 

/***************************
 * RANGE FINDER VARIABLES
 ***************************/
//at what max range should the alarm go off (in cm)
int range_alarm_value = 5;

/***************************
 * BEGIN PROGRAM
 ***************************/
void setup(){
  BG.begin();
  address_val = EEPROM.read(ADDRESSBIT);
  Serial.begin(9600);

  setup_voicebox();
  setup_ranger();

  DHT22.attach(DHTPIN);
  delay(1000);

}

void loop(){
  //check humidity and update bargraph
  activate_bargraph();
  
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


/***************************
 * BARGRAPH FUNCTION(S)
 ***************************/
/**
 * get humidity value and convert it to
 * a number between 0 and 30. Send this
 * number to fill_leds to light up
 * the appropriate number of LEDS on the
 * bargraph breakout kit
 */
void activate_bargraph(){
  if(check_humidity_sensor()){
    humidity_val = DHT22.humidity;  //get humidity
    num_leds = (int)(humidity_val/3.33); //convert to 0-30
    fill_leds(num_leds); //light up leds
  }
  //alarm if humidity is below alarm value and proper increment is reached
  if(humidity_val>=humidity_alarm_value && timer%humid_speaker_delay ==0){
    //Serial.println("Play Sound");
    speakjet.print(humidity_sounds);
  }
}
/**
 * find the inverse of the max led
 * so the green leds are the first to light up
 * then loop through and light each LED from the 
 * end to the inverse value. 
 *
 * Example: if max_led is 7 then you will light up
 *          led #'s 29 - 22;
 */
void fill_leds(int max_led){
  int inverse = 29-max_led;
  BG.clear();
  for(int i = 29; i>=inverse; i--){
    BG.paint(i, HIGH);
  } 
  BG.send();
}

/***************************
 * HUMIDITY FUNCTION(S)
 ***************************/
/**
 * checks if humidity sensor is ready
 * reuturns a boolean true or false
 */
int check_humidity_sensor(){
  chk = DHT22.read();
  //Serial.println(chk);
  switch (chk)
  {
  case 0: 
    return 1;
    break;
  default: 
    return 0; 
    break;
  }
}

/***************************
 * RANGER FUNCTION(S)
 ***************************/
/**
 * sets all necessary pins for range sensor
 * used in Setup()
 */
void setup_ranger(){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT_PULLUP);
}
/**
 * Scan the range finder values and
 * alert if the values are below a certain level
 */
void find_range(){
  int duration, distance;
  //pulse range finder
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(2000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  
  //convert pulse value to cm
  distance = (duration/2) / 29.1;
  
  //Serial.println(distance);
  //set off alarm if distance is lte alarm value
  if(distance<=range_alarm_value){
    speakjet.print(range_sounds);
  }
}

/***************************
 * MEMORY FUNCTION(S)
 ***************************/
/**
 * write a value to the current (saved) memory
 * address.
 * if the final available memory location is reached, 
 * then the memory is "full" so we 
 * change the control value to read.
 */
void mem_write(){
  //record data from sensors starting at address 0
  EEPROM.write(address_val, humidity_val);
  address_val=  address_val + 1;
  if(address_val == ADDRESSBIT){
    EEPROM.write(CONTROLBIT, READ);
  }
}
/**
 * Read all data from memeory. 
 * Once the data has been read in, reset all
 * memeory locations using reset_mem().
 */
void mem_read(){
  //wait until a Serial connection is made
  if(Serial){ 
    readData();
    reset_mem();
  }
}
/**
 * loop through all available EEPROM 
 * and output the value at each address 
 * location.
 */
void readData(){
  //loop until the highest recorded bit and 
  //output the data
  for(int i = 0; i<ADDRESSBIT; i++){
    float val = EEPROM.read(i);
    Serial.println(val, DEC);
    delay(10);
  }
  delay(100);
}
/**
 * Fully reset the EEPROM memory and 
 * memory pointer values
 */
void reset_mem(){
  //set all values to 0
  clearData(); 
  //set the address bit (saved address) back to zero
  EEPROM.write(ADDRESSBIT, 0);
  //set controller to write
  EEPROM.write(CONTROLBIT, WRITE);
  //reset address value to zero
  address_val = 0;
  //output that the memory has been reset
  Serial.println("Memory Reset");
  //wait for the serial connection to be closed
  while(Serial);
}
/**
 * Set all data in EEPROM address
 * values to 0.00
 */
void clearData(){
  for(int i = 0; i<ADDRESSBIT; i++){
    EEPROM.write(i, 0);
  }
}


/***************************
 * TIMING FUNCTION(S)
 ***************************/
/**
 * timer that keeps relative time for
 * the various sensors
 */
void increment_timer(){
  timer=timer + 1;
  if(timer>=1000){
    timer = 0;
  }
}

/***************************
 * VOICEBOX FUNCTION(S)
 ***************************/
/**
 * Sets voicebox shield to all 
 * necessary initial values
 * 
 * used in Setup();
 */
void setup_voicebox(){
  //Configure the pins for the SpeakJet module
  pinMode(txPin, OUTPUT);
  pinMode(SPK, INPUT);

  //Set up a serial port to talk from Arduino to the SpeakJet module on pin 3.
  speakjet.begin(9600);    

  //Configure the Ready pin as an input
  pinMode(RDY, INPUT);

  //Configure Reset line as an output
  pinMode(RES, OUTPUT);

  //Configure all of the Event pins as outputs from Arduino, and set them Low.
  for(int i=E0; i<=E7; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  //All I/O pins are configured. Reset the SpeakJet module
  digitalWrite(RES, LOW);
  delay(100);
  digitalWrite(RES, HIGH);

}







