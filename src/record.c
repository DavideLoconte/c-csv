//
// Created by Davide on 29/10/2021.
//

#include <string.h>
#include "record.h"


Record *record_alloc(size_t size)
{
        Record *result = malloc(sizeof(Record));

        if (result == NULL) return NULL;

        for (result->bufferSize = 2 ; size > 0; size>>=1, result->bufferSize<<=1);
        result->fields = (char **) malloc(sizeof(char *) * result->bufferSize);

        if (result->fields == NULL) {
                free(result);
                return NULL;
        }

        result->arraySize = 0;
        return result;
}

void record_reset(Record *r)
{
        for (; r->arraySize > 0; r->arraySize--) {
                free(r->fields[r->arraySize-1]);
        }
}

void record_free(Record *r)
{
        record_reset(r);
        free(r->fields);
        free(r);
}

char **record_append_str_by_value(Record *r, const char *field, size_t len)
{
        // It should loop once anyway
        for (; r->bufferSize < r->arraySize + 1; r->bufferSize <<= 1) {
                char **newArray = realloc(r->fields, r->bufferSize);
                if (newArray == NULL) {
                        return NULL;
                }
                r->fields = newArray;
        }

        r->fields[r->arraySize] = malloc(sizeof(char) * (len + 1));
        memcpy(r->fields[r->arraySize], field, (len + 1));
        r->arraySize += 1;

        return r->fields;
}

char **record_append_str_by_reference(Record *r, char *field)
{
        // It should loop once anyway
        for (; r->bufferSize < r->arraySize + 1; r->bufferSize <<= 1) {
                char **newArray = realloc(r->fields, r->bufferSize);
                if (newArray == NULL) {
                        return NULL;
                }
                r->fields = newArray;
        }

        r->fields[r->arraySize] = field;
        r->arraySize += 1;

        return r->fields;
}