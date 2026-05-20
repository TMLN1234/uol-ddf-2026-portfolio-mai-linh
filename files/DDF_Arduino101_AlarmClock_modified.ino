//LIBRARIES - all libraries can be installed via Arduino Library Manager
#include <LiquidCrystal_I2C.h> //library for LCD display
#include <Wire.h>              // library for I2C connection
#include <RTClib.h>            // library for real time clock 
#include <Button.h>            // library for easy handling of buttons: https://github.com/madleech/Button

LiquidCrystal_I2C lcd(0x27, 18, 2); //declaration of the Display
RTC_DS1307 rtc;                     //declaration of the RTC device

//PINOUT
#define buzzerPin 4
#define ledPin 10

//Alarm Setting Variables
bool isAlarmSet = false;
bool didItRing = false;
bool snoozeActive = false;
bool alarmCurrentlyRinging = false;
int AlarmHH = 12; //storing the alarm time setting - hours
int AlarmMM = 33;  //storing the alarm time setting - minutes
#define howManyRings 5 //constant defining how many beeps the alarm should make per launch

int CurrentHH = 0; //storing the current time - hours
int CurrentMM = 0; //storing the current time - minutes
bool ifTimeDisplay = true;  //boolean value to decide if to show time, useful for limiting flickering of the screen

//Button Declarations and Control Variables
Button redButton(2); 
Button whiteButton(13);
Button yellowButton(12);
Button greenButton(11);
Button blackButton(7);
Button blueButton(9);

bool whiteButtonState = false; // activate/deative alarm
bool redButtonState = false; //change to set alarm mode
bool yellowButtonState = false; //increase hour
bool greenButtonState = false; //increase minute
bool blackButtonState = false; //decrease minute
bool blueButtonState = false; //snooze button

int SnoozeHH = 0;
int SnoozeMM = 0;
int SnoozeSS = 0;

unsigned long alarmStartTime = 0; // Tracks the background beep pattern timing

//Setup function, executed once after reboot
void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin,LOW);

  Serial.begin(9600); //initialisation of serial monitor

  //LCD display initialisation
  lcd.init();
  lcd.backlight();

  //RTC initialisation
  rtc.begin();
  //rtc.adjust(DateTime(2026, 05, 07, 14, 12, 10)); //this sets the "start" time for the clock

  //Buttons initialisation
  redButton.begin();
  whiteButton.begin();
  yellowButton.begin();
  greenButton.begin();
  blackButton.begin();
  blueButton.begin();
}

//timeOnScreen function reads the time from RTC and displays it on the LCD display
void timeOnScreen(){
  DateTime now = rtc.now(); //declaring an object for storing the RTC time
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  if(now.hour()<10) lcd.print('0'); //used to maintain nice format on the display
  lcd.print(now.hour());
  CurrentHH = now.hour();
  lcd.print(":");
  if(now.minute()<10) lcd.print('0');
  lcd.print(now.minute());
  CurrentMM = now.minute();
  lcd.print(":");
  if(now.second()<10) lcd.print('0');
  lcd.print(now.second());
  lcd.print("    ");
}

//alarmToggle function switches the setting of the alarm On and Off, using the white button
int alarmToggle(){
  if(whiteButton.released()){
    Serial.print("Alarm clock state: ");
    
    // If the alarm is ringing, pressing the white button completely turns it OFF
    if(alarmCurrentlyRinging) {
      alarmCurrentlyRinging = false;
      isAlarmSet = false;
      whiteButtonState = false;
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Alarm is OFF        ");
      return 0;
    }

    whiteButtonState = !whiteButtonState;

    if(whiteButtonState){
      lcd.setCursor(0, 1);
      lcd.print("Alarm is ON         ");
      isAlarmSet = true;
    }else{
      lcd.setCursor(0, 1);
      lcd.print("Alarm is OFF        ");
      isAlarmSet = false;
      didItRing = false;
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
      alarmCurrentlyRinging = false;
      snoozeActive = false;
    }
  }

  if(isAlarmSet == 1) return 1;
  else return 0;
}

//menuToggle function switches the clock view into set-the-alarm view  
void menuToggle(){
  if(redButton.released()) redButtonState = !redButtonState;

  if(redButtonState){
    ifTimeDisplay = false;
    setAlarm();
    lcd.setCursor(0, 0);
    lcd.print("Alarm set: ");
    if(AlarmHH <10) lcd.print("0");
    lcd.print(AlarmHH);
    lcd.print(":");
    if(AlarmMM <10) lcd.print("0");
    lcd.print(AlarmMM);
           
  } 
  if(!redButtonState) 
  {
    ifTimeDisplay = true;
    return;
  }
}

