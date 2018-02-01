int LED = 11;   // alias for the blue LED

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
void blink_led()
{
  if(!ftdi_flag)
  {
    if(led_state)
    {
      digitalWrite(LED,0);    // toggle the LED Low
      delay(500);             // Blink for 500 ms
      digitalWrite(LED,1);    // toggle the LED High
      delay(500);             // Blink for 500 ms
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

