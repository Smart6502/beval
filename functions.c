#include <math.h>
#include <stdio.h>
#include <string.h>
#include "beval.h"

const char *function_strs[] = {
    "sqrt",
    "log",
    "sin",
    "cos",
    "tan",
    "torad",
    "todeg"
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

    switch (function_num)
    {
	case 0:
	    return sqrt(farg);

	case 1:
	    return log(farg);

        case 2:
            return sin(farg);

        case 3:
            return cos(farg);

        case 4:
            return tan(farg);

	case 5:
	    return torad(farg);

	case 6:
	    return todeg(farg);

        default:
            return 0;
    }
}
