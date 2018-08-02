#include "BlueLabConnection.h"

const int BlueLabConnection::ERR_INVALID_SESSION_ID=-1;
const int BlueLabConnection::ERR_EXCEPTION=-2;

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
  frame_debug="";
}

/*
 * Establish a login to the User datbase system
 * A session id is returnes for that login
*/
bool BlueLabConnection::login(String mail, String pass){
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

/*
 * Returns the highest sequence number for a given station id
 *
*/
int BlueLabConnection::getSeqNum(int station_id){   
    String data_msg="func=getSeqNumber&email="+email+"&sessionId="+String(sessionId)+"&station_id="+String(station_id);
    String res=sendMessage(host_db, portHttp, url_db, data_msg);
    
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
    JsonObject&  json= jsonBuffer.parseObject(res);
    
    numSeq = json["seq_num"];
    return numSeq;
}

/*
 * Builds a new frame by setting the frame header with the station_id, sequence number and timestamp values
 *
*/
void BlueLabConnection::newFrame(int station_id, int seq, long long timeStamp){
      last_station_id=station_id;
      last_seq=seq;
      last_timeStamp=timeStamp;
      frame_head="func=storeArrayKeyValue&sessionId="+String(sessionId)+"&email="+email+"&station_id="+String(station_id)+"&seq_num="+String(seq)+"&t_stamp="+longLongToString(timeStamp);
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

/*
 * Sends a frame with the pre filled headers (frame+data+debug)
 *
*/
int BlueLabConnection::sendFrame(){
  String res=sendMessage(host_db, portHttp, url_db, frame_head+frame_debug+frame_data);

  DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
  JsonObject&  json= jsonBuffer.parseObject(res);
    
  String err = json["error"];
  String seq_num = json["seq_num"];
  err.toUpperCase();
  if( err.equals("FALSE")) return (seq_num.toInt()) ; //("RES_OK");
  if(err.equals("ERR_INVALID_SESSION_ID")) return (ERR_INVALID_SESSION_ID);
  return (ERR_EXCEPTION);
}

/*
 * Converts a long long type value to a String
 *
*/
String BlueLabConnection::longLongToString(long long ll){
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

/*
 * Sends the last frame contents but with a new session id
 *
*/
int BlueLabConnection::sendLastFrame(){
  frame_head="func=storeArrayKeyValue&sessionId="+String(sessionId)+"&email="+email+"&station_id="+String(last_station_id)+"&seq_num="+String(last_seq)+"&t_stamp="+longLongToString(last_timeStamp);
  return sendFrame();
}

String BlueLabConnection::sendMessage(String host, int port, String url,String msg) {
  http.begin("http://"+host+":"+String(port)+url); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //GET -> "Content-Type", "text/plain");
  int httpCode = http.POST(msg);
  return http.getString();
}


