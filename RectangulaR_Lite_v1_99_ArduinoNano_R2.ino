/*             RectangulaR

RectangulaR lite software version 1.99v
Compatible with hardware version 2.6v
**do not go to sleep at receive mode**
last hardware upgrade: add more resistor => 35.2Ω

©Abdulsalam Abbod - 2022 june 18
mail : abdulsalamabbod@gmail.com

https://www.youtube.com/watch?v=E-1E68vff0w

Libraries versions that compatible:
Adafruit GFX 1.1v
Adafruit 2.5.0v
Adafruit BusIO 1.12.0v
IRremote 2.7.0v
EEPROM 'any version'

ADDITIONAL NOTES FROM WIZZTECH
MUST CHANGE PROTOCOL AT THE MOMENT USING SAMSUNG
UNCOMMENT THIS FOR BUTTON VALUE SETUP //#define SETUP  // for setting up the buttons UN COMMENT WHEN TO SET BUTTON (LINE 104)

*/
#include <avr/sleep.h>
#include <Adafruit_SSD1306.h>// SCL => A5, SDA => A4
#include <Adafruit_GFX.h>
#include <IRremote.h>
#include <EEPROM.h>

#define power 7    //Sleep Button
#define ResetPin 8
#define WiFiEnablePin 12
#define signal 10  //No related ?
#define indicator 6

bool WiFi_State = LOW;
byte y_receive = 12, x_receive = 6, c = 0;
byte y_send = 12, x_send = 6;
const byte HomeButtonPin = 2;       //Home button Pin = 2
unsigned int ButtonReadValue = 0;
const int ButtonValue[3] = {        //Must check value using, How to: https://www.instructables.com/How-to-Multiple-Buttons-on-1-Analog-Pin-Arduino-Tu/
    181,// [0] = Up    //Resistor Analog Value
    323,// [1] = Ok     //Resistor Analog Value
    248 // [2] = Down  //Resistor Analog Value
};

byte MemoryAddressLocation = 0;
bool HomeButton = false;

const byte PROGMEM Up = 0, Ok = 1, Down = 2; 

void Splash();
void MainMenu();
void JammingMode();
void SendingPage();
void ReceivingPage();
bool IsButtonPressed(byte ButtonValue_);
void PrintTheReceivedCode();
void SelectMode(byte &CurrentMode, unsigned long &CurrentTime);
void ManagePage();
void Notification(byte f);
void PageFrame(byte StartingPoint, byte EndingPoint, byte x_coordinate, byte y_coordinate, byte TextSize=1);
void GoToSleepMode();
void WakeUp();
void ReadTheButtonValue();
void ReadHomeButtonState();
void ResetSleepingTime();
void Debugging();
void Done(bool state=true);


const byte PROGMEM ReceivingPin = 4; // D4    , was 4 ---> Receiving Pin sudah OK

// putting your own code previously to send it
// or you can just make it all zeros
// this array for storing the Received codes to do somehting with them.
unsigned long ReceivedData[10] = {// these values doesn't belong to the files, its only extra way to save codes at but i wouldn't recommended :(  
    0xE0E040BF, 
    0xE0E0E01F, 
    0xE0E0D02F, 
    0xE0E048B7,
    0,
    0,
    0,
    0
};

byte code_bits[10], File=0;
const byte ScrollingSpeed = 180L; // 180 recommended

unsigned long PreviousTime0 = 0, PreviousTime2 = 0;
unsigned long PreviousSleepTime = 0, SleepTime;


IRsend IRsend; //on pin 3 (pro mini default) ---> IR Transmitter Pin
IRrecv IRrecv(ReceivingPin);
decode_results results;

Adafruit_SSD1306 display(128, 64);

/******SETTINGS******/
//  #define CustomCode // for saving codes at eeprom
//  #define DEBUG  // for Debbuging
//#define SETUP  // for setting up the buttons UN COMMENT WHEN TO SET BUTTON -----------------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!!



