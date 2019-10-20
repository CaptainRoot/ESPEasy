#ifdef USES_P244
//#######################################################################################################
//#################################### Plugin 244: Energy - EasyMeter Q3B SML D0 ##############################################
//#######################################################################################################

#define PLUGIN_244
#define PLUGIN_ID_244         244
#define PLUGIN_NAME_244       "Energy - EasyMeter Q3B SML D0"

#define PLUGIN_VALUENAME1_244 "T1"
#define PLUGIN_VALUENAME2_244 "T2"
#define PLUGIN_VALUENAME3_244 "Power"

#define P244_BUFFER_SIZE 256
boolean Plugin_244_init = false;

byte inByte; //byte to store the serial buffer
byte smlMessage[1000]; //byte to store the parsed message
const byte startSequence[] = { 0x1B, 0x1B, 0x1B, 0x1B, 0x01, 0x01, 0x01, 0x01 }; //start sequence of SML protocol
const byte stopSequence[]  = { 0x1B, 0x1B, 0x1B, 0x1B, 0x1A }; //end sequence of SML protocol

const byte powerSequenceTotal[] = { 0x07, 0x01, 0x00, 0x01, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0xFE, 0x55 }; //sequence preceeding the current "Wirkleistung" value (4 Bytes)
const byte powerSequenceA[]     = { 0x07, 0x01, 0x00, 0x15, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0xFE, 0x55 }; //sequence preceeding the current "Wirkleistung" value (4 Bytes)
const byte powerSequenceB[]     = { 0x07, 0x01, 0x00, 0x29, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0xFE, 0x55 }; //sequence preceeding the current "Wirkleistung" value (4 Bytes)
const byte powerSequenceC[]     = { 0x07, 0x01, 0x00, 0x3D, 0x07, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1B, 0x52, 0xFE, 0x55 }; //sequence preceeding the current "Wirkleistung" value (4 Bytes)

//const byte consumptionSequence[] = { 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x65, 0x00, 0x00, 0x01, 0x82, 0x01, 0x62, 0x1E, 0x52, 0xFF, 0x59 }; //sequence predeecing the current "Gesamtverbrauch" value (8 Bytes)
const byte consumptionSequence[] = { 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x01, 0x01, 0x62, 0x1E, 0x52, 0xFC, 0x69}; //sequence predeecing the current "Gesamtverbrauch" value (8 Bytes)
const byte consumptionSequenceT1[] = { 0x07, 0x01, 0x00, 0x01, 0x08, 0x01, 0xFF, 0x01, 0x01, 0x62, 0x1E, 0x52, 0x01, 0x65}; //sequence predeecing the current "Gesamtverbrauch" value (8 Bytes)
const byte consumptionSequenceT2[] = { 0x07, 0x01, 0x00, 0x01, 0x08, 0x02, 0xFF, 0x01, 0x01, 0x62, 0x1E, 0x52, 0x01, 0x65}; //sequence predeecing the current "Gesamtverbrauch" value (8 Bytes)
//                                     07     01   00    01    08    00    ff

int smlIndex; //index counter within smlMessage array
int startIndex; //start index for start sequence search
int stopIndex; //start index for stop sequence search
int stage; //index to maneuver through cases


float currentpowerTotalInWatt;

//int currentpowerA; //variable to hold translated "Power" value
//int currentpowerB; //variable to hold translated "Power" value
//int currentpowerC; //variable to hold translated "Power" value

double currentconsumptionkWh; //variable to calulate actual "Total Energy" in kWh
float  currentconsumptionkWhT1; //variable to calulate actual "Total Energy T1" in kWh
float  currentconsumptionkWhT2; //variable to calulate actual "Total Energy T2" in kWh

