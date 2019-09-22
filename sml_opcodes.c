#include "sml_opcodes.h"

/* actually define opcodes */
#define o(name, op, exec) const int SML_ ## name = op;
OPCODES
#undef o

/* define common helper functions */
int get_opcode(int inst) {
        return inst / MEM_SIZE;
}

int get_operand(int inst) {
        return inst % MEM_SIZE;
}

char * get_op_str(char * str, int opcode) {
        str[0] = '\0';
        #define o(name, op, exec) if (opcode == op) strcpy(str, #name);
        OPCODES
        #undef o
        return str;
}

