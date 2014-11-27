//////////////////////////////////////////
// SCAN SD DIRECTORIES INTO ARRAYS //////
/////////////////////////////////////////
void scanDirectory(File dir, int numTabs) {
  while(true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    String fileName = entry.name();

    if (fileName.endsWith(FILE_TYPE) && fileName.startsWith("_") == 0){
      int intCurrentDirectory = CURRENT_DIRECTORY.toInt();
      if (intCurrentDirectory > ACTIVE_BANKS) ACTIVE_BANKS = intCurrentDirectory; 
      FILE_NAMES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = entry.name();
      FILE_SIZES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = entry.size();
      FILE_DIRECTORIES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = CURRENT_DIRECTORY;
      FILE_COUNT[intCurrentDirectory]++;
    };
    // Ignore OSX Spotlight and Trash Directories 
    if (entry.isDirectory() && fileName.startsWith("SPOTL") == 0 && fileName.startsWith("TRASH") == 0) {
      CURRENT_DIRECTORY = entry.name();
      scanDirectory(entry, numTabs+1);
    } 
    entry.close();
  }
}


// TAKE BANK AND CHANNEL AND RETURN PROPERLY FORMATTED PATH TO THE FILE 
char* buildPath (int bank, int channel){
  String liveFilename = bank;
  liveFilename += "/";
  liveFilename += FILE_NAMES[bank][channel];
  char filename[18];
  liveFilename.toCharArray(filename, 17);
  return filename;
}

void reBoot(){
  delay (500);
  WRITE_RESTART(0x5FA0004);
}


//////////////////////////////////////////////
////////VARIOUS DEBUG DISPLAY FUNCTIONS //////
//////////////////////////////////////////////

// SHOW VISUAL INDICATOR OF TRACK PROGRESS IN SERIAL MONITOR 
void playDisplay(){
  int position = (playRaw1.fileOffset() %  FILE_SIZES[PLAY_BANK][PLAY_CHANNEL]) >> 21;
  int size = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL] >> 21;
  for (int i = 0; i < size; i++){
    if (i == position) Serial.print("|");
    else Serial.print("_");
  }
  Serial.println("");
}


// SHOW CURRENTLY PLAYING TRACK IN SERIAL MONITOR 
void whatsPlaying (){
  Serial.print("Bank:");
  Serial.print(PLAY_BANK);
  Serial.print(" Channel:");
  Serial.print(PLAY_CHANNEL);  
  Serial.print(" File:");  
  Serial.println (charFilename); 
}


void printFileList(){

  for (int x = 0; x < BANKS; x++){ 
    Serial.print("Bank: ");
    Serial.println(x);

    Serial.print (FILE_COUNT[x]);
    Serial.print(" ");
    Serial.print (FILE_TYPE);
    Serial.println(" Files found"); 

    for (int i = 0; i < FILE_COUNT[x]; i++){
      Serial.print (i);
      Serial.print (") ");
      Serial.print (FILE_DIRECTORIES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_NAMES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_SIZES[x][i]);
      Serial.println(" | ");
    }
  }
}


void printSettings(){
  Serial.print("MUTE=");
  Serial.println(MUTE);
  Serial.print("DECLICK=");
  Serial.println(DECLICK);
  Serial.print("ShowMeter=");
  Serial.println(ShowMeter);
  Serial.print("meterHIDE=");
  Serial.println(meterHIDE);
  Serial.print("TunePotImmediate=");
  Serial.println(TunePotImmediate);
  Serial.print("TuneCVImmediate=");
  Serial.println(TuneCVImmediate);
  Serial.print("StartPotImmediate=");
  Serial.println(StartPotImmediate);
  Serial.print("StartCVImmediate=");
  Serial.println(StartCVImmediate);
}









// DISPLAY PEAK METER IN LEDS 
void peakMeter(){
  if (peak1.available()) {
    fps = 0;
    int monoPeak = peak1.read() * 5.0;
    ledWrite((pow(2,monoPeak))-1); // 
  }
}


// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}





//////////////////////////////////////////////
////READ AND WRITE SETTINGS ON THE SD CARD ///
////VIA http://overskill.alexshu.com/?p=107 ///
//////////////////////////////////////////////




void readSDSettings(){
  char character;
  String settingName;
  String settingValue;
  settingsFile = SD.open("settings.txt");
  if (settingsFile) {
    while (settingsFile.available()) {
      character = settingsFile.read();
      while(character != '='){
        settingName = settingName + character;
        character = settingsFile.read();
      }
      character = settingsFile.read();
      while(character != '\n'){
        settingValue = settingValue + character;
        character = settingsFile.read();
        if(character == '\n'){
          /*
          //Debuuging Printing
           Serial.print("Name:");
           Serial.println(settingName);
           Serial.print("Value :");
           Serial.println(settingValue);
           */
          // Apply the value to the parameter
          applySetting(settingName,settingValue);
          // Reset Strings
          settingName = "";
          settingValue = "";
        }
      }
    }
    // close the file:
    settingsFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening settings.txt");
  }
}
/* Apply the value to the parameter by searching for the parameter name
 Using String.toInt(); for Integers
 toFloat(string); for Float
 toBoolean(string); for Boolean
 */
void applySetting(String settingName, String settingValue) {

  if(settingName == "MUTE") {
    MUTE=toBoolean(settingValue);
  }

  if(settingName == "DECLICK") {
    DECLICK=settingValue.toInt();
  }

  if(settingName == "ShowMeter") {
    ShowMeter=toBoolean(settingValue);
  }

  if(settingName == "meterHIDE") {
    meterHIDE=settingValue.toInt();
  }

  if(settingName == "TunePotImmediate") {
    TunePotImmediate=toBoolean(settingValue);
  }

  if(settingName == "TuneCVImmediate") {
    TuneCVImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartPotImmediate") {
    StartPotImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartCVImmediate") {
    StartCVImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartCVDivider") {
    StartCVDivider=settingValue.toInt();;
  }



}
// converting string to Float
float toFloat(String settingValue){
  char floatbuf[settingValue.length()];
  settingValue.toCharArray(floatbuf, sizeof(floatbuf));
  float f = atof(floatbuf);
  return f;
}
// Converting String to integer and then to boolean
// 1 = true
// 0 = false
boolean toBoolean(String settingValue) {
  if(settingValue.toInt()==1){
    return true;
  } 
  else {
    return false;
  }
}
// Writes A Configuration file
void writeSDSettings() {
  // Delete the old One
  SD.remove("settings.txt");
  // Create new one
  settingsFile = SD.open("settings.txt", FILE_WRITE);
  // writing in the file works just like regular print()/println() function
  settingsFile.print("MUTE=");
  settingsFile.println(MUTE);
  settingsFile.print("DECLICK=");
  settingsFile.println(DECLICK);
  settingsFile.print("ShowMeter=");
  settingsFile.println(ShowMeter);
  settingsFile.print("meterHIDE=");
  settingsFile.println(meterHIDE);
  settingsFile.print("TunePotImmediate=");
  settingsFile.println(TunePotImmediate);
  settingsFile.print("TuneCVImmediate=");
  settingsFile.println(TuneCVImmediate);
  settingsFile.print("StartPotImmediate=");
  settingsFile.println(StartPotImmediate);
  settingsFile.print("StartCVImmediate=");
  settingsFile.println(StartCVImmediate);
  settingsFile.print("StartCVDivider=");
  settingsFile.println(StartCVDivider);





  // close the file:
  settingsFile.close();
  //Serial.println("Writing done.");
}












