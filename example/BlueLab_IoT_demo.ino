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
*******************/
String usr_email="a@a.a";
String usr_password="a";

// Stations should have unique ids (or ensure they have unique key strings)
String moduleName = "Humi";
int station_id = 1; 

/**************************
    BlueLab IoT links
DO NOT CHANGE
*******************/
String login_host="www.tektonia.com.pt";
String login_url="/tekton/entraaqui/login.php";
String data_host="www.bluelab.pt";
String data_url="/iot/call.php";  

// maximum number of retries in case of error
#define MAX_RETRIES 5
#define DELAY_MILISECONDS 3000
#define SAMPLE_COUNT 30

BlueLabConnection *dbConn;
  
long long timeStamp=0;
int seqNum;
int count=0;
  
int adcPin=A0;
int inPin=D0;


void doReset(String err){
  Serial.print("\nERROR - Resetting module: "+err+"\n");
  EspClass esp;
  esp.restart();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nBlueLab IoT demo - VVS 2018\n\n"); 

  Serial.println("\nStation id: "+String(station_id)+"\n");

  dbConn = new BlueLabConnection(login_host, login_url, data_host, data_url);

  Serial.print("Acquiring Internet access, through: "+ssid+" ...");
  WiFi.begin(ssid.c_str(), ssid_password.c_str());
  while (WiFi.status() != WL_CONNECTED) {  
    if(count++>=MAX_RETRIES) doReset("No Internet Connection");
    WiFi.begin(ssid.c_str(), ssid_password.c_str());
    delay(3000);
  }
  Serial.println(" WiFi connected :):):)");

  Serial.print("\nLogging in to BlueLab IoT system with "+usr_email+" ...");
  count=0;
  while(!dbConn->login(usr_email, usr_password))  {
    Serial.println("Could not Login!");
    if(count++>=MAX_RETRIES) doReset("No Access to the BlueLab Iot System. user:"+usr_email+": password:"+usr_password+":");
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
    dbConn->login(usr_email, usr_password);
    seqNum=dbConn->getSeqNum(station_id);  //sequencia actual
    seqNum++;
  }
  /**/

  pinMode(inPin, INPUT);
  count=SAMPLE_COUNT;
}

      
void loop() {
  delay(DELAY_MILISECONDS);
  timeStamp+=DELAY_MILISECONDS;  // aproximatly
  int din=digitalRead(inPin);
  int adc=analogRead(adcPin);

  Serial.print("Digital In: "+String(din)+" ADC: "+String(adc)+"\n");

  if(count++>SAMPLE_COUNT){
    count=0;

    dbConn->newFrame(station_id, seqNum, timeStamp);
    dbConn->addKeyValue("pin",din);
    dbConn->addKeyValue("adc",adc);
    
    int res=dbConn->sendFrame();

    Serial.println("SendFrame: key=pin value="+String(din)+"key=adc value="+String(adc)+" result: "+res+"\n");
    if(res==seqNum) seqNum++;
    else{
      if(res==BlueLabConnection::ERR_INVALID_SESSION_ID){
        dbConn->login(usr_email, usr_password);
        dbConn->sendLastFrame();
      }
      seqNum=dbConn->getSeqNum(station_id)+1;
    } 
  }
}

