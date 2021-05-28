#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include "beval.h"

#define NUM_SIZE 25
#define STR_SIZE 30
#define CURR(line, i) (line[i])
#define NEXT(line, i) (i + 1 < line_len ? line[i + 1] : 0)
#define PREV(line, i) (line_len > 0 && i - 1 >= 0 ? line[i - 1] : 0)

tok_t *tokens = NULL;
int token_num = 0;
int pix = 0, parc = 0;
bool line_failed = false, debug_mode = false;

const char *token_strs[_tok_end] = {
    "< >",
    "INT",
    "FLT",
    "STR",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "EXP",
    "LPA",
    "RPA",
    "CMA",
    "EOF",
};

static bool is_valid_num_char(char chr)
{
    return (chr >= '0' && chr <= '9') || chr == '.' || chr == '-' || chr == '+';
}

void charon_fail(int col, char *msg, ...)
{
    fprintf(stderr, "Error at column %d: ", col);
    va_list vargs;
    va_start(vargs, msg);
    vfprintf(stderr, msg, vargs);
    fprintf(stderr , "\n");
    va_end(vargs);
    line_failed = true;
}

char *get_number(const char *line, int *i, uint8_t *ntype, const int line_len)
{
    char *number = malloc(NUM_SIZE);
    int index = 0, fda = *i, dot_encountered = -1;
    bool success = true;
    memset(number, '\0', NUM_SIZE);

    while (*i < line_len && is_valid_num_char(CURR(line, *i)))
    {
        const char c = CURR(line, *i);
	    const char next = NEXT(line, *i);

        switch (c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                break;

	    case '.':
            if (dot_encountered == -1)
            {
                if (next >= '0' && next <= '9')
                {
                dot_encountered = *i;
                *ntype = tok_flt;
                }
                else
                return number;
            }
            else
            {
                charon_fail(*i, "encountered (.) character (num_retrieval :%d), already had one at %d", fda + 1, dot_encountered + 1);
                success = false;
            }
		break;

	    default:
            if (isalnum(CURR(line, *i)))
            {
                charon_fail(*i, "encountered invalid character (num_retrieval :%d) %c", c, fda + 1);
                success = false;
            }
            else
                return number;		

        }
        if (success == false)
        {
            free(number);
            return NULL;
        }

        number[index++] = c;
        (*i)++;
    }

    return number;
}

char *get_string(const char *line, int *i, int line_len)
{
    int faa = *i;
    char *name = malloc(STR_SIZE);
    memset(name, '\0', STR_SIZE);

    while (*i < line_len)
    {
        if (!isalnum(CURR(line, *i)) && CURR(line, *i) != '_') break;
        name[*i - faa] = line[*i];

        (*i)++;
    }

    return name;
}

uint8_t get_operator(const char *line, int *i)
{
    uint8_t op_type = tok_undefined;
    
    switch (CURR(line, *i))
    {
        case '+':
            op_type = tok_add;
            break;

        case '-':
            op_type = tok_sub;
            break;

        case '*':
            op_type = tok_mul;
            break;

        case '/':
            op_type = tok_div;
            break;

        case '%':
            op_type = tok_mod;
            break;

        case '^':
            op_type = tok_exp;
            break;

        case '(':
            op_type = tok_lpar;
            break;

        case ')':
            op_type = tok_rpar;
            break;

        case ',':
            op_type = tok_comma;
            break;

        case '\0':
            op_type = tok_eof;
            break;

        default:
            charon_fail(*i, "unknown operator %c", CURR(line, *i));
            op_type = tok_undefined;
    }

    return op_type;
}

void set_token(char *data, uint8_t type, int col)
{
    token_num++;
    tokens = realloc(tokens, sizeof(tok_t) * token_num);
    tokens[token_num - 1] = (tok_t){
        .data = data,
        .type = type,
        .col = col
    };
}

