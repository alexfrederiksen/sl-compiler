#pragma once
#include <string.h>

#define MEM_SIZE 100

/* define opcodes and their translations */
#define OPCODES \
        o(READ,        10, printf("? "); scanf("%d", memory + operand);)             /* read into mem address                              */ \
        o(WRITE,       11, printf("%d\n", memory[operand]);)                         /* print from mem address                             */ \
        o(LOAD,        20, accum = memory[operand];)                                 /* load mem address into accum                        */ \
        o(STORE,       21, memory[operand] = accum;)                                 /* store accum in mem address                         */ \
        o(ADD,         30, accum += memory[operand];)                                /* add mem address to accum (leave in accum)          */ \
        o(SUB,         31, accum -= memory[operand];)                                /* subtract mem address and accum (leave in accum)    */ \
        o(DIV,         32, accum /= memory[operand];)                                /* divide mem address and accum (leave in accum)      */ \
        o(MUL,         33, accum *= memory[operand];)                                /* multiply mem address and accum (leace in accum)    */ \
        o(BRANCH,      40, inst_counter = operand - 1;)                              /* branch to another mem address                      */ \
        o(BRANCH_NEG,  41, inst_counter = accum < 0 ? operand - 1 : inst_counter;)   /* branch to another mem address if accum is negative */ \
        o(BRANCH_ZERO, 42, inst_counter = accum == 0 ? operand - 1 : inst_counter;)  /* branch to another mem address if accum is zero     */ \
        o(HALT,        43, return 1;)                                                /* end the program                                    */ \

/* declare opcode identifiers */
#define o(name, op, exec) extern const int SML_ ## name;
OPCODES
#undef o

/* define common helper functions */
int get_opcode(int inst);
int get_operand(int inst);
char * get_op_str(char * str, int opcode);

