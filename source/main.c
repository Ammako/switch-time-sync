/* Copyright (c) 2024 switchtime Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "ntp.h"

bool setsysInternetTimeSyncIsOn() {
    Result rs = setsysInitialize();
    if (R_FAILED(rs)) {
        printf("setsysInitialize failed, %x\n", rs);
        return false;
    }

    bool internetTimeSyncIsOn;
    rs = setsysIsUserSystemClockAutomaticCorrectionEnabled(&internetTimeSyncIsOn);
    setsysExit();
    if (R_FAILED(rs)) {
        printf("Unable to detect if Internet time sync is enabled, %x\n", rs);
        return false;
    }

    return internetTimeSyncIsOn;
}

Result enableSetsysInternetTimeSync() {
    Result rs = setsysInitialize();
    if (R_FAILED(rs)) {
        printf("setsysInitialize failed, %x\n", rs);
        return rs;
    }

    rs = setsysSetUserSystemClockAutomaticCorrectionEnabled(true);
    setsysExit();
    if (R_FAILED(rs)) {
        printf("Unable to enable Internet time sync: %x\n", rs);
    }

    return rs;
}

TimeServiceType __nx_time_service_type = TimeServiceType_System;
bool setNetworkSystemClock(time_t time) {
    Result rs = timeSetCurrentTime(TimeType_NetworkSystemClock, (uint64_t)time);
    if (R_FAILED(rs)) {
        printf("timeSetCurrentTime failed with %x\n", rs);
        return false;
    }
    printf("Successfully set NetworkSystemClock.\n");
    return true;
}

int main(int argc, char* argv[]) {
    consoleInit(NULL);
    printf("NTP Time Sync\n\n");

    padConfigureInput(8, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeAny(&pad);

    if (!setsysInternetTimeSyncIsOn()) {
        printf("Internet time sync is not enabled. Please enable it in System Settings.");
    } else {
        time_t currentTime;
        Result rs = timeGetCurrentTime(TimeType_UserSystemClock, (u64*)&currentTime);
        if (R_FAILED(rs)) {
            printf("timeGetCurrentTime failed with %x", rs);
        }

        struct tm* p_tm_timeToSet = localtime(&currentTime);
        time_t timeToSet = mktime(p_tm_timeToSet);

        rs = ntpGetTime(&timeToSet);
        if (R_SUCCEEDED(rs)) {
            setNetworkSystemClock(timeToSet);
        }
    }

    printf("\nPress + to exit...");

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) {
            consoleExit(NULL);
            return 0;  // return to hbmenu
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}