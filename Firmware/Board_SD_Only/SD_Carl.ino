/*
 * This File was created by Carl Demolder
 * It is a modified version of SD_Card_Stuff
 * This move makes it much easier to understand
 * Added alot of comments and Documentation on what I did
 */
#define OVER_DIM 20 // make room for up to 20 write-time overruns


char fileSize = '0';  // SD file size indicator
int blockCounter = 0; // Counting the number of blocks written for a given file size

// Arduino SD Card File and Parameters
SdFile openfile;  // want to put this before setup...
Sd2Card card(&board.spi,SD_SS);// SPI needs to be init'd before here
SdVolume volume;
SdFile root;

uint8_t* pCache;      // array that points to the block buffer on SD card
uint32_t MICROS_PER_BLOCK = 2000; // block write longer than this will get flaged
uint32_t bgnBlock, endBlock; // file extent bookends
int byteCounter = 0;    // used to hold position in cache

boolean openvol;

struct {
  uint32_t block;   // holds block number that over-ran
  uint32_t micro;  // holds the length of this of over-run
} over[OVER_DIM];
uint32_t overruns;      // count the number of overruns
uint32_t maxWriteTime;  // keep track of longest write time
uint32_t minWriteTime;  // and shortest write time
uint32_t t;        // used to measure total file write time

byte fileTens, fileOnes;  // enumerate succesive files on card and store number in EEPROM
char currentFileName[] = "00EGG00.TXT"; // file name of text file that will be stored on the SD Card. It will enumerate in hex 00 - FF
prog_char elapsedTime[] PROGMEM = {"\n%Total time mS:\n"};  // 17
prog_char minTime[] PROGMEM = {  "%min Write time uS:\n"};  // 20
prog_char maxTime[] PROGMEM = {  "%max Write time uS:\n"};  // 20
prog_char overNum[] PROGMEM = {  "%Over:\n"};               //  7
prog_char blockTime[] PROGMEM = {  "%block, uS\n"};         // 11    74 chars + 2 32(16) + 2 16(8) = 98 + (n 32x2) up to 24 overruns...
prog_char stopStamp[] PROGMEM = {  "%STOP AT\n"};      // used to stamp SD record when stopped by PC
prog_char startStamp[] PROGMEM = {  "%START AT\n"};    // used to stamp SD record when started by PC

boolean cardInit = false; // Initializing SD Card Boolean to check and make sure SD Card has been initialized

byte SDsampleCounter = 0;

// This function Initializes the SD Card
// It does the following:
void init_sd()
{
  if(sd_flag)
  {
    ftdi("Initializing SD Card...");
    SDfileOpen = setupSDcard(); // Setting up the SD card
    if(SDfileOpen)
    {
      stampSD(ACTIVATE);
    }
  }
}

// Setting up the SD Card
// It will check if there is an SD present, if the SD card is formatted properly, etc...
boolean setupSDcard()
{
  if(!cardInit)
  {
      //  This if statement is checking to see if there is an SD card mounted
      if(!card.init(SPI_FULL_SPEED, SD_SS)) 
      {
          ftdi("initialization failed. No SD Card Found.");
      } 
      else // This assumes that the SD card is mounted properly...
      {
          ftdi("SD Card is present. cardInit = true");
          cardInit = true;  //Setting Initializing Card Boolean to True
      }
      
      //  This function will occur if there is an SD Card present, but the SD card is not properly formatted
      if (!volume.init(card)) 
      { 
        // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
          ftdi("Could not find FAT16/FAT32 partition. Make sure you've formatted the card");
        return SDfileOpen;  // Returning the state of the SD Card open, in this case it would return false 
      }
   }  
   
  // The end of the this if statement states that the SD card has been initialized
  // By this point in the code, there should be a properly formatted SD Card and been properly Initialized

  incrementFileCounter(); // Accesses EEPROM to increment the file name count
  
  openvol = root.openRoot(volume);
  openfile.remove(root, currentFileName); // if there is a file with the same name, the program will over write it

  if (!openfile.createContiguous(root, currentFileName, BLOCK_COUNT*512UL)) // SdFat: openfile.createContiguous() Create and open a new contigous file of a specified size at root directory, a path with a validfile name, the desired file size
  {
      ftdi("createfdContiguous fail");
      cardInit = false;
  }

  // get the location of the file's blocks
  if (!openfile.contiguousRange(&bgnBlock, &endBlock)) // SdFat: openfile.contiguousRange() Check for contigous file and return its raw block range &bgnBlock and &endBlock
  {
      ftdi("get contiguousRange fail");
      cardInit = false;
  }

  // grab the Cache
  pCache = (uint8_t*)volume.cacheClear();
  // tell card to setup for multiple block write with pre-erase
  if (!card.erase(bgnBlock, endBlock))  // SdFat: card.erase(bgnBlock, endBlock) Erase a range of blocks, from firstBlock to lastBlock
  {
      ftdi("erase block fail");
      cardInit = false;
  }

  if (!card.writeStart(bgnBlock, BLOCK_COUNT))  // SdFat: card.writeStart() Start a write multiple blocks sequence with pre-erase, bgnBlock = First block Address, BLOCK_COUNT = Number of blocks to be pre-erased
  {
      ftdi("writeStart fail");
      cardInit = false;
  } 
  else
  {
    SDfileOpen = true;
    delay(1);
  }
  
  board.csHigh(SD_SS);  // release the spi

  
  // initialize write-time overrun error counter and min/max wirte time benchmarks
  overruns = 0;
  maxWriteTime = 0;
  minWriteTime = 65000;

  // Reset Block and Byte Counter to ensure proper Data Storage on SD Card
  byteCounter = 0;  // counter from 0 - 512
  blockCounter = 0; // counter from 0 - BLOCK_COUNT;
  
  if(SDfileOpen == true)
  {  
      // send corresponding file name to controlling program
      ftdi("SDfileOpen =  ");   ftdi(currentFileName);
  }
  return SDfileOpen;
}

