#ifndef BLUELABCONNECTION_H
#define BLUELABCONNECTION_H
#include <ESP.h>
#include "config.h"

#include <ArduinoJson.h>

#if(SSL_CLIENT)
  #include <WiFiClientSecure.h>
#else
  #if(ESP_32_DEVICE)
    #include <HTTPClient.h>   
  #else
    #include <ESP8266HTTPClient.h>
  #endif
#endif

#if(VERBOSE)
  #define WRITE_DEBUG(n) Serial.println((n))
#else
  #define WRITE_DEBUG(n)
#endif

class BlueLabConnection{
  private:
      #if(SSL_CLIENT)  
        WiFiClientSecure https;      
        int portHttp=443;
      #else
        HTTPClient http;  
        int portHttp=80;       
      #endif
      String host_login;
      String url_login;
      String host_db;
      String url_db;
      String password;
      int sessionId;
      int numSeq;
      const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 512;
      String frame_auth;
      String frame_head;
      String frame_control;
      String frame_data;
      String frame_debug;
      String last_frame_payload;
      String responseFromLastCall;
      String sendMessage(String host, int port, String url,String msg);
      int responseReceived(String res);
  public:
    static const int ERR_INVALID_SESSION_ID=-1;
    static const int ERR_EXCEPTION=-2;
    static const int ERR_INVALID_STATION_ID=-3;
    static const int ERR_INVALID_SEQUENCE_NUMBER=-4;
    
    BlueLabConnection(String l_host, String l_url, String d_host, String d_url);
    bool login(String cntct, char tipo_cntct, String pass);
    void updateLogin(String cntct, char tipo_cntct, String pass, String last_payload);
    String longLongToString(long long ll);
    void newFrame(int base, int seq, long long timeStamp);
    void addKeyValue(String key, String value);
    void addKeyValue(String key, bool value); 
    void addKeyValue(String key, char value); 
    void addKeyValue(String key, int value);   
    void addKeyValue(String key, long value); 
    void addKeyValue(String key, long long value); 
    void addKeyValue(String key, float value);   
    void addKeyValue(String key, double value); 
    void setDebug(bool deb);
    int sendFrame();
    int sendLastFrame();
    int getSeqNum(int NumBase);
    int activateStation(long long station_id);
    int getSessionId();
    void setSessionId(int sid);
    String getLastFramePayload();
    void storeLastFramePayload(String frame);
    String getRresponseFromLastCall();
    long long time;
};

#endif /* BLUELABCONNECTION_H */
