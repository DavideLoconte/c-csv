//
// Created by Davide on 30/10/2021.
//

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "csv.h"

void recordCallback(void *ctx, Record *header, Record *record)
{
//        size_t i;
//        printf("Record: %lu\n", record->arraySize);
//        for (i = 0; i < record->arraySize; i++) printf("\t%s\n", record->fields[i]);
}

void headerCallback(void *ctx, Record *header)
{
//        size_t i;
//        printf("Header: %lu\n", header->arraySize);
//        for (i = 0; i < header->arraySize; i++) printf("\t%s\n", header->fields[i]);
}

int main()
{
        CsvReader *reader = csv_reader_alloc(NULL, NULL, NULL);
        FILE *csv = fopen("../test/test.csv", "r");

        clock_t startA = clock();
        csv_reader_parse(reader, csv);
        printf("Elapsed %.3lf ms\n", (float) (clock() - startA) / 1000.0);

        fclose(csv);
        csv_reader_free(reader);
}