//  write data to SD card
// Dependents: 
//  sd_flag = true, i.e. user wants to store data on SD card
//  sd_state = true, i.e. button has been pushed enabling storage of data on SD card
//  SDfileOpen = true, i.e. file has been created on SD card
void write_sd()
{
  if(sd_flag && sd_state && SDfileOpen)
  {
    sd_writing = true;  // If the following parameters are true, then the board is writing to the SD Card
    writeDataToSDcard(SDsampleCounter++);
  }
  else
  {
    sd_writing = false; // SD card parameters haven't been met, so the Board is not writing to the SD Card
  }
}

void writeDataToSDcard(byte sampleNumber)
{
  boolean addComma = true;
  // convert 8 bit sampleCounter into HEX
  convertToHex(sampleNumber, 1, addComma);
  // convert 24 bit channelData into HEX
  for (int currentChannel = 0; currentChannel < 8; currentChannel++)
  {
    convertToHex(board.boardChannelDataInt[currentChannel], 5, addComma);
    if(board.daisyPresent == false)
    {
      if(currentChannel == 6)
      {
        addComma = false;
        if(addAuxToSD || addAccelToSD) 
        {
          addComma = true;
        }  // format CSV
      }
    }
  }
  if(board.daisyPresent)
  {
    for (int currentChannel = 0; currentChannel < 8; currentChannel++)
    {
      convertToHex(board.daisyChannelDataInt[currentChannel], 5, addComma);
      if(currentChannel == 6)
      {
        addComma = false;
        if(addAuxToSD || addAccelToSD) 
        {
          addComma = true;
        }  // format CSV
      }
    }
  }

  if(addAuxToSD == true)
  {
    // convert auxData into HEX
    for(int currentChannel = 0; currentChannel <  3; currentChannel++)
    {
      convertToHex(board.auxData[currentChannel], 3, addComma);
      if(currentChannel == 1) addComma = false;
    }
    addAuxToSD = false;
  }
  // end of aux data log
  else if(addAccelToSD == true)
  {  
    // if we have accelerometer data to log
    // convert 16 bit accelerometer data into HEX
    for (int currentChannel = 0; currentChannel < 3; currentChannel++)
    {
      convertToHex(board.axisData[currentChannel], 3, addComma);
      if(currentChannel == 1) addComma = false;
    }
    addAccelToSD = false;  // reset addAccel
  }
  // end of accelerometer data log
  // add aux data logging...
}


// Write the cache on the card
void writeCache()
{
    if(blockCounter > BLOCK_COUNT) return;
    uint32_t tw = micros();  // start block write timer
    board.csLow(SD_SS);  // take spi
    if(!card.writeData(pCache)) // SdFat: card.writeData(): Write one data block in a multiple block write sequence, Input = const uint8_t* pointer to the location of the data to be written
    {
      if (!board.streaming) 
      {
        ftdi("block write fail");
      }
      button_unpressed();
    }   // write the block
    board.csHigh(SD_SS);  // release spi
    tw = micros() - tw;      // stop block write timer
    if (tw > maxWriteTime) maxWriteTime = tw;  // check for max write time
    if (tw < minWriteTime) minWriteTime = tw;  // check for min write time
    if (tw > MICROS_PER_BLOCK) 
    {     
      // check for overrun
      if (overruns < OVER_DIM) 
      {
        over[overruns].block = blockCounter;
        over[overruns].micro = tw;
      }
      overruns++;
    }
    byteCounter = 0; // reset 512 byte counter for next block
    blockCounter++;    // increment BLOCK counter
    if(blockCounter == BLOCK_COUNT-1)
    {
      t = millis() - t;
    }
    if(blockCounter == BLOCK_COUNT)
    {
      close_sd(); // Close the SD Card and reset State Variables: SDfileOpen
      if(block_rollover_flag)
      {
          fileCounter++;  // Increment the file Counter
          init_sd();  // Initialize and Create another SD Card
      }
    }
}

