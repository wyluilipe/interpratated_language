#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum type_of_lexeme {
	operator_lex,
	operation_lex,
	function_lex,
	var_lex,
	key_lex,
	number_lex,
	bracket_lex,
	end_lex,
	err_lex,
};

bool isnumber(const char *str)
{
	int i = 0;
	while (str[i] != '\0')
		if ((str[i] < '0') || (str[i++] > '9'))
			return 0;
	return 1;
}

bool isname(const char *str)
{
	int i = 0;
	while (str[i] != '\0')
		if ((str[i] >= 'a') & (str[i] <= 'z') || (str[i] >= 'A') & (str[i] <= 'Z'))
			return 0;
	return 1;
}

bool cmprstrings(const char *str1, const char *str2)
{
	int res = 0, i = 0;
	if (strlen(str1) == strlen(str2)) {
		while (str1[i] != '\0') {
			if (str1[i] == str2[i])
				res = 1;
			else 
				return 0;
			i++;
		}
	}
	return res;
}

class Lexeme {
	type_of_lexeme type;
	char *value;
	int line;
public:
	Lexeme(int line_, const char *value_) : line(line_) {
		value = new char[strlen(value_) + 1];
		strcpy(value, value_);
		type = WhatType();
	}
	char *GetValue() {return value;}
	type_of_lexeme WhatType();
	type_of_lexeme GetType() {return type;}
	void PrintLexeme();
};

type_of_lexeme Lexeme::WhatType()
{
	if ((cmprstrings(value, ";")) || (cmprstrings(value, "=")))
		return operator_lex;
	else if ((cmprstrings(value, "+")) || (cmprstrings(value, "-")) || (cmprstrings(value, "/")) || (cmprstrings(value, "*")) || (cmprstrings(value, "&")) || (cmprstrings(value, "||")) || (cmprstrings(value, ">")) || (cmprstrings(value, "<")) || (cmprstrings(value, "<=")) || (cmprstrings(value, ">=")))
		return operation_lex;
	else if (value[0] == '?')
		return function_lex;
	else if (value[0] == '$')
		return var_lex;
	else if ((cmprstrings(value, "if")) || (cmprstrings(value, "else"))  || (cmprstrings(value, "while")) || (cmprstrings(value, "do")) || (cmprstrings(value, "var")))
		return key_lex;
	else if (isnumber(value))
		return number_lex;
	else if ((cmprstrings(value, "(")) || (cmprstrings(value, ")")) || (cmprstrings(value, "{")) || (cmprstrings(value, "}")) || (cmprstrings(value, "[")) || (cmprstrings(value, "]")) || (cmprstrings(value, ",")))
		return bracket_lex;
	else if (cmprstrings(value, "}end"))
		return end_lex;
	else
		return err_lex;
}

void Lexeme::PrintLexeme()
{
	printf("%d___ %s ___", line, value);
	if (type == operator_lex)
		printf("operator_lex\n");
	else if (type == operation_lex)
		printf("operation_lex\n");
	else if (type == function_lex)
		printf("function_lex\n");
	else if (type == var_lex)
		printf("var_lex\n");
	else if (type == key_lex)
		printf("key_lex\n");
	else if (type == number_lex)
		printf("number_lex\n");
	else if (type == bracket_lex)
		printf("bracket_lex\n");
	else if (type == end_lex)
		printf("end_lex\n");
	else 
		printf("err_lex\n");
}

class Analyzer {
	Lexeme *current;
	enum state {H, S, A, C, E, ACT/*5*/, CYCL, FUNC, CONDIT, CONDIT1, OPERATION/*10*/, PARAM, EL, OP, OP1, ERR,} CS;
public:
	Analyzer() : CS(H) {}
	void step(Lexeme cur);
	bool check();
};

