//
//  gCOM.c
//  guiriCOM_C_xCode
//
//  Created by Jose Andrade on 15/06/23.
//

#include "gCOM.h"
#include "gParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#define MAX_NESTED_FOR_LOOPS        25
#define MAX_NESTED_PROC_FUNCTIONS   25
#define MAX_LABELS                  25
#define MAX_LABELS_LENGHT           10

// cadena que almacena el programa Mingo
char *ptrMingoProgram;

// tabla con las 26 variables utilizables de la A a la Z
int variables[26] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct Token token;

struct Label
{
    char name[MAX_LABELS_LENGHT];
    char *referenceInPtrMingoProgram;

} labels_Table[MAX_LABELS];

// pila del ciclo FOR-NXT
struct For_Stack
{
    int controlVariable;                // Variable de control para contar los ciclos del FOR
    int toValue;                        // Valor TO del ciclo FOR
    char *referenceInPtrMingoProgram;
    
} for_Stack[MAX_NESTED_FOR_LOOPS];

// pila para el comando PROC tambien conocido como PROCEDURE
char *proc_Stack[MAX_NESTED_PROC_FUNCTIONS];

// indice de la cima de la pila FOR
int top_For_Stack_Index;
// indice de la cima de la pila PROC
int top_Proc_Stack_Index;


// Visualiza un error y termina la ejecucion del programa Mingo
// La terminacion anormal del programa Mingo requiere una mejor
// implementacion ya que actualmente el debugeo es a prueba y error
void show_error(char *errorMessage)
{
    printf("%s\n", errorMessage);
    
    printf("Value of error number: %d\n", errno);
    perror("Error message by perror");
    
    exit(EXIT_FAILURE);
}


// Ejecuta el comando OUT
void out_command(void)
{
    do
    {
        // Obtiene el siguiente token como parametro del commando OUT
        get_token_lexical_parser();

        switch(token.type)
        {
            case MATH_TOKEN_TYPE:
                switch(token.opcode)
                {
                    case '+':
                    case '-':
                        break;
                    default:
                        show_error("ERROR: Separador incorrecto");
                }
                break;
            case STRING_TOKEN_TYPE:
                switch(token.opcode)
                {
                    case '"':
                        printf("%s", token.stringInPtrMingoProgram);
                        break;
                    default:
                        show_error("ERROR: Separador incorrecto");
                }
                break;
            case PARAMETER_SEPARATOR_TOKEN_TYPE:
                switch(token.opcode)
                {
                    case ',':
                        break;
                    default:
                        show_error("ERROR: Separador incorrecto");
                }
                break;
            case VARIABLE_TOKEN_TYPE:
            case NUMBER_TOKEN_TYPE:
                return_program_counter_to_previous_token();
                
                printf("%d", get_value_expression_parser());
                break;
        }

    } while(token.type != END_SENTENCE_TOKEN_TYPE);

    // Aqui se debe de colocar un retorno de linea con avance de carro para el uControlador
    printf("\n");
}

// Ejecuta el comando OUTP
// aqui se pone el codigo para escribir a los puertos del uControlador
void outp_command(void)
{
    show_error("ERROR: Comando no implementado aun");
}

// Ejecuta el comando INP
// aqui se pone el codigo para leer de los puertos del uControlador
void inp_command(void)
{
    show_error("ERROR: Comando no implementado aun");
}


// encuentra el comienzo de la siguiente linea
void seek_eol(void)
{
    while(*ptrMingoProgram != '\r' && *ptrMingoProgram != '\0')
    {
        ++ptrMingoProgram;
    }

    if(*ptrMingoProgram)
    {
        ptrMingoProgram++;
    }
}


// Busca la etiqueta en la tabla de etiquetas.
// Se retorna nulo si la etiqueta no es econtrada;
// en otro caso se regresa un puntero a la posicion de la etiqueta
char *seek_label(char *labelName)
{
    for(int labelIndex = 0; labelIndex < MAX_LABELS; ++labelIndex)
    {
        if(!strcmp(labels_Table[labelIndex].name, labelName))
        {
            return labels_Table[labelIndex].referenceInPtrMingoProgram;
        }
    }

    // Etiqueta no encontrada
    return(NULL);
}


// ejecuta el comando JMP tambien conocido como SALTAR a etiqueta
void jmp_command(void)
{
    // obtiene etiqueta para hacer el salto (comando JMP)
    get_token_lexical_parser();

    // encuentra la referencia de la etiqueta en el programa Mingo
    char *labelReferenceInPtrMingoProgram = seek_label(token.stringInPtrMingoProgram);
    
    if(labelReferenceInPtrMingoProgram == (char*)'\0')
    {
        show_error("ERROR: Etiqueta no definida");
    }
    else
    {
        // mueve el program counter del programa Mingo para que
        // se ejecute a partir de la referencia de la etiqueta
        ptrMingoProgram = labelReferenceInPtrMingoProgram;
    }
}

