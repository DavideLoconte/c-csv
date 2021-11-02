//
// Created by Davide on 29/10/2021.
//

#include <ctype.h>
#include "../include/csv.h"

#define BUFFER_SIZE (1024 * 32)
#define FIELD_BUFFER_SIZE 4096
#define RECORD_SIZE 32

#define HEADER_FOUND 0X01
#define ESCAPING 0X02
#define ONE_DQUOTE_FOUND 0X04
#define TWO_DQUOTES_FOUND 0X08
#define END_OF_FILE 0x10

#define DQUOTE 34
#define NEW_LINE 10
#define CARRIAGE_RETURN 13
#define COMMA 44

#define is_normal_character(c) (0x20 == (c) || (c) == 0x21  || \
                               (0x23 <= (c) && (c) <= 0x2b) || \
                               (0x2d <= (c) && (c) <= 0x7e))

#define is_line_ending(c) ((c) == NEW_LINE || (c) == EOF)
#define is_separator(c) ((c) == COMMA)
#define is_dquote(c) ((c) == DQUOTE)
#define is_ignorable(c) ((c) == CARRIAGE_RETURN)

/**
 * This structure encapsulate some data
 * structures used throughout the parsing process
 *
 * @param buffer a dynamic string which temporarily stores the field the parser
 *               is currently reading

 * @param currentRecord A dynamic array of strings.
 *                      Once a CSV record has been completely read by the
 *                      parser, this array contains the fields of that record.
 *                      This pointer is then passed as an argument to the
 *                      recordCallback function to process its data.
 *
 * @param header A dynamic array of strings which stores the fields
 *               of the CSV header
 *
 * @param flags The first five bits holds some information on the parser state.
 *               - HEADER_FOUND: The first record (header) has been processed.
 *               - ESCAPING: The character that the parser is reading is
 *                           enclosed in double quotes and must be escaped
 *                           accordingly to the CSV syntax;
 *               - ONE_DQUOTE_FOUND: The parser encountered one double quote
 *                                   in an escaped sequence;
 *               - TWO_DQUOTES_FOUND: Two consecutive double quotes are found
 *                                    in an escaped sequence. When this happens,
 *                                    the final parsed field will contain a
 *                                    single double quote character.
 *                - END_OF_FILE: 1 if EOF is encountered
 *              Remaining bits are currently unused.
 *
 * @param currentCsv FILE * pointer of the CSV file to parse
 * @param nextchr The current character
 * @param tempBuffer Contains the content of the file
 * @param bufPos store the current position in the buffer
 */
typedef struct {
        Buffer *buffer;
        Record *currentRecord;
        Record *header;
        int flags;

        FILE *currentCsv;
        int nextchr;
        char *tempBuffer;
        size_t bufPos;
} ParsingContext;

// Private prototypes

/**
 * Process the next character
 * @param reader The CsvReader
 * @param pc the current parsing context
 */
void parse_data(CsvReader *reader, ParsingContext *pc);

/**
 * Process the next character assuming it is escaped
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void parse_escaped_data(CsvReader *reader, ParsingContext *pc);

/**
 * Execute the callback function for the currently stored record,
 * then resets the record buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void emit_record(CsvReader *reader, ParsingContext *pc);

/**
 * Takes the buffer stored in the parsing context and stores it
 * as a field in the record data structure, then resets the buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void emit_field(ParsingContext *pc);

/**
 * Start escaping the sequence after the current character
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void start_escaped_sequence(ParsingContext *pc);

/**
 * End the escaped sequence on this character and move to the beginning of the
 * next field
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void end_escaped_sequence(CsvReader *reader, ParsingContext *pc);

/**
 * Add a double quote (") to the current buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void escape_quotes(ParsingContext *pc);

/**
 * Move to the next character
 * @param pc
 * @return
 */
int get_next_character(ParsingContext *pc);
/**
 * Move to the next line
 * @param pc parsing context
 */
void get_next_line(ParsingContext *pc);


// Public Implementation

CsvReader *csv_reader_alloc(void (*headerCallback)(void *, Record *),
                            void (*recordCallback)(void *, Record *, Record *),
                            void *context)
{
        CsvReader *result = malloc(sizeof(CsvReader));

        if (result == NULL) return NULL;

        result->context = context;
        result->header = headerCallback;
        result->record = recordCallback;

        return result;
}

