#include <ESP.h>
#include <ESP8266WiFi.h>

#include "BlueLabConnection.h"

/**************************
    Internet access
change with your SSID and SSID_PASSWORD
*******************/
String ssid = "my_ssid";
String ssid_password = "my_ssid_password";

/**************************
    BlueLab IoT system access
change with your your credentials (you can use the demo credentials)
visit bluelab.pt and follow IoT link
-- create an account 
-- create an environment (by adding a station)
   (station id is unique and it is assigned to your station if the creation was a success)
*******************/
String usr_contact="a@a.a"; // ex: "+351999999999";
String usr_password="a";
boolean isEmail=true;  // email/phone

// Stations should have unique ids (or ensure they have unique key strings)
String moduleName = "BlueLab_Test";
int station_id = 1; // This station id was assigned to your station by bluelab.pt) 

/**************************
    BlueLab IoT links
DO NOT CHANGE
*******************/
String login_host="www.bluelab.pt";
String login_url="/iot/bluelab_login.php";
String data_host="www.bluelab.pt";
String data_url="/iot/calliot.php";  

#define MAX_RETRIES 10          // maximum number of retries in case of error
#define DELAY_MILISECONDS 3000  // time between samples
#define SAMPLE_COUNT 30         // writes to the database every DELAY_MILISECONDS * SAMPLE_COUNT (90) seconds
#define DIM_FILTER	8			      // number of previous samples from which mean value is calculated - filter size

BlueLabConnection *dbConn;      // Connection to the Database
  
long long timeStamp=0;          // timeStamp of the data in the frame
int seqNum;                     // sequence number of the frame
int count=0;                    // a retry counter
  
int adcPin=A0;                  // an analog input
int inPin=D0;                   // a digital input

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
  Serial.print("\nERROR - Resetting module: "+err+"\n");
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
  Serial.println("\nBlueLab IoT demo - VVS 2018\n\nhttps://bluelab.pt"); 

  Serial.println("\nStation id: "+String(station_id)+"\n");

  dbConn = new BlueLabConnection(login_host, login_url, data_host, data_url);

  Serial.print("Acquiring Internet access, through: "+ssid+" ...");
  WiFi.begin(ssid.c_str(), ssid_password.c_str());
  while (WiFi.status() != WL_CONNECTED) {  
    if(count++>=MAX_RETRIES) doReset("No Internet Connection");
    WiFi.begin(ssid.c_str(), ssid_password.c_str());
    Serial.print(".");
    delay(3000);
  }
  Serial.println(" WiFi connected :):):)");

  Serial.print("\nLogging in to BlueLab IoT system with "+usr_contact+" ...");
  count=0;
  while(!dbConn->login(usr_contact, isEmail, usr_password))  {
    Serial.println("Could not Login!");
    if(count++>=MAX_RETRIES) doReset("No Access to the BlueLab Iot System. user:"+usr_contact+": password:"+usr_password+":");
    delay(3000);
  }
  Serial.println(" Logged in :):):)");
  
  seqNum=dbConn->getSeqNum(station_id);  //sequencia actual
  seqNum++;

  timeStamp=0;
  dbConn->newFrame(station_id, seqNum, timeStamp);
  dbConn->addKeyValue("reset",0);
  int res=dbConn->sendFrame();
  Serial.println("\nSendFrame: key=reset value=0 seqNum="+String(res)+"\n");
  if(res==BlueLabConnection::ERR_INVALID_SESSION_ID){
    dbConn->login(usr_contact, isEmail, usr_password);
    seqNum=dbConn->getSeqNum(station_id);  //sequencia actual
    seqNum++;
  }
  /**/

  pinMode(inPin, INPUT);
  buildLowPassFilter();
  count=SAMPLE_COUNT;
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
  int din=digitalRead(inPin);
  int adc=analogRead(adcPin);
  float adc_value=lowPassFilter(adc);
  
  Serial.print("Digital In: "+String(din)+" ADC: "+String(adc)+" ADC_mean: "+String(adc_value)+"\n");

  if(count++>SAMPLE_COUNT){
    count=0;

    dbConn->newFrame(station_id, seqNum, timeStamp);  // Create a new dataframe
    dbConn->addKeyValue("pin",din);                   // Add a key-value pair
    dbConn->addKeyValue("adc",adc_value);             // Add another key-value pair
    
    int res=dbConn->sendFrame();                      // Send the dataframe and receive the sequence number of the last stored value
    Serial.println("SendFrame: key=pin value="+String(din)+"key=adc value="+String(adc_value)+" result: "+res+"\n");
    if(res==seqNum) seqNum++;                         // If the last stored dataframe sequence number is the same of our dataframe's then prepare for next one
    else{
      if(res==BlueLabConnection::ERR_INVALID_SESSION_ID){  //otherwise if we were using a outdated session number
        dbConn->login(usr_contact, isEmail, usr_password);  // reconnect for a new session id
        int other_res=dbConn->sendLastFrame();             // resend the last frame .... check the sequence numbers again as above // if(other_res==seqNum) seqNum++;     
      }
      // else ;                                             // it is another error, it should be understood and solved and then resend the last frame
      seqNum=dbConn->getSeqNum(station_id)+1;         // get the sequence number of the next frame to be sent, to make sure we are synched again ... 
    } 
  }
}