void Analyzer::step(Lexeme cur)
{
	switch(CS) {
		case H:
			if (cmprstrings(cur.GetValue(), "var"))
				CS = A;
			else if (cmprstrings(cur.GetValue(), "{"))
				CS = ACT;
			else
				CS = ERR;
			break;
		case A:
			if (cur.GetValue()[0] == '$')
				CS = C;
			else
				CS = ERR;
			break;
		case C:
			if (('A' <= cur.GetValue()[0]) & (cur.GetValue()[0]<= 'Z') || ('a' <= cur.GetValue()[0]) & (cur.GetValue()[0]<= 'z'))
				CS = C;
			else if (cmprstrings(cur.GetValue(), ","))
				CS = A;
			else if (cmprstrings(cur.GetValue(), ";"))
				CS = H;
			else
				CS = ERR;
			break;
		case ACT:
			if (cmprstrings(cur.GetValue(), "if"))
				CS = CYCL;
			else if (cmprstrings(cur.GetValue(), "while"))
				CS = CYCL;
			else if (cur.GetValue()[0] == '?')
				CS = FUNC;
			else if (cmprstrings(cur.GetValue(), "}"))
				CS = ACT;
			else if (cmprstrings(cur.GetValue(), "}end"))
				CS = S;
			else if (cur.GetType() == var_lex)
				CS = OPERATION;
			else
				CS = ERR;
			break;
		case CYCL:
			if (cmprstrings(cur.GetValue(), "("))
				CS = CONDIT;
			else
				CS = ERR;
			break;
		case CONDIT:
			if ((cur.GetValue()[0] == '$') || (cur.GetType()) == number_lex)
				CS = CONDIT1;
			else
				CS = ERR;
			break;
		case CONDIT1:
			if (cur.GetType() == operation_lex)
				CS = CONDIT;
			else if (cmprstrings(cur.GetValue(), ")"))
				CS = E;
			else
				CS = ERR;
			break;
		case E:
			if (cmprstrings(cur.GetValue(), "{"))
				CS = ACT;
			else 
				CS = ERR;
			break;
		case FUNC:
			if (cmprstrings(cur.GetValue(), "("))
				CS = PARAM;
			else
				CS = ERR;
			break;
		case PARAM:
			if (cmprstrings(cur.GetValue(), ")"))
				CS = EL;
			else if ((cur.GetType() == number_lex) || (cur.GetType() == var_lex))
				CS = PARAM;
			else
				CS = PARAM;
			break;
		case EL:
			if ((cmprstrings(cur.GetValue(), ";")))
				CS = ACT;
			else
				CS = ERR;
			break;
		case OPERATION:
			if (cmprstrings(cur.GetValue(), "="))
				CS = OP;
			else
				CS = ERR;
			break;
		case OP:
			if ((cur.GetType() == var_lex) || (cur.GetType() == number_lex))
				CS = OP1;
			else if (cur.GetType() == function_lex)
				CS = FUNC;
			else
				CS = ERR;
			break;
		case OP1:
			if (cur.GetType() == operation_lex)
				CS = OP;
			else if (cmprstrings(cur.GetValue(), ";"))
				CS = ACT;
			else
				CS = ERR;
			break;
		case S:
			break;
		case ERR:
			break;
	}
	printf("[%d]\n", CS);
}

bool Analyzer::check()
{
	return (CS == S);
}

char *make_buff_bigger(char *buff)
{
	int len = strlen(buff);
	char *new_buff;
	new_buff = new char [(len + 1) * 2];
	strcpy(new_buff, buff);
	buff = new_buff;
	return buff;
}

char *add_letter_to_buff(char *buff, char c, long unsigned int j)
{
	if (j == strlen(buff) - 1)
		buff = make_buff_bigger(buff);
	buff[j] = c;
	return buff;
}

char *make_buff_zero(char *buff, int i)
{
	for (int j = 0; j <= i; j++)
		buff[j] = '\0';
	return buff; 
}

void run(FILE *f)
{
	int c, line = 1, n = 8, i = 0;
	Analyzer machine;
	char *buff;
	buff = new char[n];
	buff = make_buff_zero(buff, n);
	while ((c = fgetc(f)) != EOF) {
		if ((c == '\n') || (c == ' ') || (c == '\t')) {
			if (c == '\n')
				line++;
			if (buff[0] != '\0') {
				machine.step(Lexeme(line, buff));
				(Lexeme(line, buff)).PrintLexeme();
			}
			buff = make_buff_zero(buff, i);
			i = 0;
		} else if ((c == ',') || (c == ';') || (c == '(') || (c == ')') || (c == '{') || (c == '>') || (c == '<') || (c == '=')) {
			if (buff[0] != '\0') {
				machine.step(Lexeme(line, buff));
				(Lexeme(line, buff)).PrintLexeme();
			}
			buff = make_buff_zero(buff, i);
			i = 0;
			buff = add_letter_to_buff(buff, c, i);
			machine.step(Lexeme(line, buff));
			(Lexeme(line, buff)).PrintLexeme();
			buff = make_buff_zero(buff, i);
			i = 0;
		} else if (c == '}') {
			buff = add_letter_to_buff(buff, c, i++);
		} else {
			if (i == n - 1)
				n *= 2;
			buff = add_letter_to_buff(buff, c, i++);
		}
	}
	if (buff[0] != '\0') {
		machine.step(Lexeme(line, buff));
		(Lexeme(line, buff)).PrintLexeme();
	}
	if (machine.check())
		printf("YES\n");
	else
		printf("NO\n");
}

int main(int argc, char **argv)
{
	FILE *fp;
	if ((fp = fopen(argv[1], "ra")) == NULL) {
		perror("fopen");
		exit(1);
	}
	run(fp);
	fclose(fp);
	return 0;
}