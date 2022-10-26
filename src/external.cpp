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
   struct memory *mem = static_cast<struct memory*>(userp);
 
   byte *ptr = static_cast<byte*>(realloc(mem->response, mem->size + realsize + 1));
   if(ptr == NULL)
     return 0;  /* out of memory! */
 
   mem->response = (ptr);
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;
 
   return realsize;
}

void doM4() {
    if ((counter >= 3) & (command[0] == (counter-1)) & (command[2] == 0x43)) {
        command[counter] = 0;
        writeROM(command, 0, 3);
        switch (command[1]) {
            case 0x28: // C_HTTPGETMEM
                CURL *curl;
                if (responseBuffer.size > 0) free(responseBuffer.response);
                responseBuffer = {nullptr, 0};
                curl = curl_easy_init();
                curl_easy_setopt(curl, CURLOPT_URL, &command[5]);
                curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, (command[4] << 8) | command[3]);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "m4");
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
                curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                byte response[2];
                response[0] = responseBuffer.size & 0xff;
                response[1] = (responseBuffer.size >> 8) & 0xff;
                writeROM(response, 3, 2);
                break;
            case 0x29: // C_COPYBUF
                word offset;
                offset = (command[4] << 8) | command[3];
                writeROM(responseBuffer.response+offset, 3, responseBuffer.size-offset);
                break;
            case 0x24: // C_TIME
                byte timeData[20];
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(reinterpret_cast<char*>(timeData), 20, "%H:%M:%S %Y-%m-%d", timeinfo);
                writeROM(timeData, 3, 20);
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