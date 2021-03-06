Arduino Sensor Pack Toy
===
      
The Arudino Sensor Pack Toy is made to work with various components to alert    
a user of certain environmental changes around the device.   

In order to store specific data, the Arduino EEPROM library is used to manipulate   
the Arduino's built in storage space. The program fills the storage up to   
the 510th address. The other two addresses, 511 and 512, are used as control bits.    
    
The 511th address is used to store the previously written to address. This way, the device   
can lose power and pick up where it left off recording when plugged back in.    
     
The 512th is used a control bit to tell the program when to write to the EEPROM and   
when to read from it. If the device is set to read, it will wait until the user opens   
a Serial monitor before releasing the data. A java program has been written with this library     
to take the data from the EEPROM and display it in a user-friendly graph.  

The java program collects data from the [Virtuabotix DHT22 Temperature & Humidity Sensor](https://www.virtuabotix.com/product/virtuabotix-dht22-temperature-humidity-sensor-arduino-microcontroller-circuits/)   
and creates a graphical representation of the data collected over time.
    
>By default, this project is set up for an Arduino Leonardo. In order to use other versions of the Arduino or other microcontrollers, some pin configurations may need to change. Please refer to the components links below for details on how to set up each component for your device. 

##Usage    
In order to use this application, you first need to install [Java](http://www.oracle.com/technetwork/java/javase/downloads/index-jsp-138363.html) and [set up the RXTX Java library](http://playground.arduino.cc/Interfacing/Java#.UzlPUvldV8E). 

[Download](https://github.com/scaperoth/ArduinoToy/archive/master.zip) or [clone](github-windows://openRepo/https://github.com/scaperoth/ArduinoToy) this project  and navigate to the ArduinoToy/sensational_toy.ino file. Load this into your [Arduino IDE](http://arduino.cc/en/main/software), compile, and install.

###Usage Notes & Customization   
This project can easily be customized to match your hardware. All of the function and variable definitions are found in the R24U.h file.   
   
Most of the main values (pin configurations, alert values, and others) can be manipulated using the given "setter" functions found in the R24U.h file. 

##Components   
* [Virtuabotix DHT22 Temperature & Humidity Sensor](https://www.virtuabotix.com/product/virtuabotix-dht22-temperature-humidity-sensor-arduino-microcontroller-circuits/)
* [SainSmart HC-SR04 Ranging Detector](http://www.sainsmart.com/ultrasonic-ranging-detector-mod-hc-sr04-distance-sensor.html)
* [Sparkfun Bar Graph Breakout Kit DEV-10936](https://www.sparkfun.com/products/10936)
* [Sparkfun VoiceBox Shield DEV-10661](http://www.sainsmart.com/ultrasonic-ranging-detector-mod-hc-sr04-distance-sensor.html)

