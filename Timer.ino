#include <LiquidCrystal.h>
#define START_STOP_B A2
#define SECONDS_B A1
#define MINUTES_B A0

LiquidCrystal display(3,2,7,6,5,4); 
long timerControlInt = 0; // Timer 2 interrupt count
long current, last; // used for button press duration calculation
void timerMode(int8_t mode) { // TCNT2 control
  TCCR2A = 0; // reset registers
  TCCR2B = 0;
  TIMSK2 = 0;
  TCNT2 = 0;
  timerControlInt = 0; // reset
  if (mode == 1){
    TCCR2B |= 1; // set up TCNT2 to 16 MHz and turn on overflow interrupts
    TIMSK2 |= 1; 
  }
}

void timerControl();

struct Buttons {
  uint8_t startStop:1;
  uint8_t minutes:1;
  uint8_t seconds:1;
} buttons;

class Timer { // Timer START
  public:
  int8_t raw_time, seconds, minutes, status;
  void update() { // main function
    raw_time++;
    if (raw_time == 2) {
      raw_time = 0;
      if(seconds == 0) {
        minutes--;
        seconds = 59;
      } else {
        seconds--;
        if(seconds == 0 && minutes == 0) {
          reset();
          alarm();
        }
      }
    }
    show();
  }
  void alarm() { // anything could be placed here
    tone(8,1000,1000);
  }
  void set(int8_t mins, int8_t secs) { // minutes and seconds setup function
    seconds = secs;
    if(seconds >= 60)
      seconds = 0;
      
    minutes = mins;
    if(minutes >= 60)
      minutes = 0;
    show();
  }
  void reset() { // resets and stops timer
    raw_time = 0;
    seconds = 0;
    minutes = 0;
    timerMode(0);
    show();
    status = 0;
  }
  void start () { //starts timer
    if(!(seconds == 0 && minutes == 0)){ // if time is 00:00, play alarm immediately
      timerMode(1);
      status = 1;
      show();
    } else alarm();
  }
  void stop() { // pause timer
    timerMode(0);
    status = 0;
    show();
  }
  void show() { // display clock on the lcd display
    display.clear();
    display.setCursor(5,0);
    if (minutes < 10) {
      display.print('0');
      display.print(minutes);
    } else 
      display.print(minutes);

      if (raw_time == 0 || status == 0)
        display.print(':');
      else
        display.print(' ');
        
    if (seconds < 10) {
      display.print('0');
      display.print(seconds);
    } else 
      display.print(seconds);
  }
  Timer() { // Timer class constructor
    raw_time = 0;
    seconds = 0;
    minutes = 0;
    status = 0;
  }
} timer1; // Timer END

void setup() {
  // put your setup code here, to run once:
  display.begin(16,2);
  pinMode(START_STOP_B, INPUT); // input is 3 buttons
  pinMode(MINUTES_B, INPUT);
  pinMode(SECONDS_B, INPUT);
  sei(); // enable iterrupts
  timer1.show();
}

void loop() { // void loop START
  buttons.startStop = digitalRead(START_STOP_B); // read button states
  buttons.minutes = digitalRead(MINUTES_B);
  buttons.seconds = digitalRead(SECONDS_B);
  if (!buttons.startStop && !buttons.minutes && buttons.seconds) { // executes if only the seconds button is pressed
    timer1.set(timer1.minutes, timer1.seconds+1); // increment seconds
    last = millis();
    while(digitalRead(SECONDS_B)) { // while seconds button is held
      current = millis();
      if(current - last > 333) {
        timer1.set(timer1.minutes, timer1.seconds+1); // keep incrementing seconds
        last = millis();
      }
    }
    timer1.show();
  }
  if (!buttons.startStop && buttons.minutes && !buttons.seconds){ // executes if only the minutes button is pressed
    timer1.set(timer1.minutes+1, timer1.seconds); // increment minutes
    last = millis();
    while(digitalRead(MINUTES_B)) { // while minutes button is held
      current = millis();
      if(current - last > 333) {
        timer1.set(timer1.minutes+1, timer1.seconds); // keep incrementing minutes
        last = millis();
      }
    }
    timer1.show();
  }
  if (buttons.startStop && !buttons.minutes && !buttons.seconds) { // executes if only the start and stop button is pressed
    if (timer1.status == 0) // toggle timer
      timer1.start();
    else if(timer1.status == 1)
      timer1.stop();
    last = millis();
    while(digitalRead(START_STOP_B)) { // if the start and stop is held
      current = millis();
      if (current - last > 1000) { 
        timer1.reset(); // reset the timer
        last = millis();
      }
    }
  }  
} // void loop END

void timerControl() { // transforms timer interrupts into seconds
  timerControlInt++;
  if(timerControlInt == 31250) { // transforms 31250 interrupts into 0.5 seconds
    timerControlInt = 0;
    timer1.update();
  }
}

ISR(TIMER2_OVF_vect) { // interrupt calls transformation function
  timerControl();
}
