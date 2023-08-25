#include "main.h"

#define STORAGE_NAMESPACE "storage"

//Global variables

extern SemaphoreHandle_t State_read_mutex;

/*********************************************************************************************************
 
Description : Function used to configure the eeprom
Input argumenst : 
1. size_t size : size of eeprom need to intialize , recommended it 4096 
Output argumenst : bool : init is sucess or failure

*********************************************************************************************************/
bool eeprom_begin_0(size_t size )
{
	if (size <= 0) {
		#if DEBUG_EEPROM
		ESP_LOGI("EEPROM","size <= 0\n");
		#endif
		return false;
	}
	if (size > SPI_FLASH_SEC_SIZE) {
		size = SPI_FLASH_SEC_SIZE;
	}
	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "eeprom0");
	if (_mypart == NULL) {
		#if DEBUG_EEPROM
		ESP_LOGI("EEPROM","_mypart is NULL");
		#endif
		return false;
	}
	size = (size + 3) & (~3);

	if (eeprom0._data) {
		free(eeprom0._data);
	}

	eeprom0._data = malloc(size* (sizeof(unsigned char)));
	if(eeprom0._data == NULL)
	{
		ESP_LOGI("EEPROM","eeprom0._data is NULL");
		return false;
	}
	eeprom0._size = size;

	xSemaphoreTake(State_read_mutex, portMAX_DELAY);
	if (esp_partition_read (_mypart,0, (void *) eeprom0._data,eeprom0._size)==ESP_OK) {
		#if DEBUG_EEPROM
		ESP_LOGI("EEPROM","esp_partition_read true\n");
		#endif
		 xSemaphoreGive(State_read_mutex);
		return true;
	}
	 xSemaphoreGive(State_read_mutex);
	return false;

}
/*********************************************************************************************************
 
Description : Function used to configure the eeprom
Input argumenst : 
1. size_t size : size of eeprom need to intialize , recommended it 4096
Output argumenst : bool : init is sucess or failure

*********************************************************************************************************/

bool eeprom_begin_1(size_t size )
{
	if (size <= 0) {
		return false;
	}
	if (size > SPI_FLASH_SEC_SIZE) {
		size = SPI_FLASH_SEC_SIZE;
	}
	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "eeprom1");
	if (_mypart == NULL) {
		return false;
	}
	size = (size + 3) & (~3);

	if (eeprom1._data) {
		free(eeprom1._data);
	}

	eeprom1._data = malloc(size* (sizeof(unsigned char)));
	if(eeprom1._data == NULL)
	{
		return false;
	}
	eeprom1._size = size;

	if (esp_partition_read (_mypart,0, (void *) eeprom1._data,eeprom1._size)==ESP_OK) {
		return true;
	}

	return false;

}
/*********************************************************************************************************
 
Description : Function used to configure the eeprom for wifi credintials
Input argumenst : 
1. size_t size : size of eeprom need to intialize , recommended it 4096
Output argumenst : bool : init is sucess or failure

*********************************************************************************************************/

bool Wifi_credentials_memory_begin(size_t size )
{
	if (size <= 0) {
		return false;
	}
	if (size > SPI_FLASH_SEC_SIZE) {
		size = SPI_FLASH_SEC_SIZE;
	}
	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "ITE_OTA"); // this partition was made for ITE OTA , now its reused for credentials
	if (_mypart == NULL) {
		return false;
	}
	size = (size + 3) & (~3);

	if (wifi_credentials_memory_access_point._data) {
		free(wifi_credentials_memory_access_point._data);
	}

	wifi_credentials_memory_access_point._data = malloc(size* (sizeof(unsigned char)));
	if(wifi_credentials_memory_access_point._data == NULL)
	{
		return false;
	}
	wifi_credentials_memory_access_point._size = size;

	if (esp_partition_read (_mypart,0, (void *) wifi_credentials_memory_access_point._data,wifi_credentials_memory_access_point._size)==ESP_OK) {
		return true;
	}

	return false;

}
/*********************************************************************************************************
 
Description : Function used to read data form eeprom
Input argumenst : 
1. size_t address : Address from which data needs to read
Output argumenst : char : size of read data

*********************************************************************************************************/
char eeprom_read_0(size_t address)
{
	if((size_t)address >= eeprom0._size) {
		#if DEBUG_EEPROM
		ESP_LOGI("EEPROM","(size_t)address >= eeprom0._size\n");
		#endif
		return 0;
	}

	if (!eeprom0._data) {
		#if DEBUG_EEPROM
		ESP_LOGI("EEPROM","!eeprom0._data\n");
		#endif
		return 0;
	}
	return eeprom0._data[address];
}
/*********************************************************************************************************
 
Description : Function used to read data form eeprom
Input argumenst : 
1. size_t address : Address from which data needs to read
Output argumenst : char : size of read data

*********************************************************************************************************/

char eeprom_read_1(size_t address)
{
	if (address < 0 || (size_t)address >= eeprom1._size) {
		return 0;
	}

	if (!eeprom1._data) {
		return 0;
	}

	return eeprom1._data[address];
}
/*********************************************************************************************************
 
Description : Function used to read data form eeprom
Input argumenst : 
1. size_t address : Address from which data needs to read
Output argumenst : char : size of read data

*********************************************************************************************************/

