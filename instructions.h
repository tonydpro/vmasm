#ifndef INSTRUCTIONS_H_INCLUDED
#define INSTRUCTIONS_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define REGISTER   0b001
#define NUM_CONST  0b010
#define POINTER    0b100

#define LVALUE   1
#define GETVALUE 2

typedef struct instruction instruction;
typedef unsigned char       Uint8;
typedef unsigned short int  Uint16;
typedef unsigned int        Uint32;

struct instruction
{
    int (*ptrfonction)(char*,int,FILE*);
};

int ishexdigit (int);
instruction  instr  (int (*) (char*,int,FILE*));
FILE* s_fopen(const char*,const char*);
int ispointer   (const char*,size_t);
int value       (char*,Uint8*,Uint32,int);
int strtohex    (char*);
int dx2         (char*,size_t);

int error      (const char*);
int off        (char*,int,FILE*);
int date       (char*,int,FILE*);
int calc       (char*,int,FILE*);
int mov        (char*,int,FILE*);
int jump       (char*,int,FILE*);
int mov        (char*,int,FILE*);
int test       (char*,int,FILE*);
int jmpif      (char*,int,FILE*);
int io         (char*,int,FILE*);
int disp       (char*,int,FILE*);
int str        (char*,int,FILE*);
int charac     (char*,int,FILE*);
int movcurs    (char*,int,FILE*);
int aff_charac (char*,int,FILE*);
int wait       (char*,int,FILE*);
int clear      (char*,int,FILE*);
int stackPush  (char*,int,FILE*);
int stackPop   (char*,int,FILE*);
int call       (char*,int,FILE*);
int ret        (char*,int,FILE*);
int nstr       (char*,int,FILE*);
int mem        (char*,int,FILE*);
int open       (char*,int,FILE*);
int close      (char*,int,FILE*);
int read       (char*,int,FILE*);
int write      (char*,int,FILE*);
int seek       (char*,int,FILE*);

int strtohex   (char*);
int dx         (char*);
void inscr     (char*,FILE*,Uint32,int);

#endif // INSTRUCTIONS_H_INCLUDED
