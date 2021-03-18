#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H
#include <stdio.h>
struct Labels;
struct  Table;
/*Provide a file after fopen and it checks if the file has been opened correctly*/
int checkFileOpen(FILE *input);
/*Provide a string and it removes extra spaces for easier parsing, also checks if line is empty or not*/
char* removeUnwanted(char *str);
/*Receives a file name, table and label linked list and the DC parameter (amount of .data and .string lines in file)*/
void outputToFile(char *fileName, int DC);
#endif