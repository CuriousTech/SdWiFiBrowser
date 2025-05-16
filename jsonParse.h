#ifndef JSONPARSE_H
#define JSONPARSE_H

#include <Arduino.h>

struct jsCbFunc
{
  const char *pszName;
  void (*pFunction)(int32_t nValue, char *pszValue);
};

class JsonParse
{
public:
  JsonParse(void);
  void process(char *p, jsCbFunc jsonList[]);
private:
  char *skipwhite(char *p);
};

#endif // JSONPARSE_H