#ifdef CustomCode
// 5 files each file has size of 10 codes each code are 4 bytes size which is quite enough for IR codes
unsigned long MyCustomCode[60]={
// Default File0 
    0xE0E040BF, 
    0xE0E0E01F, 
    0xE0E0D02F, 
    0xE0E048B7,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEAAADEF,
    
// File1    
    0xF111111, 
    0xAFFFFFE, 
    0xAFFFFFE, 
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEF11111,

// File2
    0xF222222, 
    0xAFFFFFE, 
    0xAFFFFFE, 
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEF22222,
    
// File3   
    0xF333333, 
    0xAFFFFFE, 
    0xAFFFFFE, 
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEF33333,
    
// File4
    0xF444444, 
    0xAFFFFFE, 
    0xAFFFFFE, 
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEF44444,
    
// File5   
    0xF555555, 
    0xAFFFFFE, 
    0xAFFFFFE, 
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xAFFFFFE,
    0xEF55555
    
    
};
#endif

void setup() {
    
#ifdef CustomCode // for saving your own IR codes to use them later, without the need to receive them to save them!
 byte mycode =0;
   // save code at eeprom (values stay even after powering RectangulaR off)
      for (int i =0; i<400;i+=4) {
        EEPROM.put(i,MyCustomCode[mycode]);
        mycode>60?mycode=0:mycode++;
    }
#endif


    pinMode(power, OUTPUT);
    pinMode(WiFiEnablePin, OUTPUT);
    pinMode(indicator, OUTPUT);
    pinMode(ResetPin, OUTPUT);
    digitalWrite(power, HIGH);  
    digitalWrite(ResetPin, LOW);
    
    pinMode(HomeButtonPin, INPUT_PULLUP);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    
#ifdef SETUP // if setup is defined then RectangulaR will enter setting Buttons values only and other functions would not work!
  Serial.begin(9600);
  while(true){
      ReadTheButtonValue();
      ReadHomeButtonState();
      Serial.print("Analog Button Read : ");
      Serial.println(ButtonReadValue);
      Serial.print("Home Button Read : ");
      Serial.println(HomeButton);
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      
      display.setCursor(10,24);
      display.println("Analog Value :");
      display.setCursor(100,24);
      display.println(ButtonReadValue);
      
     display.setCursor(10,44);
     display.println("Home State :");
     display.setCursor(100,44);
      display.println(HomeButton);
      
      display.display();
      delay(100);
  }
  
#endif


    Splash();   delay(990);

   MainMenu();

}

void loop() {/* Nothing here! */}


void Splash() {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(13, 0);
    display.println(F("WizzTech")); display.fillRoundRect(27, 21, 75, 36, 3, SSD1306_WHITE); //first rect (the bigger)
    display.fillRoundRect(30, 24, 69, 30, 3, SSD1306_BLACK); //second rect
    display.fillRoundRect(78, 28, 16, 3, 2, SSD1306_WHITE); //small one
    display.fillRoundRect(92, 28, 3, 16, 2, SSD1306_WHITE); //small one
    display.display();
}


