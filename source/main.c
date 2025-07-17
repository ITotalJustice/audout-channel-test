// set this to 1 to enable nxlink logging
// upload your nro using "nxlink -s *.nro" to have printf be sent to your terminal
#define NXLINK_LOG 0

#include <switch.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#if NXLINK_LOG
#include <unistd.h>
#endif

// prints to the screen and displays it immediately
__attribute__((format (printf, 1, 2)))
static void consolePrint(const char* f, ...) {
    va_list argv;
    va_start(argv, f);
    vprintf(f, argv);
    va_end(argv);
    consoleUpdate(NULL);
}

static void Test(bool upper) {
    if (upper) {
        consolePrint("Performing test using channel count in upper nibble\n");
    } else {
        consolePrint("Performing test using channel count in lower nibble\n");
    }

    const AudioDeviceName DeviceNameIn = {0};
    AudioDeviceName DeviceNameOut = {0};

    u32 SampleRateOut;
    u32 ChannelCountOut;
    PcmFormat Format;
    AudioOutState State;

    for (int i = 0; i < 7; i++) {
        serviceClose(audoutGetServiceSession_AudioOut());

        const u32 rate = 48000; // only rate supported by the default audout.
        const u32 count = upper ? (1 << 16) : i;

        if (R_FAILED(audoutOpenAudioOut(DeviceNameIn.name, DeviceNameOut.name, rate, count, &SampleRateOut, &ChannelCountOut, &Format, &State))) {
            printf("failed channel: %d rate: %d\n", i, rate);
        } else {
            printf("[%d] SampleRate: %u wanted: %d\n", i, SampleRateOut, rate);
            printf("[%d] ChannelCount: %u\n", i, ChannelCountOut);
            printf("[%d] PcmFormat: %u\n", i, Format);
            printf("[%d] DeviceState: %u\n\n", i, State);
        }
    }

    consolePrint("done\n\n");
}

// called before main
void userAppInit(void) {
    Result rc;

    if (R_FAILED(rc = appletLockExit())) // block exit until everything is cleaned up
        diagAbortWithResult(rc);
    if (R_FAILED(rc = socketInitializeDefault()))
        diagAbortWithResult(rc);
    if (R_FAILED(rc = audoutInitialize()))
        diagAbortWithResult(rc);
}

// called after main has exit
void userAppExit(void) {
    audoutExit();
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

    consolePrint("Press (A) to test upper\n");
    consolePrint("Press (B) to test lower\n");
    consolePrint("Press (+) to exit\n\n");

    // loop until + button is pressed
    while (appletMainLoop()) {
        padUpdate(&pad);

        const u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu
        else if (kDown & HidNpadButton_A)
            Test(true);
        else if (kDown & HidNpadButton_B)
            Test(false);

        svcSleepThread(1000000);
    }

    #if NXLINK_LOG
    close(fd);
    #endif
    consoleExit(NULL); // exit console display
}
