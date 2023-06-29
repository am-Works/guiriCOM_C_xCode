//
//  gParser.c
//  guiriCOM_C_xCode
//
//  Created by Jose Andrade on 16/06/23.
//

#include "gParser.h"
#include "gCOM.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

struct Commands_Table
{
    char command[6];
    char opcode;
    
} commands_Table[] = {
        "out",  OUT_COMMAND_OPCODE,
        "outp", OUTPORT_COMMAND_OPCODE,
        "in",   INPUT_COMMAND_OPCODE,
        "inp",  INPUT_COMMAND_OPCODE,
        "if",   IF_COMMAND_OPCODE,
        "then", THEN_COMMAND_OPCODE,
        "for",  FOR_COMMAND_OPCODE,
        "to",   TO_COMMAND_OPCODE,
        "nxt",  NXT_COMMAND_OPCODE,
        "jmp",  JMP_COMMAND_OPCODE,
        "proc", PROC_COMMAND_OPCODE,
        "ret",  RET_COMMAND_OPCODE,

        "end",  END_COMMAND_OPCODE,
        "",     END_COMMAND_OPCODE
    };


int add_or_substract_operations_parser_level1(void);

// realiza las operaciones aritmeticas
int execute_arithmetic_operation(char mathOperator, int firstOperand, int secondOperand)
{
    int result = 0;

    switch(mathOperator)
    {
        case '-' :
            result = firstOperand - secondOperand;
            break;
            
        case '+' :
            result = firstOperand + secondOperand;
            break;
            
        case '*' :
            result = firstOperand * secondOperand;
            break;
            
        case '/' :
            result = firstOperand / secondOperand;
            break;
            
        case '%' :
            result = firstOperand - ((firstOperand / secondOperand) * secondOperand);
            break;
            
        case '^' :
            result = pow(firstOperand, secondOperand);
            break;
    }
    
    return result;
}

// procesa expresiones entre parentesis
int round_brackets_parser_level5(void)
{
    int resultado = 0;
    
    if((token.opcode == '(') && (token.type == MATH_TOKEN_TYPE))
    {
        get_token_lexical_parser();
        
        resultado = add_or_substract_operations_parser_level1();
        
        if(token.opcode != ')')
        {
            show_error("ERROR: Parentesis no balanceados");
        }
        
        get_token_lexical_parser();
    }
    else
    {
        // Encuentra el valor de un numero o variable
        switch(token.type)
        {
            case VARIABLE_TOKEN_TYPE:
                if(!isalpha(*token.stringInPtrMingoProgram))
                {
                    show_error("ERROR: No es una variable");
                }
                resultado = variables[toupper(token.stringInPtrMingoProgram[0]) - 'A'];
                get_token_lexical_parser();
                break;
                
            case NUMBER_TOKEN_TYPE:
                resultado=atoi(token.stringInPtrMingoProgram);
                get_token_lexical_parser();
                break;
                
            default:
                show_error("ERROR: Error de sintaxis ");
        }
    }
    
    return resultado;
}

// pone signo + o -
int unary_operation_parser_level4(void)
{
    char sign = 0;

    if((token.type == MATH_TOKEN_TYPE) && (token.opcode == '+' || token.opcode == '-'))
    {
        sign = token.opcode;
        get_token_lexical_parser();
    }
    
    int resultado = round_brackets_parser_level5();

    if(sign == '-')
    {
        // cambia el signo a negativo
        resultado = -(resultado);
    }
    
    return resultado;
}

// procesa exponente
int power_operation_parser_level3(void)
{
    int resultado = unary_operation_parser_level4();
    
    if(token.opcode == '^')
    {
        get_token_lexical_parser();
        
        resultado = execute_arithmetic_operation('^', resultado, unary_operation_parser_level4());
    }
    
    return resultado;
}


// multiplica o divide
int multiply_or_divide_operations_parser_level2(void)
{
    int resultado = power_operation_parser_level3();
    char mathOperator;

    while((mathOperator = token.opcode) == '*' || mathOperator == '/' || mathOperator == '%')
    {
        get_token_lexical_parser();
        
        resultado = execute_arithmetic_operation(mathOperator, resultado, power_operation_parser_level3());
    }
    
    return resultado;
}

 
// suma o resta
int add_or_substract_operations_parser_level1(void)
{
    char mathOperator;
    int resultado = multiply_or_divide_operations_parser_level2();
    
    while((mathOperator = token.opcode) == '+' || mathOperator == '-')
    {
        get_token_lexical_parser();
        
        resultado = execute_arithmetic_operation(mathOperator, resultado, multiply_or_divide_operations_parser_level2());
    }
    
    return resultado;
}

// obtiene el valor con el analizador de expresiones (matematicas)
int get_value_expression_parser(void)
{
    get_token_lexical_parser();
    
    if(token.type == END_SENTENCE_TOKEN_TYPE)
    {
        show_error("ERROR: Ninguna expresion presente");
    }
    
    int value = add_or_substract_operations_parser_level1();
    
    return_program_counter_to_previous_token();
    
    return value;
}


