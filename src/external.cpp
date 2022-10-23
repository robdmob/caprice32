#include "external.h"
#include "cap32.h"

#include <time.h>
#include <curl/curl.h>

byte rtcReg = 0;
byte counter = 0;
byte command[256];

time_t rawtime;
struct tm *timeinfo;

struct memory {
   char *response;
   size_t size;
 };
 
 static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
 {
   size_t realsize = size * nmemb;
   struct memory *mem = (struct memory *)userp;
 
   void *ptr = realloc(mem->response, mem->size + realsize + 1);
   if(ptr == NULL)
     return 0;  /* out of memory! */
 
   mem->response = (char*)ptr;
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;
 
   return realsize;
 }

void doM4() {
    if (counter > 0) {
        struct memory chunk = {0};
        CURL *curl;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, command);
        curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 2048L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        writeROM(6, (byte *)chunk.response, chunk.size);    
    }
    counter = 0;
}

byte int_to_bcd(int num) {

    return (num % 10) + ((num / 10) << 4);

}

byte external_IN(reg_pair port) {

    byte ret_val = 0xff;
    
    // RTC
    if (port.w.l == 0xfd14) {   
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        switch(rtcReg) {
            case 0:
                ret_val = int_to_bcd(timeinfo->tm_sec);
                break;
            case 2:
                ret_val = int_to_bcd(timeinfo->tm_min);
                break;
            case 4:
                ret_val = int_to_bcd(timeinfo->tm_hour);
                break;
            case 7:
                ret_val = int_to_bcd(timeinfo->tm_mday);
                break;
            case 8:
                ret_val = int_to_bcd(timeinfo->tm_mon+1);
                break;
            case 9:
                ret_val = int_to_bcd(timeinfo->tm_year-100);
                break;
        }
    }

    return ret_val;  
}

void external_OUT(reg_pair port, byte val) {

    // Exit
    if (port.w.l == 0xffff) cleanExit((int)val);

    // RTC
    if (port.w.l == 0xfd15) rtcReg = val;

    // M4
    if (port.w.l == 0xfe00) command[counter++] = val;
    if (port.w.l == 0xfc00) doM4();

}