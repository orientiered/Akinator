#ifndef STRING_STREAM_H
#define STRING_STREAM_H

typedef struct {
    char *buffer;
    char *pos;
    size_t size;
} stringStream_t;

typedef struct {
    wchar_t *buffer;
    wchar_t *pos;
    size_t size;
} wstringStream_t;


stringStream_t   stringStreamCtor(size_t size);
wstringStream_t wstringStreamCtor(size_t size);

int  stringStreamDtor( stringStream_t *stream);
int wstringStreamDtor(wstringStream_t *stream);

int  stringStreamPrintf( stringStream_t *stream, const    char *fmt, ...);
int wstringStreamPrintf(wstringStream_t *stream, const wchar_t *fmt, ...);

void  stringStreamClear( stringStream_t *stream);
void wstringStreamClear(wstringStream_t *stream);

wchar_t *stringStreamToWchar(stringStream_t *stream);


#endif
