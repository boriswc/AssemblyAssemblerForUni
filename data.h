#ifndef DATA_H
#define DATA_H
/*This file contains all the data that is used in all files, like the definitions of the linked lists*/
/*defines and enums that appear multiple times or are used multiple times                            */
#define MAX_NUMBER_OF_WORDS     3
#define MAX_NUMBER_OF_TYPES     3
#define MAX_LABEL_LENGTH        31
#define MAX_LINE_LENGTH         81
#define MAX_CHARS_RECEIVED      1024
#define NUMBER_OF_OPCODES       8
#define NUMBER_OF_BITS          13
#define NUMBER_OF_INSTRUCTIONS  16
#define HEX_CHARACTERS          3
#define FILE_EXTENSION_LENGTH   3
#define TYPE_ZERO_OPERAND       0
#define TYPE_ONE_OPERAND        1
#define TYPE_TWO_OPERAND        2
#define TYPE_THREE_OPERAND      3
#define NONE                   -1
#define IMMEDIATE               0
#define DIRECT                  1
#define RELATIVE                2
#define REGISTER                3

/*Enum used for parsers and recognizing states when checking text lines*/
enum { OUT, IN_WORD, SPACE, IN_STRING, FINISHED };
/*This struct will be used to save the information on all labels on the first run*/
struct Labels {
    int address;
    char *symbol;
    char *attribute;
    struct Labels *next;
};
/*This struct will be used as the table that saves all the information and
 * addresses of the .as file*/
struct Table {
    char ARE;
    int address;
    char *sourceCode;
    char *machineCode;
    struct Table *next;
};
/*Will be used for faster access to missing values in the table after the first run instead of going through*/
/*the whole table once again we will just add the missing values only.                                      */
struct MissingValues {
    struct Table *missingEntry;
    struct MissingValues *next;
};
/*This struct contains the data type lines (.string and .data) so we can add them at the end of the table*/
struct DataSet {
    char *sourceCode;
    int lineNumber;
    struct DataSet *next;
};
/*This struct contains the different addresing method types for sources and dest operands for all the opcodes*/
struct AddressingMethods {
    int opcode;
    int sourceType[MAX_NUMBER_OF_TYPES];
    int destType[MAX_NUMBER_OF_TYPES];
};
/*This struct contains all the assembly commands/instructions*/
struct Instructions {
    int opcode;
    int funct;
    const char *name;
    int numberOfOperands; /*0, 1 or 2 / if 2 dest and source operand, if 1 only dest, if 0 well.. none*/
    struct AddressingMethods *addMethod;    /*This way we have easier access to the operand addressing types for each inst*/
};
#endif