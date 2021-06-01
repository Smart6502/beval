#ifndef BEVAL_H
#define BEVAL_H
#include <stdbool.h>
#include <stdint.h>

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

double parse_function();
double parse_summands();
double to_degrees(double radians);
double to_radians(double degrees);
void free_tokens();
void charon_fail(int col, char *msg, ...);

extern int pix;
extern tok_t *tokens;
extern bool line_failed, debug_mode;

#endif
