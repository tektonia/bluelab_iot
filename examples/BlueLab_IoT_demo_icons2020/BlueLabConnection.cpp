#include "BlueLabConnection.h"

// Fingerprint SHA1 - https://blueLab.pt
const uint8_t fingerprint[20] = {0x93, 0xDD, 0x67, 0x60, 0xDE, 0x66, 0x6D, 0x5A, 0x6C, 0xEC, 0xA5, 0x00, 0x9A, 0xBF, 0xC4, 0xB6, 0xCB, 0x4D, 0xD2, 0xCC};
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
"-----END CERTIFICATE-----\n";

/*
 * Create a new object.
 * l_host and l_url - identify the User database system
 * d_host and d_url - identoty the IoT data database system
*/
BlueLabConnection::BlueLabConnection(String l_host, String l_url, String d_host, String d_url){
  host_login=l_host;
  url_login=l_url;
  host_db=d_host;
  url_db=d_url;
  frame_head="";
  frame_data="";
  frame_control="";
  frame_debug="";
  time=0;
#if(SSL_CLIENT && !ESP_32_DEVICE)  
  https.setFingerprint(fingerprint);
  https.setTimeout(2000);   // miliseconds
#endif  
#if(SSL_CLIENT && ESP_32_DEVICE)  
  https.setCACert(root_ca);
  https.setTimeout(2000);   // miliseconds
#endif
 
}

/*
 * Establish a login to the User datbase system
 * A session id is returned for that login
*/
void BlueLabConnection::updateLogin(String cntct, char tipo_cntct, String pass, String last_payload){
  /**/
    String login_msg="";
    String res;
    password=pass;
    if(tipo_cntct=='A' || tipo_cntct=='O'){
      frame_auth="direct="+cntct+"&auth="+tipo_cntct;
      login_msg=frame_auth;
    }else{
      frame_auth=(tipo_cntct=='E'?"email=":"tlm=")+cntct+"&auth="+tipo_cntct;
      login_msg=frame_auth+"&password="+pass;
    }
    last_frame_payload=last_payload;
}
/*
 * Establish a login to the User datbase system
 * A session id is returned for that login
*/
bool BlueLabConnection::login(String cntct, char tipo_cntct, String pass){
  /**/
    String login_msg="";
    String res;
    password=pass;
    if(tipo_cntct=='A' || tipo_cntct=='O'){
      frame_auth="direct="+cntct+"&auth="+tipo_cntct;
      login_msg=frame_auth;
    }else{
      frame_auth=(tipo_cntct=='E'?"email=":"tlm=")+cntct+"&auth="+tipo_cntct;
      login_msg=frame_auth+"&password="+pass;
    }
        
    res=sendMessage(host_login, portHttp, url_login, login_msg);
    
    responseFromLastCall="login Response: "+res;
    DynamicJsonDocument  json(JSON_BUFFER_SIZE);
    DeserializationError jsonError=deserializeJson(json, res);
    if(jsonError.code()!=DeserializationError::Ok){
      sessionId=0;
      return false;
    }
    String error= json["error"];
    time = ((long long)json["time"]); // em segundos
    error.toLowerCase(); 
    if(!error.equals("false")) sessionId=0;
    else sessionId = json["session_id"];
    return sessionId!=0;
}

/*
 * Returns the highest sequence number for a given station id
 *
*/
int BlueLabConnection::activateStation(long long station_id, String txt){   
    String data_msg="func=createStationIfNotExists&"+frame_auth;
    data_msg+="&sessionId="+String(sessionId)+"&mac="+String((unsigned long) (station_id >> 32))+String((unsigned long) station_id & 0x0FFFFFFFF);
    data_msg+="&name="+txt;
    // data_msg+="&name="+String(sessionId)+"&lat="+String(station_id)+"&lon="+String(station_id)+"&obs="+String(station_id);
    String res=sendMessage(host_db, portHttp, url_db, data_msg);
    DynamicJsonDocument  json(JSON_BUFFER_SIZE);
    DeserializationError jsonError=deserializeJson(json, res);
    int id=ERR_INVALID_STATION_ID;
    if(jsonError.code()==DeserializationError::Ok) id=json["station_id"];
    responseFromLastCall="activateStation Response: "+res;
    WRITE_DEBUG(responseFromLastCall);
    return id;
}

/*
 * Returns the highest sequence number for a given station id
 *
*/
int BlueLabConnection::getSeqNum(int station_id){   
    String data_msg="func=getSeqNumber&"+frame_auth;
    data_msg+="&sessionId="+String(sessionId)+"&station_id="+String(station_id);
    String res=sendMessage(host_db, portHttp, url_db, data_msg);
    DynamicJsonDocument  json(JSON_BUFFER_SIZE);
    DeserializationError jsonError=deserializeJson(json, res);
    if(jsonError.code()!=DeserializationError::Ok){
      numSeq=ERR_INVALID_SEQUENCE_NUMBER;
    } else numSeq = json["seq_num"];
    responseFromLastCall="getSeqNum Response: "+res;
    WRITE_DEBUG(responseFromLastCall);
    return numSeq;
}

