#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

bool isExit(char *input)
{
    return strncmp(input, "exit", 4) == 0;
}

bool isSubscribe(char *input)
{
    return strncmp(input, "subscribe", 9) == 0;
}

bool isUnsubscribe(char *input)
{
    return strncmp(input, "unsubscribe", 11) == 0;
}

const char *convert_type(uint8_t type)
{
    switch (type)
    {
    case 0:
        return "INT";
    case 1:
        return "SHORT_REAL";
    case 2:
        return "FLOAT";
    case 3:
        return "STRING";
    default:
        break;
    }
    return NULL;
}
