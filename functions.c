#include <math.h>
#include <stdio.h>
#include <string.h>
#include "beval.h"

const char *function_strs[] = {
    "sin",
    "cos",
    "tan"
};

const uint8_t function_args[] = {
    1,
    1,
    1
};

double torad(double degrees)
{
    return degrees * M_PI / 180;
}

double todeg(double radians)
{
    return radians * 180 / M_PI;
}

double parse_function()
{
    uint8_t function_num = -1;

    for (long unsigned int j = 0; j < sizeof(function_strs) / sizeof(const char *); j++)
        if (!strcmp(function_strs[j], tokens[pix].data))
            function_num = j;

    pix++;
    pix++;

    double farg = parse_atom();
    pix++;

    printf("%d", tokens[pix].type);

    switch (function_num)
    {
        case 0:
            return sin(farg);

        case 1:
            return cos(farg);

        case 2:
            return tan(farg);

        default:
            return 0;
    }
}