/*
 * Builds a new frame by setting the frame header with the station_id, sequence number and timestamp values
 *
*/
void BlueLabConnection::newFrame(int station_id, int seq, long long timeStamp){
      frame_head="func=storeArrayKeyValue&"+frame_auth+"&sessionId="+String(sessionId)+frame_debug;
      frame_control="&station_id="+String(station_id)+"&seq_num="+String(seq)+"&t_stamp="+longLongToString(timeStamp);
      frame_data="";
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/   
void BlueLabConnection::addKeyValue(String key, String value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, bool value){
  frame_data+="&dados["+key+"]="+String(value);
} 

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, char value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, int value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/  
void BlueLabConnection::addKeyValue(String key, long value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, long long value){
  frame_data+="&dados["+key+"]="+longLongToString(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, float value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Adds a key,value pair to the data header of the frame
 *
*/
void BlueLabConnection::addKeyValue(String key, double value){
  frame_data+="&dados["+key+"]="+String(value);
}

/*
 * Sets the debug header for all frames. Results from the sendFrame() and sendLastFrame() functions are more verbose
 *
*/
void BlueLabConnection::setDebug(bool deb){
  frame_debug=deb?"&debug=true":"";
}

int BlueLabConnection::responseReceived(String res){
  DynamicJsonDocument  json(JSON_BUFFER_SIZE);
  DeserializationError jsonError=deserializeJson(json, res);
  if(jsonError.code()==DeserializationError::Ok){
    String err = json["error"];
    String seq_num = json["seq_num"];
    if(seq_num=="") return(ERR_EXCEPTION);
    err.toUpperCase();
    if( err.equals("FALSE")) return (seq_num.toInt()) ; //("RES_OK");
    String err_msg = json["error_msg"];
    if(err_msg.startsWith("ERR_INVALID_SESSION_ID")) return (ERR_INVALID_SESSION_ID);
  }
  WRITE_DEBUG("EXCEPTION: responseReceived ###"+res+"###");
  return (ERR_EXCEPTION);
}

/*
 * Sends a frame with the pre filled headers (frame+data+debug)
 *
*/
int BlueLabConnection::sendFrame(){
  last_frame_payload=frame_control+frame_data;
  String res=sendMessage(host_db, portHttp, url_db, frame_head+frame_control+frame_data);
  responseFromLastCall="sendFrame Response: ###"+host_db+" "+portHttp+" "+url_db+" "+frame_head+" "+frame_control+" "+frame_data+"### "+res;
  WRITE_DEBUG(responseFromLastCall);
  return responseReceived(res);
}

/*
 * Converts a long long type value to a String
 *
*/
String BlueLabConnection::longLongToString(long long ll){
  if(ll==0) return "0";
  boolean sinal=false;
  if(ll<0) {ll=-ll; sinal=true;}
  #define LONG_LONG_LEN 50
  char buf[LONG_LONG_LEN];
  int idx=LONG_LONG_LEN-1;
  buf[idx]=0; // end of String
  for(;idx>0 && ll>0; ){
    buf[--idx]=ll%10 +'0';
    ll=ll/10;
  }
  if(sinal && idx>0){
    buf[--idx]='-';
  }
  return String((char *)&buf[idx]);
}   

String BlueLabConnection::getLastFramePayload(){  return last_frame_payload; }

void BlueLabConnection::storeLastFramePayload(String frame){  last_frame_payload=frame; }

String BlueLabConnection::getRresponseFromLastCall(){ return responseFromLastCall; }

int BlueLabConnection::getSessionId(){ return sessionId; }

void BlueLabConnection::setSessionId(int sid){ sessionId=sid; }

/*
 * Sends the last frame contents but with a new session id
 *
*/
int BlueLabConnection::sendLastFrame(){
  frame_head="func=storeArrayKeyValue&"+frame_auth+"&sessionId="+String(sessionId)+frame_debug;
  String res=sendMessage(host_db, portHttp, url_db, frame_head+last_frame_payload);
  responseFromLastCall="sendLastFrame Response: "+res;
  WRITE_DEBUG(responseFromLastCall);
  return responseReceived(res);
}

#if(SSL_CLIENT)
  String BlueLabConnection::sendMessage(String host, int port, String url,String msg) {
    if (!https.connect(host.c_str(), port)) {
      WRITE_DEBUG("HTTPS connection failed "+host+" "+String(port));
      https.stop();
      return "";
    }
    String txt="POST "+url+" HTTP/1.1\r\nHost: "+host+":"+String(port)+"\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Content-Length: "+ String(msg.length())+"\r\n\r\n"+msg+"\r\nConnection: close\r\n\r\n";
    int httpsCode=https.print(txt);
    
    //WRITE_DEBUG("sendMessage --> "+txt+"\n\r"); //host:"+host+" url:"+url+" msg:"+msg+"\n\r"); 

    while (https.connected() && https.readStringUntil('\n')!="\r") ; // flush out received headers 
    https.readStringUntil('\n');  //Ignore first string (length in hexa)
    String str = https.readStringUntil('\n');  //Read line
    https.stop();
    //WRITE_DEBUG("sendMessage result: "+str);
    return str;
  } 
#else
  String BlueLabConnection::sendMessage(String host, int port, String url, String msg) {
      if (!http.begin("http://"+host+":"+String(port)+url)){
        WRITE_DEBUG("HTTPS connection failed "+host+" "+String(port));
        http.end();
        return "";        
      }
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.POST(msg);
      //WRITE_DEBUG("sendMessage --> host:"+host+" url:"+url+" msg:"+msg); 
      String str=""; 
      if (httpCode > 0) {
        str=http.getString();
      } else str=http.errorToString(httpCode);
      http.end();
      //WRITE_DEBUG("sendMessage result: "+str);
      return str;
  }
#endif
