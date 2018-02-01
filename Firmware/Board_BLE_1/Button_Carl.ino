/*
 * Button File written to simplify Button functions
 * Initialize functions and buttons
 */

//  Button Variables
int pushButton = 17;        // the button is on pin D17
int pushButtonValue;        // used to hold the latest button reading
int lastPushButtonValue;    // used to remember the last button state
boolean button_pushed = false; // Boolean used to determine if button has been pushed or not pushed

// Initialize Button
// Presetting the value of the button to low
void init_button()
{
  pinMode(pushButton, INPUT);        // set the button pin direction
  pushButtonValue = lastPushButtonValue = LOW; // Initializing Push Button Values to 0 (LOW)
  ftdi("Button initialized");
}


// Button interrupt sequence to start and stop SD writing process
void button_interrupt()
{
  pushButtonValue = digitalRead(pushButton);    // See if the PROG button has been pressed
  //  Old = LOW, New = HIGH
  if(pushButtonValue == HIGH && lastPushButtonValue == LOW)  // if it's gone from LOW to HIGH
  {
    button_pushed = true;  // Button has been pressed
  }
  //  Old = High, New = Low
  else if(pushButtonValue == LOW && lastPushButtonValue == HIGH)
  {
      if(button_pushed == true && data_recording == false)
      {
        fileCounter = 0;    // Reseting the file Counter to 0
        init_sd();  // Initializing SD Card
        data_recording = true;  // Enable Data Recording
        led_state = true;  // Turn off Blinking LED
        board.streamStart(); // Start streaming from the ADS1299
        sd_state = true;    //If the user presses the button, it enables writing to the SD card
        ftdi("User pressed Button");
      }
      else
      {
        data_recording = false; // Disable Data Recording
        close_sd(); // Closing SD Card Card
        board.streamStop(); // Stop Streaming from the ADS1299
        led_state = false;  // Turn Off Blinking LED
//        toggle_led(1);  // Turn LED Steady On   
        sd_state = false;    // Disable the user from writing to SD card
        ftdi("User Unpressed Button");
      }
      button_pushed = false; // Resetting Button Pushed
  }
  lastPushButtonValue = pushButtonValue; // keep track of the changes!
}
