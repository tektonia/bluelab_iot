#ifndef BLUELABCONNECTION_H
#define BLUELABCONNECTION_H

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

class DataBaseConnection{
  private:
      HTTPClient http;  
      int portHttp=80; 
      String host_login;
      String url_login;
      String host_db;
      String url_db;
      String email;
      String password;
      int sessionId;
      int numSeq;
      const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 512;
      String frame_head;
      String frame_data;
      String frame_debug;
      int last_base;
      int last_seq;
      long long last_timeStamp;
      String sendMessage(String host, int port, String url,String msg);
  public:

    DataBaseConnection(String l_host, String l_url, String d_host, String d_url);
    bool login(String mail, String pass);
    int getSessionId(int nBase);
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
    String sendFrame();
    String sendLastFrame();
    int getSeqNum(int NumBase);
};

#endif /* BLUELABCONNECTION_H */

