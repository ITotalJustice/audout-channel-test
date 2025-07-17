// set this to 1 to enable nxlink logging
// upload your nro using "nxlink -s *.nro" to have printf be sent to your terminal
#define NXLINK_LOG 0

#include <switch.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if NXLINK_LOG
#include <unistd.h>
#endif


// called before main
void userAppInit(void) {
    Result rc;

    if (R_FAILED(rc = appletLockExit())) // block exit until everything is cleaned up
        diagAbortWithResult(rc);
    if (R_FAILED(rc = socketInitializeDefault()))
        diagAbortWithResult(rc);
}

// called after main has exit
void userAppExit(void) {
    socketExit();
    appletUnlockExit(); // unblocks exit to cleanly exit
}

int main(int argc, char** argv) {
    #if NXLINK_LOG
    int fd = nxlinkStdio();
    #endif

    consoleInit(NULL); // consol to display to the screen

    // init controller
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    printf("Press (+) to exit\n\n");

    consoleUpdate(NULL);

    // loop until + button is pressed
    while (appletMainLoop()) {
        padUpdate(&pad);

        const u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        svcSleepThread(1000000);
    }

    #if NXLINK_LOG
    close(fd);
    #endif
    consoleExit(NULL); // exit console display
}
