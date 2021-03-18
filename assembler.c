/********************************DESCRIPTION************************************/
/* This program is an assembler for the assembly language basically transalting*/
/* assembly to machine code to ready make it ready for other two steps.        */
/* This is done in a few steps:                                                */
/* 1) The program gets the file names from the arguments passed when calling   */
/*    the program, it then tries to open the first file and checks it          */
/* 2) The program then eliminates all types of unnecesary parts like extra     */
/*    spaces and comments.                                                     */
/* 3) Then the progrma divides and saves the data from the lines while checking*/
/*    for syntax errors in the assembly                                        */
/* 4) If no errors are found it prints the output into .ob, .ent and .ext files*/
/*    and continues to the next file if there is such repeating the process    */
/*******************************************************************************/
#include "assembler.h"
#include "data.h"
#include "label.h"
#include "parser.h"
#include "file_management.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*Global linked list with all the addressing methods for source and dest of each opcode excluding those that lack operands (opcodes < 14)*/
struct AddressingMethods addressingMethods[NUMBER_OF_OPCODES] = {   /*Note, -1/NONE are just to be ignored*/
    {0,{IMMEDIATE,DIRECT,REGISTER},{DIRECT,REGISTER,NONE}}, 
    {1,{IMMEDIATE,DIRECT,REGISTER},{IMMEDIATE,DIRECT,REGISTER}}, 
    {2,{IMMEDIATE,DIRECT,REGISTER},{DIRECT,REGISTER,NONE}}, 
    {4,{DIRECT,NONE,NONE},{DIRECT,REGISTER,NONE}},
    {5,{NONE},{DIRECT,REGISTER,NONE}}, 
    {9,{NONE},{DIRECT,RELATIVE,NONE}},
    {12,{NONE},{DIRECT,REGISTER,NONE}}, 
    {13,{NONE},{IMMEDIATE,DIRECT,REGISTER}} 
};
/*Global linked list with all instructions, with their opcodes, and all necesarry information*/
struct Instructions instructions[NUMBER_OF_INSTRUCTIONS] = {
    {0, 0, "mov", 2, &addressingMethods[0]},  {1, 0, "cmp", 2,  &addressingMethods[1]},
    {2, 10, "add", 2, &addressingMethods[2]}, {2, 11, "sub", 2, &addressingMethods[2]},
    {4, 0, "lea", 2, &addressingMethods[3]},  {5, 10, "clr", 1, &addressingMethods[4]}, 
    {5, 11, "not", 1, &addressingMethods[4]}, {5, 12, "inc", 1, &addressingMethods[4]},
    {5, 13, "dec", 1, &addressingMethods[4]}, {9, 10, "jmp", 1, &addressingMethods[5]},
    {9, 11, "bne", 1, &addressingMethods[5]}, {9, 12, "jsr", 1, &addressingMethods[5]},
    {12, 0, "red", 1, &addressingMethods[6]}, {13, 0, "prn", 1, &addressingMethods[7]},
    {14, 0, "rts", 0, NULL}, {15, 0, "stop", 0, NULL}
};
struct DataSet *dataSet;            /*Global variable for the data type lines*/
struct Table *tableList;            /*Global linked list for the main table*/
struct Labels *labelsList;          /*Global linked list for the main table*/ 
struct MissingValues *missingValues;/*Global linked list for the missing values*/
/*While the following global variables could have been prevented by just passing them to functions*/
/*Since they are used for most functions making them globals makes sense                          */
int g_lineNumber = 0;               /*Global variable to keep track of the line number*/
int g_address = 100;                /*Global variable to keep track of the address*/
int g_errors = 0;                   /*Global variable to keep track of the errors*/
int g_externals = 0;                /*Global variable to know if there are externals, if 0 we won't create .ext file*/
int g_entries = 0;                  /*Global variable to know if there are entries, if 0 we won't create .ent file*/

