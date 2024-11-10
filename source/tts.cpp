#include "stdio.h"
#include "stdlib.h"
#include "wchar.h"
#include "stdarg.h"

#include "tts.h"


int ttsInit() {
    return 0;
}

int ttsSpeak(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    FILE *ttsFile = fopen("tts/request.txt", "a");
    vfwprintf(ttsFile, fmt, args);
    fclose(ttsFile);
    va_end(args);

    return 0;
}

int ttsPrintf(const wchar_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    FILE *ttsFile = fopen("tts/request.txt", "a");
    vfwprintf(ttsFile, fmt, args);
    fclose(ttsFile);
    va_end(args);

    va_list argsStdout;
    va_start(argsStdout, fmt);
    vwprintf(fmt, argsStdout);
    va_end(args);

    return 0;
}

int ttsFlush() {
    FILE *ttsFile = fopen("tts/request.txt", "a");
    fwprintf(ttsFile, L"__SPEAK__\n");
    fclose(ttsFile);
    return 0;
}
