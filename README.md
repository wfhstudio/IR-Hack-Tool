# IR-Hack-Tool
This is to clone any Infrared Remote Control 

Libraries versions that compatible (must use these libraries)
Adafruit GFX 1.1v
Adafruit 2.5.0v
Adafruit BusIO 1.12.0v
IRremote 2.7.0v
EEPROM 'any version'

ADDITIONAL NOTES FROM WIZZTECH
MUST CHANGE PROTOCOL AT THE MOMENT USING SAMSUNG (LINE 372,521,537,539) You can check other protocols in IRremote.h (in the library IRremote 2.7.0v)

ANALOG BUTTON VALUE:
UNCOMMENT THIS FOR BUTTON VALUE SETUP for setting up the buttons UN-COMMENT WHEN TO SET BUTTON (LINE 105) CHANGE LINE 43-45
The resistors are used in this projects have value shown below:
The value of Analog button to be inputted in line 42-45 in the skecth i.e.
    181,// [0] = Up    //Resistor Analog Value
    323,// [1] = Ok    //Resistor Analog Value
    248 // [2] = Down  //Resistor Analog Value

Additional info: Learn how to set up analog button value:https://www.instructables.com/How-to-Multiple-Buttons-on-1-Analog-Pin-Arduino-Tu/

How to build it: https://www.youtube.com/watch?v=a7XYZyDeuzY
    
