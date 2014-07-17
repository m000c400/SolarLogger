
#include <GSM.h>

#include <SdFat.h>
//#include <SdFatUtil.h>  // define FreeRam()
#include <I2cMaster.h>
#include <SoftRTClib.h>


//#define GSMCONNECTTIMEOUT 120000
#define READINTERVAL 60  //Seconds!!!

#define CHIP_SELECT 5
#define MAXRETRY 5

void PrintSeconds(int Seconds);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#if !MEGA_SOFT_SPI
#error set MEGA_SOFT_SPI nonzero in libraries/SdFat/SdFatConfig.h
#endif  // MEGA_SOFT_SPI
// Is a Mega use analog pins 4, 5 for software I2C
const uint8_t RTC_SCL_PIN = 59;
const uint8_t RTC_SDA_PIN = 58;
SoftI2cMaster i2c(RTC_SDA_PIN, RTC_SCL_PIN);

#elif defined(__AVR_ATmega32U4__)
#if !LEONARDO_SOFT_SPI
#error set LEONARDO_SOFT_SPI nonzero in libraries/SdFat/SdFatConfig.h
#endif  // LEONARDO_SOFT_SPI
// Is a Leonardo use analog pins 4, 5 for software I2C
const uint8_t RTC_SCL_PIN = 23;
const uint8_t RTC_SDA_PIN = 22;
SoftI2cMaster i2c(RTC_SDA_PIN, RTC_SCL_PIN);

#else  // defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Not Mega use hardware I2C
// enable pull-ups on SDA/SCL
TwiMaster i2c(true);
#endif  // defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

RTC_DS1307 RTC(&i2c); // define the Real Time Clock object
// file system object
SdFat sd;



char GPRS_APN[100];
char GPRS_LOGIN[100]; 
char GPRS_PASSWORD[100];

char WWWServer[100];
char WWWPath[100];
int WWWPort = 80;


// PIN Number
#define PINNUMBER ""

// initialize the library instance
GSMClient gsmClient;
GPRS gprsService;
GSM gsmAccess(true); 
GSMScanner gsmScanner;

int GSMConnected = false;
int GPRSAttached = false;
int WWWServerConnected = false;

int FirstPin = 10;
int LastPin = 15;
unsigned int InputValue[20];


void setup()
{
  DateTime now;
  
  SetPinModes();
  
  Serial.begin(57600);
  
  Serial.println("Starting Arduino Solar Logger.");
  Serial.println("Loading Configuration");
  
  Serial.println("Starting RTC");
  RTC.begin();
  now = RTC.now();  
  Serial.print(now.year(), DEC); Serial.print('/'); Serial.print(now.month(), DEC); Serial.print('/'); Serial.print(now.day(), DEC); Serial.println();
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC); Serial.print(':'); Serial.print(now.second(), DEC); Serial.println();

  Serial.println("Starting SD Card");
  if (!sd.begin(CHIP_SELECT)) sd.initErrorHalt();

  Serial.println("Load Configuration");
  LoadConfiguration();
  
  Serial.print("GPRS APN "); Serial.println(GPRS_APN);
  Serial.print("GPRS LOGIN ");Serial.println(GPRS_LOGIN);
  Serial.print("GPRS Password "); Serial.println(GPRS_PASSWORD);
  Serial.print("Remote Server "); PrintServerAddress(false); Serial.println(WWWPath);
}

void loop()
{
  char Request[200];
  DateTime now;
  
  GSMConnected = false;
  GPRSAttached = false;
  WWWServerConnected = false;
  
  
  Serial.println("Waiting To Read Next Sample\r\nTime to Reading..");

  
  do
  {
     now = RTC.now();
     PrintSeconds( READINTERVAL - (now.unixtime() % READINTERVAL) );
  } while( (now.unixtime() % READINTERVAL) != 0);

  Serial.println("\r\nMaking Reading....");
  
  ReadInputValues();
  WriteFileLog();  
  
  Serial.println("Connecting to GSM Service");
  ConnectToGSM();
  if (GSMConnected == true)
  {
    Serial.print("Current carrier is ");
    Serial.println(gsmScanner.getCurrentCarrier());
    
    Serial.print("Signal Strength: ");
    Serial.print(gsmScanner.getSignalStrength());
    Serial.println(" [0-31]");
    
    Serial.println("Attaching To GPRS");
    AttachGPRS();
    if(GPRSAttached == true)
    {
      ReadInputValues();
      Serial.println("Make Web Request");
      AttachToServer();
      if(WWWServerConnected == true)
      {
        FormWebRequest(Request);
        Serial.println(Request);
        gsmClient.println(Request);
        gsmClient.print("Host: ");
        gsmClient.println(WWWServer);
        gsmClient.println("Connection: close");
        gsmClient.println();
        DetatchFromServer();
      }    
    }
  }
  Serial.println("Shutting Down GSM..");
  CloseGSM();
  Serial.println("GSM Down");
}


void SetPinModes(void)
{
  pinMode(2,INPUT);
  pinMode(10, OUTPUT);
}

void ConnectToGSM(void)
{
  int i;
  
  if(GSMConnected == false)
  {
    for(i=0;i<MAXRETRY;i++)
    {
      if(gsmAccess.begin(PINNUMBER, true, true)==GSM_READY)
      {
        Serial.println("GSM Connected");
        GSMConnected = true;
        return;
      }
      Serial.println("Wait...");
      delay(1000);
    }
    Serial.println("GSM Not Available");
    return;
  }
  Serial.println("GSM Connected Already");
}

