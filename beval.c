#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

#define NUM_SIZE 25
#define STR_SIZE 30
#define CURR(line, i) (line[i])
#define NEXT(line, i) (i + 1 < line_len ? line[i + 1] : 0)
#define PREV(line, i) (line_len > 0 && i - 1 >= 0 ? line[i - 1] : 0)

enum token_type
{
    tok_undefined,
    tok_int,
    tok_flt,
    tok_str,
    tok_add,
    tok_sub,
    tok_mul,
    tok_div,
    tok_mod,
    tok_exp,
    tok_lpar,
    tok_rpar,
    tok_comma,
    tok_eof,
    _tok_end
};

typedef struct
{
    uint8_t type;
    int col;
    char *data;
} tok_t;

typedef struct
{
    tok_t *tokens;
    int toks_num;
} toks_t;

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
    "LPO",
    "RPO",
    "CMA",
    "EOF",
};

bool line_failed = false, debug_mode = false;

static bool is_valid_num_char(char chr)
{
    return (chr >= '0' && chr <= '9') || chr == '.' || chr == '-' || chr == '+';
}

void free_tokens(toks_t *toks);

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

void set_token(toks_t *tokens, char *data, uint8_t type, int col)
{
    tokens->toks_num++;
    tokens->tokens = realloc(tokens->tokens, sizeof(tok_t) * tokens->toks_num);
    tokens->tokens[tokens->toks_num - 1] = (tok_t){
        .data = data,
        .type = type,
        .col = col
    };
}

toks_t *tokenize_line(const char *line)
{
    const int line_len = strlen(line);
    int i = 0, px = 0;
    line_failed = false;
    toks_t *ret = malloc(sizeof(toks_t));
    ret->toks_num = 0;
    ret->tokens = malloc(0);

    while (i < line_len)
    {
        if (isspace(CURR(line, i))) { i++; continue; }
        
        if (CURR(line, i) == '#') break;

        if (isdigit(CURR(line, i)))
        {
            uint8_t ntype = tok_int;
	        px = i;
            char *num = get_number(line, &i, &ntype, line_len);
	        set_token(ret, num, ntype, px);
	        continue;
	    }

        if (isalpha(CURR(line, i)) || CURR(line, i) == '_')
        {
            px = i;
            char *string = get_string(line, &i, line_len);
            set_token(ret, string, tok_str, px);
            continue;
        }

        px = i;
        uint8_t op_type = get_operator(line, &i);
        char *op_str = "<o/>";
        set_token(ret, op_str, op_type, px);

        if (line_failed)
        {
            free_tokens(ret);
            return NULL;
        }

        i++;
    }

    return ret;
}

/* Uses PEMDAS (Parenthesis, exponents, multiplication, division, addition, subtraction) */
void parse_line(toks_t *toks)
{
    toks = toks;
}

void print_tokens(toks_t *tokens)
{
    printf("Tokens:\n"); 
    for (int i = 0; i < tokens->toks_num; i++)
    {
        printf("ID: %d	Type: %s    Linep: [%d]     Symbol: %s\n",
            i,
            token_strs[tokens->tokens[i].type],
            tokens->tokens[i].col,
            tokens->tokens[i].data);
    }
}

void free_tokens(toks_t *toks)
{
    if (toks->toks_num < 0)
	    for (int i = 0; i < toks->toks_num; i++)
	        free(toks->tokens[i].data);

    free(toks->tokens);
    free(toks);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcmp(argv[1], "--debug"))
        debug_mode = true;

    while (1)
    {
        char *line = readline("-> ");
        toks_t *ltoks = tokenize_line(line);	
        if (ltoks != NULL)
        {
            if (debug_mode) print_tokens(ltoks);
            parse_line(ltoks);
            free_tokens(ltoks);
            free(line);
        }
    }
}