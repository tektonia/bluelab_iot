#include <ESP.h>
#include "config.h"
#include "BlueLabConnection.h"
#if(ESP_32_DEVICE)
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include "Uteis.h"
#include "Flash.h"

Uteis util;

// GPIO Pin definition
#if(ESP_32_DEVICE)
  #define DIO0 0
  #define ADC0 36
  #define ADC6 34 

  #define ADC ADC0
#else
  #define DIO0 0
  #define ADC0 A0

  #define ADC ADC0
#endif

/**************************
    Internet access
change with your SSID and SSID_PASSWORD
*******************/
String ssid = "my_ssid";
String ssid_password = "my_ssid_password";

/**************************
    BlueLab IoT links
DO NOT CHANGE
*******************/
String login_host="www.bluelab.pt";
String login_url="/iot/bluelab_login.php";
String data_host="www.bluelab.pt";
String data_url="/iot/calliot.php";  

/**************************
    BlueLab IoT system access
change with your your credentials (you can use the demo credentials)
visit bluelab.pt and follow IoT link
-- create an account 
-- create an environment (by adding a station)
   (station id is unique and it is assigned to your station if the creation was a success)
-- or create the station as it is done in theis example
*******************/
String usr_contact="ICONS2020"; //"mail@mail.mail"; // ex: "+351999999999";
String usr_password="password";
char tipoCntct='O';  // 'E' email, 'O' other, 'A' auto, 'T' phone

// Stations should have unique ids (or ensure they have unique key strings)
String moduleName = "BlueLab_Test";
unsigned long long macAddr =0LL; // mac addr; byte mac[6];  // 4D0E680502C
#define UNIQUE_ID_STR (String((unsigned long) (macAddr >> 32))+String((unsigned long) macAddr & 0x0FFFFFFFF))
int station_id=0; // This station id is assigned to your station by bluelab.pt

/* IoT example
  This program uses two pins as input, one analog and one digital.
  The analog input ADC0 if refered to input A0 on ESP8266, and to GPIO36 on ESP32
  The digital input is the push button named boot or flash which corresponds to D3(pin 0) in ESP8266, and GPIO0 on ESP32

  The program samples both inputs. The digital value is used as it is read. The analog value is filtered through a moving average filter.
  
*/

#define MAX_RETRIES 10          // maximum number of retries in case of error
#define DELAY_MILISECONDS 2500  // time between samples
#define SAMPLE_COUNT 2          // writes to the database every DELAY_MILISECONDS * SAMPLE_COUNT (90) seconds
#define DIM_FILTER	 3			    // number of previous samples from which mean value is calculated - filter size

BlueLabConnection *dbConn;      // Connection to the Database
  
long long timeStamp=0;          // timeStamp of the data in the frame
long long elapsedTime=0;             // in micros
int seqNum;                     // sequence number of the frame
int count=0;                    // a retry counter
  
int adcPin=ADC;                  // analog input pin
int inPin=DIO0;                   // digital input pin

// Filter
float values[DIM_FILTER];       // structure for the filter
int idx;                        // index into the filter structure
float meanValue;                // mean value produced by the filter

/*
 * doReset 
 *
 * Reset the module
*/
void doReset(String err){
  if(VERBOSE) Serial.print("\nERROR - Resetting module: "+err+"\n");
  EspClass esp;
  esp.restart();
}

/*
 * buildLowPassFilter 
 *
 * initialize the filter structure
*/
void buildLowPassFilter(){
  idx=0; meanValue=0.0F; 
  for(int i=0;i<DIM_FILTER; i++) values[i]=0.0F;
}

/*
 * lowPassFilter 
 *
 * Input the filter with a new value from the same variable and returns the filtered mean value
*/
float lowPassFilter(float value){
  value/=(float)DIM_FILTER;
  if(isnan(value)) return value;
  float old = values[idx];
  meanValue+=value-old;
  values[idx]=value;
  if(++idx>=DIM_FILTER) idx=0;
  return meanValue;
}

void reConnect(){
  Serial.print("Reconnecting through: "+ssid+" and user "+usr_contact+" ...");
  dbConn = new BlueLabConnection(login_host, login_url, data_host, data_url);
  WiFi.begin(ssid.c_str(), ssid_password.c_str());
  count=0;
  while (WiFi.status() != WL_CONNECTED) {  
    if(count++>=MAX_RETRIES) doReset("No Internet Connection! Verify SSID and password");
    Serial.print(".");
    delay(3000);
  }
  count=0;
  while(!dbConn->login(usr_contact, tipoCntct, usr_password))  {
    if(count++>=MAX_RETRIES) doReset("No Access to the BlueLab Iot System. Verify user and password!");
    delay(3000);
  }  
}

//////////////

