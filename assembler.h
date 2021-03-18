#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
struct Labels;
struct  Table;
struct DataSet;
struct MissingValues;
/*Provide a pointer that just went through malloc to check if memory allocation was managed correctly*/
void checkMalloc(void *pointer);
/*Receives the head of of a Labels struct linked list and it will free all the relevant memories*/
void freeAll();
#endif