/*Main function which will run all the other functions and takes care of all the managing*/
int main(int argc, char *argv[]){
    FILE *inputFile;
    int index, DC; /*IC = address - 100*/
    char line[MAX_CHARS_RECEIVED];
    char *labelIndex;
    char *fileName;
    
    if(argc < 2){
        printf("Wrong usage, expected: %s file_name.as\nExiting Program....",argv[0]);
        exit(-1);
    }
    for(argv++; *argv; argv++){
        fileName = (char*)malloc(strlen(*argv)+FILE_EXTENSION_LENGTH+1);
        strcpy(fileName, *argv);
        inputFile = fopen(strcat(fileName, ".as"),"r");
        if(checkFileOpen(inputFile)){
            dataSet = NULL;     /*Resetting variables for each assembly file*/
            tableList = NULL;
            labelsList = NULL;
            missingValues = NULL;
            g_errors = g_lineNumber = DC = g_entries = g_externals = 0;
            g_address = 100;
            while(fgets(line, MAX_CHARS_RECEIVED, inputFile)){
                g_lineNumber++;             
                index = 0;
                if(strlen(line) <= MAX_LINE_LENGTH){    /*Checks if the line exceeds the max line length*/
                    if((strcpy(line ,removeUnwanted(line)))){ /*Removes unwanted and extra stuff like extra spaces and comments */
                                              /*comments and skips the line if it's a comment line or empty line*/
                        if((labelIndex = strchr(line, ':')) && !strstr(line, "extern") && !strstr(line, "entry")){  /* : Label*/
                            index = labelIndex - line;
                            if(!(strstr(line, ".data")) && !(strstr(line, ".string")))
                                pushLabel(checkLabel(line, index), "code");                                        
                        }              
                        else
                            if(((labelIndex = strstr(line, ".extern")))){ /*Extern line*/
                                index = labelIndex - line + 7;                              
                                pushLabel(checkLabel(line, index), "external");
                            }
                        else
                            if((labelIndex = strstr(line, ".entry"))){ /*Entry line*/
                                g_entries++;
                                index = labelIndex - line + 6;                                                                  
                                pushLabel(checkLabel(line, index), "entry");
                            }                                                     
                        if(!strstr(line, ".extern") && !strstr(line, ".entry")){    /*Instruction line*/
                            if(!(strstr(line, ".data")) && !(strstr(line, ".string")))          
                                parser(index, line);
                            else                                      /*Saving data to push at the end of the table If there is a*/
                                pushDataSet(line, index);   /*label it will save it in the function to the labels table*/      
                        }  
                    }  
                }
                else{
                    printf("Error in line %d:\nLength exceeding limit of 80\n", g_lineNumber);
                    g_errors++;
                }
            }
            insertDataSets(&DC);   /*Inserts the .data and .string lines saved to the end of the table list*/
            if(!g_errors){      /*If there were no errors in the file it fills the missing values and outputs to file*/             
                insertMissing(missingValues, labelsList);
                outputToFile(*argv, DC);
            }
            else
                printf("There are errors in the assembly files.\n\nNote: DATA errors appear last\n");
            freeAll();               
            fclose(inputFile);
            free(fileName);
        }
    }
    return 0;
}
/*Frees memory of all non empty linked lists*/
void freeAll(){
    struct Labels *last_label_head;
    struct Table *last_table_head;
    struct DataSet *last_data_head;
    struct MissingValues *last_missing_head;

    while (labelsList != NULL){
        last_label_head = labelsList;
        labelsList = labelsList->next;
        if(last_label_head->symbol)
            free(last_label_head->symbol);
        free(last_label_head);
    }
    while (tableList != NULL){
        last_table_head = tableList;
        tableList = tableList->next;
        if(strcmp(last_table_head->sourceCode, " ") && last_table_head->sourceCode)
            free(last_table_head->sourceCode);
        if(last_table_head->machineCode)
            free(last_table_head->machineCode);
        free(last_table_head);
    }
    while (dataSet != NULL){
        last_data_head = dataSet;
        dataSet = dataSet->next;
        free(last_data_head);
    }
    while (missingValues != NULL){
        last_missing_head = missingValues;
        missingValues = missingValues->next;
        free(last_missing_head);
    }
}
/*Checks if memory allocation from malloc occurred correctly*/
void checkMalloc(void *pointer){
    if(pointer == NULL){
        printf("Error assigning memory\nExiting Program\n");
        exit(-1);
    }
}