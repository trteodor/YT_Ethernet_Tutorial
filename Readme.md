# Ethernet driver tutorial with LWiP integration 

**_DIRECTORY: "Application" contain provided example_**

This repository contains the sources created during the Ethernet tutorial video in Polish leanguage. 
The video is available at this link: https://youtu.be/9Bin-fLZq6M


Directory: Drivers/Ethernet/* contain prepared files for ethernet  driver
Dicrectory: applications/*/* the prepared examples have been placed...

To check changes history please call: "gitk&" in this project at him location to see changes in diffrent implementation states

This project was created only for educational purposes.
Created software is prepared for Board- NUCLEO-F767ZI

* You are ready to start adventure with this examples when you have this tools in your environment (environment variables)!

    * arm-none-eabi-gcc 9.3.1
    * OpenOCD 0.11.0
    * make 4.2.1 / or 3.81

    * The versions aren't important on 99% ;)

To clone this repo please use following command:
* git clone --recurse-submodules

Finally:

 **_Call "make" in this project at him location to compile provided example

You can call "make flash" to program MCU.
