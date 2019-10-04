#include "curl_interface.hpp"
curl_interface::curl_interface(std::string curl_collector_url):_curl_url(curl_collector_url){};
							    

bool curl_interface::post_metrics(std::string & data){

  CURL *curl = curl_easy_init();
  CURLcode result;
  bool res = false;
 
  if(curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type:application/json");
    
    /* point to ves collector */
    curl_easy_setopt(curl, CURLOPT_URL, _curl_url.c_str());

    /* Set header */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    /* size of the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    
    /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
 
    result = curl_easy_perform(curl);
    if (result != CURLE_OK){
      _error.assign(curl_easy_strerror(result));
    }
    else{
        res = true;
    }

    curl_easy_cleanup(curl);
    curl_free(headers);
  }
  else{
    _error.assign("Could not instantiate curl handle !");
  }

  return res;
}


std::string curl_interface::getError(void) const{
  return _error;
}
