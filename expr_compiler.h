#pragma once

#define EXPR_LEN 100

struct Operator {
	char c;
	int prec;
	void (* eval)(int);
};

char * to_postfix(char * dst, char * src, struct Operator * ops, int op_count, int linenum);
void eval_expression(char * infix, struct Operator * ops, int op_count, int linenum);
