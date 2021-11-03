//
// Created by Davide on 29/10/2021.
//

#include <string.h>
#include <utils.h>
#include "record.h"

void record_resize(Record *record, size_t size);
int record_realloc(Record *record);

Record *record_alloc(size_t size)
{
        Record *result = malloc(sizeof(Record));

        if (result == NULL)
                return NULL;

        result->bufferSize = 1;
        record_resize(result, size);
        result->fields = malloc(sizeof(char *) * (result->bufferSize));

        if (result->fields == NULL) {
                free(result);
                return NULL;
        }

        result->arraySize = 0;
        return result;
}

inline void record_reset(Record *r) 
{
        size_t i;
        for (i = 0; i < r->arraySize; i++) {
                free(r->fields[i]);
        }
        r->arraySize = 0;
}

void record_resize(Record *record, size_t size)
{
        while (size > record->bufferSize) {
                record->bufferSize *= 2;
        }
}

void record_free(Record *r)
{
        record_reset(r);
        free(r->fields);
        free(r);
}

char **record_append_str(Record *r, const char *field, size_t len)
{
        if ((r->arraySize + 1) >= r->bufferSize) {
                r->bufferSize *= 2;
                record_realloc(r);
        }

        r->fields[r->arraySize] = string_duplicate(field, len);
        r->arraySize += 1;
        return r->fields;
}

int record_realloc(Record *record)
{
        char **newRecords = realloc(record->fields, record->bufferSize * sizeof *record->fields);
        if (newRecords == NULL) {
                return -1;
        }
        record->fields = newRecords;
        return 0;
}