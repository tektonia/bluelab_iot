#ifndef FLASHH_H  // there is a FLASH_H already used somewhere!!!!
#define FLASHH_H
#include <ESP.h>
#include <EEPROM.h>


class Flash{
      
  public:
    //static RtcDS3231 <TwoWire> *rtc;
    static void begin(); //RtcDS3231 <TwoWire> *r);
    static void end();
    static String getUserPassword();
    static void setUserPassword(String str);
    static String getUserContact();
    static void setUserContact(String str);
    static char getUserContactType();
    static void setUserContactType(char c);
    static String getSSID();
    static void setSSID(String str);
    static String getSSID_PWD();
    static void setSSID_PWD(String str);
    static void setBaseNumber(int n);
    static int getBaseNumber();
    static void setModuleName(String nome);
    static String getModuleName();
       
  private:
     static String read(int addr, int len, char var[]);
     static void write(int addr, int len, char var[]);
     static void write(int addr, int len, String str);  
     static uint8_t read(int addr);
     static int readInt(int addr);
     static void read(int addr, int *v);
     static void write(int addr, uint8_t n);
     static void writeInt(int addr, int n);
     static void writeChar(int addr, char c);
     static char readChar(int addr);     
     static void writeBool(int addr, bool b);
     static bool readBool(int addr);
        
    #define USR_CNTCT_ADDR (0)
    #define USR_CNTCT_LEN 80
    static char user_cntct[USR_CNTCT_LEN];
    
    #define USER_PWD_ADDR (USR_CNTCT_ADDR + USR_CNTCT_LEN)
    #define USER_PWD_LEN 80
    static char user_pwd[USER_PWD_LEN];
       
    #define SSID_ADDR (USER_PWD_ADDR + USER_PWD_LEN)
    #define SSID_LEN 37
    static char ssid[SSID_LEN];
        
    #define SSID_PWD_ADDR (SSID_ADDR + SSID_LEN)
    #define SSID_PWD_LEN 129
    static char ssid_pwd[SSID_PWD_LEN];

    #define CNTCT_TYPE_ADDR (SSID_PWD_ADDR + SSID_PWD_LEN)
    #define CNTCT_TYPE_LEN sizeof(char)
   
    #define MODULE_NAME_ADDR (CNTCT_TYPE_ADDR + CNTCT_TYPE_LEN)
    #define MODULE_NAME_LEN 80
    static char moduleName[MODULE_NAME_LEN];
    

    #define FIM_ADDR (MODULE_NAME_ADDR + MODULE_NAME_LEN)

    #define TOTAL_USADO (FIM_ADDR)
};
#endif /* FLASHH_H */
