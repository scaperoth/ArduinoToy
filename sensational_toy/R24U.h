/*####################################################################
 * FILE: R24U.H 
 * AUTHORS: Matt Scaperoth, Niyi Odumosu, Joseph Burns
 * VERSION: 1.0
 * PURPOSE: Hold all definitions, functions, and declarations
 *          for sensational_toy.ino
 * LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 * NOTES: Use setter functions to change timer and pin variables
 *        Use various other functions to control behavior of sensors and 
 *        actuators.
 *        
 *        The layout of this file follows the function prototypes.
 *        
 * HISTORY:
 *
 #######################################################################*/

/***************************
 * CONSTANT DEFINITIONS
 ***************************/
//VOICEBOX Definitions
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

//MEMORY MANIPULATION
#define READ 0
#define WRITE 1
#define CONTROLBIT 512
#define ADDRESSBIT 511

#define NULLTERM '\0'

/*#################################
 #
 #     FUNCTION PROTOTYPES
 #       (for reference)
 #
 #################################*/

//PIN SETTERS
void set_range_pins(int, int);
void set_humidity_pins(int); 

//ALARM SETTERS
void set_humidity_alarm(int);
void set_range_alarm(int);
void set_mem_full_led(int);

//TIMER SETTERS
void set_global_delay(double);
void set_rom_sensor_delay(double);
void set_humidity_sensor_delay(double);

//VOICEBOX FUNCTION(S)
void setup_voicebox(void);
void play_sounds(char sounds[]);
void check_sound_lock();

//LED FUNCTION(S)
void setup_leds(void);
void turn_on_full_rgb(void);
void turn_off_rgb(void);
void alert_led(int);
void reset_fade(void);

//HUMIDITY FUNCTION(S)
int check_humidity_sensor(void);

//BARGRAPH FUNCTION(S)
void fill_leds(int);
void activate_bargraph();

//RANGER FUNCTION(S)
void setup_ranger(void);
void find_range(void);

//C0 SENSOR FUNCTION(S)
void get_C0_value();

//MEMORY FUNCTION(S)
void clearData(void);
void reset_mem(void);
void readData(void);
void mem_write(void);
void mem_read(void);

//TIMING FUNCTION(S)
void increment_timer(void);


/*##############################
 #      
 #       COMPONENT SETUP
 #
 #############################*/

/*********************************
 * SPARKJET VOICEBOX VARIABLES
 * must come first because of pin setup
 * GND on SPK-
 * VCC on SPK+
 *********************************/
//Create a SoftSerial Objet
SoftwareSerial speakjet = SoftwareSerial(0, txPin);

//The message array contains the command for sounds to be sent in order to inunciate the words "All your base belong to us." Check the SpeakJet Manual for more information
//on producing words
//All              Your         Base                 Are     Belong                       to          us
char message[] = {
  20, 96, 21, 114, 22, 88, 23, 5, 8, 135, 8, 146, 5, 128, 153, 5, 170, 154, 8, 188, 5, 152, 5, 170, 8,128,146,8,135,8,144,5,8,191,162,5,8,134,187};

//The sounds array contains the commands to send robot sounds to the SpeakJet chip.

char range_sounds[] = {
  207,208,4,207,22,222,200,19,205,221,255, NULLTERM};

char humidity_sounds[] = {
  254,223,4,222,5,207,4,238, NULLTERM};
  
char c02_sounds[] = {
  240, NULLTERM};

/***************************
 * TIMING VARIABLES
 ***************************/
//init relative timer
int timer = 0;
int action_timer = 0; //keep parallel timer for sounds and other actions so they don't occur simultaneously
//delay for whole program
int global_delay = 100;
int sound_delay = 1; //default delay for all sounds
int delay_minutes_conv = (60000/global_delay);
int delay_seconds_conv = (1000/global_delay);

//delay for various sensors and components
double rom_minutes = .0017; //how often to save sensor data
double humid_speaker_seconds = 10; //how long to wait to alarm
double ranger_speaker_seconds = 1; //how long to wait to alarm

int rom_delay = rom_minutes * delay_minutes_conv; //minutes relative to global delay

