#ifndef PARSER_H
#define PARSER_H
struct Table;
struct Labels;
struct DataSet;
struct MissingValues;
/*Provide a number and it will return a 12 bit binary as a string*/
char* toBinary(int number);
/*Receives the address of a Table and missing values linked list's head, index, line string and address' string and   */
/*separates instructions and operands and using the respective functions to check if they are legal within parameters*/
void parser(int index, char *line);
/*Receives the index of the instruction found previously, index of the current operand, the operand string*/
/*and the address of the operandType (which is empty at the moment and the function will change it)      */
char* checkOperand(int instructionIndex, int operandIndex, char *operand, int *operandType);
/*Receives an instruction string and checks if it's permitted and doesn't break any rules*/
int checkInstruction(char *instruction);
/*Receives the missing values and labels linked lists and fills the missing values with the labels*/
void insertMissing();
/*Receives the address of the head of a Table linked list, address,  source code, machine code, ARE char and pushes*/
/*as new node at the end of linked list*/
void pushTable(char *sourceCode, char *machineCode, char ARE);
/*Pushes an entry of .data or .string to the DataSet linked list to then enter it to the main table at the end*/
void pushDataSet(char *line, int index);
/*Receives a DataSet, addresses of a Table and Labels linked list and the address of the amount of .data and .strings*/
/*And pushes a new entry into the DataSet linked list*/
int insertDataSets(int *DC);
#endif