//         MENU
void MainMenu() {
 
    const byte x = 32, Rows[5] = {12, 23, 36, 49, 62};
    byte shift = 0,  CurrentMode = 1;     
    while(true) {
        SleepTime = millis();
        unsigned long  CurrentTime = millis();
        unsigned long wait = millis();
        
        Debugging();
        ReadTheButtonValue();
        ReadHomeButtonState(); //  Serial.println(ButtonReadValue);
        //  delay(90);
        SelectMode(CurrentMode, CurrentTime);

        CurrentMode > 5?CurrentMode = 1: 0;
        CurrentMode < 1?CurrentMode = 5: 0;

        if(CurrentMode == 5) shift = 13;
        else if(CurrentMode < 2) shift = 0;

  // Do not go to sleep mode if WiFi is IsSelectede                  
        if((SleepTime - PreviousSleepTime) > 180000L && !WiFi_State) {
            ResetSleepingTime();
            GoToSleepMode();
        }


        if(IsButtonPressed(ButtonValue[Ok]) && (wait-PreviousTime2) > 600) {

            switch(CurrentMode) {
                case 1:
                JammingMode(); break;
       #ifndef CustomCode        
                case 2:
           ReceivingPage(); break;
        #endif        
                case 3:
                PreviousTime0 = CurrentTime; // ResetPin time
                SendingPage(); break;
                
                case 4:
                PreviousTime0 = CurrentTime;
                ManagePage(); break;
                
                case 5:
                ResetSleepingTime();
                WiFi_State = !WiFi_State;
                digitalWrite(WiFiEnablePin, WiFi_State);
                PreviousTime2 = wait;
                break;

            }
        }

        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setCursor(x, Rows[0]-shift);
        display.println(F("Jamming"));
        display.fillRect(0, 0, 100, 9, SSD1306_BLACK);

        display.setCursor(x, Rows[1]-shift);
        display.println(F("Receiving"));
        display.setCursor(x, Rows[2]-shift);
        display.println(F("Sending"));
        display.setCursor(x, Rows[3]-shift);
        display.println(F("Manage"));

        if(WiFi_State)
        display.fillCircle(70, Rows[4]-shift+2, 3, WHITE);
        else
        display.fillCircle(50, Rows[4]-shift+2, 3, BLACK);
        PageFrame(48, 32, 53, 0);    
        display.println(F("Menu"));

        display.setCursor(x, Rows[4]-shift);
        display.println(F("WiFi"));
        display.drawRoundRect(6, Rows[CurrentMode-1]-3-shift, 117, 13, 3, SSD1306_WHITE);
        

        Debugging();


        display.display();

    }
}


void JammingMode() {
 
    byte Intensity = 0;
    while(true) {
        Debugging();
          unsigned long CurrentTime = millis();
        ReadHomeButtonState();
 
        analogWrite(indicator, Intensity);
      Intensity >= 250?Intensity = 0: Intensity += 18;
        if(HomeButton) {
            digitalWrite(indicator, LOW);
            ResetSleepingTime(); break;
        }

        if((CurrentTime - PreviousTime0) > 10) {
            
            PreviousTime0 = CurrentTime;         
            IRsend.sendSAMSUNG(0xFFFFFFF, 32); // HEX or DEC works original NEC replaced to SAMSUNG
        }
    }
}


//RECEIVING CODE AND RECEIVE PAGE
void ReceivingPage() {

    IRrecv.enableIRIn();
   byte Code_Position = 0;
    display.clearDisplay();
    
// draw the page frame with title
    PageFrame(22, 100, 26, 0);

    display.println(F("Receiving"));
    display.fillRect(83, 0, 12, 8, SSD1306_BLACK);
    display.setCursor(94, 0);
    display.println(F(" bit"));

    while(true) {
        SleepTime = millis();
        ReadHomeButtonState();
        Debugging();
        
        if (IRrecv.decode(&results)) {
            //Receive...
            if(Code_Position < 10 && (results.value) > 1) {
                code_bits[Code_Position] = IRrecv.results.bits;

                display.fillRect(83, 0, 15, 10, SSD1306_BLACK);
                display.setCursor(85, 0);
                display.println(code_bits[Code_Position], DEC);
                display.setCursor(94, 0);
                display.println(F(" bit"));

                digitalWrite(indicator, HIGH);
                ReceivedData[Code_Position] = (results.value);
                display.setCursor(x_receive, y_receive);
                display.println(ReceivedData[Code_Position], HEX);
                ResetSleepingTime();
                display.display();
                delay(50);
                digitalWrite(indicator, LOW);
                Code_Position++;
                y_receive += 10;
            }

            if(y_receive >= 54 && x_receive == 6) {
                y_receive = 12; x_receive = 65;
            }
            else if(y_receive >= 54 && x_receive == 65) {
                Code_Position = 99;
            }
            IRrecv.resume();
        }

        if(HomeButton) {
            ResetSleepingTime();break;
        }

        display.display();
    }
}


