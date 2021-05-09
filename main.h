#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "instructions.h"

#define NB_IMG 3
#define ecran images[0]



SDL_Surface* init(int x,int y,const char* titre)
{
    SDL_Surface* screen = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
        exit(EXIT_FAILURE);

    if ((screen =
         SDL_SetVideoMode(x,y,32,SDL_DOUBLEBUF | SDL_HWSURFACE)) == NULL)
        exit(EXIT_FAILURE);

    SDL_WM_SetCaption(titre,NULL);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(500,10);

    return screen;
}

void cleanup(SDL_Surface** images)
{
    int img;
    for (img = 0 ; img < NB_IMG ; img++)
        SDL_FreeSurface(images[img]);
    SDL_Quit();
}

FILE* s_fopen(const char* cfile,const char* mode)
{
    FILE* file = NULL;

    if ((file = fopen(cfile,mode)) == NULL)
        exit(EXIT_FAILURE);

    return file;
}

void* s_malloc(size_t size)
{
    void* ptr = NULL;

    if ((ptr = malloc(size)) == NULL)
        exit(EXIT_FAILURE);

    return ptr;
}

SDL_Surface* s_IMG_Load(const char* cimage)
{
    SDL_Surface* surface = NULL;

    if ((surface = IMG_Load(cimage)) == NULL)
    {
        fprintf(stderr,"Impossible de charger l'image \'%s\' !",cimage);
        exit(EXIT_FAILURE);
    }

    return surface;
}

size_t sizeofFile(const char* cfile)
{
    size_t size;
    FILE* file = s_fopen(cfile,"rb");
    fseek(file,0,SEEK_END);
    size = ftell(file);
    rewind(file);
    fclose(file);
    return size;
}

void exec(VM* vm,instruction* JEU,instruction2* OP)
{
    vm->ESP_REG = vm->paramstack->last;
    vm->EIP_REG += JEU[vm->RAM[vm->EIP_REG]].ptrfonction(vm,OP);

    if (vm->EIP_REG > vm->exe_size)
    {
        fprintf(stderr,"Instruction pointer error : %u bad adress.\n",vm->EIP_REG);
        exit(EXIT_FAILURE);
    }
}


#endif // MAIN_H_INCLUDED