//setAlarm function operates Yellow and Green Buttons to set the desired alarm time in the set-the-alarm view
int setAlarm(){
  if(redButtonState){
    if(yellowButton.released()){
      AlarmHH++;
      if(AlarmHH>23) AlarmHH=0;
      return 1;
    }
    if(greenButton.released()){
      AlarmMM++;
      if(AlarmMM>59) AlarmMM=0;
      return 1;
    }
    if(blackButton.released()){
      AlarmMM--;
      if(AlarmMM<0) AlarmMM=59;
      return 1;
    }
  }else return 0;
}   

//----------------------------SNOOZE FUNCTION------------------------------
void snoozeFunction(){
  if(blueButton.released() && alarmCurrentlyRinging){
    // Stop current alarm noise instantly
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
    alarmCurrentlyRinging = false;
    snoozeActive = true;
    
    DateTime now = rtc.now();
    SnoozeHH = now.hour();
    SnoozeMM = now.minute();
    SnoozeSS = now.second() + 10; // 10 second snooze for quick testing

    // Handle time rollover variables safely
    if(SnoozeSS > 59){
      SnoozeSS -= 60;
      SnoozeMM++;
      if(SnoozeMM > 59){
        SnoozeMM = 0;
        SnoozeHH++;
        if(SnoozeHH > 23){
          SnoozeHH = 0;
        }
      }
    }
    lcd.setCursor(0,1);
    lcd.print("Snooze Active   ");
  }
}

//--------------------ALARM RING------------------------------------------------
//alarmRing function checks the alarm conditions and enables the buzzer 
void alarmRing(){

  // NORMAL ALARM TRIGGER
  if(isAlarmSet == true &&
     CurrentHH == AlarmHH &&
     CurrentMM == AlarmMM &&
     didItRing == false){
      alarmCurrentlyRinging = true;
      didItRing = true;
      alarmStartTime = millis(); // Start pattern stopwatch
      lcd.setCursor(0,1);
      lcd.print("Alarm Rings!    ");
  }

  // SNOOZE ALARM TRIGGER
  DateTime now = rtc.now();
  if(snoozeActive == true &&
     now.hour() == SnoozeHH &&
     now.minute() == SnoozeMM &&
     now.second() == SnoozeSS){
      alarmCurrentlyRinging = true;
      snoozeActive = false;      // Turn off tracking flag so it doesn't loop trigger
      alarmStartTime = millis(); // Reset stopwatch for this ring cycle
      lcd.setCursor(0,1);
      lcd.print("Alarm Rings!    ");
  }

  // NON-BLOCKING ALARM SOUNDER
  if(alarmCurrentlyRinging == true) {
    unsigned long elapsed = millis() - alarmStartTime;
    unsigned long cycleTime = elapsed % 900; // Loops a 900ms double-beep cycle window

    if(cycleTime < 150){
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
    } 
    else if(cycleTime < 250){
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
    } 
    else if(cycleTime < 400){
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
    } 
    else {
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
    }
  }
}

void resetAlarmFlag(){
  static int lastMinute = -1;
  if(CurrentMM != lastMinute){
    didItRing = false;
    lastMinute = CurrentMM;
  }
}

void displaySnoozeCountdown(){
  if(snoozeActive){
    DateTime now = rtc.now();
    int currentTotalSeconds = now.hour() * 3600 + now.minute() * 60 + now.second();
    int snoozeTotalSeconds = SnoozeHH * 3600 + SnoozeMM * 60 + SnoozeSS;
    int remaining = snoozeTotalSeconds - currentTotalSeconds;
    
    if(remaining < 0){
      remaining = 0;
    }
    int remainingMinutes = remaining / 60;
    int remainingSeconds = remaining % 60;
    
    lcd.setCursor(0,1);
    lcd.print("Snooze: ");
    if(remainingMinutes < 10) lcd.print("0");
    lcd.print(remainingMinutes);
    lcd.print(":");
    if(remainingSeconds < 10) lcd.print("0");
    lcd.print(remainingSeconds);
    lcd.print("   ");
  }
}

//LOOP - main function of our programme
void loop() {
  // Read all physical button states at the start of every iteration
  redButton.read();
  whiteButton.read();
  yellowButton.read();
  greenButton.read();
  blackButton.read();
  blueButton.read();

  if(ifTimeDisplay) timeOnScreen(); //displaying the clock
  alarmToggle();                    //checking if the alarm function is on or off (and handling its input)
  menuToggle();                     //checking if the set-the-alarm view is on or off (and handling its input)
  snoozeFunction();
  resetAlarmFlag();
  displaySnoozeCountdown();
  alarmRing();                      //checking if the the alarm should ring (and executing it if yes)
}
