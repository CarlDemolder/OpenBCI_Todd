#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>
#include <OpenBCI_Wifi_Master_Definitions.h>
#include <OpenBCI_Wifi_Master.h>

#include <DSPI.h>
#include <OBCI32_SD.h>
#include <EEPROM.h>

//  Custom User Characteristic Parameters
byte sampling_rate = 0b110; // Byte value used to set the sampling rate
/*  Variable Sample rate and Settings. These Sampling rate values are for 8 channels. 
 * 250 Hz = 0b110
 * 500 Hz = 0b101
 * 1000 Hz = 0b100
 * 2000 Hz = 0b011
 * 4000 Hz = 0b010
 * 8000 Hz = 0b001
 * 16000 Hz = 0b000
 */

uint32_t BLOCK_COUNT = 3122000; // Max number of blocks to be written for a file, i.e. setting the recording time of the board
/* 36.6667 Block Count Units per Second (Theoretrical)
 * BLOCK_COUNT = 11000; Time = 5 mins
 * BLOCK_COUNT = 33000; Time = 15 mins
 * BLOCK_COUNT = 66000; Time = 30 mins
 * BLOCK_COUNT = 131000; Time = 1 Hr
 * BLOCK_COUNT = 261000; Time = 2 Hrs
 * BLOCK_COUNT = 521000; Time = 4 Hrs
 * BLOCK_COUNT = 1561000; Time = 12 Hrs
 * BLOCK_COUNT = 3122000; Time = 24 Hrs
 */
 
//Flags set by user to enable certain functions
boolean ftdi_flag = false;            // Boolean used to determine if an FDTI serial to UART converter is attached to the board
boolean sd_flag = true;               // Boolean used to enable or disable SD Writing and Recording
boolean ble_flag = false;             // Boolean used to enable or disable BLE 
boolean daisy_flag = false;           // Boolean used to enable or disable Daisy Board Integration
boolean block_rollover_flag = false;  // Boolean used to enable Block writing roll-over, i.e. continue recording onto the next block  
boolean accel_flag = true;            // Boolean used to enable Accelerometer Data to be recorded and written to SD Card

//Variables used across Added Files
boolean data_recording = false;     // Boolean used to determine if the user wants to start recording data 
boolean led_state = false;          // used to toggle the on-board LED
boolean sd_state = false;           // Boolean used to toggle SD write/read card file
boolean SDfileOpen = false;         // Boolean used to determine if the SD card file is open or closed
boolean sd_writing = false;         // Boolean used to determine if the SD card file is open, initialized, and is writing
boolean addAccelToSD = false;       // On writeDataToSDcard() call adds Accel data to SD card write, Default = false
boolean addAuxToSD = false;         // On writeDataToSDCard() call adds Aux data to SD card write, Default = false
byte fileCounter = 0;               // file counter to reset and create a new file with a new name if the block is full. 

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
