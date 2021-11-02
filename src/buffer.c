//
// Created by Davide on 29/10/2021.
//

#include <stdlib.h>
#include <string.h>

#include "buffer.h"

Buffer *buffer_alloc(size_t len)
{
        Buffer *result = malloc(sizeof(Buffer));

        if (result == NULL) return NULL;
        for (result->bufferLength = 2 ; len > 0; len>>=1, result->bufferLength<<=1);

        result->buffer = malloc(sizeof(char) * result->bufferLength);

        if (result->buffer == NULL) {
                free(result);
                return NULL;
        }

        result->buffer[0] = 0;
        result->stringLength = 0;
        return result;
}

inline void buffer_free(Buffer *buffer) { free(buffer->buffer); free(buffer); }
inline void buffer_reset(Buffer *buffer) { buffer->buffer[0] = 0; buffer->stringLength = 0; }

char *append_str(Buffer *buf, const char *str, size_t len)
{
        char should_realloc = 0;
        size_t newlen = buf->stringLength + len;
        char *newBuf = NULL;

        for (; buf->bufferLength < newlen + 1; buf->bufferLength <<= 1, should_realloc = 1);

        if (should_realloc) {
                newBuf = realloc(buf->buffer, buf->bufferLength);
                if (newBuf == NULL) {
                        return NULL;
                }
                buf->buffer = newBuf;
        }

        buf->buffer[buf->stringLength] = str[0];
        while (len != 0) {
                buf->buffer[len + buf->stringLength] = str[len];
                len--;
        }

        buf->stringLength = newlen;
        return buf->buffer;
}

char *append_char(Buffer *buf, char c)
{
        // It should loop once anyway
        for (; buf->bufferLength < buf->stringLength + 1; buf->bufferLength <<= 1) {
                char *newBuf = realloc(buf->buffer, buf->bufferLength);
                if (newBuf == NULL) {
                        return NULL;
                }
                buf->buffer = newBuf;
        }

        buf->buffer[buf->stringLength] = c;
        buf->buffer[++buf->stringLength] = 0;

        return buf->buffer;
}