//  Getting the file count from EEPROM to increment the file number if necessary
//  These values are stored locally on the MCU, i.e. in EEPROM
void incrementFileCounter()
{
    if(fileCounter == 0)
    {
        fileTens = EEPROM.read(0);
        fileOnes = EEPROM.read(1);
        // if it's the first time writing to EEPROM, seed the file number to '00'
        if(fileTens == 0xFF | fileOnes == 0xFF)
        {
            fileTens = fileOnes = '0';
        }
        fileOnes++;   // increment the file name
        if (fileOnes == ':'){fileOnes = 'A';}
        if (fileOnes > 'F')
        {
            fileOnes = '0';         // hexify
            fileTens++;
            if(fileTens == ':'){fileTens = 'A';}
            if(fileTens > 'F'){fileTens = '0';fileOnes = '1';}
        }
        EEPROM.write(0,fileTens);     // store current file number in eeprom
        EEPROM.write(1,fileOnes);
        currentFileName[0] = fileTens;  //Modifying the file name to change the tens place
        currentFileName[1] = fileOnes;  //Modifying the file name to change the ones place
        currentFileName[5] = '0';  //Modifying the file name to reflect the file number
        currentFileName[6] = '0';  //Modifying the file name to reflect the file number
    }
    else
    {
        String hexString = String(fileCounter, HEX);  // Converting byte to hex   
        while (hexString.length() < 2) 
        {
          hexString = "0" + hexString;  // Padding the Value with a Zero if need be
        }
        currentFileName[5] = hexString[0];  //Modifying the file name to reflect the file number
        currentFileName[6] = hexString[1];  //Modifying the file name to reflect the file number
    }
}

//    CONVERT RAW BYTE DATA TO HEX FOR SD STORAGE
void convertToHex(long rawData, int numNibbles, boolean useComma)
{
  for (int currentNibble = numNibbles; currentNibble >= 0; currentNibble--)
  {
    byte nibble = (rawData >> currentNibble*4) & 0x0F;
    if (nibble > 9)
    {
      nibble += 55;  // convert to ASCII A-F
    }
    else
    {
      nibble += 48;  // convert to ASCII 0-9
    }
    pCache[byteCounter] = nibble;
    byteCounter++;
    if(byteCounter == 512)
    {
      writeCache();
    }
  }
  if(useComma == true)
  {
    pCache[byteCounter] = ',';
  }
  else
  {
    pCache[byteCounter] = '\n';
  }
  byteCounter++;
  if(byteCounter == 512)
  {
    writeCache();
  }
}

//  Called when the user wants to close and finish writing to the file
//  writes stats to the SD card about how long it took to write, etc...
void writeFooter()
{
  for(int i=0; i<17; i++)
  {
    pCache[byteCounter] = pgm_read_byte_near(elapsedTime+i);
    byteCounter++;
  }
  convertToHex(t, 7, false);

  for(int i=0; i<20; i++)
  {
    pCache[byteCounter] = pgm_read_byte_near(minTime+i);
    byteCounter++;
  }
  convertToHex(minWriteTime, 7, false);

  for(int i=0; i<20; i++)
  {
    pCache[byteCounter] = pgm_read_byte_near(maxTime+i);
    byteCounter++;
  }
  convertToHex(maxWriteTime, 7, false);

  for(int i=0; i<7; i++)
  {
    pCache[byteCounter] = pgm_read_byte_near(overNum+i);
    byteCounter++;
  }
  convertToHex(overruns, 7, false);
  for(int i=0; i<11; i++)
  {
    pCache[byteCounter] = pgm_read_byte_near(blockTime+i);
    byteCounter++;
  }
  if (overruns) 
  {
    uint8_t n = overruns > OVER_DIM ? OVER_DIM : overruns;
    for (uint8_t i = 0; i < n; i++) 
    {
      convertToHex(over[i].block, 7, true);
      convertToHex(over[i].micro, 7, false);
    }
  }
  for(int i=byteCounter; i<512; i++)
  {
    pCache[i] = NULL;
  }
  writeCache();
}

void stampSD(boolean state)
{
  unsigned long time = millis();
  if(state)
  {
    for(int i=0; i<10; i++)
    {
      pCache[byteCounter] = pgm_read_byte_near(startStamp+i);
      byteCounter++;
      if(byteCounter == 512)
      {
        writeCache();
      }
    }
  }
  else
  {
    for(int i=0; i<9; i++)
    {
      pCache[byteCounter] = pgm_read_byte_near(stopStamp+i);
      byteCounter++;
      if(byteCounter == 512)
      {
        writeCache();
      }
    }
  }
  convertToHex(time, 7, false);
}


// Close SD Card by first Deactivating the stampSD
// Then by closing the SD file
//  Dependents:
//    -sd_flag = true, so user wants to write to sd card
//    -sd_state = true, so user presses button the enable sd card writing
//    -SDfileOpen = true, so the SD card file is opened and has been initialized
void close_sd()
{
  if(sd_flag && sd_state && SDfileOpen)
  {
//        stampSD(DEACTIVATE);
//        writeFooter();
        board.csLow(SD_SS);  // take spi
        card.writeStop();     // SdFat: End a write multiple blocks sequence
        openfile.close();     // Close the file created on the SD Card; Arduino SD Library function
        board.csHigh(SD_SS);  // release the spi       
        SDfileOpen = false;   // Setting SD file boolean to false = closed
        ftdi("Closing SD Card");
  }
}