void tokenize_line(const char *line)
{
    const int line_len = strlen(line);
    int i = 0, px = 0;
    line_failed = false;
    token_num = 0;
    tokens = malloc(0);

    while (i < line_len)
    {
        if (isspace(CURR(line, i))) { i++; continue; }
        
        if (CURR(line, i) == '#') break;

        if (isdigit(CURR(line, i)))
        {
            uint8_t ntype = tok_int;
	        px = i;
            char *num = get_number(line, &i, &ntype, line_len);
	        set_token(num, ntype, px);
	        continue;
	    }

        if (isalpha(CURR(line, i)) || CURR(line, i) == '_')
        {
            px = i;
            char *string = get_string(line, &i, line_len);
            set_token(string, tok_str, px);
            continue;
        }

        px = i;
        uint8_t op_type = get_operator(line, &i);
        char *op_str = "<o/>";
        set_token(op_str, op_type, px);

        if (line_failed)
        {
            free_tokens();
        }

        i++;
    }
}

double parse_atom()
{
    bool below_zero = false;
    double atom = 0;
    
    if (tokens[pix].type == tok_sub)
    {
	below_zero = true;
	pix++;
    }
    
    if (tokens[pix].type == tok_add)
	pix++;

    if (tokens[pix].type == tok_int || tokens[pix].type == tok_flt)
    {
    	atom = atof(tokens[pix].data);
    	pix++;
    }

    if (tokens[pix].type == tok_str)
    {
        atom = parse_function();
    }

    return below_zero ? -atom : atom;
}

double parse_facts()
{
    double num0 = parse_atom();
    if (debug_mode) printf("parse_facts: num0: %g\n", num0);

    for (;;)
    {
	uint8_t oper = tokens[pix].type;
	if (debug_mode) printf("parse_facts: <o/>: %d\n", oper);

	if (oper != tok_mul && oper != tok_div)
	    return num0;

	pix++;

	double num1 = parse_atom();
	if (oper == tok_div)
	{
	    if (num1 == 0)
	    {
		charon_fail(pix, "Division by zero");
		return 0;
	    }
	    num0 /= num1;
	}
	else
	    num0 *= num1;

	if (debug_mode) printf("parse_facts: <o/>: num0: %g\n", num0);
    }
}

double parse_summands()
{
    double num0 = parse_facts();
    if (debug_mode) printf("parse_summands: num0: %g\n", num0);
    
    for (;;)
    {
	uint8_t oper = tokens[pix].type;
	if (debug_mode) printf("parse_summands: <o/>: %d\n", oper);
	
	if (oper != tok_sub && oper != tok_add)
	    return num0;

	pix++;

	double num1 = parse_facts();
	if (oper == tok_sub)
	    num0 -= num1;
	else
	    num0 += num1;

	if (debug_mode) printf("parse_summands: <o/>: num0: %g\n", num0);
    }
}

/* Uses PEMDAS (Parenthesis, exponents, multiplication, division, addition, subtraction) */
void parse_line()
{
    if (!strcmp(tokens[0].data, "exit"))
    {
        free_tokens();
        exit(0);
    }

    double result = parse_summands();
    printf("	%g\n", result);
}

void print_tokens()
{
    if (token_num)
    {
	printf("Tokens:\n"); 
    
    	for (int i = 0; i < token_num; i++)
    	{
            printf("ID: %d	Type: %s (%d)    Linep: [%d]	Symbol: %s\n",
            	i,
            	token_strs[tokens[i].type],
		tokens[i].type,
            	tokens[i].col,
            	tokens[i].data);
    	}
     }
   
     putchar('\n');
}

void free_tokens()
{
    if (token_num < 0)
	    for (int i = 0; i < token_num; i++)
	        free(tokens[i].data);

    free(tokens);

    token_num = 0;
    pix = 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcmp(argv[1], "--debug"))
        debug_mode = true;

    while (1)
    {
        char *line = readline("\033[1;35m-> \033[0;0m");
        tokenize_line(line);	
        if (!line_failed)
        {
            if (debug_mode)
                print_tokens();
            
            if (token_num > 0)
                parse_line();
            
            free_tokens();
            free(line);
        }
    }
}