//        SEND VIEWWWW
void SendingPage() {

    byte Code_Position = 0,
    Column = 0,
    Row = 1;
    bool AutoSend = false;
    bool IsSelected = false;
    const byte PROGMEM ly[5] = { 9, 19,  29, 39, 49 }, lx[2] = { 2, 67 };
    display.clearDisplay();


    while(true) {
        unsigned long CurrentTime = millis();
        SleepTime = millis();
        
        Debugging();
        ReadHomeButtonState();
        ReadTheButtonValue();

        if(!IsSelected) {
            SelectMode(Row, CurrentTime);
        }

        // Do not go to sleep mode while WiFi is active
        if(WiFi_State) ResetSleepingTime();

        // Sleep after 10 mintues if nothing happend
        if((SleepTime - PreviousSleepTime) > 600000L) {
            ResetSleepingTime();
            GoToSleepMode();
        }

        if(Column == 0) {
            Code_Position = Row-1;
        }
        else if(Column == 1) {
            Code_Position = Row+4;
        }

        // deselect the code if 'ok' pressed
        if(IsButtonPressed(ButtonValue[Ok]) && ((CurrentTime-PreviousTime0) > 600) && IsSelected) {
            AutoSend = false;
            display.clearDisplay();
            IsSelected = false;
            PreviousTime0 = CurrentTime;
        }

        // exit if you didn't select any code
        if((!IsSelected) && (HomeButton)) {
            PreviousTime0 = CurrentTime;
            ResetSleepingTime();break;
        }

        // save the code at EEPROM (home button)
        if(((CurrentTime-PreviousTime0) > 1800) && (HomeButton) && (IsSelected)) {

            EEPROM.put(MemoryAddressLocation, ReceivedData[Code_Position]);
            MemoryAddressLocation += 4;
            IsSelected = false;
            Notification(2); /* 2 for "Saved" notification*/
            PreviousTime0 = CurrentTime;
        }
        
        PrintTheReceivedCode();

        display.fillRect(83, 0, 12, 8, SSD1306_BLACK);
        display.setCursor(84, 0);
        display.println(code_bits[Code_Position], DEC);
        display.setCursor(95, 0);
        display.println(F(" bit"));

        // selecting the code to transmit it
        if(IsButtonPressed(ButtonValue[Ok]) && ((CurrentTime-PreviousTime0) > 600)) {
            display.fillCircle(lx[Column]+51, ly[Row-1]+5, 3, WHITE);
            display.display();
            IsSelected = true;
            PreviousTime0 = CurrentTime; // ResetPin time
        }
        
        // Send "code" if Up Button is pressed
        if((IsButtonPressed(ButtonValue[Up])) && (IsSelected)) {
            IRsend.sendSAMSUNG(ReceivedData[Code_Position], 32);
              delay(300);
         
            if((ReceivedData[Code_Position] != 0xcf20d) && (ReceivedData[Code_Position] != 0xc728d)) {
                delay(1300);    }
            else {
                delay(300);     }
        }

        /**************************************/
        // enable auto-send if Down Button is pressed
        if(IsButtonPressed(ButtonValue[Down]) && (IsSelected)) {
            AutoSend = true;
        }
        // sending code automatically
        else if(AutoSend == true && IsSelected) {
            IRsend.sendSAMSUNG(ReceivedData[Code_Position], 32);
            delay(300);
            IRsend.sendLG(ReceivedData[Code_Position], code_bits[Code_Position]);

            if((ReceivedData[Code_Position] != 0xcf20d) && (ReceivedData[Code_Position] != 0xc728d)) {
                delay(1300);    }
            else {
                delay(300);    }
      }

        if(Column == 1 && Row > 5) {
            Row = 1; Column = 0;
        }
        if(Column == 0 && Row < 1) {
            Row = 5; Column = 1;
        }
        if(Column == 0 && Row > 5) {
            Row = 1; Column = 1;
        }
        if(Column == 1 && Row < 1) {
            Row = 5; Column = 0;
        }

        display.drawRoundRect(lx[Column], ly[Row-1], 60, 12, 3, SSD1306_WHITE);

        display.display();
    }
    
}



