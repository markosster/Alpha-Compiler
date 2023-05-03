#ifndef UTILS_H
#define UTILS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* --- For color printing --- */

#define PRINT_GREEN(fout, ...)       \
    {                                \
        fprintf(fout, "\033[0;32m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

#define PRINT_RED(fout, ...)         \
    {                                \
        fprintf(fout, "\033[0;31m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

#define PRINT_BLUE(fout, ...)        \
    {                                \
        fprintf(fout, "\033[0;34m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

#define PRINT_PURPLE(fout, ...)      \
    {                                \
        fprintf(fout, "\033[0;35m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

#define PRINT_CYAN(fout, ...)        \
    {                                \
        fprintf(fout, "\033[0;36m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

#define PRINT_YELLOW(fout, ...)      \
    {                                \
        fprintf(fout, "\033[0;33m"); \
        fprintf(fout, __VA_ARGS__);  \
        fprintf(fout, "\033[0m");    \
    }

/**
 * @brief Counts the digits of a given unsigned integer number
 *
 * @param num an unsigned integer number
 * @return unsigned int
 */
unsigned int count_digits(unsigned int num);

/**
 * @brief Create a New Symbol Name object. Made up names for anonymous functions or temporary (cached) variables
 *
 * @param num
 * @param isForAnonymFunc 1 for Anonymous Function creation, 0 for Temporary Variable creation
 * @return char*
 */
char *createNewSymbolName(int num, int isForAnonymFunc);

/**
 * @brief
 *
 * @param str
 * @return char*
 */
char *toLowerString(const char *str);

/**
 * @brief 
 * 
 * @param str 
 * @return char* 
 */
char *toUpperString(const char *str);

/**
 * @brief 
 * 
 * @param key 
 * @param hashsize 
 * @return unsigned 
 */
unsigned strhash(char *key, unsigned hashsize);

/**
 * @brief 
 * 
 * @param num 
 * @param hashsize 
 * @return unsigned 
 */
unsigned numhash(double num, unsigned hashsize);

/**
 * @brief 
 * 
 * @param str 
 * @return int 
 */
int isStrNum(const char *str);

char *basename(char const *path);

char *float_tostr(double num);

#endif