//
//  gCOM.h
//  guiriCOM_C_xCode
//
//  Created by Jose Andrade on 15/06/23.
//

#ifndef gCOM_h
#define gCOM_h

#include <stdio.h>


// Tipos de Tokens
#define MATH_TOKEN_TYPE                 1   // + - * / ^ % = ( ) > <
#define STRING_TOKEN_TYPE               2   // "
#define PARAMETER_SEPARATOR_TOKEN_TYPE  3   // ,
#define VARIABLE_TOKEN_TYPE             4
#define NUMBER_TOKEN_TYPE               5
#define COMMAND_TOKEN_TYPE              6
#define END_SENTENCE_TOKEN_TYPE         7

// Opcodes
#define OUT_COMMAND_OPCODE          1
#define OUTPORT_COMMAND_OPCODE      2
#define INPUT_COMMAND_OPCODE        3
#define INPUTPORT_COMMAND_OPCODE    4
#define IF_COMMAND_OPCODE           5
#define FOR_COMMAND_OPCODE          6
#define NXT_COMMAND_OPCODE          7
#define JMP_COMMAND_OPCODE          8
#define PROC_COMMAND_OPCODE         9
#define RET_COMMAND_OPCODE          10
#define END_COMMAND_OPCODE          11
#define THEN_COMMAND_OPCODE         12
#define TO_COMMAND_OPCODE           13
#define EOL_END_SENTENCE_OPCODE     14
#define EOP_END_SENTENCE_OPCODE     16      // End of Program


// almacena el programa Mingo
extern char *ptrMingoProgram;
// array con las 26 variables utilizables (A a la Z) de tipo entero
extern int variables[26];

extern struct Token
{
    char stringInPtrMingoProgram[80];   // cadena del token
    char type;                          // tipo de token 🤔
    char opcode;                        // representacion interna de token
} token;

void interpreter(char *programFile);
void show_error(char *errorMessage);

#endif /* gCOM_h */
