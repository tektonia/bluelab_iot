#include "BlueLabConnection.h"

DataBaseConnection::DataBaseConnection(String l_host, String l_url, String d_host, String d_url){
  host_login=l_host;
  url_login=l_url;
  host_db=d_host;
  url_db=d_url;
  frame_head="";
  frame_data="";
  frame_debug="";
}

bool DataBaseConnection::login(String mail, String pass){
  /**/
    email=mail;
    password=pass;
    String login_msg="email="+mail+"&password="+pass;
    String res;
    
    res=sendMessage(host_login, portHttp, url_login, login_msg);
    
    //Serial.print("Resposta: "+res);
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
    JsonObject& json = jsonBuffer.parseObject(res);
    String error= json["error"];
    error.toLowerCase(); 
    if(!error.equals("false")) sessionId=0;
    else sessionId = json["session_id"];
    return sessionId!=0;
}

int DataBaseConnection::getSeqNum(int NumBase){   
    String data_msg="func=getSeqNumber&email="+email+"&sessionId="+String(sessionId)+"&station_id="+String(NumBase);
    String res=sendMessage(host_db, portHttp, url_db, data_msg);
    
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
    JsonObject&  json= jsonBuffer.parseObject(res);
    
    numSeq = json["seq_num"];
    return numSeq;
}

    void DataBaseConnection::newFrame(int base, int seq, long long timeStamp){
      last_base=base;
      last_seq=seq;
      last_timeStamp=timeStamp;
      frame_head="func=storeArrayKeyValue&sessionId="+String(sessionId)+"&station_id="+String(base)+"&seq_num="+String(seq)+"&t_stamp="+longLongToString(timeStamp);
      frame_data="";
    }
    
    void DataBaseConnection::addKeyValue(String key, String value){
      frame_data+="&dados["+key+"]="+String(value);
    }
    
    void DataBaseConnection::addKeyValue(String key, bool value){
      frame_data+="&dados["+key+"]="+String(value);
    } 
    
    void DataBaseConnection::addKeyValue(String key, char value){
      frame_data+="&dados["+key+"]="+String(value);
    }
     
    void DataBaseConnection::addKeyValue(String key, int value){
      frame_data+="&dados["+key+"]="+String(value);
    }
        
    void DataBaseConnection::addKeyValue(String key, long value){
      frame_data+="&dados["+key+"]="+String(value);
    }
      
    void DataBaseConnection::addKeyValue(String key, long long value){
      frame_data+="&dados["+key+"]="+longLongToString(value);
    }
      
    void DataBaseConnection::addKeyValue(String key, float value){
      frame_data+="&dados["+key+"]="+String(value);
    }
        
    void DataBaseConnection::addKeyValue(String key, double value){
      frame_data+="&dados["+key+"]="+String(value);
    }
    void DataBaseConnection::setDebug(bool deb){
      frame_debug=deb?"&debug=true":"";
    }
      
    String DataBaseConnection::sendFrame(){
      String res=sendMessage(host_db, portHttp, url_db, frame_head+frame_debug+frame_data);

      DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
      JsonObject&  json= jsonBuffer.parseObject(res);
    
      String err = json["RESULT"];
       err.toUpperCase();
      return err;
   }

String DataBaseConnection::longLongToString(long long ll){
      if(ll==0) return "0";
      #define LONG_LONG_LEN 50
      char buf[LONG_LONG_LEN];
      int idx=LONG_LONG_LEN-1;
      buf[idx]=0;// fim de String
      for(;idx>0 && ll>0; ){
        buf[--idx]=ll%10 +'0';
        ll=ll/10;
      }
      return String((char *)&buf[idx]);
    }   

String DataBaseConnection::sendLastFrame(){
  frame_head="func=storeArrayKeyValue&sessionId="+String(sessionId)+"&station_id="+String(last_base)+"&seq_num="+String(last_seq)+"&t_stamp="+longLongToString(last_timeStamp);
  return sendFrame();
}

String DataBaseConnection::sendMessage(String host, int port, String url,String msg) {
  http.begin("http://"+host+":"+String(port)+url); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //GET -> "Content-Type", "text/plain");
  int httpCode = http.POST(msg);
  return http.getString();
}


