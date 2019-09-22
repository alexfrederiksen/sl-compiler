#pragma once

#define SYM_COUNT 100
#define FILE_SIZE 100
#define TEMP_VARS 1

/* flag for compilation failure */
extern int compile_failed;

void format_quotes(char * dst, char * src, int size);
void parse_error(char * found, char * expected, int linenum);

int is_lowercase(char * str);
int is_num(char * str);

int add_symbol(char type, int value, int address);
struct Symbol * get_symbol(int address);
int get_symbol_addr(char type, int value);

void write_inst(int opcode, int operand);
void write_data(int data);

void write_mul(int operand);
void write_div(int operand);
void write_add(int operand);
void write_sub(int operand);

char * expect_token(char * expected, int expected_len, int expect_num, int linenum);

void compile_expression(int mem_addr, int isnew, int end_of_line, int linenum);
void compile_line(char * line, int pass);
void compile(char * filename);

void write_to_file(char * filname);
