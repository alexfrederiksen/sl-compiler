#include "sl_compiler.h"
#include "expr_compiler.h" /* for compiling expressions */
#include "sml_opcodes.h"   /* for SML opcodes           */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
 * Simple BASIC Language Compiler
 * A solo project by Alexander Frederiksen
 */

/* flag for compilation failure */
int compile_failed = 0;

void format_quotes(char * dst, char * src, int size) {
	if (src[0] == '@') {
                /* no quotes */
                strcpy(dst, src + 1);
		//dst[strlen(src) - 1] = '\0';
        } else {
                /* add quotes */
		sprintf(dst, "'%s'", src);
		
        }
}

void parse_error(char * found, char * expected, int linenum) {
	/* add quotes if string doesn't start with '@' */
        static char found_str[100];
        static char expected_str[100];
        if (found != NULL) format_quotes(found_str, found, 100);
        if (expected != NULL) format_quotes(expected_str, expected, 100);
	if (found == NULL) {
                printf("[Parsing Error] Line %02d : expected %s token.\n", linenum, expected_str);
        } else if (expected != NULL) {
                printf("[Parsing Error] Line %02d : found %s token, expected %s token.\n", linenum, found_str, expected_str);
        } else {
                printf("[Parsing Error] Line %02d : unparsable.\n", linenum);
        }

        compile_failed = 1;
}


/* expression operators (allows for expansion of ops) */
struct Operator expr_ops[] = {
	{ '*', 2, write_mul },
	{ '/', 2, write_div },
	{ '+', 1, write_add },
	{ '-', 1, write_sub }
};

/* count of operators */
int expr_op_count = 4;

/* map of symbols */
struct Symbol {
	/* Constant:    'C'
	 * Line Number: 'L'
	 * Variable:    'V' */
	char type;
	/* value can be the constant, the line
	 * number or the variable character */
	int value;
	/* place in actual memory it will reside */
	int address;
} symbols[SYM_COUNT];

/* count for symbols */
int symbol_count = 0;

/* SML memory */
int memory[MEM_SIZE] = { 0 };
/* flags for second pass (initialized in main) */
int flags[FILE_SIZE];

/* counts for symbol placement */
int fore_count = 0;
int back_count = MEM_SIZE - TEMP_VARS - 1;

/* checks if entire string is lowercase */
int is_lowercase(char * str) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (!islower(str[i])) return 0;
	}
	return 1;
}

int is_num(char * str) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (!isdigit(str[i])) return 0;
	}
	return 1;
}

char * consume_str(char * str, int (* func)(int)) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		str[i] = func(str[i]);
	}
	return str;
}

char * to_lower(char * str) {
	return consume_str(str, tolower);
}

int add_symbol(char type, int value, int address) {
	struct Symbol * symbol = symbols + symbol_count;
	symbol -> type = type;
	symbol -> value = value;
	symbol -> address = address;
	symbol_count++;
	return address;
}

struct Symbol * get_symbol(int address) {
	for (int i = 0; i < symbol_count; i++) {
		if ((symbols + i) -> address == address) {
			return symbols + i;
		}
	}
	return NULL;
}

int get_symbol_addr(char type, int value) {
	/* check if symbol exists */
	for (int i = 0; i < symbol_count; i++) {
		struct Symbol symbol = symbols[i];
		if (symbol.type == type &&
		    symbol.value == value) {
			return symbol.address;
		}
	}

	/* create a new one */
	if (type == 'L') {
		/* should load on next pass */
		return -1;
	} else if (type == 'C' || type == 'V') {
		int addr = add_symbol(type, value, back_count);
		if (type == 'C') {
			/* write constant value */
			write_data(value);
		} else {
			/* default to zero for variables */
			write_data(0);
		}
		return addr;
	} else {
		printf("Invalid symbol type '%c'.\n", type);
		return 0;
	}
}

