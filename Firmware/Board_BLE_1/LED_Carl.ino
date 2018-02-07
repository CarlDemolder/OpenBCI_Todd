int LED = 11;   // alias for the blue LED
unsigned long previousMillis = 0;        // Non-Blocking LED previous Time Storage
unsigned long currentMillis = 0;    // Initializing Non-Blocking
const long led_interval = 500;          // Interval at which to Blink (milliseconds)
int current_led = 1;                    // Initializing LED current State

// Initializing the LED instead of using UART
void init_led()
{
  if(!ftdi_flag)
  {
    pinMode(LED, OUTPUT);   // set the LED pin to output
  }
  toggle_led(1);    // toggle the LED HIGH to a steady light on
}

//  Blink LED if LED is initialized, i.e. ftdi_flag is set to false
//  Depends on the state of the LED as well
//  Only Blink LED is the board is writing to the SD Card
void blink_led()
{
  if(!ftdi_flag && led_state && sd_writing)
  {
      currentMillis = millis(); // Getting current Milisecond Timer
      if((currentMillis - previousMillis) >= led_interval)
      {
            previousMillis = currentMillis;
            if(current_led)
            {
              current_led = 0;
              digitalWrite(LED,current_led);    // toggle the LED Low    
            }
            else
            {
              current_led = 1;
              digitalWrite(LED, current_led); // toggle the LED High
            }
      }
  }
  else
  {
    toggle_led(1);
    if(!ftdi_flag)
    {
      digitalWrite(LED, 1); // Toggle the LED High 
    }
  }
}

//  Toggle LED On/Off
void toggle_led(int led_state)
{
  if(!ftdi_flag)
  {
    if(!led_state)
    {
      digitalWrite(LED,led_state);    // toggle the LED HIGH  
    }
  }
}

