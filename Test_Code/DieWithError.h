/* DieWithError.h */
#ifndef DIEWITHERROR_H
#define DIEWITHERROR_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
static void DieWithError(const char *msg) {
    perror(msg);
    exit(1);
}
#endif