void setPasswordAndSSID(char *c){
  ssid= util.parseString(&c);
  Serial.println(ssid); 
  Flash::setSSID(ssid); 
  Serial.println(Flash::getSSID());
  ssid_password= util.parseString(&c);
  Serial.println(ssid_password); 
  Flash::setSSID_PWD(ssid_password); 
  Serial.println(Flash::getSSID_PWD());
}

#define prompt() Serial.print("\n> ")
#define help() Serial.print("\n? Commands\nU readUserLoginAndPassword\nR readContactType\nS showStatus\nI readSSIDandPassword\nN setModuleName\nQ quit")

void readSSIDandPassword(){
  util.getEOL();
  ssid=util.readString("SSID: ");
  Flash::setSSID(ssid);
  ssid_password=util.readString(" PASSWORD: ");
  Flash::setSSID_PWD(ssid_password);
}

void readContactType(){
  util.getEOL();
  tipoCntct=util.readChar("usr_contact_type (E,T,A,O): ");
  util.getEOL();
  Flash::setUserContactType(tipoCntct);
}

void readLoginAndPassword(){
  util.getEOL();
  usr_contact=util.readString("usr_contact: ");
  Flash::setUserContact(usr_contact);
  usr_password=util.readString(" usr_password: ");
  Flash::setUserPassword(usr_password);
}

void setModuleName(){
  util.getEOL();
  moduleName=util.readString("Module name: ");
  Flash::setModuleName(moduleName);
}

void showNetworks(){
  int n = WiFi.scanNetworks(false, true);
  String ssid;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t* BSSID;
  int32_t channel;
  bool isHidden;
  
  #if(!ESP_32_DEVICE)
    for (int i = 0; i < n; i++){
      WiFi.getNetworkInfo(i, ssid, encryptionType, RSSI, BSSID, channel, isHidden);    
      Serial.printf("%d: |%s|, Ch:%d (%ddBm) %s %s\n", i + 1, ssid.c_str(), channel, RSSI, encryptionType == ENC_TYPE_NONE ? "open" : String(encryptionType).c_str(), isHidden ? "hidden" : "");
    }
  #endif 
}

void showStatus(){
  Serial.println("Module Satus");
  Serial.println("  UserContactType: #"+String(Flash::getUserContactType())+"#");    
  Serial.println("  UserContact: #"+Flash::getUserContact()+"#");
  Serial.println("  UserPassword: #"+Flash::getUserPassword()+"#");
  Serial.println("  SSID: #"+Flash::getSSID()+"#");
  Serial.println("  SSID_PWD: #"+Flash::getSSID_PWD()+"#");
  Serial.println("  Module Name: #"+Flash::getModuleName()+"#");  
}

/////////////

/*
 * GetConfig
*/
void getConfig(){
  Serial.println("\n\n");
  showStatus();
  Serial.println("\n\n");
  help();
  prompt();
  int entrada=-1;     
  int cont=80;   
  Serial.print("Press a key to enter Setup ...");
  do{
    entrada=Serial.read();
    if(entrada==-1){
      delay(100);
      cont--;
    }else break;
  }while(cont>0);
  while(cont>0){
      entrada= entrada | 0x20;
      switch(entrada){
        case '?': help(); break;
        case 'u': readLoginAndPassword(); break;
        case 'r': readContactType(); break;   
        case 's': showStatus(); break;
        case 'i': readSSIDandPassword(); break;
        case 'n': setModuleName(); break;        
        //case 'w': showNetworks(); break;
      }
      if(entrada=='q') break; 
      prompt();
      while(Serial.available()<=0) ;
      entrada=Serial.read();
  }
  moduleName=Flash::getModuleName();
  ssid_password=Flash::getSSID_PWD();
  ssid=Flash::getSSID();
  tipoCntct=Flash::getUserContactType();
  usr_contact=Flash::getUserContact();
  usr_password=Flash::getUserPassword();
}