/* writes instructions to 'memory' using 'fore_count' */
void write_inst(int opcode, int operand) {
	if (back_count <= fore_count) {
		/* overflow problem */
		printf("Error: memory mapping overflow.\n");
		compile_failed = 2;
		return;
	}
	memory[fore_count++] = opcode * 100 + operand;
}

/* writed data to 'memory' using 'back_count' */
void write_data(int data) {
	if (back_count <= fore_count) {
		/* overflow problem */
		printf("Error: memory mapping overflow.\n");
		compile_failed = 2;
		return;
	}
	memory[back_count--] = data;
}

/* --- expression operations --- 
 * for each, assume the first operand is already 
 * in the accumulator and will leave the result in 
 * the accumulator. */

void write_mul(int operand) {
	write_inst(SML_MUL, operand);
}

void write_div(int operand) {
	write_inst(SML_DIV, operand);	
}

void write_add(int operand) {
	write_inst(SML_ADD, operand);
}

void write_sub(int operand) {
	write_inst(SML_SUB, operand);
}

void break_returns(char * token) {
	int len = strlen(token);
	/* break off an carriage returns */
	for (int i = 0; i < len; i++) {
		if (token[i] == '\n') {
			token[i] = '\0';
			break;
		}
	}

}

char * expect_token(char * expected, int expected_len, int expect_num, int linenum) {
	char * tok = strtok(NULL, " ");
	if (tok == NULL || (expected != NULL && strcmp(to_lower(tok), expected) != 0)) {
		if (expected != NULL) {
			/* expecting something specific */
			parse_error(tok, expected, linenum);
			return NULL;
		} else {
			/* just expecting something */
			parse_error(NULL, "@another", linenum);
			return NULL;
		}
	}

	/* if something was specifically expected,
	 * then we already have it */
	if (expected != NULL) return tok;

	/* expect a specific length */
	if (expected_len > 0 && strlen(tok) != expected_len) {
		char tmp_str[20] = "";
		sprintf(tmp_str, "@length %d", expected_len);
		parse_error(tok, tmp_str, linenum);
		return NULL;
	}

	int isnum = is_num(tok);
	/* expect a lowercase token */
	/*
	if (!isnum && !is_lowercase(tok)) {
		parse_error(tok, "@lowercase", linenum);
		return NULL;
	} */

	/* expect number */
	if (expect_num == 1 && !isnum) {
		parse_error(tok, "@numeral", linenum);
		return NULL;
	}

	/* expect nonnumber (prevent some bs) */
	if (expect_num == 2 && isnum) {
		parse_error(tok, "@nonnumeral", linenum);
		return NULL;
	}



	return tok;
}

#define EXPECT_TOKEN(p1, p2, p3, p4)  \
	expect_token(p1, p2, p3, p4); \
	if (compile_failed) return;
	

/* compiles an expression that is being tokenized */
void compile_expression(int mem_addr, int isnew, int token_size, int linenum) {
	char * token = strtok(NULL, " ");
	/* remove null char to get rest of string (if not the last one) */
	int null_pos = strlen(token);
	if (null_pos < token_size) {
		token[null_pos] = ' ';
	} else if (is_num(token) && isnew) {
		/* one token, must not be an expression, 
		 * constant assignment, for the first time */
		memory[mem_addr] = atoi(token);
		return;
	}
	/* evaluate that expression */
	eval_expression(token, expr_ops, expr_op_count, linenum);
	/* store result in 'mem_addr' */
	write_inst(SML_STORE, mem_addr);
}

