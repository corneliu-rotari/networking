#include <string.h>
#include <stdbool.h>

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
