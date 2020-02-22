#include "Uteis.h"

char Uteis::readChar(String msg){
  Serial.print(msg);
  while(Serial.available()<=0) ;
  char c=Serial.read();
  Serial.print(c);
  return(c);
}

void Uteis::getEOL(){
  do{ 
    while(Serial.available()<=0) ;
    int c=Serial.read(); 
     if(c=='\n' || c=='\r') break;
  }while(true);
}

String Uteis::readString(String msg){
  char buf[255];
  int idx=0;
  Serial.print(msg);
  do{
     while(Serial.available()<=0) ;
     buf[idx]=Serial.read(); 
     if(buf[idx]=='\n' || buf[idx]=='\r') break;
     Serial.print(String(buf[idx]));
     idx++;
   }while(idx<sizeof(buf));
   buf[idx]=0;
   return(String(buf));
}

String Uteis::parseString(char **c){
  char *r=*c;
  while(!isspace(**c)) (*c)++;
  **c=0;
  (*c)++;
  return String(r);
}