void compile_line(char * line, int pass) {
	break_returns(line);
	/* record null char pos */
	int end_of_line = strlen(line);
	/* tokenize line */
	char * linenum_str = strtok(line, " ");
	/* ignore blank lines */
	if (linenum_str == NULL) return;

	int linenum;
	if (is_num(linenum_str)) {
		linenum = atoi(linenum_str);
		if (linenum > FILE_SIZE) {
			if (pass == 1) parse_error(NULL, "@line number in bounds", linenum);
			return;
		}
	} else {
		if (pass == 1) parse_error(linenum_str, "@line number", -1);
		return;
	}
	/* add line symbol on first pass */
	if (pass == 1) add_symbol('L', linenum, fore_count);
	else if (flags[linenum] != -1) {
		/* allow second pass for 'goto' commands */
		fore_count = flags[linenum];
	} else return;

	//if (pass == 2) printf("Line %d flagged for pass 2.\n", linenum);

	/* get command */
	char * command = EXPECT_TOKEN(NULL, 0, 0, linenum);
	/* allow lines without commands */
	if (command == NULL) return;
	/* turn to lowercase */
	to_lower(command);

	/* --- parse commands --- */

	/* REM command */
	if (strcmp(command, "rem") == 0) {
		/* just do nothing */
	/* INPUT command */
	} else if (strcmp(command, "input") == 0) {
		char * var_name = EXPECT_TOKEN(NULL, 1, 2, linenum);
		write_inst(SML_READ, get_symbol_addr('V', var_name[0]));
	/* LET command */
	} else if (strcmp(command, "let") == 0) {
		/* test if a new symbol was created */
		int old_symbol_count = symbol_count;
		char * var_name = EXPECT_TOKEN(NULL, 1, 2, linenum);
		int var_addr = get_symbol_addr('V', var_name[0]);
		char * equalsign = EXPECT_TOKEN("=", 0, 0, linenum);
		compile_expression(var_addr, symbol_count > old_symbol_count, 
				end_of_line - (equalsign - line) - strlen(equalsign) - 1, linenum);
	/* PRINT command */
	} else if (strcmp(command, "print") == 0) {
		char * var_name = EXPECT_TOKEN(NULL, 1, 2, linenum);
		write_inst(SML_WRITE, get_symbol_addr('V', var_name[0]));
	/* GOTO command */
	} else if (strcmp(command, "goto") == 0) {
		char * label = EXPECT_TOKEN(NULL, 0, 1, linenum);
		int label_addr = get_symbol_addr('L', atoi(label));
		if (label_addr == -1) {
			if (pass == 1) {
				/* add flag here and make sure there's room for instruction */
				flags[linenum] = fore_count++;
			} else printf("We somehow missed a line on the first pass.\n");
		} else {
			write_inst(SML_BRANCH, label_addr);
		}
	/* IF command */
	} else if (strcmp(command, "if") == 0) {
		char * var1_name = EXPECT_TOKEN(NULL, 1, 2, linenum);
		EXPECT_TOKEN("==", 0, 0, linenum);
		char * var2_name = EXPECT_TOKEN(NULL, 1, 2, linenum);
		EXPECT_TOKEN("goto", 0, 0, linenum);
		char * label = EXPECT_TOKEN(NULL, 0, 1, linenum);
		if (pass == 1) {
			/* write comparing logic on first pass */
			int var1_addr = get_symbol_addr('V', var1_name[0]);
			int var2_addr = get_symbol_addr('V', var2_name[0]);
			write_inst(SML_LOAD, var1_addr);
			write_inst(SML_SUB, var2_addr); 
		}
		/* write branch instruction */
		int label_addr = get_symbol_addr('L', atoi(label));
		if (label_addr == -1) {
			if (pass == 1) {
				/* add flag here and make sure there's room for instruction */
				flags[linenum] = fore_count++;
			} else printf("We somehow missed a line on the first pass.\n");
		} else {
			write_inst(SML_BRANCH_ZERO, label_addr);
		}
	/* END command */
	} else if (strcmp(command, "end") == 0) {
		write_inst(SML_HALT, 0);

	} else {
		/* some unknown command */
		parse_error(command, "@valid command", linenum);
		return;
	}
}