//delay for sounds
int humid_speaker_delay = humid_speaker_seconds * delay_seconds_conv; //seconds relative to global delay
int ranger_speaker_delay = ranger_speaker_seconds * delay_seconds_conv; //seconds relative to global delay

int sound_playing= 0; //lock for sounds

/***************************
 * LED VARIABLES
 ***************************/
int fade_reset = 40;
int fadeAmount = fade_reset; //amount to fade by each iteration
int brightness = 0; //start fade value at 0
int memory_full_pin = 11;//memory full pin used to alert user 
int rgb_redPin = A5;
int rgb_bluePin = A4;
int rgb_grnPin = A3;
//when the EEPROM is full by fading in and out

/***************************
 * HUMIDITY SETUP
 * DTA to Pin 6
 ***************************/
dht22 DHT22;
int DHTPIN = 6;     // what pin we're connected to
int humidity_val= 0;
int chk;
//at what humidity level should the alarm go off
int humidity_alarm_value = 40; 

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
 * RANGE FINDER VARIABLES
 ***************************/
//at what max range should the alarm go off (in cm)
int range_alarm_value = 5;
int trigPin = 9; //trig to pin 9
int echoPin = 10; //echo to pin 10

/***************************
 * C0 SENSOR VARIABLES
 ***************************/
int c0sensorval;
int c0sensorPin = A0;

/***************************
 * MEM CONTROL VARIABLES
 ***************************/
int control_val = 0;
int address_val = 0;

/*#################################################
 # FUNCTIONS: CAN BE CALLED FROM THE MAIN PROGRAM
 # TO SET AND CONTROL VARIOUS ELEMENTS OF THE DEVICE
 # 
 #   CONTENTS:
 #   PIN SETTERS
 #   ALARM SETTERS
 #   TIMER SETTERS
 #   LED FUNCTION(S)
 #   HUMIDITY FUNCTION(S)
 #   BARGRAPH FUNCTION(S)
 #   RANGER FUNCTION(S)
 #   MEMORY FUNCTION(S)
 #   TIMING FUNCTION(S)
 #   VOICEBOX FUNCTION(S)
 ###################################################
 
/***************************
 * PIN SETTERS
 *
 * CONTENTS:
 *   void set_range_pins(int, int)
 *   void set_humidity_pins(int)
 ***************************/
void set_range_pins(int new_trigPin, int new_echoPin){
  trigPin = new_trigPin;
  echoPin = new_echoPin;
}

void set_humidity_pins(int new_DHT22_pin){
  DHTPIN = new_DHT22_pin;
}

/***************************
 * ALARM SETTERS
 * 
 * CONTENTS:
 *   void set_humidity_alarm(int)
 *   void set_range_alarm(int)
 *   void set_mem_full_led(int)
 *   void check_timer_variables()
 ***************************/
void set_humidity_alarm(int new_humidity_alarm){
  humidity_alarm_value = new_humidity_alarm;
}

void set_range_alarm(int new_range_alarm){
  range_alarm_value = new_range_alarm;
}

void set_mem_full_led(int new_mem_full_pin){
  memory_full_pin = new_mem_full_pin;
}

/*********************************
 * TIMER SETTERS
 * 
 * CONTENTS:
 *   void set_global_delay(double)
 *   void set_rom_sensor_delay(double)
 *   void set_humidity_sensor_delay(double)
 *********************************/
void set_global_delay(double gl_delay){
  global_delay = gl_delay;
}

void set_rom_sensor_delay(double mew_rom_delay){
  rom_delay = mew_rom_delay * (60000/global_delay); //minutes relative to global delay
}

void set_humidity_sensor_delay(double new_humidity_delay){
  humid_speaker_delay = new_humidity_delay * (1000/global_delay); //seconds relative to global delay
}

/***************************
 * VOICEBOX FUNCTION(S)
 *
 * CONTENTS:
 *   void setup_voicebox()
 *   void play_sounds(char sounds[])
 *   void check_sound_lock()
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
/**
 * play voicebox sounds passed into function
 * sets lock for sound playing and 
 * resets the action_timer
 */
void play_sounds(char sounds[], int delay_value){
  //Serial.println("Play Sound");
  turn_on_full_rgb();
  speakjet.print(sounds);
  sound_playing = 1;
  sound_delay = delay_value;
  action_timer=0; //restart action timer
}
/**
 * Resets control variables for 
 * speaker sounds to maintain global delay
 *
 * when adding a new sound action, add a new
 * if statement here
 */
