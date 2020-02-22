#include <ESP.h>
#include "config.h"
#include "BlueLabConnection.h"
#if(ESP_32_DEVICE)
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

// GPIO Pin definition
#if(ESP_32_DEVICE)
  #define DIO0 0
  #define ADC0 36
  #define ADC6 34 
#else
  #define DIO0 0
  #define ADC0 A0
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
String usr_contact="Demo"; //"mail@mail.mail"; // ex: "+351999999999";
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
#define DELAY_MILISECONDS 3000  // time between samples
#define SAMPLE_COUNT 30         // writes to the database every DELAY_MILISECONDS * SAMPLE_COUNT (90) seconds
#define DIM_FILTER	 3			    // number of previous samples from which mean value is calculated - filter size

BlueLabConnection *dbConn;      // Connection to the Database
  
long long timeStamp=0;          // timeStamp of the data in the frame
int seqNum;                     // sequence number of the frame
int count=0;                    // a retry counter
  
int adcPin=ADC0;                  // analog input pin
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
  Serial.println(" Logged in :):):)");
  station_id=dbConn->activateStation(macAddr);
  if(station_id<=0){
     doReset("ERR_INVALID_STATION_ID after good login");  // this should never happen!
  }
  seqNum=dbConn->getSeqNum(station_id);  // get current sequence
  if(seqNum<0){
     doReset("ERR_INVALID_SEQUENCE_NUMBER after good login");  // this should never happen! There may be communication failures
  }
  seqNum++;                              // increment sequence

  timeStamp=0;
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
  timeStamp+=DELAY_MILISECONDS;  // approximately number of miliseconds since reset
  int din=digitalRead(inPin);    // read digital value
  int adc=analogRead(adcPin);    // read analog value
  float adc_value=lowPassFilter(adc);  // set analog value into filter input, and get mean value
  
  Serial.print("New sample; Digital In: "+String(din)+" ADC: "+String(adc)+" ADC_mean: "+String(adc_value)+"\n");

  if(count++>=SAMPLE_COUNT){
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