// ejecuta el comando IF
void if_command(void)
{
    int rightConditionalExpressionValue = get_value_expression_parser();

    // obtiene el operador logico
    get_token_lexical_parser();
    
    if(!strchr("=<>", token.opcode))
    {
        show_error("ERROR: Error de sintaxis. Operador logico no permitido ");
    }

    char logicalOperator = token.opcode;

    int leftConditionalExpressionValue = get_value_expression_parser();

    int conditionalExpressionResult = 0;
    
    // determina el resultado de la condicion
    switch(logicalOperator)
    {
        case '=':
            if(rightConditionalExpressionValue==leftConditionalExpressionValue)
            {
                conditionalExpressionResult=1;
            }
            break;
            
        case '<':
            if(rightConditionalExpressionValue<leftConditionalExpressionValue)
            {
                conditionalExpressionResult=1;
            }
            break;
            
        case '>':
            if(rightConditionalExpressionValue>leftConditionalExpressionValue)
            {
                conditionalExpressionResult=1;
            }
            break;
    }
    
    if(conditionalExpressionResult)
    {
        // si es verdad la condicion, se procesa la parte THEN del IF
        get_token_lexical_parser();

        if(token.opcode != THEN_COMMAND_OPCODE)
        {
            show_error("ERROR: THEN esperado");
        }
    }
    else
    {
        // El lenguaje Mingo no tiene implementado el ELSE
        seek_eol();
    }
}


// Funcion push para la pila for_Stack
void for_push(struct For_Stack index)
{
    if(top_For_Stack_Index > MAX_NESTED_FOR_LOOPS)
    {
        show_error("ERROR: Demasiados ciclos FOR anidados");
    }

    for_Stack[top_For_Stack_Index] = index;
    top_For_Stack_Index++;
}


// Funcion pop para la pila for_Stack
struct For_Stack for_pop(void)
{
    top_For_Stack_Index--;

    if(top_For_Stack_Index < 0)
    {
        show_error("ERROR: NXT sin FOR");
    }

    return for_Stack[top_For_Stack_Index];
}

// Comando FOR, contraparte del comando NXT
void for_command(void)
{
    // lee la variable de control para conteo de ciclos FOR
    get_token_lexical_parser();
    
    if(!isalpha(token.stringInPtrMingoProgram[0]))
    {
        show_error("ERROR: No es una variable");
    }

    struct For_Stack forStackIndex;

    forStackIndex.controlVariable=toupper(token.stringInPtrMingoProgram[0]) - 'A';

    // lee el signo igual
    get_token_lexical_parser();

    if(token.opcode != '=')
    {
        show_error("ERROR: Signo (=) igual esperado");
    }

    // obtiene el valor inicial
    variables[forStackIndex.controlVariable] = get_value_expression_parser();

    get_token_lexical_parser();

    // lee y descarta el TO
    if(token.opcode != TO_COMMAND_OPCODE)
    {
        show_error("ERROR: TO esperado");
    }

    forStackIndex.toValue = get_value_expression_parser();

    // si el ciclo puede ejecutarse al menos una vez, empuja valor a la pila
    if(forStackIndex.toValue >= variables[forStackIndex.controlVariable])
    {
        forStackIndex.referenceInPtrMingoProgram = ptrMingoProgram;
        for_push(forStackIndex);
    }
    else
    {    // en otro caso, salta el codigo a la siguiente iteracion
        while(token.opcode != NXT_COMMAND_OPCODE)
        {
            get_token_lexical_parser();
        }
    }
}

// Ejecuta el comando NXT, contraparte del FOR
void nxt_command(void)
{
    struct For_Stack forStackIndex = for_pop();

    // incrementa la variable de control
    variables[forStackIndex.controlVariable]++;
    
    if(variables[forStackIndex.controlVariable] > forStackIndex.toValue)
    {
        // ciclo for terminado ya que el valor de la variable de control
        // es mayor la variable TO
        return;
    }

    // otro caso, añade elemento para continuar con la siguiente iteracion
    for_push(forStackIndex);

    ptrMingoProgram = forStackIndex.referenceInPtrMingoProgram;
}

// Ejecuta el comando IN
void in_command(void)
{
    get_token_lexical_parser();

    // A-Z: 1 de 26 variables utilizables
    char variableIndex = toupper(token.stringInPtrMingoProgram[0]) - 'A';

    // Lee la variable desde consola
    printf("? ");
    scanf("%d", &variables[variableIndex]);
}

