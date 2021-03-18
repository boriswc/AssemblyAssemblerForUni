#ifndef LABEL_H
#define LABEL_H
/*This file includes all related to labels*/

/************************************************************************************/
/* Receives the address of the head of a Labels linked list, symbol string, address */
/* of the int address ,atribute and pushes as new node at the end of linked list    */
/************************************************************************************/
void pushLabel(char *symbol, char *attribute);
/*************************************************************************************/
/* Provide and a string and index for when the label starts and it will return the*/
/* label after checking ifit's legal or return NULL if it encounters any issues   */
/**********************************************************************************/
char* checkLabel(char *line, int index);
#endif