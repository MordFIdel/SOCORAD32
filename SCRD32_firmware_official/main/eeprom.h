#ifndef EEPROM_h
#define EEPROM_h

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define EEPROM_TAG "Eeprom"
#define DEBUG_EEPROM 1

//structure and enum
struct EEPROM_0 {
	unsigned int  _sector;
	unsigned char* _data;
	size_t _size;
};
struct EEPROM_0 eeprom0;

struct EEPROM_1 {
	unsigned int  _sector;
	unsigned char* _data;
	size_t _size;
};
struct EEPROM_1 eeprom1;

struct wifi_credentials_memory {
	unsigned int  _sector;
	unsigned char* _data;
	size_t _size;
};
struct wifi_credentials_memory wifi_credentials_memory_access_point;

//funtion prototype
bool beginEEPROM(size_t size);
bool eeprom_begin_0(size_t size );
char eeprom_read_0(size_t address);
void readEPPROM(char data[], char length, uint32_t address);
void writeEPPROM(char data[], char length, uint32_t address);
void readEPPROM_1(char data[], char length, uint32_t address);
void writeEPPROM_1(char data[], char length, uint32_t address);
void eeprom_write_0(size_t address ,char value );
char eeprom_commit_0( void);
void eeprom_stop_0(void);
bool eeprom_begin_1(size_t size );
char eeprom_read_1(size_t address);
void eeprom_write_1(size_t address ,uint8_t value );
char eeprom_commit_1( void);
void eeprom_stop_1(void);
bool Wifi_credentials_memory_begin(size_t size );
char Wifi_credentials_memory_read(size_t address);
void Wifi_credentials_memory_write(size_t address ,uint8_t value );
char Wifi_credentials_memory_commit(void);
void Wifi_credentials_memory_stop(void);
void Wifi_credentials_write(char data[], char length, uint32_t address);
void Wifi_credentials_read(char data[], char length, uint32_t address);

#endif