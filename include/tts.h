#ifndef TTS_H
#define TTS_H


int ttsInit();
int ttsSpeak(const wchar_t* fmt, ...);
int ttsPrintf(const wchar_t *fmt, ...);
int ttsFlush();

#endif
