/*
 * This file deals with the general functions of the Board, such as start up, close down, uart, sensors, etc..
 */

// if you want both, you MUST set and clear one of the variables every sample
boolean addAccelToSD = false; // On writeDataToSDcard() call adds Accel data to SD card write
boolean addAuxToSD = false; // On writeDataToSDCard() call adds Aux data to SD card write

// Initializing the FTDI function instead of setting up the LED
// TX is D11
// RX is D12 

void init_board()
{ 
  ftdi("Initializing board"); 
  
  board.setSampleRate(0b110); // Setting Sample Rate to 250 Hz
  if(daisy_flag)
  {
    board.daisyPresent = true; // Telling the board that there is a Daisy Board Attached
  }
  else
  {
    board.daisyPresent = false; // Telling the board that there is not a Daisy Board attached
  }
  
  board.activateChannel(1);
  board.activateChannel(2);
  board.activateChannel(3);
  board.activateChannel(4);
  board.activateChannel(5);
  board.activateChannel(6);
  board.activateChannel(7);
  board.activateChannel(8);
  ftdi("Activated channels 1-8");

//  board.useAccel(true); // Notify the board we want to use accel data
//  addAuxToSD = false;  // Option to add/remove auxiliary data to stream
  
}

void init_ftdi()
{
  if(ftdi_flag)
  {
    board.beginSerial1(); // Setup Serial1, so pins D11 and D12 are dedicated to Serial1
  }
  if(ble_flag)
  {
    board.beginSerial0(); // Setup Serial0 for communication over BLE 
  }
}

// Use the FTDI function to communicate data through Serial1 port
//This function depends on the ftdi_flag boolean. This can be turned on/off by changing the ftdi_flag
void ftdi(String message)
{
//  if(!board.streaming)
//  {
    if(ftdi_flag)
    {
      Serial1.println(message); // Taking the input message and sending it through Serial1 port
    }
    if(ble_flag)
    {
      Serial0.print(message); // Taking the input message and sending it through Serial0 port
    }
//  }
}

//  Reads from the ADS1299 chip
//  Depends on the data_recording flag set by the button push
void readChannels()
{
    if(data_recording)
    {
        if(board.channelDataAvailable)
        {
          board.updateChannelData();  // Read from the ADS(s), store data, set channelDataAvailable flag to false 
          write_sd(); // Write Data to the SD Card depends on sd_flag, sd_state, SDfileOpen 
        }
    }
}

