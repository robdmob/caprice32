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
   byte *response;
   size_t size;
 };

 struct memory responseBuffer = {nullptr, 0};
 
 static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
 {
   size_t realsize = size * nmemb;
   struct memory *mem = static_cast<memory*>(userp);
 
   void *ptr = realloc(mem->response, mem->size + realsize + 1);
   if(ptr == NULL)
     return 0;  /* out of memory! */
 
   mem->response = static_cast<byte*>(ptr);
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;
 
   return realsize;
}

void doM4() {
    if ((counter >= 3) & (command[0] == (counter-1)) & (command[2] == 0x43)) {
        command[counter] = 0;
        switch (command[1]) {
            case 0x28: // C_HTTPGETMEM
                CURL *curl;
                CURLcode res;
                curl = curl_easy_init();
                curl_easy_setopt(curl, CURLOPT_URL, &command[5]);
                curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, (command[4] << 8) | command[3]);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                byte response[2];
                response[0] = responseBuffer.size;
                response[1] = static_cast<word>(responseBuffer.size) >> 8;
                writeROM(response, 2);
                break;
            case 0x29: // C_COPYBUF
                writeROM(responseBuffer.response, responseBuffer.size);
                break;
            case 0x24: // C_TIME
                byte timeData[20];
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(reinterpret_cast<char*>(timeData), 20, "%H:%M:%S %Y-%m-%d", timeinfo);
                writeROM(timeData, 20);
                break;
        }
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
    if (port.w.l == 0xffff) {
        if (val == 0xff) showGui();
        else cleanExit(val);
    }

    // RTC
    if (port.w.l == 0xfd15) rtcReg = val;

    // M4
    if (port.w.l == 0xfe00) command[counter++] = val;
    if (port.w.l == 0xfc00) doM4();

}