void PrintTheReceivedCode() {
    byte Column = 1;
    PageFrame(32, 91, 36, 0);
    //      38 ~78  43  0
    display.println(F("Sending"));

    y_send = 12;
    x_send = 5;

    for(byte Row = 0; Row < 10; Row++) {
        if(Row > 4 && Column == 1) {
            y_send = 12; x_send = 70; Column = 0;
        }

        display.setCursor(x_send, y_send);
        if(ReceivedData[Row] > 0) {
            display.println(ReceivedData[Row], HEX);
            y_send += 10;
        }
    }
    

}


void ManagePage() {
   bool IsSelected = false;
    byte CurrentMode = 1, Code_Position = 0;
    const int PROGMEM File_Data[7] = {0, 40, 80, 120, 160, 200, 240};
    const byte x = 32,
    Rows[3] = {
        15,//12
        26,//23
        39//36
    };

    display.clearDisplay();
    PageFrame(33, 68, 37, 0);

    while(true) {
        SleepTime = millis();
        unsigned long CurrentTime = millis();
        Debugging();
        ReadTheButtonValue();
        ReadHomeButtonState();
        
        if(!IsSelected)
        SelectMode(CurrentMode, CurrentTime);

        if(HomeButton && CurrentTime-PreviousTime0>700 ) {
            if(!IsSelected){
            ResetSleepingTime(); break;}
            else {
                IsSelected = false;
                display.fillCircle(18,Rows[1]+3, 3, BLACK);
               PreviousTime0 = CurrentTime;
            }
        }
         
        // Sleep after 3 minutes if nothing happend 180000L 
        if((SleepTime - PreviousSleepTime) > 180000L) {
           ResetSleepingTime();
            GoToSleepMode();
        }

        CurrentMode > 3?CurrentMode = 1: 0;
        CurrentMode < 1?CurrentMode = 3: 0;
        
       if(IsSelected && (CurrentTime-PreviousTime0>300)){
        
           if(IsButtonPressed(ButtonValue[Up])){
            File++; PreviousTime0 = CurrentTime;
           }
        
           else if(IsButtonPressed(ButtonValue[Down])){
            File--;  PreviousTime0 = CurrentTime;
           }
        
            File>5?File=0:0;
       }

        
        // select file to Import
          if(IsButtonPressed(ButtonValue[Ok])&&CurrentMode==2 && !IsSelected && (CurrentTime-PreviousTime0>700)){
            IsSelected = true;
            PreviousTime0 = CurrentTime;
           }
       
        

         if(IsButtonPressed(ButtonValue[Ok]) && ((CurrentTime-PreviousTime0) > 700)) {
            PreviousTime0 = CurrentTime;
            
            switch(CurrentMode) {
                case 2: // Import ReceivedData from EEPROM
               if(IsSelected){
                for(byte CodeFilePosition = File_Data[File]; CodeFilePosition < File_Data[File+1]; CodeFilePosition += 4) {
                    unsigned long Code_;
                    EEPROM.get(CodeFilePosition, Code_);
                    ReceivedData[Code_Position] = 0;// clear data
                    ReceivedData[Code_Position] = Code_;// then add thw code 
                    Code_Position++;
                }
                Notification(1); // 1 for ManagePage
                IsSelected = false;
                
               }
                break;

                case 1: // Clear chache in Ram except EEPROM
                for(byte d = 0; d < 10; d++) {
                    ReceivedData[d] = 0;
                    Code_Position = 0; x_receive = 6; y_receive = 12;// reset position
                }
                Notification(1);
                break;


                case 3: // Turn Off (go to sleep)
                GoToSleepMode();
                break;


                default: break;
            }
            PreviousTime0 = CurrentTime;
        }
        display.clearDisplay();

        PageFrame(41, 47, 47, 0);
        if(IsSelected) display.fillCircle(18,Rows[1]+3 , 3, WHITE);
        else display.fillCircle(18,Rows[1]+3 , 3, BLACK);
        
        display.println(F("Manage"));
        display.setCursor(46, 15);
        display.setCursor(x, Rows[0]);
        display.println(F("Clear Cache"));
        display.setCursor(x, Rows[1]);
        display.println(F("Import File"));
        display.setCursor(99, Rows[1]);
        display.println(String(File));
        display.setCursor(x, Rows[2]);
        display.println(F("Turn Off"));
        display.drawRoundRect(6, Rows[CurrentMode-1]-3, 117, 13, 3, SSD1306_WHITE);
        display.display();
  }

}