char Wifi_credentials_memory_read(size_t address)
{
	if (address < 0 || (size_t)address >= wifi_credentials_memory_access_point._size) {
		return 0;
	}

	if (!wifi_credentials_memory_access_point._data) {
		return 0;
	}

	return wifi_credentials_memory_access_point._data[address];
}
/*********************************************************************************************************
 
Description : Function used to write data into eeprom
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char value : values which needs to written into the above address
Output argumenst : void

*********************************************************************************************************/
void eeprom_write_0(size_t address , char value )
{

	if (address < 0 || (size_t)address >= eeprom0._size)
		return;
	if(!eeprom0._data)
		return;

	// Optimise _dirty. Only flagged if data written is different.
	uint8_t* pData = &eeprom0._data[address];
	if (*pData != value)
	{
		*pData = value;
	}
}

/*********************************************************************************************************
 
Description : Function used to write data into eeprom
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char value : values which needs to written into the above address
Output argumenst : void

*********************************************************************************************************/
void eeprom_write_1(size_t address ,uint8_t value )
{

	if (address < 0 || (size_t)address >= eeprom1._size)
		return;
	if(!eeprom1._data)
		return;

	// Optimise _dirty. Only flagged if data written is different.
	uint8_t* pData = &eeprom1._data[address];
	if (*pData != value)
	{
		*pData = value;
	}
}
/*********************************************************************************************************
 
Description : Function used to write data into eeprom
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char value : values which needs to written into the above address
Output argumenst : void

*********************************************************************************************************/
void Wifi_credentials_memory_write(size_t address ,uint8_t value )
{

	if (address < 0 || (size_t)address >= wifi_credentials_memory_access_point._size)
		return;
	if(!wifi_credentials_memory_access_point._data)
		return;

	// Optimise _dirty. Only flagged if data written is different.
	uint8_t* pData = &wifi_credentials_memory_access_point._data[address];
	if (*pData != value)
	{
		*pData = value;
	}
}
/*********************************************************************************************************
 
Description : Function used to save the written data , it basically pushes the data into the pyh eeprom
Input argumenst : void
Output argumenst : char : true if success

*********************************************************************************************************/
char eeprom_commit_0(void)
{
	bool ret = false;
	if (!eeprom0._size)
	{
		ESP_LOGE(EEPROM_TAG, "eeprom0._size = %d", eeprom0._size);
		return false;
	}
	if (!eeprom0._data)
	{
		ESP_LOGE(EEPROM_TAG, "eeprom0._data error");
		return false;
	}
		

	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "eeprom0");
	if (_mypart == NULL) {
		ESP_LOGE(EEPROM_TAG, "_mypart is NULL");
		return false;
	}

	if (esp_partition_erase_range(_mypart, 0, SPI_FLASH_SEC_SIZE) != ESP_OK)
	{	  
		ESP_LOGE(EEPROM_TAG, "partition erase err.");
	}
	else
	{
		if (esp_partition_write(_mypart, 0, (void *)eeprom0._data, eeprom0._size) == ESP_ERR_INVALID_SIZE)
		{
			ESP_LOGE(EEPROM_TAG, "error in Write");
		}
		else
		{	
			ret = true;
		}
	}
	return ret;
}
/*********************************************************************************************************
 
Description : Function used to save the written data , it basically pushes the data into the pyh eeprom
Input argumenst : void
Output argumenst : char : true if success

*********************************************************************************************************/

char eeprom_commit_1(void)
{
	bool ret = false;
	if (!eeprom1._size)
		return false;
	if (!eeprom1._data)
		return false;

	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "eeprom1");
	if (_mypart == NULL) {
		return false;
	}

	if (esp_partition_erase_range(_mypart, 0, SPI_FLASH_SEC_SIZE) != ESP_OK)
	{
		// ESP_LOGE(EEPROM_TAG, "partition erase err.");
	}
	else
	{
		if (esp_partition_write(_mypart, 0, (void *)eeprom1._data, eeprom1._size) == ESP_ERR_INVALID_SIZE)
		{
			//ESP_LOGE(EEPROM_TAG, "error in Write");
		}
		else
		{
			ret = true;
		}
	}
	return ret;
}
/*********************************************************************************************************
 
Description : Function used to save the written data , it basically pushes the data into the pyh eeprom
Input argumenst : void
Output argumenst : char : true if success

*********************************************************************************************************/

char Wifi_credentials_memory_commit(void)
{
	bool ret = false;
	if (!wifi_credentials_memory_access_point._size)
		return false;
	if (!wifi_credentials_memory_access_point._data)
		return false;

	const esp_partition_t * _mypart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY, "ITE_OTA");
	if (_mypart == NULL) {
		return false;
	}

	if (esp_partition_erase_range(_mypart, 0, SPI_FLASH_SEC_SIZE) != ESP_OK)
	{
		// ESP_LOGE(EEPROM_TAG, "partition erase err.");
	}
	else
	{
		if (esp_partition_write(_mypart, 0, (void *)wifi_credentials_memory_access_point._data, wifi_credentials_memory_access_point._size) == ESP_ERR_INVALID_SIZE)
		{
			//ESP_LOGE(EEPROM_TAG, "error in Write");
		}
		else
		{
			ret = true;
		}
	}
	return ret;
}

