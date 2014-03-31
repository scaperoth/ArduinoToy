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
#define trigPin 9
#define echoPin 10

/***************************
 * MEM CONTROL VARIABLES
 ***************************/
int control_val = 0;
int address_val = 0;

/***************************
 * TIMING VARIABLES
 ***************************/
int timer = 0;
int global_delay = 100;
double rom_minutes = .1;
double humid_speaker_seconds = 4; 
int rom_delay = rom_minutes * (60000/global_delay);
int humid_speaker_delay = humid_speaker_seconds * (1000/global_delay);

/***************************
 * BARGRAPH SETUP
 ***************************/
SFEbarGraph BG;
int num_leds = 0;

/***************************
 * HUMIDITY SETUP
 ***************************/

dht22 DHT22;
int humidity_val= 0;
int chk;


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
  activate_bargraph();
  find_range();
  control_val = EEPROM.read(CONTROLBIT);
  /**/
  if(!Serial && control_val==WRITE && timer%rom_delay==0){
     mem_write();
   }
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
 *
 */
void activate_bargraph(){
  if(check_humidity_sensor()){
    humidity_val = DHT22.humidity;
    num_leds = (int)(humidity_val/3.33);

    fill_leds(num_leds);


  }
  if(humidity_val>=40 && timer%humid_speaker_delay ==0){
    //Serial.println("Play Sound");
    speakjet.print(humidity_sounds);
  }
}
/**
 *
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
 *
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
 *
 */
void setup_ranger(){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT_PULLUP);
}
/**
 *
 */
void find_range(){
  int duration, distance;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(2000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  //Serial.println(distance);
  if(distance<=5){
    speakjet.print(range_sounds);
  }
}

/***************************
 * MEMORY FUNCTION(S)
 ***************************/
/**
 *
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
 *
 */
void mem_read(){
  if(Serial){
    readData();
    reset_mem();
  }
}
/**
 *
 */
void readData(){
  for(int i = 0; i<ADDRESSBIT; i++){
    float val = EEPROM.read(i);
    Serial.println(val, DEC);
    delay(10);
  }
  delay(100);
}
/**
 *
 */
void reset_mem(){
  clearData();
  EEPROM.write(ADDRESSBIT, 0);
  EEPROM.write(CONTROLBIT, WRITE);
  address_val = 0;
  Serial.println("Memory Reset");
  while(Serial);
}
/**
 *
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
 *
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
 *
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







