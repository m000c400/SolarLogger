#include "Configuration.h"

#define APPID "SL1"

Configuration::Configuration(void)
{
}

void Configuration::LoadConfiguration(char *APN, int sizeAPN, char *Login, int sizeLogin, char *Password, int sizePassword)
{
  //EEPROM_read((void *)&cs,sizeof(cs));  
}


int Configuration::EEPROM_write(void *Object, unsigned int Size)
{
    const byte* p = (byte*)Object;
    unsigned int i;
    for (i = 0; i < Size; i++)
          EEPROM.write(i, *p++);
    return i;
}

int Configuration::EEPROM_read(void *Object, unsigned int Size)
{
    byte* p = (byte*)(void*)Object;
    unsigned int i;
    for (i = 0; i < Size; i++)
          *p++ = EEPROM.read(i);
    return i;
}