/*********************************************************************************************************
 
Description : Function used to free the eerpom
Input argumenst : void
Output argumenst : void

*********************************************************************************************************/
void eeprom_stop_0(void)
{
	free((void *)eeprom0._data);
	eeprom0._data=NULL;
}

/*********************************************************************************************************
 
Description : Function used to free the eerpom
Input argumenst : void
Output argumenst : void

*********************************************************************************************************/
void eeprom_stop_1(void)
{
	free((void *)eeprom1._data);
	eeprom1._data=NULL;
}
/*********************************************************************************************************
 
Description : Function used to free the eerpom
Input argumenst : void
Output argumenst : void

*********************************************************************************************************/
void Wifi_credentials_memory_stop(void)
{
	free((void *)wifi_credentials_memory_access_point._data);
	wifi_credentials_memory_access_point._data=NULL;
}
/*********************************************************************************************************
 
Description : Function used to write data into eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of input data
3. char data[] : input data
Output argumenst : void

*********************************************************************************************************/
void writeEPPROM(char data[], char length, uint32_t address)
{ 
	for (int i = 0; i < length; i++) {
		eeprom_write_0(address + i, data[i]);
	}

	if(eeprom_commit_0()){
		ESP_LOGI("EEPROM","EEPROM successfully committed\n");
	}
	else {
		ESP_LOGE("EEPROM","ERROR! EEPROM commit failed\n");
	}
}
/*********************************************************************************************************
 
Description : Function used to write data into eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of input data
3. char data[] : input data
Output argumenst : void

*********************************************************************************************************/
void writeEPPROM_1(char data[], char length, uint32_t address)
{ 
	for (int i = 0; i < length; i++) {
	eeprom_write_1(address + i, data[i]);
	}

	if(eeprom_commit_1()){
		//ESP_LOGI("EEPROM","EEPROM successfully committed\n");
	}
	else {
		//ESP_LOGE("EEPROM","ERROR! EEPROM commit failed\n");
	}
}
/*********************************************************************************************************
 
Description : Function used to write data into eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of input data
3. char data[] : input data
Output argumenst : void

*********************************************************************************************************/
void Wifi_credentials_write(char data[], char length, uint32_t address)
{ 
	for (int i = 0; i < length; i++) {
	Wifi_credentials_memory_write(address + i, data[i]);
	}

	if(Wifi_credentials_memory_commit()){
		//ESP_LOGI("EEPROM","EEPROM successfully committed\n");
	}
	else {
		//ESP_LOGE("EEPROM","ERROR! EEPROM commit failed\n");
	}
}
/*********************************************************************************************************
 
Description : Function used to read data from eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of read data
3. char data[] : output read data
Output argumenst : void

*********************************************************************************************************/
void readEPPROM(char data[], char length, uint32_t address)
{
	for (int i = 0; i < length; i++) {
	data[i] = eeprom_read_0(address + i);
	}
}
/*********************************************************************************************************
 
Description : Function used to read data from eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of read data
3. char data[] : output read data
Output argumenst : void

*********************************************************************************************************/
void readEPPROM_1(char data[], char length, uint32_t address)
{
	for (int i = 0; i < length; i++) {
	data[i] = eeprom_read_1(address + i);
	}
}
/*********************************************************************************************************
 
Description : Function used to read data from eeprom in string form
Input argumenst : 
1. size_t address : Address into which data needs to written
2. char length : length of read data
3. char data[] : output read data
Output argumenst : void

*********************************************************************************************************/
void Wifi_credentials_read(char data[], char length, uint32_t address)
{
	int i;
	for (i = 0; i < length; i++) {
	data[i] = Wifi_credentials_memory_read(address + i);
	}
	data[i] = '\0';
}
/*********************************************************************************************************
 
Description : Function used to configure the eeprom
Input argumenst : 
1. size_t size : size of eeprom need to intialize , recommended it 4096
Output argumenst : bool : init is sucess or failure

*********************************************************************************************************/
bool beginEEPROM(size_t size)
{
	bool local_return_status_1 = true;
	bool local_return_status_2 = true;
	bool local_return_status_3 = true;

	local_return_status_1 = eeprom_begin_0(size);
	local_return_status_2 = eeprom_begin_1(size);
	local_return_status_3 = Wifi_credentials_memory_begin(size);
	if( (local_return_status_1 == false ) )
	{
		ESP_LOGE("EEPROM","Failed to begin eeprom_0");
		return false;
	}
	if( (local_return_status_2 == false) )
	{
		ESP_LOGE("EEPROM","Failed to begin eeprom_1");
		return false;
	}
	// if( (local_return_status_3 == false) )
	// {
	// 	ESP_LOGE("EEPROM","Failed to begin wifi credentials memory");
	// 	return false;
	// }

	return true;
	
}

