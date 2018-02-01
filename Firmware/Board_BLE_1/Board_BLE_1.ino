#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>
#include <OpenBCI_Wifi_Master_Definitions.h>
#include <OpenBCI_Wifi_Master.h>

#include <DSPI.h>
#include <OBCI32_SD.h>
#include <EEPROM.h>

//Flags set by user to enable certain functions
boolean ftdi_flag = false;   // Boolean used to determine if an FDTI serial to UART converter is attached to the board
boolean sd_flag = true;    // Boolean used to enable or disable SD Writing and Recording
boolean ble_flag = false;   // Boolean used to enable or disable BLE 
boolean daisy_flag = false; // Boolean used to enable or disable Daisy Board Integration

//Variables used across Added Files
boolean data_recording = false; // Boolean used to determine if the user wants to start recording data 
boolean led_state = false;        // used to toggle the on-board LED
boolean sd_state = false; // Boolean used to toggle SD write/read card file
boolean SDfileOpen = false; // Boolean used to determine if the SD card file is open or closed
byte fileCounter = 0; // file counter to reset and create a new file with a new name if the block is full. 

void setup() 
{
  board.begin(); // Starting an OpenBCI Board Object
  init_ftdi();  // Initializing FTDI serial depending on ftdi_flag
  init_board(); // Initializing OpenBCI Board
  init_led();   // Initializing LED serial depending on ftdi_flag
  init_button();  // Initialize Button
  ftdi("Setup()");
}

void loop() 
{
  blink_led(); //Blink the LED until the button has been pressed
  button_interrupt(); //Interrupt notification service to begin SD data recording
  readChannels(); // Read the Channels from the ADS1299 if data_recording flag is true
  board.loop(); // Continously looping for board. Resetting Board timers, etc...
}
