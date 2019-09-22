#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sml_opcodes.h"

/**
 * Simpletron Machine Language (SML) simulator
 *
 * A solo project by Alexander Frederiksen
 */


#define MEM_SIZE 100

/* define memory map */
int memory[MEM_SIZE];
int inst_counter = 0;
int accum = 0;

/* returns 1 stopped */
int exec_inst(int inst) {
	int opcode = get_opcode(inst);
	int operand = get_operand(inst);
	
	/* create if statement list of opcodes */
	#define o(name, op, exec) if (opcode == op) { exec }
	OPCODES
	#undef o

	return 0;
}

void run_program() {
	int run_state = 0;
	inst_counter = 0;
	accum = 0;
	do {
		/* run instruction */
		run_state = exec_inst(memory[inst_counter]);
		/* go to next instruction */
		inst_counter++;
	} while (!run_state && inst_counter < MEM_SIZE);
}

void check_program() {

}

int load_program(char * filename) {
	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		printf("File '%s' does not exist.\n", filename);
		return 1;
	}
	
	for (;;) {
		int mem_addr;
		int inst;
		int items = fscanf(file, " %d %d", &mem_addr, &inst);
		/* ignore side comments */
		fscanf(file, " //%*[^\n]");
		if (items < 0) break;
		if (items < 2) {
			printf("Parsing error.\n");
		}
		/* make sure its a valid mem address */
		if (mem_addr < 0 || mem_addr >= MEM_SIZE) {
			printf("Could not allocate memory address (%d).\n", mem_addr);
		} else {
			memory[mem_addr] = inst;
			char op_str[20];
			get_op_str(op_str, get_opcode(inst));
			printf("[%02d]: %+05d %20s %02d\n", mem_addr, inst, 
					op_str, get_operand(inst));
		}
	}
	fclose(file);

	/* make sure there aren't any syntax errors */
	check_program();

	return 0;
}

int main(void) {
	char filename[100];
	for (;;) {
		printf("Enter a file to run (.sml file): ");
		scanf(" %99s", filename);
		if (strcmp(filename, "q") != 0) {
			/* load the program into memory */
			printf("Loading program into memory...\n");
			int error = load_program(filename);
			if (!error) {
				/* prompt user to run program */
				printf("Run program (Y/n)? ");
				char in;
				scanf(" %c", &in);
				if (in != 'n' && in != 'N') {	
					printf("\n--- [ Program started ] ---\n");
					run_program();
					printf("--- [ Program terminated ] ---\n");
				}
			}
		} else break;
		printf("\n");

		/* take out loop because its no longer neccessary */
		break;
	}
	return 0;
}
