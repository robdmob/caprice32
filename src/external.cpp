#include "external.h"

#include <time.h>

byte rtcReg = 0;
byte counter = 0;
char url[256];

time_t rawtime;
struct tm *timeinfo;

void doM4() {
    if (counter > 0) {

        system(url);

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

    // RTC
    if (port.w.l == 0xfd15) rtcReg = val;

    // M4
    if (port.w.l == 0xfe00) url[counter++] = val;
    if (port.w.l == 0xfc00) doM4();

}