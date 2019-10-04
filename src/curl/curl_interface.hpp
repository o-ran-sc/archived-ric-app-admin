#include <iostream>
#include <curl/curl.h>
#include <cstdio>

#ifndef CURL_INTERFACE
#define CURL_INTERFACE

class curl_interface {

public:
  curl_interface(std::string);
  bool post_metrics(std::string & data);
  std::string getError(void) const;
  
private:
  std::string _curl_url;
  std::string _error;
};


#endif
