#include <GSM.h>

#define GSMCONNECTTIMEOUT 120000
#define READINGDELAY 300000

char GPRS_APN[] = "general.t-mobile.uk";
char GPRS_LOGIN[] = "user"; 
char GPRS_PASSWORD[] = "wap";

char WWWServer[] ="solarspain.dyndns.org";
char WWWPath[] = "/solar/upload.php";
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

int FirstPin = 2;
int LastPin = 5;
unsigned int InputValue[20];


void setup()
{
  int notConnected = true;
  
  SetPinModes();
  
  Serial.begin(57600);
  
  Serial.println("Starting Arduino Solar Logger.");
  Serial.println("Loading Configuration");
  
  Serial.print("GPRS APN "); Serial.println(GPRS_APN);
  Serial.print("GPRS LOGIN ");Serial.println(GPRS_LOGIN);
  Serial.print("GPRS Password "); Serial.println(GPRS_PASSWORD);
  Serial.print("Remote Server "); PrintServerAddress(false); Serial.println(WWWPath);
  
}

void loop()
{
  char Request[200];

  GSMConnected = false;
  GPRSAttached = false;
  WWWServerConnected = false;
  
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
  
  Serial.println("Waiting....");
  delay(60000);  
}


void SetPinModes(void)
{
  pinMode(2,INPUT);
}

void ConnectToGSM(void)
{
  unsigned long TimeOut = GSMCONNECTTIMEOUT;
  unsigned long ConnectTime = millis();
  
  if(GSMConnected == false)
  {
    while((millis() - ConnectTime) < TimeOut)
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
  unsigned long TimeOut = GSMCONNECTTIMEOUT;
  unsigned long ConnectTime = millis();
  
  if(GPRSAttached == false)
  {
    while((millis() - ConnectTime) < TimeOut)
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
  unsigned long TimeOut = GSMCONNECTTIMEOUT;
  unsigned long ConnectTime = millis();
 
  if(WWWServerConnected == false)
  {
    while((millis() - ConnectTime) < TimeOut)
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
    sprintf(temp,"%d=%d&", i-1, InputValue[i]);
    strcat(Dest,temp);
  }
  
  Dest[strlen(Dest)-1] = '\0';
  strcat(Dest," HTTP/1.1");
}
