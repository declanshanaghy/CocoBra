#include <Bounce.h>

// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

#define TRIGGER_COUNT 3
#define TRIGGER_TIME 1000
#define POP_INTERVAL 5000

#define POP_TIME 50
#define SHOW_TIME 5000
#define SLEEP_TIME 120000

#define MAIN_LED_STEADY 100
#define AUX_LED_MIN 25
#define AUX_LED_MAX 150

#define MAIN_LED_PIN 9  
#define POWER_PIN 10
#define TRIGGER_PIN 0

#define N_LEDS 5

int leds[] = {1, 2, 3, 7, 8};

// We need to declare the data exchange
// variable to be volatile - the value is
// read from memory.
volatile boolean readTrigger = false;

unsigned long cumulative = 0;
unsigned long firstTrigger = 0;
unsigned long lastTrigger = 0;
unsigned long lastMainBounce = 0;
unsigned long lastAuxBounce = 0;
unsigned long auxBounceTime = 0;
unsigned int triggerCount = 0;
unsigned long lastPop = 0;

Bounce trigger = Bounce(TRIGGER_PIN, 50);

void setup() {           
  // initialize the digital pin as an output.
  for (int i=0; i<N_LEDS; i++) {
    pinMode(leds[i], OUTPUT);     
    digitalWrite(leds[i], LOW);
  }
  
  pinMode(MAIN_LED_PIN, OUTPUT);     
  pinMode(TRIGGER_PIN, INPUT);
  
  lastTrigger = millis();
  firstTrigger = millis();
  lastPop = millis();
  
  int flash = 100;
  digitalWrite(MAIN_LED_PIN, HIGH);
  delay(flash);
  digitalWrite(MAIN_LED_PIN, LOW);
    
  for (int i=0; i<N_LEDS; i++) {
    digitalWrite(leds[i], HIGH);
    delay(flash);
    digitalWrite(leds[i], LOW);
  }
  
  enableInterrupt();
}

void enableInterrupt() {   
  PCMSK0 = (1 << PCINT0);  //Enable interrupts on PCINT0
  GIMSK = (1 << PCIE0);    //Enable interrupts period for PCI0 (PCINT7:0)
  sei();                   //Enables interrupts in SREG altogether
}

// Install the interrupt routine.
ISR(PCINT0_vect){
  cumulative += millis();
  readTrigger = true;
  if ( firstTrigger == 0 ) 
    firstTrigger = millis();
}

// the loop routine runs over and over again forever:
void loop() {
  if (triggerCount >= TRIGGER_COUNT) {
    goBouncy();
    if ( lastTrigger == 0 ) {
      randomSeed(cumulative);
      lastMainBounce = 0;
      lastAuxBounce = 0;
      lastTrigger = millis();    
    }
    if ( millis() > lastTrigger + SHOW_TIME ) {
      reset();
    }
    return;
  }
  else if (millis() > lastPop + POP_INTERVAL) {
      digitalWrite(MAIN_LED_PIN, HIGH);
      delay(POP_TIME);
      digitalWrite(MAIN_LED_PIN, LOW);
      lastPop = millis();
  }
  
  if ( readTrigger ) {
    readTrigger = false;
    trigger.update();
    if ( trigger.risingEdge() ) {
      triggerCount++;      
      int pin = random(N_LEDS);  
      digitalWrite(pin, HIGH);
      delay(50);
      digitalWrite(pin, LOW);
    }
  }
  else if ( firstTrigger != 0 && millis() > firstTrigger + TRIGGER_TIME ) {
    // Not enough triggers within the time window, reset
    reset();
  }
    
  if ( millis() > lastTrigger + SLEEP_TIME ) {
    powerOff();
  }
}

void powerOff() {
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
}

void reset() {
  for (int i=0; i<N_LEDS; i++) {
    digitalWrite(leds[i], LOW);
  }
  digitalWrite(MAIN_LED_PIN, LOW);
  triggerCount = 0;
  firstTrigger = 0;
  lastTrigger = 0;
  auxBounceTime = 0;
  lastMainBounce = 0;
  lastAuxBounce = 0;
  lastPop = millis();
}

void goBouncy() {
  //Flash the main LEDS in a steady sequence
  if ( millis() > lastMainBounce + MAIN_LED_STEADY ) {
    lastMainBounce = millis();
    int v = digitalRead(MAIN_LED_PIN);
    digitalWrite(MAIN_LED_PIN, !v);
  }
  
  //Flash the other LEDs randomly
  if ( millis() > lastAuxBounce + auxBounceTime ) {
    lastAuxBounce = millis();
    auxBounceTime = random(AUX_LED_MIN, AUX_LED_MAX);
    int pin = random(N_LEDS);  
    int v = digitalRead(leds[pin]);
    digitalWrite(leds[pin], !v);
  }
}
