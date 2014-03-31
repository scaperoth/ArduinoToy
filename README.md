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
    
>This project uses an Arduino Leonardo. <

##Components
	- [Virtuabotix DHT22 Temperature & Humidity Sensor](https://www.virtuabotix.com/product/virtuabotix-dht22-temperature-humidity-sensor-arduino-microcontroller-circuits/)
	- [SainSmart HC-SR04 Ranging Detector](http://www.sainsmart.com/ultrasonic-ranging-detector-mod-hc-sr04-distance-sensor.html)
	- [Sparkfun Bar Graph Breakout Kit DEV-10936](https://www.sparkfun.com/products/10936)
	- [Sparkfun VoiceBox Shield DEV-10661](http://www.sainsmart.com/ultrasonic-ranging-detector-mod-hc-sr04-distance-sensor.html)

