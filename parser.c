#include "data.h"
#include "parser.h"
#include "assembler.h"
#include "label.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern int g_errors;
extern int g_lineNumber;
extern int g_address;
extern int g_externals;
extern struct Instructions instructions[];
extern struct MissingValues *missingValues;
extern struct Table *tableList;
extern struct DataSet *dataSet;
extern struct Labels *labelsList;

/*Checks if the instruction sent is in the instructions table/is correct*/
int checkInstruction(char *instruction){
    int i;
    for (i = 0; i < NUMBER_OF_INSTRUCTIONS; i++) {
        if (strcmp(instruction, instructions[i].name) == 0)
        return i;
    }
    printf("Error in line %d:\nWrong instruction name...\n", g_lineNumber);
    g_errors++;
    return 0;
}
/*Receives the index of the instruction (that checkInstruction returns), index of the operand, and the operand itself,
And checks that the operand is between the permitted parameters*/
char* checkOperand(int instructionIndex, int operandIndex, char *operand, int *operandType){
    int i, opCode;
    char *code; /*Might need to check if I can free this or what*/

    if(operand[0] == '#'){                                              /*Type 0 operand, immediate*/
        *operandType = TYPE_ZERO_OPERAND;
        if(operand[1] != '-' && operand[1] != '+' && !isdigit(operand[1])){
            printf("Error in line %d:\nWrong input after #\n", g_lineNumber);
            g_errors++;
            return NULL;
        }
        else{
            if(operand[1] == '+')
                opCode = (atoi((operand+2)));
            else
                opCode = (atoi((operand+1)));        
            if(opCode != 0){
                code = toBinary(opCode);                      
            }
            else
                code = "000000000000";
        }
    }
    else
        if(operand[0] == '%' && isalpha(operand[1])){                    /*Type 2 operand, relative*/
            *operandType = TYPE_TWO_OPERAND;
            code = "?";
        }
    else
        if(operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7'){ /*Type 3 operand, register*/
            *operandType = TYPE_THREE_OPERAND;
            code = (char*) malloc(sizeof(char) * NUMBER_OF_BITS);
            strcpy(code,"000000000000");
            code[NUMBER_OF_BITS - 2 - (operand[1] - '0')]= '1';
        }
    else{                                                               /*Type 1 operand, direct*/
        *operandType = TYPE_ONE_OPERAND;
        for (i = 0; i < NUMBER_OF_INSTRUCTIONS; i++){                   /*Checking for saved/forbidden words*/
            if(strcmp(operand, instructions[i].name) == 0){
                printf("Error in line %d:\nOperand contains forbidden/saved word\n", g_lineNumber);
                g_errors++;
                return "Operand contains forbidden/saved word";
            }
        }
        code = "?"; /*NULL for now*/       
    }                                                                      /*Finding opCode */       
    if(*operandType != TYPE_ZERO_OPERAND && *operandType != TYPE_TWO_OPERAND && !isalpha(operand[0])){
        printf("Error in line %d:\nOperand must start with a letter\n", g_lineNumber);
        g_errors++;
    }
    for (i = 0; i < strlen(operand); i++){
        if(*operandType != TYPE_ZERO_OPERAND && *operandType != TYPE_TWO_OPERAND && !isalpha(operand[i]) && !isdigit(operand[i])){
            printf("Error in line %d:\nSpecial characters not allowed\n", g_lineNumber);
            g_errors++;
        }
        else
            if(i > 1 && !isalpha(operand[i]) && !isdigit(operand[i])){
                printf("Error in line %d:\nSpecial characters not allowed\n", g_lineNumber);
                g_errors++;
            }
    }
    if(operandIndex == 1 && instructions[instructionIndex].numberOfOperands == 2){
        for (i = 0; i < MAX_NUMBER_OF_TYPES; i++) {
            if(*operandType == instructions[instructionIndex].addMethod->sourceType[i]){
                return code;
            }
        }            
    }
    else{
        for (i = 0; i < MAX_NUMBER_OF_TYPES; i++) {
            if(*operandType == instructions[instructionIndex].addMethod->destType[i]){
                return code;
            }
        }
    }
    printf("Error in line %d:\nWrong operand type\n", g_lineNumber);
    g_errors++;
    return "error";
}
/*Receives the line and parses the operation and operands, checks that they are correct and inserts them into table*/
void parser(int index, char *line){
    unsigned int number;
    int i, x, state = OUT, word = 0, commaCounter = 0;
    int operandsTypes[MAX_NUMBER_OF_WORDS-1];
    char *machineCode[MAX_NUMBER_OF_WORDS];
    char *words[MAX_NUMBER_OF_WORDS];
    char *sourceCode;
    char ARE;
    int length = strlen(line);
    
    if (index)
        i = index + 1;
    else
        i = 0;
    for (x = i;i < strlen(line) && line[i]; i++){
        switch (state){
            case OUT:
                if (isspace(line[i])){
                    state = SPACE;
                } 
                else
                    if(line[i] == ','){
                        printf("Error in line %d:\nWrong comma placement before instruction\n", g_lineNumber);
                        g_errors++;
                        state = SPACE;
                        commaCounter = 1;
                    }
                else{
                    state = IN_WORD;
                    x = i;
                    commaCounter = 0;                
                }
            break;                             
            case SPACE:                        /*There shouldn't be a comma before the first operand or multiple commas*/
                if(line[i] == ','){                    
                    if(word < 2){
                        printf("Error in line %d:\nWrong comma placement before first operand\n", g_lineNumber);
                        g_errors++;
                    }
                    else
                        if(commaCounter > 0){
                            printf("Error in line %d:\nWrong comma placement, consecutive commas\n", g_lineNumber);
                            g_errors++;
                        }
                    commaCounter = 1;
                }
                else
                    if(!isspace(line[i])){
                        if(word > 1 && commaCounter == 0){
                            printf("Error in line %d:\nMissing comma between operands\n", g_lineNumber);
                            g_errors++;
                        }
                        state = IN_WORD;
                        x = i;
                        commaCounter = 0;
                    }                   
            break;
            case IN_WORD:
                    if (isspace(line[i]) || line[i] == ',' || line[i+1] == '\0'){ /*Found an word*/
                        if(line[i+1] == '\0' && !isspace(line[i]))
                            i++;
                        state = SPACE;
                        if(word == 0){  /*Instruction*/
                            if(line[i] == ','){
                                commaCounter = 1;
                                printf("Error in line %d:\nWrong comma placement (Before first operand)\n", g_lineNumber);
                                g_errors++;
                                words[word] = (char*)malloc((i-x)*sizeof(char));
                                checkMalloc(words[word]);               
                                strncpy(words[word], line + x, i - x);
                                words[word][i - x] = '\0';
                                index = checkInstruction(words[word]);                   
                            }
                            else{
                                commaCounter = 0;
                                words[word] = (char*)malloc((i-x+1)*sizeof(char));
                                checkMalloc(words[word]);               
                                strncpy(words[word], line + x, i - x);
                                words[word][i - x] = '\0';
                                index = checkInstruction(words[word]);
                            }
                            if(g_errors)
                                free(words[word]);
                            }
                        else                /*Operands*/
                            if(word <= instructions[index].numberOfOperands && word > 0){
                                if(line[i] == ','){
                                    if(commaCounter > 0){
                                        printf("Error in line %d:\nWrong comma placement, consecutive commas\n", g_lineNumber);
                                        g_errors++;
                                    }
                                    else
                                        commaCounter = 1;
                                }
                                else
                                    commaCounter = 0;                               
                                words[word] = (char*)malloc(MAX_LINE_LENGTH*sizeof(char));
                                strncpy(words[word], line + x, i - x);
                                words[word][i - x] = '\0';
                                machineCode[word] = (char*)malloc(NUMBER_OF_BITS*sizeof(char));                        
                                checkMalloc(machineCode[word]);
                                strcpy(machineCode[word],checkOperand(index, word, words[word],
                                        &operandsTypes[word-1]));
                                if(g_errors)
                                    free(machineCode[word]);                      
                            }
                        else
                            if(word > instructions[index].numberOfOperands){    
                                if(line[i] != ',')
                                    commaCounter = 0;
                                else
                                    commaCounter = 1;                   
                                printf("Error in line %d:\nToo many operands for instruction\n", g_lineNumber); 
                                g_errors++;                   
                            }
                        word++;
                    }
            break;
        }
    }
    if(word-1 < instructions[index].numberOfOperands){
        printf("Error in line %d:\nMissing %d operand/s\n", g_lineNumber, instructions[index].numberOfOperands-(word-1)); 
        g_errors++;               
    }
    if(commaCounter == 1){
        printf("Error in line %d:\nComma after last operand\n", g_lineNumber); 
        g_errors++;               
    }
    if(!g_errors){
        sourceCode = (char*)malloc(MAX_LINE_LENGTH*sizeof(char));
        checkMalloc(sourceCode);
        strncpy(sourceCode, line, length-1);
        sourceCode[length-1] = '\0';
        if(instructions[index].numberOfOperands == 1)
            operandsTypes[1] = operandsTypes[0];
        
        machineCode[0] = (char*)malloc(NUMBER_OF_BITS*sizeof(char));
        checkMalloc(machineCode[0]);
        for(i = 0; i < NUMBER_OF_BITS - 1; i++){     /*Opcode machine code*/
            if(i < 4){
                number = 1u << (4 - 1 - i);  /*Bits 1 to 4*/
                machineCode[0][i] = ((instructions[index].opcode & number) ? '1' : '0');  
            }
            else
                if(i >= 4 && i < 8){    /*Bits 4 to 8*/
                    if(instructions[index].funct){  /*Funct machine code*/
                        number = 1u << (8 - 1 - i);         
                        machineCode[0][i] = ((instructions[index].funct & number) ? '1' : '0');
                    }
                    else
                        machineCode[0][i] = '0';                  
                }        
            else                                    /*Operand 1 machine code*/
                if (i >= 8 && i < 10){    /*Bits 8 to 10*/          
                    if(instructions[index].numberOfOperands == 2 && operandsTypes[0] > 0 && operandsTypes[0] <= 3){
                        number = 1u << (10 - 1 - i);
                        machineCode[0][i] = ((operandsTypes[0] & number) ? '1' : '0');
                    }
                    else 
                        machineCode[0][i] = '0';
                }                
            else                                     /*Operand 2 machine code*/ 
                if (i >= 10){  /*Bits 10 to 12*/                                    
                    if(instructions[index].numberOfOperands > 0 && operandsTypes[1] > 0 && operandsTypes[1] <= 3){
                        number = 1u << (12 - 1 - i);  
                        machineCode[0][i] = ((operandsTypes[1] & number) ? '1' : '0');
                    }
                    else 
                        machineCode[0][i] = '0';
                }
        }
        machineCode[0][i] = '\0';
             
        for (i = 0; i < MAX_NUMBER_OF_WORDS && word > 0; i++, word--){      /*Start push table*/  
            if(i > 0 && (operandsTypes[i-1] == 0 || operandsTypes[i - 1] == 3 || operandsTypes[i - 1] == 2))
                ARE = 'A';
            else
                if(i > 0 && operandsTypes[i - 1] == 1)
                    ARE = 'R';                          /*Temporally putting R but might change to E if external*/               
            pushTable((i==0 ? sourceCode : words[i]),
                machineCode[i], (i == 0 ? 'A' : ARE));
            g_address++;
        }
    }
}
/*Fills the details of the labels and missing information from the first run*/
void insertMissing(){
    struct Labels *last_label = labelsList;
    int address;
    char *code;
    while(missingValues != NULL){   
        while(last_label != NULL){
            if(strstr(missingValues->missingEntry->sourceCode, last_label->symbol)){
                if(strcmp(last_label->attribute, "external")){
                    address = last_label->address;
                    if(strchr(missingValues->missingEntry->sourceCode, '%'))
                        address -= (missingValues->missingEntry->address);
                    code = toBinary(address);
                    strcpy(missingValues->missingEntry->machineCode, code);
                    break;
                }
                else{
                    g_externals++;
                    strcpy(missingValues->missingEntry->machineCode, "000000000000");
                    missingValues->missingEntry->ARE = 'E';
                }
            }
            last_label = last_label->next;
        }
        last_label = labelsList;
        missingValues = missingValues->next;
    }
}
/*Used to push a new line to the main table's linked list */
void pushTable(char *sourceCode, char *machineCode, char ARE){
    struct Table *new_table = (struct Table*)malloc(sizeof(struct Table));
    struct Table *last_table_head = tableList;
    struct MissingValues *last_missing_head;
    struct MissingValues *new_missing_value;
    int missing;
    if(missingValues != NULL)
        last_missing_head = missingValues;    
    checkMalloc(new_table);
    
    new_table->address = g_address;
    new_table->sourceCode = sourceCode;
    new_table->machineCode = machineCode;
    new_table->ARE = ARE;
    new_table->next = NULL;
    
    if(!(missing = strcmp(machineCode,"?"))){
        new_missing_value = (struct MissingValues*)malloc(sizeof(struct MissingValues));
        checkMalloc(new_missing_value);             
        new_missing_value->missingEntry = new_table;
        new_missing_value->next = NULL;
    }
    if(tableList == NULL){
        tableList = new_table;
        if(!missing)
            missingValues = new_missing_value;
    }
    else{
        while(last_table_head->next != NULL)
            last_table_head = last_table_head->next;
        if(!missing){
            if(missingValues == NULL)
                missingValues = new_missing_value;
            else{
                while(last_missing_head->next != NULL) /*CHECK HOW TO FIX THIS SHIT*/
                    last_missing_head = last_missing_head->next;
                last_missing_head->next = new_missing_value;
            }
        }
        last_table_head->next = new_table;
    }
}
/*Converts a number to a 12 bit binary code string*/
char* toBinary(int number){
    unsigned int mask, i;   
    char *code = (char*)malloc(NUMBER_OF_BITS*sizeof(char));    
    checkMalloc(code);
    for(i = 0; i < NUMBER_OF_BITS - 1; i++){
        mask = 1u << (NUMBER_OF_BITS - 2 - i);
        code[i] = ((number & mask) ? '1' : '0'); /*Might want it to be numberOfBits - i or something like that*/
    }                                               /*remember what you thought of when speaking with beni */
    code[i] = '\0';
    return code;
}
/*Pushes a .data or .string into the DataSet table so we can save it and enter it to the main table at the end*/
void pushDataSet(char *line, int index){
    struct DataSet *last_head = dataSet;
    struct DataSet *new_data_set;
    
    new_data_set = (struct DataSet *)malloc(sizeof(struct DataSet));
    checkMalloc(new_data_set);
    new_data_set->sourceCode = (char*)malloc(MAX_LINE_LENGTH*sizeof(char));
    strcpy(new_data_set->sourceCode, line);
    new_data_set->lineNumber = g_lineNumber;
    new_data_set->next = NULL;
    if(dataSet == NULL){
        dataSet = new_data_set;
    }
    else{
        while(last_head->next != NULL) 
            last_head = last_head->next;      
        last_head->next = new_data_set;
    }
}
/*Inserts the .data and .string to the table (at the end since we are running this last)*/
int insertDataSets(int *DC){
    int i, j, x, data_number, state ,counter = 0, commaCounter, signCounter;
    char *code, *index, *line, data[MAX_LINE_LENGTH];
    while(dataSet){
        state = OUT;
        counter = 0;
        commaCounter = 0;
        line = dataSet->sourceCode;
        if((index = strchr(line,':'))){
            i = index - line;
            pushLabel(checkLabel(line, i), "data"); 
        }
        if((index = strstr(dataSet->sourceCode, ".data"))){
            for (i = index - dataSet->sourceCode + 5; i < MAX_LINE_LENGTH && line[i]; i++){
                switch (state){
                    case OUT:
                        if(isspace(line[i]))
                            state = SPACE;
                        else
                            if(line[i] == ','){
                                printf("Error in line %d:\nWrong comma placement\n", dataSet->lineNumber);
                                g_errors++;
                                commaCounter = 1;
                            }
                        else
                            if(!isdigit(line[i])){
                                printf("Error in line %d:\nNon digit placement in .data\n", dataSet->lineNumber);
                                g_errors++;
                                commaCounter = 0;
                            }
                        else{
                            state = IN_WORD;
                            x = i;
                            commaCounter = 0;
                        }
                    break;
                    case SPACE:
                        if((line[i] == ',')){
                            if(counter == 0){
                                printf("Error in line %d:\nWrong comma placement\n", dataSet->lineNumber);
                                g_errors++;
                            }
                            if(commaCounter > 0){
                                printf("Error in line %d:\nWrong comma placement, consecutive comma\n", dataSet->lineNumber);
                                g_errors++;
                            }
                            commaCounter = 1;
                        }
                        else
                            if(isalpha(line[i])){
                                printf("Error in line %d\nNon digit placement in .data\n", dataSet->lineNumber);
                                g_errors++;
                                commaCounter = 0;
                                counter++;
                            }
                        else
                            if(isdigit(line[i]) || (signCounter = line[i] == '-') || (signCounter = line[i] == '+')){                                
                                commaCounter = 0;
                                state = IN_WORD;
                                x = i; 
                            }
                    break;
                    case IN_WORD:
                        if((line[i] == '-' || line[i] == '+') && signCounter > 0){
                            printf("Error in line %d:\nWrong/multiple number signs placement (- or +)\n", dataSet->lineNumber);
                            g_errors++;
                        }
                        else
                            if(line[i] == ',' && commaCounter > 0){            
                                printf("Error in line %d:\nConsecutive comma placement\n", dataSet->lineNumber);
                                g_errors++;
                            }
                        else
                            if((line[i] == ',') || (isspace(line[i]) && (line[i+1] == ',' || line[i+1] == '\0')) 
                                                               || line[i+1] == '\0'){
                                if(line[i] == ',')
                                    commaCounter = 1;
                                else
                                    commaCounter = 0;
                                state = SPACE;
                                if(!g_errors){
                                    strncpy(data, line+x, (line[i+1] == '\0'? i-x+1 : i-x));
                                    data[(line[i+1] == '\0'? i-x+1 : i-x)] = '\0';
                                    data_number = atoi(data);                                
                                    code = toBinary(data_number);                        
                                    pushTable((counter == 0 ? dataSet->sourceCode : " "), code , 'A');
                                    (*DC)++;
                                    g_address++;
                                    counter++;
                                }
                                signCounter = 0;
                            }
                        else    
                            if(!isdigit(line[i])){
                                printf("Error in line %d\nNon digit placement inbetween number .data\n", dataSet->lineNumber);
                                commaCounter = 0;
                                g_errors++;
                            }
                    break;
                }
            }
            if(commaCounter > 0){
                printf("Error in line %d:\nComma placement after last digit\n", dataSet->lineNumber);
                g_errors++;
            }
            if(counter == 0){
                printf("Error in line %d:\nNo numbers in .data/empty\n", dataSet->lineNumber);
                g_errors++;
            }
        }
        else
            if((index = strstr(dataSet->sourceCode, ".string"))){
                state = OUT;                                       
                for (i = (index - dataSet->sourceCode)+7;line[i]; i++){                    
                    switch (state){
                        case OUT:
                            if(isspace(line[i]))
                                state = SPACE;                                               
                            else if(line[i] == '\"'){
                                state = IN_STRING;
                                x = i+1;
                                }
                                else{
                                    printf("Error in line %d:\nInvalid input in .string\n", dataSet->lineNumber);
                                    g_errors++;
                                }
                        break;
                        case SPACE:
                            if(line[i] == ','){
                                printf("Error in line %d:\nComma placement in .string\n", dataSet->lineNumber);
                                g_errors++;
                            }
                            else
                                if(line[i] == '\"'){
                                    state = IN_STRING;
                                    x = i+1;
                                }
                            else
                                if(!isspace(line[i])){
                                    printf("Error in line %d:\nNon-space character placement before string opening (\")\n", dataSet->lineNumber);
                                }

                        break;                    
                        case IN_STRING:
                            if(line[i] == '\"'){
                                state = FINISHED;
                                if(!g_errors){
                                    strncpy(data, line+x, i-x);
                                    data[i-x] = '\0';
                                    for (j = 0; j < (i-x)+1; j++){                                        
                                        data_number = (int)data[j]; /*Receiving ASCII code*/
                                        code = toBinary(data_number);                                        
                                        pushTable((counter == 0 ? (dataSet->sourceCode) : " "), code , 'A');
                                        (*DC)++;
                                        g_address++;
                                        counter++;
                                    }
                                }                                              
                            }
                        break;
                        case FINISHED:
                            if(!isspace(line[i])){
                                printf("Error in line %d:\nNon-space character after string\n", dataSet->lineNumber);
                                g_errors++;
                            }
                        break;
                    }
                }
                if(state != FINISHED){
                    printf("Error in line %d:\nIlegal string format\n", dataSet->lineNumber);
                    g_errors++;
                }
            }
        dataSet = dataSet->next; 
    }
    return g_errors;
}