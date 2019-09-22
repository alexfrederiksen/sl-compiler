#include "expr_compiler.h"
#include "sml_opcodes.h"
#include "sl_compiler.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
 * Expression compiler
 * A solo project by Alexander Frederiksen
 */

#define STACK_SIZE 100

int stack[STACK_SIZE];
int stack_size;

int get_prec(char c, struct Operator * ops, int count) {
	for (int i = 0; i < count; i++) {
		if ((ops + i) -> c == c)
			return (ops + i) -> prec;
	}
	return 0;
}

int eval_op(char c, int operand, struct Operator * ops, int count) {
	for (int i = 0; i < count; i++) {
		struct Operator * op = ops + i;
		if (op -> c == c) {
			/* evaluate */
			(op -> eval)(operand);
		}
	}	
}

void push_stack(int c) {
	stack[stack_size++] = c;
}

int pop_stack() {
	return stack[--stack_size];
}

int peek_stack() {
	return stack[stack_size - 1];
}

void cat_str(char * str, char c) {
	int len = strlen(str);
	str[len] = c;
	str[len + 1] = '\0';
}

void attempt_space(char * str) {
	if (str[strlen(str) - 1] != ' ') {
		cat_str(str, ' ');
	}
}

char * to_postfix(char * dst, char * src, struct Operator * ops, int op_count, int linenum) {
	dst[0] = '\0';
	/* reset the stack */
	stack_size = 0;
	int len = strlen(src);
	for (int i = 0; i < len; i++) {
		char c = src[i];
		int prec = get_prec(c, ops, op_count);
		if (c == '(') {
			/* of the highest precedence */
			push_stack(c);
		} else if (c == ')') {
			/* we can now pop to the '(' */
			while (stack_size > 0 && peek_stack() != '(') {
				attempt_space(dst);
				cat_str(dst, pop_stack());
			}
			if (stack_size <= 0) {
				parse_error(")", "(", linenum);
				return dst;
			} else pop_stack();
		} else if (prec > 0) {
			/* is an operator 
			 * pop stack until lower precedence */
			while (stack_size > 0 && peek_stack() != '(' && 
			       get_prec(peek_stack(), ops, op_count) >= prec) {
				attempt_space(dst);
				cat_str(dst, pop_stack());
			}
			/* then push operator to stack */
			push_stack(c);
		} else {
			/* not an operator */
			if (c != ' ') {
				cat_str(dst, c);
			} else attempt_space(dst);
		}
	}
	while (stack_size > 0) {
		attempt_space(dst);
		cat_str(dst, pop_stack());
	}
}

void eval_expression(char * infix, struct Operator * ops, int op_count, int linenum) {
	/* convert infix to postfix form */
	char postfix[EXPR_LEN];
	to_postfix(postfix, infix, ops, op_count, linenum);
	if (compile_failed) return;
	//printf("[DEBUG]: postfix of '%s': '%s'\n", infix, postfix);
	/* parse postfix 
	 * reset stack */
	stack_size = 0;
	char * token = strtok(postfix, " ");
	while (token != NULL) {
		char c = token[0];
		int len = strlen(token);
		if (get_prec(c, ops, op_count) == 0) {
			/* is an operand, get address */
			int addr;
			if (is_num(token)) {
				/* constant */
				addr = get_symbol_addr('C', atoi(token));
			} else if (isalpha(c) && len == 1) {
				/* variable */
				addr = get_symbol_addr('V', c);
			} else {
				parse_error(token, "@valid operand", linenum);
				return;
			}
			push_stack(addr);
		} else {
			/* is an operator */
			if (stack_size < 2) {
				parse_error(NULL, "@two operands", linenum);
				return;
			}
			int o2 = pop_stack();
			int o1 = pop_stack();
			/* call callback */
			if (o1 == -1) {
				/* first operand in accum */
				
			} else {
				if (o2 == -1) {
					/* use allocated temp address */
					write_inst(SML_STORE, MEM_SIZE - 1);
					o2 = MEM_SIZE - 1;
				}
				/* load first operand into accum */
				write_inst(SML_LOAD, o1);
			}
			/* write custom instructions for op */
			eval_op(c, o2, ops, op_count);
			/* push accum value (optimization instead of using temps) */
			push_stack(-1);
		}
		/* get next token */
		token = strtok(NULL, " ");
	}

	/* if one operand token remains (not the accum), it is a simple assignment */
	if (stack_size == 1 && peek_stack() != -1) {
		/* load operand to accum */
		write_inst(SML_LOAD, pop_stack());
	}
}


