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
        for (result->bufferLength = 1 ; result->bufferLength < len; result->bufferLength *= 2);

        result->buffer = malloc(sizeof(char) * result->bufferLength);

        if (result->buffer == NULL) {
                free(result);
                return NULL;
        }

        result->buffer[0] = 0;
        result->stringLength = 0;
        return result;
}

inline void buffer_free(Buffer *buffer) {
        free(buffer->buffer);
        free(buffer);
}
inline void buffer_reset(Buffer *buffer) { buffer->buffer[0] = 0; buffer->stringLength = 0; }

char *buffer_append(Buffer *buf, char c)
{
        // It should loop once anyway
        for (; buf->bufferLength <= buf->stringLength + 1; buf->bufferLength *= 2) {
                buf->buffer =  realloc(buf->buffer, buf->bufferLength * sizeof *buf->buffer);
                if (buf->buffer == NULL)
                        return NULL;
        }

        buf->buffer[buf->stringLength] = c;
        buf->stringLength += 1;
        buf->buffer[buf->stringLength] = 0;
        return buf->buffer;
}

