#ifndef INSTRUCTIONS_H_INCLUDED
#define INSTRUCTIONS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <SDL/SDL.h>

#include "operations.h"

#define RET_REG    R[15]
#define CALC_REG   R[14]
#define EIP_REG    R[13]
#define ESP_REG    R[12]
#define EVENT_REG  R[11]
#define TEST_REG   R[10]
#define STK2_REG   R[9]
#define STK1_REG   R[8]


#define REGISTER   0b001
#define NUM_CONST  0b010
#define POINTER    0b100


Uint32 getvalue (VM*,Uint8*,Uint32,int*);
Uint32 lvalue   (VM*,Uint8*,Uint32,int*);

int off        (VM*,instruction2*);
int date       (VM*,instruction2*);
int calc       (VM*,instruction2*);
int mov        (VM*,instruction2*);
int jump       (VM*,instruction2*);
int cmp        (VM*,instruction2*);
int je_jne     (VM*,instruction2*);
int io         (VM*,instruction2*);
int regaff     (VM*,instruction2*);
int str        (VM*,instruction2*);
int charac     (VM*,instruction2*);
int movcurs    (VM*,instruction2*);
int aff_charac (VM*,instruction2*);
int wait       (VM*,instruction2*);
int clear      (VM*,instruction2*);
int stackPush  (VM*,instruction2*);
int stackPop   (VM*,instruction2*);
int call       (VM*,instruction2*);
int ret        (VM*,instruction2*);
int nstr       (VM*,instruction2*);
int mem        (VM*,instruction2*);
int open       (VM*,instruction2*);
int close      (VM*,instruction2*);
int read       (VM*,instruction2*);
int write      (VM*,instruction2*);
int seek       (VM*,instruction2*);


#endif // INSTRUCTIONS_H_INCLUDED
