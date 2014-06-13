#ifndef CONFIGURATION_h
#define CONFIGURATION_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#include <EEPROM.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

#include <EEPROM.h>


// These defs cause trouble on some versions of Arduino
#undef round


/////////////////////////////////////////////////////////////////////
class Configuration 
{
  public:
    Configuration();
    void LoadConfiguration(char *APN, int sizeAPN, char *Login, int sizeLogin, char *Password, int sizePassword);
      
  protected:
				
  private:
    int EEPROM_write(void *Object, unsigned int Size);
    int EEPROM_read(void *Object, unsigned int Size);
};


#endif 

