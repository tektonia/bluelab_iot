#include "Flash.h"

char Flash::user_cntct[USR_CNTCT_LEN];
char Flash::user_pwd[USER_PWD_LEN];
char Flash::ssid[SSID_LEN];
char Flash::ssid_pwd[SSID_PWD_LEN];
char Flash::moduleName[MODULE_NAME_LEN];

/*
RtcDS3231 <TwoWire> *Flash::rtc=NULL;
*/
void Flash::begin(){ //RtcDS3231 <TwoWire> *r){
  //rtc=r;
  EEPROM.begin(2048);
}

void Flash::end(){
  EEPROM.end();
}

String Flash::getUserPassword(){
  return read(USER_PWD_ADDR, USER_PWD_LEN, Flash::user_pwd);
}

void Flash::setUserPassword(String str){
  write(USER_PWD_ADDR, USER_PWD_LEN, str);
}

String Flash::getUserContact(){
  return read(USR_CNTCT_ADDR, USR_CNTCT_LEN, Flash::user_cntct);
}

void Flash::setUserContact(String str){
  write(USR_CNTCT_ADDR, USR_CNTCT_LEN, str);  
}

char Flash::getUserContactType(){
  return readChar(CNTCT_TYPE_ADDR);
}

void Flash::setUserContactType(char car){
  writeChar(CNTCT_TYPE_ADDR, car); 
}

String Flash::getSSID(){
  return read(SSID_ADDR, SSID_LEN, Flash::ssid);
}

void Flash::setSSID(String str){
  write(SSID_ADDR, SSID_LEN, str);  
}

String Flash::getSSID_PWD(){
  return read(SSID_PWD_ADDR, SSID_PWD_LEN, Flash::ssid_pwd);
}

void Flash::setSSID_PWD(String str){
  write(SSID_PWD_ADDR, SSID_PWD_LEN, str);  
}

void Flash::setModuleName(String str){
  write(MODULE_NAME_ADDR, MODULE_NAME_LEN, str);  
}

String Flash::getModuleName(){
  return read(MODULE_NAME_ADDR, MODULE_NAME_LEN, Flash::moduleName);
}

//////////// Funcões de leitura e escrita gerais

uint8_t Flash::read(int addr){
  return EEPROM.read(addr);
}

void Flash::write(int addr, uint8_t n){
  EEPROM.write(addr, n);
  EEPROM.commit();
}

String Flash::read(int addr, int len, char var[]){
    for(char i=0; i<len; i++) var[i]= (char) EEPROM.read(addr+i);
    var[len-1]=0;
    return(String(var));
}

void Flash::write(int addr, int len, String str){
    char *var=(char *)str.c_str();
    for(char i=0; i<len; i++) EEPROM.write(addr+i, (uint8_t) var[i]);
    EEPROM.commit();
}

void Flash::write(int addr, int len, char var[]){
    for(char i=0; i<len; i++) EEPROM.write(addr+i, (uint8_t) var[i]);
    EEPROM.commit();
}

void Flash::writeInt(int addr, int val){
    EEPROM.write(addr, (uint8_t) (val & 0x0FF));
    EEPROM.write(addr+1, (uint8_t) ((val>>8) & 0x0FF));
    EEPROM.commit();
}

int Flash::readInt(int addr){
    return ((int) (EEPROM.read(addr) + (EEPROM.read(addr+1)<<8)));
}

void Flash::writeChar(int addr, char val){
    EEPROM.write(addr, (uint8_t) (val));
    EEPROM.commit();
}

char Flash::readChar(int addr){
    return ((char) EEPROM.read(addr));
}

void Flash::writeBool(int addr, bool val){
    EEPROM.write(addr, (uint8_t) (val));
    EEPROM.commit();
}

bool Flash::readBool(int addr){
    return ((bool) EEPROM.read(addr));
}

void Flash::read(int addr, int *val){
    *val=(int) (EEPROM.read(addr) + (EEPROM.read(addr+1)<<8));
}