void CloseGSM(void)
{
  gsmAccess.shutdown();
}

void AttachGPRS(void)
{
  int i;
  
  if(GPRSAttached == false)
  {
    for(i=0;i<MAXRETRY;i++)
    {
      if(gprsService.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD)!=GPRS_READY)
      {
        Serial.println("Attached to GPRS Connected");
        GPRSAttached = true;
        return;
      }
      Serial.println("Wait...");
      delay(1000);
    }
    Serial.println("GPRS Not Available");
    return;
  }
  Serial.println("GPRS Attached Already");
}

void AttachToServer(void)
{
  int i;
  
  if(WWWServerConnected == false)
  {
    for(i=0;i<MAXRETRY;i++)
    {
      if(gsmClient.connect(WWWServer, WWWPort) == true)
      {
         WWWServerConnected = true;
         Serial.print("Connected to "); PrintServerAddress(true);
         return;
      }
      Serial.println("Wait...");
      delay(1000);
    }
    Serial.print("Unable to Connect to "); PrintServerAddress(true);
    return;
  }  
  Serial.print("Already Connected to "); PrintServerAddress(true);
}
 
void DetatchFromServer(void)
{
  if(WWWServerConnected==true)
  {
    gsmClient.stop();
    WWWServerConnected = false;
    Serial.print("Detatched from Server "); PrintServerAddress(true);
  }
}

void PrintServerAddress(int CRLF)
{
  Serial.print(WWWServer); Serial.print(":"); 
  if(CRLF)
    Serial.println(WWWPort,DEC);
  else
    Serial.print(WWWPort,DEC);
}

void ReadInputValues(void)
{
  int i;
  
  for(i=FirstPin;i<=LastPin;i++)
  {
    InputValue[i] = analogRead(i);
  }
}

void FormWebRequest(char *Dest)
{
  int i;
  char temp[50];
  
  strcpy(Dest,"GET ");
  strcat(Dest,WWWPath);
  strcat(Dest,"?");
  
  for(i=FirstPin;i<=LastPin;i++)
  {
    sprintf(temp,"%d=%d&", (i+1)-FirstPin, InputValue[i]);
    strcat(Dest,temp);
  }
  
  Dest[strlen(Dest)-1] = '\0';
  strcat(Dest," HTTP/1.1");
}

void  LoadConfiguration(void)
{
  SdFile ConfigFile;
  char FileName[] = "/config.txt";
  char buffer[100];

  strcpy(GPRS_APN,"bluevia.movistar.es");
  strcpy(GPRS_LOGIN,"");
  strcpy(GPRS_PASSWORD,"");

  strcpy(WWWServer,"solarspain.dyndns.org");
  strcpy(WWWPath,"/solar/upload.php");

  Serial.print("Opening "); Serial.println(FileName);
  if(ConfigFile.open(FileName,O_READ) == 1)
  {
    Serial.println("Config File Opened");
    while(ConfigFile.fgets(buffer,100)>1)
    {
      if(buffer[strlen(buffer)-1] == '\n') buffer[strlen(buffer)-1] = '\0';
      
      Serial.print("Read Line.."); Serial.println(buffer);
      if(strstr(buffer,"apn=") != NULL)
      {
        strcpy(GPRS_APN,&buffer[4]);
      }
      else if(strstr(buffer,"login=") != NULL)
      {
        strcpy(GPRS_LOGIN,&buffer[6]);
      }
      else if(strstr(buffer,"password=") != NULL)
      {
        strcpy(GPRS_PASSWORD,&buffer[9]);
      }
      else if(strstr(buffer,"server=") != NULL)
      {
        strcpy(WWWServer,&buffer[7]);
      }
      else if(strstr(buffer,"path=") != NULL)
      {
        strcpy(WWWPath,&buffer[5]);
      }
    }
    ConfigFile.close();
  }
}  


void WriteFileLog(void)
{
  char filepath[100];
  char filebuffer[100];
  DateTime now;
  SdFile LogFile;
 
  now=RTC.now();
  
  sprintf(filepath,"/%04d%02d%02d.csv",now.year(),now.month(),now.day());
  
  if(LogFile.open(filepath,O_RDWR | O_CREAT | O_AT_END) == 1)
  {
    Serial.print("Opened file "); Serial.println(filepath);
    sprintf(filebuffer,"%04d/%02d/%02d, %02d:%02d:%02d, %d, %d, %d, %d, %d, %d",now.year(),now.month(),now.day(),now.hour(), now.minute(), now.second(), InputValue[FirstPin],InputValue[FirstPin+1],InputValue[FirstPin+2],InputValue[FirstPin+3],InputValue[FirstPin+4],InputValue[FirstPin+5]);
    Serial.println("************************************Write To Log File**************");
    Serial.println(filepath);
    Serial.println(filebuffer);
    LogFile.println(filebuffer);
    Serial.println("************************************Write To Log File**************");
    LogFile.close();
  }
  else
  {
    Serial.println("Error opening log file");
  }
}

void PrintSeconds(int Value)
{
  static int Last = 0;
  int CompareValue;
  
  CompareValue = Value / 10;
  
  if(CompareValue != Last)
  {
    Serial.println(Value);
    Last = CompareValue;
  }
}