void Notification(byte NotificationMode) {
    digitalWrite(indicator, HIGH);
    switch (NotificationMode) {
        case 1:
        Done(true);// true for Done!
        break;

        case 2:
        Done(false);// false for saved!
        break;

    }
    delay(500);
    digitalWrite(indicator, LOW);
    display.clearDisplay();
}

void Done(bool state){
    display.fillRoundRect(32, 16, 64, 32, 3, SSD1306_WHITE);
    display.setCursor(51, 28);
    display.setTextColor(BLACK);
    if(state) 
    display.println(F("Done!"));
    else display.println(F("Saved"));
    display.display();
}



void PageFrame(byte StartingPoint, byte EndingPoint, byte x_coordinate, byte y_coordinate, byte TextSize) {
    display.drawRoundRect(0, 2, 128, 62, 3, SSD1306_WHITE);
    display.drawFastHLine(StartingPoint, 2, EndingPoint, SSD1306_BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(TextSize);
    display.setCursor(x_coordinate, y_coordinate);
}

void SelectMode(byte &CurrentMode, unsigned long &CurrentTime) {

    ReadTheButtonValue();

    // Go Up    use 1006 analog value => 0  
    if(IsButtonPressed(ButtonValue[Up]) && ((CurrentTime-PreviousTime0) > ScrollingSpeed)) {
        CurrentMode--;
        display.clearDisplay();
        PreviousTime0 = CurrentTime;
        ResetSleepingTime();
    }

    // Go Down   use 1016 analog value => 2
    else if((IsButtonPressed(ButtonValue[Down]) && ((CurrentTime-PreviousTime0) > ScrollingSpeed))) {
        CurrentMode++;
        display.clearDisplay();
        PreviousTime0 = CurrentTime;
        ResetSleepingTime();
    }
}

void GoToSleepMode() {
    // turn everthing off
    display.clearDisplay();
    Notification(3);
    digitalWrite(WiFiEnablePin, LOW);
    display.display();
    digitalWrite(power, LOW);

    //conSAMSUNGt the sleep mode to main home button
    attachInterrupt(digitalPinToInterrupt(HomeButtonPin), WakeUp, LOW); //IsSelectedate Interrupt for waking up

    // sleep mode type
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable(); // enabling sleep mode
    sleep_mode(); // program stop here (sleep)

    sleep_disable(); // disable sleep mode

}

void WakeUp() {
    detachInterrupt(digitalPinToInterrupt(HomeButtonPin));
    Notification(3); // blink indicator
    digitalWrite(power, HIGH);
    digitalWrite(ResetPin, HIGH);
    delay(100);
    digitalWrite(ResetPin, LOW);
}

bool IsButtonPressed(int ButtonValue_) {
    byte Threshold = 4;
    int Maximum,  Minimum;
    ReadTheButtonValue();
    Maximum = ButtonReadValue+Threshold;
    Minimum = ButtonReadValue-Threshold;
    if(ButtonValue_ > Minimum && ButtonValue_ < Maximum) {
        return true;
    }
    else return false;
}

void Debugging(){
    #ifdef  DEBUG
      display.setCursor(100,24);
      display.println(ButtonReadValue);
     display.setCursor(100,34);
      display.println(HomeButton);
      
  #endif   
    
}

void ResetSleepingTime(){
      PreviousSleepTime = SleepTime;
}


void ReadTheButtonValue() {
    ButtonReadValue = analogRead(A0);
}


void ReadHomeButtonState() {
    HomeButton = digitalRead(HomeButtonPin) == 1?false:true;
}