void check_sound_lock(){
  if(action_timer%sound_delay ==0 && sound_playing == 1){
    sound_playing = 0;
    turn_off_rgb();
  }
}

/***************************
 * LED FUNCTION(S)
 * 
 * CONTENTS:
 *  void setup_leds()
 *  reset_fade()
 *  void turn_on_full_rgb()
 *  turn_off_rgb()
 *  void alert_led(int)
 ***************************/
/**
 * Sets initial values of 
 * LEDS
 */
void setup_leds(){
  pinMode(memory_full_pin, OUTPUT);
  pinMode(rgb_redPin, OUTPUT);
  pinMode(rgb_bluePin, OUTPUT);
  pinMode(rgb_grnPin, OUTPUT);
}
/**
 * resets fade values
 */
void reset_fade(){
  //reset fades
  fadeAmount = fade_reset; 
  brightness = 0;
}
/**
 * turns on all values of rgb
 */
void turn_on_full_rgb(){
  digitalWrite(rgb_redPin, HIGH);    
  digitalWrite(rgb_bluePin, HIGH);   
  digitalWrite(rgb_grnPin, HIGH);   
}
/**
 * turns off all pi of rgb
 */
void turn_off_rgb(){
  digitalWrite(rgb_redPin, LOW);    
  digitalWrite(rgb_bluePin, LOW);   
  digitalWrite(rgb_grnPin, LOW);   
}
/** 
 * fades a pin in and out based on the 
 * global delay time. 
 */
void alert_led(int fadepin){
  // set the brightness of pin 9:
  analogWrite(fadepin, brightness);    

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade: 
  if (brightness == 0 || brightness == 240) {
    fadeAmount = -fadeAmount ; 
  }     
}


/***************************
 * HUMIDITY FUNCTION(S)
 *
 * CONTENTS:
 *   int check_humidity_sensor()
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
 * BARGRAPH FUNCTION(S)
 * 
 * CONTENTS:
 *   void fill_leds(int)
 *   void activate_bargraph()
 ***************************/
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
  if(humidity_val>=humidity_alarm_value && !sound_playing){
    play_sounds(humidity_sounds, humid_speaker_delay);
  }
}

/***************************
 * RANGER FUNCTION(S)
 * 
 * CONTENTS:
 *   void setup_ranger()
 *   void find_range()
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
  if(distance<=range_alarm_value && !sound_playing){
    play_sounds(range_sounds, ranger_speaker_delay);
  }
}

/***************************
 * C0 SENSOR FUNCTION(S)
 * 
 * CONTENTS:
 *   void get_C0_value()
 ***************************/
void get_C0_value(){
  c0sensorval = analogRead(c0sensorPin);       // read analog input pin 0
  //do something with the c0 sensor...
  //Serial.println(sensorValue, DEC);  // prints the value read
}

/***************************
 * MEMORY FUNCTION(S)
 * 
 * CONTENTS:
 *   void clearData()
 *   void reset_mem()
 *   void readData()
 *   void mem_write()
 *   void mem_read()
 ***************************/

/**
 * Set all data in EEPROM address
 * values to 0.00
 */
void clearData(){
  for(int i = 0; i<ADDRESSBIT; i++){
    EEPROM.write(i, 0);
  }
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
  //output value to indicate the memory has been reset
  Serial.println("-1");
  //wait for the serial connection to be closed
  while(Serial);
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
  alert_led(memory_full_pin);
  //wait until a Serial connection is made
  if(Serial){ 
    digitalWrite(memory_full_pin,LOW);
    reset_fade();
    readData();
    reset_mem();
  }
}

/***************************
 * TIMING FUNCTION(S)
 * 
 * CONTENTS:
 *   void increment_timer()
 ***************************/
/**
 * timer that keeps relative time for
 * the various sensors
 */
void increment_timer(){
  //increment global timer and action timer
  timer=timer + 1;
  action_timer = action_timer +1 ;
  //unlock timer if necessary
  check_sound_lock();
  if(timer>=1000){
    timer = 0;
  }
}