// obtiene token con el analizador lexico
void get_token_lexical_parser(void)
{
    char *referenceTo_Token_stringInPtrMingoProgram_OriginalStart = token.stringInPtrMingoProgram;

    // ignora espacios en blanco y tabuladores
    while(*ptrMingoProgram == ' ' || *ptrMingoProgram == '\t') // || *ptrMingoProgram == '\n')
    {
        ++ptrMingoProgram;
    }

    // Identifica si es el fin del programa MINGO (fin del archivo)
    if(*ptrMingoProgram == '\0')
    {
        token.stringInPtrMingoProgram[0] = '\0';
        token.type = END_SENTENCE_TOKEN_TYPE;
        token.opcode = EOP_END_SENTENCE_OPCODE;
        return;
    }

    // Identifica si es el '\r' para separar tokens y sentencias
    //if(*ptrMingoProgram == '\r')
    if(*ptrMingoProgram == '\r' || *ptrMingoProgram == '\n')
    {
        ++ptrMingoProgram;
        token.stringInPtrMingoProgram[0] = '\0';
        token.type = END_SENTENCE_TOKEN_TYPE;
        token.opcode = EOL_END_SENTENCE_OPCODE;
        return;
    }

    // es un operador matematico
    if(strchr("+-*/^%=()><", *ptrMingoProgram))
    {
        token.type = MATH_TOKEN_TYPE;
        // token.opcode almancena el operador matematico
        token.opcode = *ptrMingoProgram;
        ptrMingoProgram++;

        return;
    }
    
    // es una cadena
    if(strchr("\"", *ptrMingoProgram))
    {
        token.type = STRING_TOKEN_TYPE;
        // token.opcode almancena la comilla \"
        token.opcode = *ptrMingoProgram;
        ptrMingoProgram++;

        while(*ptrMingoProgram != '"' && *ptrMingoProgram != '\r' && *ptrMingoProgram != '\0')
        {
            *referenceTo_Token_stringInPtrMingoProgram_OriginalStart++ = *ptrMingoProgram++;
        }

        if(*ptrMingoProgram != '"')
        {
            show_error("ERROR: Error de sintaxis ");
            return;
        }

        ptrMingoProgram++;
        *referenceTo_Token_stringInPtrMingoProgram_OriginalStart = '\0';
        
        return;
    }
    
    // es un separador de parametros
    if(strchr(",", *ptrMingoProgram))
    {
        token.type = PARAMETER_SEPARATOR_TOKEN_TYPE;
        // token.opcode almancena la coma
        token.opcode = *ptrMingoProgram;
        ptrMingoProgram++;

        return;
    }

    // es un numero
    if(isdigit(*ptrMingoProgram))
    {
        // Busca fin del numero
        while(!strchr("+-*/^%=(),><\" \r\t\n", *ptrMingoProgram))
        {
            *referenceTo_Token_stringInPtrMingoProgram_OriginalStart++ = *ptrMingoProgram++;
        }
        
        *referenceTo_Token_stringInPtrMingoProgram_OriginalStart = '\0';
        token.type = NUMBER_TOKEN_TYPE;
        token.opcode = 0;
        
        return;
    }

    // es una variable o comando
    if(isalpha(*ptrMingoProgram))
    {
        // busca el fin de la variable o comando
        while(!strchr("+-*/^%=(),><\" \r\t\n", *ptrMingoProgram))
        {
            *referenceTo_Token_stringInPtrMingoProgram_OriginalStart++ = *ptrMingoProgram++;
        }
        
        *referenceTo_Token_stringInPtrMingoProgram_OriginalStart = '\0';

        referenceTo_Token_stringInPtrMingoProgram_OriginalStart = token.stringInPtrMingoProgram;
        
        while(*referenceTo_Token_stringInPtrMingoProgram_OriginalStart)
        {
            // para simplificar, todas las variables o comando se manejan en minusculas, es decir, Mingo no es case sensitive
            *referenceTo_Token_stringInPtrMingoProgram_OriginalStart = tolower(*referenceTo_Token_stringInPtrMingoProgram_OriginalStart);
            
            referenceTo_Token_stringInPtrMingoProgram_OriginalStart++;
        }

        // Si no se encuentra el token en la tabla de commandos se queda como VARIABLE
        token.type = VARIABLE_TOKEN_TYPE;
        token.opcode = 0;

        // busca el token en la tabla de comandos
        for(int index = 0; *commands_Table[index].command; index++)
        {
            if(!strcmp(commands_Table[index].command, token.stringInPtrMingoProgram))
            {
                token.type = COMMAND_TOKEN_TYPE;
                token.opcode = commands_Table[index].opcode;
                break;
            }
        }
        
        return;
    }

    show_error("ERROR: Token desconocido");
}

// pone el puntero de referencia del programa Mingo al token anterior
void return_program_counter_to_previous_token(void)
{
    char *t = token.stringInPtrMingoProgram;

    for(; *t; t++)
    {
        ptrMingoProgram--;
    }
}
