#include "data.h"
#include "label.h"
#include "assembler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern int g_errors;
extern int g_address;
extern int g_lineNumber;
extern struct Labels *labelsList;
/*******************************************************************/
/* Receives the address of the label's head, a symbol string and  */
/* attribute stringand pushes a new label to the label linked list*/
/******************************************************************/
void pushLabel(char *symbol, char *attribute){
    struct Labels *last_head = labelsList;
    struct Labels *new_label;

   if(symbol != NULL){
        new_label = (struct Labels*)malloc(sizeof(struct Labels));
        checkMalloc(new_label);
        if (strcmp(attribute, "external") && strcmp(attribute, "entry"))
            new_label->address = g_address;
        else
            new_label->address = 0;
        new_label->symbol = symbol;
        new_label->attribute = attribute;
        new_label->next = NULL;
        
        if(labelsList == NULL){
            labelsList = new_label;
        }
        else{
            while(last_head->next){   /*From the head instead of next to check all labels in table for duplicate*/
                if(!strcmp(symbol, last_head->symbol)){
                    if(!strcmp(attribute, "entry")){
                        if(!strcmp(last_head->attribute, "external")){
                            printf("Error in line %d:\nLabel cannot be both entry and external\n", g_lineNumber);
                            g_errors++;
                            break;
                        }
                        else{
                            if(strcmp(last_head->attribute, "entry")){
                                attribute = (char*)malloc(NUMBER_OF_BITS*sizeof(char));
                                sprintf(attribute, "%s, entry", last_head->attribute);                                
                                last_head->attribute = attribute;                 
                                return;
                            }
                        }
                    }
                    else
                        if(!strcmp(attribute, "external")){
                            if(strstr(last_head->attribute, "entry")){
                                printf("Error in line %d:\nLabel cannot be both entry and external\n", g_lineNumber);
                                g_errors++;
                                return;
                            }   
                        }
                    else
                        if(!strcmp(last_head->attribute, "entry")){
                            last_head->attribute = attribute;
                            attribute = (char*)malloc(NUMBER_OF_BITS*sizeof(char));
                            sprintf(attribute, "%s, entry", last_head->attribute);
                            last_head->attribute = attribute;                 
                            last_head->address = g_address;
                            return;
                        }
                    else{
                        printf("Error in line %d:\nDuplicate label name\n", g_lineNumber);
                        g_errors++;
                    }
                }                
                last_head = last_head->next;  
            }
            last_head->next = new_label;
        }
    }
}
/*Receives a string and index and checks if there are any errors in the label, if not it returns the relevant part for entering into the table*/
char* checkLabel(char *line, int index){
    char *label = (char*)malloc((index)+1 * sizeof(char));
  
    int i, x, state = OUT, error = 0;

    if(strchr(line, ':') && !strstr(line, "extern") && !strstr(line, "entry")){
        strncpy(label, line, index);
        label[index] = '\0';
        if(!isalpha(line[0]) && !isspace(line[0])){
            printf("Error in line %d:\nNon-letter start of label\n", g_lineNumber);
            error = g_errors++;
        }        
        for (i = 1; i < index; i++){
        if(!isalpha(label[i]) && !isdigit(label[i])){
            if(isspace(label[i]))
                printf("Error in line %d:\nWhite spaces not allowed in label\n", g_lineNumber);                
            else
                printf("Error in line %d:\nSpecial characters not allowed in label\n", g_lineNumber);
                error = g_errors++;
            }
        }
    }
    else
        for (i = index; line[i]; i++){
            switch(state){
                case OUT:
                    if(isspace(line[i]))
                        state = SPACE;
                    else{
                        printf("Error in line %d:\nMissing space after .extern/.entry\n", g_lineNumber);
                        g_errors++;
                    }
                break;
                case SPACE:                    
                    if(!isalpha(line[i]) && !isspace(line[i])){
                        printf("Error in line %d:\nNon-letter start of label\n", g_lineNumber);
                        error = g_errors++;
                    }
                    else{                         
                        state = IN_WORD;
                        x = i;
                    }
                break;
                case IN_WORD:
                    if(!isalpha(line[i]) && !isspace(line[i]) && !isdigit(line[i])){
                        printf("Error in line %d:\nSpecial characters are not allowed for labels\n", g_lineNumber);
                        error = g_errors++;
                    }
                    else
                        if(line[i] == ','){
                            printf("Error in line %d:\nComma in .extern operand\n", g_lineNumber);
                            error = g_errors++;
                        }
                    else
                        if(isspace(line[i])){
                            state = FINISHED;
                            strncpy(label, line+x, i-x);
                            label[i-x] = '\0';
                        }
                    else
                        if(line[i+1] == '\0'){           
                            state = FINISHED;
                            strncpy(label, line+x, i-x+1);
                            label[i-x+1] = '\0';
                        }
                break;
                case FINISHED:
                    if(!isspace(line[i])){
                        printf("Error in line %d:\nNon-space character after label\n", g_lineNumber);
                        error = g_errors++;
                    }
                break;
            }
        }
    if(index > MAX_LABEL_LENGTH){
        printf("Error in line %d:\nLabel is too long\n", g_lineNumber);
        error = g_errors++;
    }
    if(!strlen(label)){
        printf("Error in line %d:\nEmpty Label\n", g_lineNumber);
        error = g_errors++;
    }
    if(error){
        free(label);
        return NULL;
    }
    else
        return label;
}