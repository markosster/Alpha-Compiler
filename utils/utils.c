#include "utils.h"

unsigned int count_digits(unsigned int num)
{
    unsigned int digits = 0;
    while (num > 0)
    {
        num = num / 10;
        digits++;
    }
    if (digits == 0)
        return 1;
    return digits;
}

char *createNewSymbolName(int num, int isForAnonymFunc)
{
    assert(isForAnonymFunc == 1 || isForAnonymFunc == 0);
    char *num_str = (char *)calloc((count_digits(num) + 3), sizeof(char));
    if (isForAnonymFunc)
        sprintf(num_str, "_f%d", num);
    else /* else, is for temporary variable */
        sprintf(num_str, "_t%d", num);
    return num_str;
}

char *toLowerString(const char *str)
{
    int i;
    char *tmp = malloc(strlen(str) + 1);
    memset(tmp, 0, strlen(str) + 1);
    for (i = 0; str[i]; i++)
        tmp[i] = tolower(str[i]);
    return tmp;
}

char *toUpperString(const char *str)
{
    int i;
    char *tmp = malloc(strlen(str) + 1);
    memset(tmp, 0, strlen(str) + 1);
    for (i = 0; str[i]; i++)
        tmp[i] = toupper(str[i]);
    return tmp;
}

unsigned strhash(char *key, unsigned hashsize)
{
    unsigned HASH_MULT = 65599;
    size_t ui;
    unsigned int uiHash = 0U;
    for (ui = 0U; key[ui] != '\0'; ui++)
        uiHash = uiHash * HASH_MULT + key[ui];
    return (uiHash % hashsize);
}

unsigned numhash(double num, unsigned hashsize)
{
    return ((unsigned)num % hashsize);
}

int isStrNum(const char *str)
{
    int i;

    for (i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]) && str[i] != '.')
            return 0;
    }
    return 1;
}

char *basename(char const *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return strdup(path);
    else
        return strdup(s + 1);
}

char *float_tostr(double num)
{
    char *str = NULL;
    double fract_part = 0, int_part = 0;

    fract_part = modf(num, &int_part);

    if (fract_part == 0)
        asprintf(&str, "%g", num);
    else
        asprintf(&str, "%lf", num);

    return str;
}