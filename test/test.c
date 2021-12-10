//
// Created by Davide on 30/10/2021.
//

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "csv.h"

size_t fields = 0;
size_t records = 0;



void headerCallback(void *ctx, Record *header)
{
        fields += header->arraySize;
        records += 1;
        printf("Record [%lu]:\n", header->arraySize);
        for (int i = 0; i < header->arraySize; i++) {
                printf("\t%d: %s\n", i, header->fields[i]);
        }
}

void recordCallback(void *ctx, Record *header, Record *record) { headerCallback(ctx, record); }

int main()
{
        size_t i;
        double sum = 0.0;
        int iterations = 1;
        for (i = 0; i < iterations; i++) {
                struct timeval start, end;
                gettimeofday(&start, NULL);

                CsvReader *reader = csv_reader_alloc(&headerCallback, &recordCallback, NULL);
                FILE *csv = fopen("../test/test2.csv", "r");

                if (!csv) fprintf(stderr, "Cannot open file\n");

                csv_reader_parse(reader, csv);

                fclose(csv);
                csv_reader_free(reader);
                gettimeofday(&end, NULL);
                sum += (end.tv_sec * 1000.0) + (end.tv_usec / 1000.0) - (start.tv_sec * 1000.0) - (start.tv_usec / 1000.0);
        }

        printf("Read %lu records %lu fields in %.3lf ms\n", records/iterations, fields/iterations, sum/iterations);
        return 0;
}

int main2()
{
        CsvReader *reader = csv_reader_alloc(&headerCallback, &recordCallback, NULL);
        FILE *csv = fopen("../test/test.csv", "r");

        if (!csv) fprintf(stderr, "Cannot open file\n");

        csv_reader_parse(reader, csv);

        fclose(csv);
        csv_reader_free(reader);
}