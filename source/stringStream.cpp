#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>

#include "stringStream.h"

stringStream_t stringStreamCtor(size_t size) {
    stringStream_t stream = {NULL, NULL, size};
    stream.buffer = stream.pos = (char *) calloc(size, sizeof(char));
    return stream;
}

int stringStreamDtor(stringStream_t *stream) {
    free(stream->buffer);
    stream->pos = stream->buffer = NULL;
    return 0;
}

wstringStream_t wstringStreamCtor(size_t size) {
    wstringStream_t stream = {NULL, NULL, size};
    stream.buffer = stream.pos = (wchar_t *) calloc(size, sizeof(wchar_t));
    return stream;
}

int wstringStreamDtor(wstringStream_t *stream) {
    free(stream->buffer);
    stream->pos = stream->buffer = NULL;
    return 0;
}

int stringStreamPrintf(stringStream_t *stream, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int bytesWritten = vsnprintf(stream->pos, stream->size + stream->buffer - stream->pos, fmt, args);
    stream->pos += bytesWritten;
    va_end(args);
    return bytesWritten;
}

int wstringStreamPrintf(wstringStream_t *stream, const wchar_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int bytesWritten = vswprintf(stream->pos, stream->size + stream->buffer - stream->pos, fmt, args);
    stream->pos += bytesWritten;
    va_end(args);
    return bytesWritten;
}

void stringStreamClear( stringStream_t *stream) {
    stream->pos = stream->buffer;
    stream->buffer[0] = '\0';
}

void wstringStreamClear(wstringStream_t *stream) {
    stream->pos = stream->buffer;
    stream->buffer[0] = L'\0';
}

wchar_t *stringStreamToWchar(stringStream_t *stream) {
    size_t fullLen = (size_t) (stream->pos - stream->buffer + 1);
    wchar_t *wcs = (wchar_t *) calloc(fullLen, sizeof(wchar_t));
    mbstowcs(wcs, stream->buffer, fullLen);
    return wcs;
}