inline void csv_reader_free(CsvReader *reader) {free(reader);}

void csv_reader_parse(CsvReader *reader, FILE *csvFile)
{
        ParsingContext pc;

        pc.buffer = buffer_alloc(FIELD_BUFFER_SIZE);
        pc.currentRecord = record_alloc(RECORD_SIZE);
        pc.header = NULL;
        pc.flags = 0x00;
        pc.currentCsv = csvFile;
        pc.nextchr = 0;
        pc.tempBuffer = calloc(sizeof (char), (BUFFER_SIZE));
        pc.bufPos = 0;

        while (!(pc.flags & END_OF_FILE)) {
                pc.nextchr = get_next_character(&pc);

                if (!(pc.flags & (ESCAPING | END_OF_FILE)))
                        parse_data(reader, &pc);
                else if (!(pc.flags & END_OF_FILE))
                        parse_escaped_data(reader, &pc);
                else
                        emit_record(reader, &pc);
        }

        record_free(pc.currentRecord);
        record_free(pc.header);
        buffer_free(pc.buffer);
        free(pc.tempBuffer);
}

// Private

inline int get_next_character(ParsingContext *pc)
{
        if (!(pc->bufPos >= BUFFER_SIZE || pc->tempBuffer[pc->bufPos+1] == 0)) {
                return (int) pc->tempBuffer[++pc->bufPos];
        } else {
                get_next_line(pc);
                return (int) pc->tempBuffer[0];
        }
}

inline void get_next_line(ParsingContext *pc)
{
        pc->bufPos = 0;
        if (!fgets(pc->tempBuffer, BUFFER_SIZE, pc->currentCsv))
                pc->flags |= END_OF_FILE;
}


void emit_record(CsvReader *reader, ParsingContext *pc)
{
        record_append_str_by_value(pc->currentRecord,
                                   pc->buffer->buffer,
                                   pc->buffer->stringLength);
        buffer_reset(pc->buffer);

        if (pc->flags & HEADER_FOUND) {

                if (reader->record)
                        reader->record(reader->context,
                                       pc->header,
                                       pc->currentRecord);

                record_reset(pc->currentRecord);

        } else {
                pc->flags |= HEADER_FOUND;
                pc->header = pc->currentRecord;
                pc->currentRecord = record_alloc(pc->header->arraySize);

                if (reader->header)
                        reader->header(reader->context, pc->header);
        }

        pc->flags &= ~ONE_DQUOTE_FOUND;
}

void emit_field(ParsingContext *pc)
{
        record_append_str_by_value(pc->currentRecord,
                                   pc->buffer->buffer,
                                   pc->buffer->stringLength);
        buffer_reset(pc->buffer);
}

void start_escaped_sequence(ParsingContext *pc)
{
        pc->flags |= ESCAPING;
        pc->flags &= ~(ONE_DQUOTE_FOUND | TWO_DQUOTES_FOUND);
        buffer_reset(pc->buffer);
}

void parse_data(CsvReader *reader, ParsingContext *pc)
{
        if (is_normal_character(pc->nextchr))
                append_char(pc->buffer, (char) pc->nextchr);

        else if (is_separator(pc->nextchr))
                emit_field(pc);

        else if (is_line_ending(pc->nextchr))
                emit_record(reader, pc);

        else if (is_dquote(pc->nextchr))
                start_escaped_sequence(pc);
}

void parse_escaped_data(CsvReader *reader, ParsingContext *pc)
{
        if (!is_dquote(pc->nextchr)) {
                if (!(pc->flags & ONE_DQUOTE_FOUND))
                        append_char(pc->buffer, (char) pc->nextchr);
                else
                        end_escaped_sequence(reader, pc);
        }
        else {
                if (!(pc->flags & ONE_DQUOTE_FOUND))
                        pc->flags |= ONE_DQUOTE_FOUND;
                else
                        escape_quotes(pc);
        }
}

void end_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        while (!(is_separator(pc->nextchr) || is_line_ending(pc->nextchr)))
                pc->nextchr = get_next_character(pc);

        pc->flags &= ~(ONE_DQUOTE_FOUND | ESCAPING);
        emit_record(reader, pc);
}

inline void escape_quotes(ParsingContext *pc)
{
        pc->flags &= ~ONE_DQUOTE_FOUND;
        append_char(pc->buffer, '"');
}