boolean Plugin_244(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_244;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = false;
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_244);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
               PSTR(PLUGIN_VALUENAME1_244));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1],
               PSTR(PLUGIN_VALUENAME2_244));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2],
               PSTR(PLUGIN_VALUENAME3_244));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(F("Baud Rate"), F("p244_baud"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
      	addFormNumericBox(F("Data bits"), F("p244_data"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);

        byte choice = ExtraTaskSettings.TaskDevicePluginConfigLong[2];
        String options[3];
        options[0] = F("No parity");
        options[1] = F("Even");
        options[2] = F("Odd");
        int optionValues[3];
        optionValues[0] = 0;
        optionValues[1] = 2;
        optionValues[2] = 3;
        addFormSelector(F("Parity"), F("p244_parity"), 3, options, optionValues, choice);

      	addFormNumericBox(F("Stop bits"), F("p244_stop"), ExtraTaskSettings.TaskDevicePluginConfigLong[3]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        ExtraTaskSettings.TaskDevicePluginConfigLong[0] = getFormItemInt(F("p244_baud"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[1] = getFormItemInt(F("p244_data"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[2] = getFormItemInt(F("p244_parity"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[3] = getFormItemInt(F("p244_stop"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        stage = 0;

        LoadTaskSettings(event->TaskIndex);
        if ((ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) && (ExtraTaskSettings.TaskDevicePluginConfigLong[1] != 0))
        {

          #if defined(ESP8266)
            byte serialconfig = 0x10;
          #endif

          #if defined(ESP32)
            uint32_t serialconfig = 0x8000010;
          #endif

          serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[2];
          serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[1] - 5) << 2;

          if (ExtraTaskSettings.TaskDevicePluginConfigLong[3] == 2)
            serialconfig += 0x20;

          #if defined(ESP8266)
            Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[0], (SerialConfig)serialconfig);
          #endif

          #if defined(ESP32)
            Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[0], serialconfig);
          #endif

          Plugin_244_init = true;
        }

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        success = true;
        break;
      }


    case PLUGIN_SERIAL_IN:
      {
        switch (stage) {
            case 0:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findStartSequence"));
              findStartSequence(); // look for start sequence
              break;
            case 1:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findStopSequence"));
              findStopSequence(); // look for stop sequence
              break;
            case 2:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findpowerSequenceTotal"));
              findpowerSequenceTotal(); //look for power sequence and extract
              break;

            case 3:
              addLog(LOG_LEVEL_ERROR, F("S244: Stage findpowerSequenceA"));
              //findpowerSequenceA(); //look for power sequence and extract
              break;
            case 4:
              addLog(LOG_LEVEL_ERROR, F("S244: Stage findpowerSequenceB"));
              //findpowerSequenceB(); //look for power sequence and extract
              break;
            case 5:
              addLog(LOG_LEVEL_ERROR, F("S244: Stage findpowerSequenceC"));
              //findpowerSequenceC(); //look for power sequence and extract
              break;

            case 6:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findConsumptionSequence"));
              findConsumptionSequence(); //look for consumption sequence and exctract
              break;
            case 7:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findConsumptionSequenceT1"));
              findConsumptionSequenceT1(); //look for consumption sequence and exctract
              break;
            case 8:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage findConsumptionSequenceT2"));
              findConsumptionSequenceT2(); //look for consumption sequence and exctract
              break;
            case 9:
              //addLog(LOG_LEVEL_ERROR, F("S244: Stage publishMessage"));
              publishMessage(); // do something with the result



              break;
          }

          success = true;
          break;
        }

        case PLUGIN_READ:
        {
          UserVar[event->BaseVarIndex] = currentconsumptionkWhT1;
          UserVar[event->BaseVarIndex + 1] = currentconsumptionkWhT2;
          UserVar[event->BaseVarIndex + 2] = currentpowerTotalInWatt;
          success = true;
          break;
        }

    }
  return success;
}

void findStartSequence() {
  while (Serial.available())
  {
    inByte = Serial.read(); //read serial buffer into array

    if (inByte == startSequence[startIndex]) //in case byte in array matches the start sequence at position 0,1,2...
    {
      smlMessage[startIndex] = inByte; //set smlMessage element at position 0,1,2 to inByte value
      startIndex++;
      if (startIndex == sizeof(startSequence)) //all start sequence values have been identified
      {
        //Serial.println("Match found");
        stage = 1; //go to next case
        smlIndex = startIndex; //set start index to last position to avoid rerunning the first numbers in end sequence search
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }
}



void findStopSequence() {
  while (Serial.available())
  {
    inByte = Serial.read();
    smlMessage[smlIndex] = inByte;
    smlIndex++;

    if (inByte == stopSequence[stopIndex])
    {
      stopIndex++;

      if (stopIndex == sizeof(stopSequence))
      {
        stage = 2;
        stopIndex = 0;
      }
    }
    else
    {
      stopIndex = 0;
    }
  }
}

void findpowerSequenceTotal() {
  byte temp; //temp variable to store loop search data
  startIndex = 0; //start at position 0 of exctracted SML message
  byte power[4]; //array that holds the extracted 4 byte "Wirkleistung" value
  int currentpower; //variable to hold translated "Power" value

  for(unsigned int x = 0; x < sizeof(smlMessage); x++){ //for as long there are element in the exctracted SML message

    temp = smlMessage[x]; //set temp variable to 0,1,2 element in extracted SML message

    if (temp == powerSequenceTotal[startIndex]) //compare with power sequence
    {
      startIndex++;

      if (startIndex == sizeof(powerSequenceTotal)) //in complete sequence is found
      {
        for(unsigned int y = 0; y< 4; y++)
        { //read the next 4 bytes (the actual power value)
          power[y] = smlMessage[x+y+1]; //store into power array
        }
        stage = 6; // go to stage 3
        startIndex = 0;
      }
    }
    else
    {
      startIndex = 0;
    }
  }

  currentpower = ((power[0] << 24) | (power[1] << 16) | (power[2] << 8) | power[3]); //merge 4 bytes into single variable to calculate power value
  currentpowerTotalInWatt = (float)currentpower / 100.0;
}

/*
void findpowerSequenceA() {
  byte temp; //temp variable to store loop search data
  startIndex = 0; //start at position 0 of exctracted SML message
  byte power[4]; //array that holds the extracted 4 byte "Wirkleistung" value

  for(int x = 0; x < sizeof(smlMessage); x++){ //for as long there are element in the exctracted SML message

    temp = smlMessage[x]; //set temp variable to 0,1,2 element in extracted SML message

    if (temp == powerSequenceA[startIndex]) //compare with power sequence
    {
      startIndex++;

      if (startIndex == sizeof(powerSequenceA)) //in complete sequence is found
      {
        for(int y = 0; y< 4; y++)
        { //read the next 4 bytes (the actual power value)
          power[y] = smlMessage[x+y+1]; //store into power array
        }
        stage = 4; // go to stage 3
        startIndex = 0;
      }
    }
    else
    {
      startIndex = 0;
    }
  }
   currentpowerA = ((power[0] << 24) | (power[1] << 16) | (power[2] << 8) | power[3]); //merge 4 bytes into single variable to calculate power value
}

void findpowerSequenceB() {
  byte temp; //temp variable to store loop search data
  startIndex = 0; //start at position 0 of exctracted SML message
  byte power[4]; //array that holds the extracted 4 byte "Wirkleistung" value

  for(int x = 0; x < sizeof(smlMessage); x++){ //for as long there are element in the exctracted SML message

    temp = smlMessage[x]; //set temp variable to 0,1,2 element in extracted SML message

    if (temp == powerSequenceB[startIndex]) //compare with power sequence
    {
      startIndex++;

      if (startIndex == sizeof(powerSequenceB)) //in complete sequence is found
      {
        for(int y = 0; y< 4; y++)
        { //read the next 4 bytes (the actual power value)
          power[y] = smlMessage[x+y+1]; //store into power array
        }
        stage = 5; // go to stage 3
        startIndex = 0;
      }
    }
    else
    {
      startIndex = 0;
    }
  }
   currentpowerB = ((power[0] << 24) | (power[1] << 16) | (power[2] << 8) | power[3]); //merge 4 bytes into single variable to calculate power value
}

void findpowerSequenceC() {
  byte temp; //temp variable to store loop search data
  startIndex = 0; //start at position 0 of exctracted SML message
  byte power[4]; //array that holds the extracted 4 byte "Wirkleistung" value

  for(int x = 0; x < sizeof(smlMessage); x++){ //for as long there are element in the exctracted SML message

    temp = smlMessage[x]; //set temp variable to 0,1,2 element in extracted SML message

    if (temp == powerSequenceC[startIndex]) //compare with power sequence
    {
      startIndex++;

      if (startIndex == sizeof(powerSequenceC)) //in complete sequence is found
      {
        for(int y = 0; y< 4; y++)
        { //read the next 4 bytes (the actual power value)
          power[y] = smlMessage[x+y+1]; //store into power array
        }
        stage = 6; // go to stage 3
        startIndex = 0;
      }
    }
    else
    {
      startIndex = 0;
    }
  }
   currentpowerC = ((power[0] << 24) | (power[1] << 16) | (power[2] << 8) | power[3]); //merge 4 bytes into single variable to calculate power value
}
*/

void findConsumptionSequence() {
  byte temp;
  byte consumption[8]; //array that holds the extracted 8 byte "Gesamtverbrauch" value
  unsigned long currentconsumption; //variable to hold translated "Total Energy" value

  startIndex = 0;

  for(unsigned int x = 0; x < sizeof(smlMessage); x++){
    temp = smlMessage[x];
    if (temp == consumptionSequence[startIndex])
    {
      startIndex++;
      if (startIndex == sizeof(consumptionSequence))
      {
        for(unsigned int y = 0; y< 8; y++){
          //hier muss für die folgenden 8 Bytes hoch gezählt werden
          consumption[y] = smlMessage[x+y+1];
        }
        stage = 7;
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }

   currentconsumption = ((consumption[0] << 56) | (consumption[1] << 48) | (consumption[2] << 40) | (consumption[3] << 32) | (consumption[4] << 24) | (consumption[5] << 16) | (consumption[6] << 8) | consumption[7]); //combine and turn 8 bytes into one variable
   currentconsumptionkWh = (double)currentconsumption / 1000.0;
}

void findConsumptionSequenceT1() {
  byte temp;
  byte consumption[8]; //array that holds the extracted 8 byte "Gesamtverbrauch" value
  unsigned int currentconsumptionT1; //variable to hold translated "Total Energy T1" value

  startIndex = 0;

  for(unsigned int x = 0; x < sizeof(smlMessage); x++){
    temp = smlMessage[x];
    if (temp == consumptionSequenceT1[startIndex])
    {
      startIndex++;
      if (startIndex == sizeof(consumptionSequenceT1))
      {
        for(unsigned int y = 0; y< 4; y++){
          //hier muss für die folgenden 8 Bytes hoch gezählt werden
          consumption[y] = smlMessage[x+y+1];
        }
        stage = 8;
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }

   currentconsumptionT1 = ((consumption[0] << 24) | (consumption[1] << 16) | (consumption[2] << 8) | consumption[3]) * 10; //combine and turn 4 bytes into one variable
   currentconsumptionkWhT1 = (float)currentconsumptionT1 / 1000.0;
}


void findConsumptionSequenceT2() {
  byte temp;
  byte consumption[8]; //array that holds the extracted 8 byte "Gesamtverbrauch" value
  unsigned int currentconsumptionT2; //variable to hold translated "Total Energy T2" value

  startIndex = 0;

  for(unsigned int x = 0; x < sizeof(smlMessage); x++){
    temp = smlMessage[x];
    if (temp == consumptionSequenceT2[startIndex])
    {
      startIndex++;
      if (startIndex == sizeof(consumptionSequenceT2))
      {
        for(unsigned int y = 0; y< 4; y++){
          //hier muss für die folgenden 8 Bytes hoch gezählt werden
          consumption[y] = smlMessage[x+y+1];
        }
        stage = 9;
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }

   currentconsumptionT2 = ((consumption[0] << 24) | (consumption[1] << 16) | (consumption[2] << 8) | consumption[3]) * 10; //combine and turn 4 bytes into one variable
   currentconsumptionkWhT2 = (float) currentconsumptionT2 / 1000.0;
}

void publishMessage() {
  String log = F("P244:  SML: ");
    log += "Power Total: ";
    log +=currentpowerTotalInWatt;
    //log += " Power A: ";
    //log +=currentpowerA;
    //log += " Power B: ";
    //log +=currentpowerB;
    //log += " Power C: ";
    //log +=currentpowerC;
    log += " Energy Total : ";
    log += currentconsumptionkWh;

    log += " T1: ";
    log += currentconsumptionkWhT1;

    log += " T2: ";
    log += currentconsumptionkWhT2;
    addLog(LOG_LEVEL_INFO, log);
    stage = 0;
}

#endif // USES_P244