// Ejecuta el comando PROC (PROCEDURE), contraparte del comando RET
void proc_command(void)
{
    get_token_lexical_parser();

    char *labelReferenceInPtrMingoProgram = seek_label(token.stringInPtrMingoProgram);

    if(labelReferenceInPtrMingoProgram == (char*)'\0')
    {
        show_error("ERROR: Etiqueta no definida");
    }
    else
    {
        top_Proc_Stack_Index++;

        if(top_Proc_Stack_Index == MAX_NESTED_PROC_FUNCTIONS)
        {
            show_error("ERROR: Demasiados PROCs anidados");
        }
        
        // proc_push: Añade elemento en la parte superior de la pila proc_Stack
        proc_Stack[top_Proc_Stack_Index] = ptrMingoProgram;
        

        // el puntero del programa MINGO se mueve para que empiece a partir de labelPosition
        ptrMingoProgram = labelReferenceInPtrMingoProgram;
    }
}


// Comando RET, retorna desde comando PROC
void ret_command(void)
{
    if(top_Proc_Stack_Index == 0)
    {
        show_error("ERROR: RET sin PROC");
    }

    // proc_pop: Elimina el elemento superior de la pila proc_Stack
    ptrMingoProgram = proc_Stack[top_Proc_Stack_Index--];
}


// Comando END que indica el fin de un programa MINGO
void end_command(void)
{
    token.type = END_SENTENCE_TOKEN_TYPE;
    token.opcode = EOP_END_SENTENCE_OPCODE;
}


// asigna un valor a una variable
void assign_value_to_variable(void)
{
    // obtiene el nombre del token variable
    get_token_lexical_parser();
    
    if(!isalpha(token.stringInPtrMingoProgram[0]))
    {
        show_error("ERROR: No es una variable");
    }

    // A-Z: 1 de 26 variables utilizables
    int variableIndex = toupper(token.stringInPtrMingoProgram[0]) - 'A';

    // busca el token con el operador de asignacion igual
    get_token_lexical_parser();
    
    if(token.opcode != '=')
    {
        show_error("ERROR: Signo (=) igual esperado");
    }

    // obtiene el valor para asignar a la variable y asigna el valor
    variables[variableIndex] = get_value_expression_parser();
}


// Define un apuntador a funcion
typedef void(*APFUNC)( void );

// arreglo de las funciones que despachan los comandos
APFUNC mingo_commands_table[] =
{
    out_command, outp_command, in_command, inp_command, if_command, for_command, nxt_command, jmp_command, proc_command, ret_command, end_command
};

// Inicializa la tabla de etiquetas
void initialize_labels_table(void)
{
    char *referenceTo_ptrMingoProgram_OriginalStart = ptrMingoProgram;
    
    int labelIndex = 0;
    
    // Inicializa el array de etiquetas. NULL indica que la etiqueta en el array esta sin usar */
    for(labelIndex = 0; labelIndex < MAX_LABELS; ++labelIndex )
    {
        labels_Table[labelIndex].name[0] = '\0';
    }
        
    // Busca etiquetas en todo el programa MINGO
    do
    {
        get_token_lexical_parser();
            
        if(token.type==NUMBER_TOKEN_TYPE)
        {
            // busca el indice de la siguiente posicion libre en el array de etiquetas
            for(labelIndex = 0; labelIndex < MAX_LABELS; ++labelIndex)
            {
                if(labels_Table[labelIndex].name[0] == 0)
                {
                    break;
                }
                    
                if(!strcmp(labels_Table[labelIndex].name, token.stringInPtrMingoProgram))
                {
                    show_error("ERROR: Etiqueta duplicada encontrada");
                }
            }
                
            if(labelIndex >= MAX_LABELS)
            {
                show_error("ERROR: Tabla de etiquetas llena");
            }
                
            strcpy(labels_Table[labelIndex].name, token.stringInPtrMingoProgram);
            
            labels_Table[labelIndex].referenceInPtrMingoProgram = ptrMingoProgram;
        }
            
        // si no es una linea en blanco, encuentra siguiente linea
        if(token.type != END_SENTENCE_TOKEN_TYPE && token.opcode != EOL_END_SENTENCE_OPCODE)
        {
            seek_eol();
        }
            
    } while (token.type != END_SENTENCE_TOKEN_TYPE || token.opcode != EOP_END_SENTENCE_OPCODE);
    
    // restaura programa Mingo
    ptrMingoProgram = referenceTo_ptrMingoProgram_OriginalStart;
}

void interpreter(char *programFile)
{
    ptrMingoProgram = programFile;
    
    initialize_labels_table();
    
    // inicializa los indices de las pilas FOR y PROC
    top_For_Stack_Index = 0;
    top_Proc_Stack_Index = 0;

    do
    {
        get_token_lexical_parser();

        // Ejecuta tarea dependiendo del tipo de token
        switch(token.type)
        {
            case VARIABLE_TOKEN_TYPE:
                return_program_counter_to_previous_token();
                assign_value_to_variable();
                break;
                
            case COMMAND_TOKEN_TYPE:
                (*mingo_commands_table[token.opcode - 1])();
                break;
                
        }
        
    } while(token.type != END_SENTENCE_TOKEN_TYPE || token.opcode != EOP_END_SENTENCE_OPCODE);
}
