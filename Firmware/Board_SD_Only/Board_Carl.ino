/*
 * This file deals with the general functions of the Board, such as start up, close down, uart, sensors, etc..
 */

// Initializing the FTDI function instead of setting up the LED
// TX is D11
// RX is D12 

void init_board()
{ 
  ftdi("Initializing board"); 
  board.setSampleRate(sampling_rate); // Setting Sample Rate of board to user set sampling rate
  
  if(daisy_flag)
  {
    board.daisyPresent = true; // Telling the board that there is a Daisy Board Attached
    board.attachDaisy();
    board.activateChannel(9);
    board.activateChannel(10);
    board.activateChannel(11);
    board.activateChannel(12);
    board.activateChannel(13);
    board.activateChannel(14);
    board.activateChannel(15);
    board.activateChannel(16);
    ftdi("Activated channels 9-16");
  }
  else
  {
    board.removeDaisy();  // Remove Daisy Board if attached
    board.daisyPresent = false; // Telling the board that there is not a Daisy Board attached
  }

  // Automatically Activate Channels 1 - 8
  board.activateChannel(1);
  board.activateChannel(2);
  board.activateChannel(3);
  board.activateChannel(4);
  board.activateChannel(5);
  board.activateChannel(6);
  board.activateChannel(7);
  board.activateChannel(8);
  ftdi("Activated channels 1-8");

  if(accel_flag)
  {
      board.enable_accel(50); // Enable Accelerometer to sample at 50 Hz
      board.useAccel(true); // Notify the board we want to use accel data
  }
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
          readAccel();  // Write Accel Data to SD Card 
          write_sd(); // Write Data to the SD Card depends on sd_flag, sd_state, SDfileOpen 
        }
    }
}

//  Reads and Checks to see if the Accelerometer has data to be sent
//  This Data is available at 50 Hz by default. Could be set to 25 Hz
void readAccel()
{
  // Checks to see if user has enabled Accelerometer Data to be written to SD Card
  if(accel_flag)
  {
     // Check to see if accel has new data
      if(board.curAccelMode == board.ACCEL_MODE_ON) 
      {
        if(board.accelHasNewData()) 
        {
          // Get new accel data
          board.accelUpdateAxisData();

          // Tell the SD_Card_Stuff.ino to add accel data in the next write to SD
          addAccelToSD = true; // Set false after writeDataToSDcard()
        }
      } 
      else 
      {
        addAuxToSD = true;
      }
  }    
}

