#ifndef UTEIS_H
#define UTEIS_H

#include <stdio.h>
#include <ESP.h>

class Uteis{
  public:
    String readString(String msg);
    char readChar(String msg);
    void getEOL();
    String parseString(char **c);
};
#endif /* UTEIS_H */
