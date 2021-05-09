#ifndef OPERATIONS_H_INCLUDED
#define OPERATIONS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <SDL/SDL.h>
#define _mode_ mode()

typedef struct VM VM;
typedef struct instruction2 instruction2;
typedef struct instruction instruction;
typedef struct stack stack;


struct VM
{
    int state;
    Uint32* R;
    Uint8* RAM;
    SDL_Surface* screen,*charset,*curseur;
    unsigned int x,y;
    size_t exe_size;
    stack* callstack,*paramstack;
    FILE* files[8];
};

struct instruction
{
    int (*ptrfonction)(VM*,instruction2* OP);
};

struct instruction2
{
    int (*ptrfonction)(int,int);
};

struct stack
{
    Uint8  array[768];
    int    last;
};

instruction  instr  (int (*) (VM*,instruction2* OP));
instruction2 instr2 (int (*) (int,int));

void aff_char  (VM*,char);
int csascii    (char);
void push      (stack*,Uint32);
void pop       (stack*,Uint32*);
int add        (int a,int b);
int sub        (int a,int b);
int mul        (int a,int b);
int _div       (int a,int b);
int mod        (int a,int b);
int shl        (int a,int b);
int shr        (int a,int b);
int or         (int a,int b);
int and        (int a,int b);
int xor        (int a,int b);

int mode(void);

#endif