/*
 * setup 
 *
 * Initialize the program,
 * establish a wireless link,
 * login into the bluelab.pt system,
 * retrieve the last sequence number of the stored data frames,
 * initialize the filter for the analog variable
 * prepare to sample the inputs (one digital and one analog)
*/
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nBlueLab IoT demo - VVS 2019\n\nhttps://bluelab.pt/iot"); 
  WiFi.macAddress((byte *)&macAddr);  // get MAC address

  Serial.println("\nUnique id: "+UNIQUE_ID_STR+"\n");
  Flash::begin();
  getConfig();

  dbConn = new BlueLabConnection(login_host, login_url, data_host, data_url);

  Serial.print("Acquiring Internet access, through: "+ssid+" ...");
  WiFi.begin(ssid.c_str(), ssid_password.c_str());
  while (WiFi.status() != WL_CONNECTED) {  
    if(count++>=MAX_RETRIES) doReset("No Internet Connection! Verify SSID and password");
    Serial.print(".");
    delay(3000);
  }
  Serial.println(" WiFi connected :):):)");

  Serial.print("\nLogging into BlueLab IoT system with "+usr_contact+" ...");
  count=0;
  while(!dbConn->login(usr_contact, tipoCntct, usr_password))  {
    Serial.println("Could not Login!");
    if(count++>=MAX_RETRIES) doReset("No Access to the BlueLab Iot System. Verify user and password!");
    delay(3000);
  }
  elapsedTime=micros();
  Serial.println(" Logged in :):):)");
  station_id=dbConn->activateStation(macAddr, moduleName);
  if(station_id<=0){
     doReset("ERR_INVALID_STATION_ID after good login");  // this should never happen!
  }
  seqNum=dbConn->getSeqNum(station_id);  // get current sequence
  if(seqNum<0){
     doReset("ERR_INVALID_SEQUENCE_NUMBER after good login");  // this should never happen! There may be communication failures
  }
  seqNum++;                              // increment sequence

  timeStamp=dbConn->time*1000000LL; // in microseconds
  dbConn->newFrame(station_id, seqNum, timeStamp);
  dbConn->addKeyValue("reset",0);                     // reset key
  int res=dbConn->sendFrame();
  Serial.println("\nSendFrame "+String(seqNum)+": key=reset value=0 seqNum="+String(res)+"\n");
  if(res==BlueLabConnection::ERR_INVALID_SESSION_ID){
    doReset("ERR_INVALID_SESSION_ID after sending Reset key");  // this should never happen!
  }
  /**/

  pinMode(inPin, INPUT);  // Configure digital pin for input
  buildLowPassFilter();   // Build filter for analog input
  count=SAMPLE_COUNT;     // First sample is sent to the database
  #if(ESP_32_DEVICE)
    adcAttachPin(ADC); // 
    analogSetAttenuation(ADC_0db); // default ADC_11db
    analogReadResolution(12); // default 12
  #endif
}

/*
 * loop 
 *
 * wait for the sampling time (!),
 * read the inputs( one digital and one analog),
 * input the filter with the analog reading and receive the mean value
 * every SAMPLE_COUNT samples send a dataframe to the system
 * prepare to sample the inputs
*/      
void loop() {
  delay(DELAY_MILISECONDS);
  elapsedTime=micros()-elapsedTime;
  if(elapsedTime<0) elapsedTime+=(unsigned long) (-1);
  timeStamp+=elapsedTime; //DELAY_MILISECONDS*1000L;  // add approximately number of miliseconds since reset (timeout in microseconds)
  elapsedTime=micros();
  int din=digitalRead(inPin);    // read digital value
  int adc=analogRead(adcPin);    // read analog value
  float adc_value=lowPassFilter(adc);  // set analog value into filter input, and get mean value
  
  Serial.print("New sample; Digital In: "+String(din)+" ADC: "+String(adc)+" ADC_mean: "+String(adc_value)+"\n");

  if(++count>=SAMPLE_COUNT){
    count=0;

    dbConn->newFrame(station_id, seqNum, timeStamp);  // Create a new dataframe
    dbConn->addKeyValue("pin",din);                   // Add a key-value pair
    dbConn->addKeyValue("adc",adc_value);             // Add another key-value pair

    Serial.println("SendFrame: key=pin value="+String(din)+"key=adc value="+String(adc_value)+"\n");
    int res=dbConn->sendFrame();                      // Send the dataframe and receive the sequence number of the last stored value
    if(res>0 && res==seqNum) seqNum++;                         // If the last stored dataframe sequence number is the same of our dataframe's then prepare for next one
    else{
      Serial.println("SendFrame: received sequence = "+res);
      if(res==BlueLabConnection::ERR_INVALID_SESSION_ID){  //otherwise if we were using a outdated session number
        dbConn->login(usr_contact, tipoCntct, usr_password);   // reconnect for a new session id
      }
      else{
        String payload=dbConn->getLastFramePayload();      // save last frame's payload
        reConnect();                                       // it is another error, it should be understood and solved and then resend the last frame
        dbConn->storeLastFramePayload(payload);            // restore last frame's payload to invoque sendLastFrame()
      }
      seqNum=dbConn->getSeqNum(station_id);         // get the sequence number of the next frame to be sent, to make sure we are synched again ...       
      if(seqNum<0){
        doReset("ERR_INVALID_SEQUENCE_NUMBER after reconnect attempt");  // this should never happen! There may be communication failures
      } else seqNum++;
      int other_res=dbConn->sendLastFrame();             // resend the last frame .... check the sequence numbers again as above //
      if(other_res>0 && other_res==seqNum) {seqNum++; return; }      // else if error persists then it should be understood and solved
    } 
  }
}
