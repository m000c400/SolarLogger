#include <EEPROM.h>
//#include <SD.h>
#include <GSM.h>


#include "Configuration.h"


Configuration *SystemConfiguration;
char GPRS_APN[] = "general.t-mobile.uk";
char GPRS_LOGIN[] = "user"; 
char GPRS_PASSWORD[] = "wap";

char WWWServer[] ="solarserver.dyndns.org";
char WWWPath[] = "/";
int WWWPort = 80;


// PIN Number
#define PINNUMBER ""


// initialize the library instance
GSMClient gsmClient;
GPRS gprsService;
GSM gsmAccess; 
GSMScanner gsmScanner;


void setup()
{
  int notConnected = true;
  
  Serial.begin(57600);
  while (!Serial) {};
  
  Serial.println("Starting Arduino Solar Logger.");
  Serial.println("Loading Configuration");
  //SystemConfiguration = new Configuration();
  //SystemConfiguration->LoadConfiguration(GPRS_APN,sizeof(GPRS_APN), GPRS_LOGIN,sizeof(GPRS_LOGIN), GPRS_PASSWORD,sizeof(GPRS_PASSWORD));
  
  //Serial.print("GPRS APN "); Serial.println(GPRS_APN);
  //Serial.print("GPRS LOGIN ");Serial.println(GPRS_LOGIN);
  //Serial.print("GPRS Password "); Serial.println(GPRS_PASSWORD);
  //Serial.print("Remote Server "); Serial.print(WWWServer); Serial.print(":"); Serial.print(WWWPort,DEC); Serial.print(WWWPath);
  
  Serial.println("Connecting to GSM Service");

  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) 
    {
      Serial.println("GSM Service Connected");
      notConnected = false;
    }
    else
    {
      Serial.println("Not connected Halting");
      do{}while(1);
    }
  }
  
  Serial.println("Scanning available networks. May take some seconds.");
  Serial.println(gsmScanner.readNetworks());
  Serial.print("Current carrier: ");
  Serial.println(gsmScanner.getCurrentCarrier());
    
  // returns strength and ber
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  Serial.print("Signal Strength: ");
  Serial.print(gsmScanner.getSignalStrength());
  Serial.println(" [0-31]");
  
  Serial.println("Connecting to GPRS Service");
  if(gprsService.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD)!=GPRS_READY)
  {
    
    return;
  } 
  Serial.println("Connected to GPRS Service");
 
}

void loop()
{
  Serial.println("Scanning available networks. May take some seconds.");
  Serial.println(gsmScanner.readNetworks());
  Serial.print("Current carrier: ");
  Serial.println(gsmScanner.getCurrentCarrier());
    
  // returns strength and ber
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  Serial.print("Signal Strength: ");
  Serial.print(gsmScanner.getSignalStrength());
  Serial.println(" [0-31]");
  
  Serial.println("Conecting to Server");
  if(gsmClient.connect(WWWServer, WWWPort))
  {
    Serial.println("Connected");
    gsmClient.print("GET ");
    gsmClient.print(WWWPath);
        
    gsmClient.println(" HTTP/1.0");
    gsmClient.println();
    
    boolean test = true;
    while(test)
    {
      // if there are incoming bytes available 
      // from the server, read and check them
      if (gsmClient.available())
      {
        char c = client.read();
        response += c;
        
        // cast response obtained from string to char array
        char responsechar[response.length()+1];
        response.toCharArray(responsechar, response.length()+1);
        
        // if response includes a "200 OK" substring
        if(strstr(responsechar, "200 OK") != NULL)
        {
          Serial.println(oktext);
          Serial.println("TEST COMPLETE!");
          test = false;
        }
      }
    }
    gsmClient.stop();
  }
  else
  {
    Serial.println("Unable to Connect");
  }
  
}