void compile(char * filename) {
	FILE * infile = fopen(filename, "r");
	if (infile == NULL) {
		printf("File '%s' doesn't exist.\n", filename);
		compile_failed = 1;
		return;	
	}

	/* do two passes over the file */
	int pass = 1;
	for (;;) {
		char * line;
		size_t len;
		int status = getline(&line, &len, infile); 
		if (status >= 0) {
			int old_fore_count = fore_count;
			/* attempt to keep parsing if compile_failed */
			int old_fail = compile_failed;
			compile_failed = 0;
			compile_line(line, pass);
			compile_failed = (old_fail > compile_failed) ? old_fail : compile_failed;
			/* restore the fore_count on second pass (new instructions should
			 * not be introduced) */
			if (pass == 2) fore_count = old_fore_count;
		} else if (pass == 1) {
			pass++;
			/* reset cursor to beginning of file and do second pass */
			fseek(infile, 0, SEEK_SET);	
		} else break;
	}
	fclose(infile);
}

void write_to_file(char * filename) {
	FILE * file = fopen(filename, "w");
	char op_name[20];
	for (int i = 0; i < MEM_SIZE; i++) {
		if (i < fore_count || i > back_count) {
			fprintf(file, "%02d %+05d", i, memory[i]);
			struct Symbol * symbol = get_symbol(i);
			if (symbol != NULL && symbol -> type != 'L') {
				if (symbol -> type == 'C') {
					fprintf(file, " // %20s %d", "CONSTANT", symbol -> value);
				} else {
					fprintf(file, " // %20s %c", "VARIABLE", symbol -> value);
				}
			} else {
				int opcode = get_opcode(memory[i]);
				int operand = get_operand(memory[i]);
				get_op_str(op_name, opcode);
				if (strlen(op_name) > 0) {
					fprintf(file, " // %20s %02d", op_name, operand);
				}
			}
			fprintf(file, "\n");
		}
	}
	fclose(file);	
}

void print_memory() {
	char op_name[20];
	printf("ADDR  INST            OPERATION OPERAND\n");
	printf("-----------------------------------------------\n");
	for (int i = 0; i < MEM_SIZE; i++) {
		/* print out only allocated sections of memory */
		if (i < fore_count || i > back_count) {
			printf("[%02d]: %+05d ", i, memory[i]);
			struct Symbol * symbol = get_symbol(i);
			if (symbol != NULL && symbol -> type != 'L') {
				if (symbol -> type == 'C') {
					printf("%20s %d", "CONSTANT", symbol -> value);
				} else {
					printf("%20s %c", "VARIABLE", symbol -> value);
				}
			} else if (MEM_SIZE - i <= TEMP_VARS) {
				printf("%20s %d", "TEMP VAR", MEM_SIZE - i);
			} else {
				int opcode = get_opcode(memory[i]);
				int operand = get_operand(memory[i]);
				get_op_str(op_name, opcode);
				printf("%20s %02d", op_name, operand);
				/* print symbol (other than line) */
			}
			printf("\n");
		} else if (i == fore_count) {
			printf("                .\n");
			printf("                .\n");
		}
	}
}

int main(void) {
	/* fill flags with -1 */
	for (int i = 0; i < FILE_SIZE; i++) flags[i] = -1;
	//print_memory();
	
	char filename[100];
	printf("Enter a file to compile (.sl file): ");
	scanf(" %99s", filename);
	printf("Compiling...\n");
	compile(filename);
	
	if (compile_failed) {
		printf("Compilation failed.\n");
		return 0;
	} else printf("Compilation success.\n");

	printf("\n    --- [ SML Translation ] ---\n");
	print_memory();	
	printf("Write to file (y/N)? ");
	char write;
	scanf(" %c", &write);
	if (write != 'N' && write != 'n') {
		/* write memory to file */
		char * dot = strchr(filename, '.');
		strcpy(dot + 1, "sml");
		printf("Writing to %s... ", filename);
		write_to_file(filename);
		printf("Done.\n");
	}	
	return 0;
}
