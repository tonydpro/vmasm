#include "operations.h"

#define REGISTER   0b001
#define NUM_CONST  0b010
#define POINTER    0b100

#define Uint24(x,n) (((x[n] * 65536) + (x[n + 1] * 256) + x[n + 2]) & 0xFFFFFF)

Uint32 RAMs (Uint8*,unsigned int);
Uint32 REGs (Uint32*,int);


void aff_char  (VM* vm,char character)
{
    if (_mode_ == 1) return;

    if (character == 13)
    {
        vm->x = 0;
        vm->y ++;
        goto endline;
    }

    int c = csascii(character);

    if (c == -1)
        return;

    SDL_Rect pos = {8 * vm->x,12 * vm->y},source = {8 * c,0,8,12};
    SDL_BlitSurface(vm->charset,&source,vm->screen,&pos);

    SDL_UpdateRect(vm->screen,pos.x,pos.y,8,12);

    vm->x ++;

    if (vm->x == 80)
    {
        vm->x = 0;
        vm->y++;
    }

    endline:
        if (vm->y == 25)
        {
            vm->x = 0;
            vm->y = 0;
            SDL_FillRect(vm->screen,NULL,SDL_MapRGB(vm->screen->format,0,0,0));
        }
}

int csascii    (char c)
{
    if (c >= 32 && c <= 128)
        return c - 32;

    else return -1;
}

void push (stack* Sstack,Uint32 elem24bits)
{
    if (Sstack->last > 765)
    {
        fprintf(stderr,"Stack overflow error.\n");
        exit(EXIT_FAILURE);
    }

    Sstack->array[Sstack->last]     = elem24bits >> 16 & 0x000000FF;
    Sstack->array[Sstack->last + 1] = elem24bits >> 8  & 0x000000FF;
    Sstack->array[Sstack->last + 2] = elem24bits       & 0x000000FF;

    Sstack->last += 3;
}

void pop (stack* Sstack,Uint32* ptr24bits)
{
    if (Sstack->last <= 0)
    {
        fprintf(stderr,"Error pop stack : %d.\n",Sstack->last);
        exit(EXIT_FAILURE);
    }

    *ptr24bits =
        (Sstack->array[Sstack->last - 3] * 65536)
       +(Sstack->array[Sstack->last - 2] *   256)
       + Sstack->array[Sstack->last - 1];

    Sstack->last -= 3;
}

Uint32 getvalue(VM* vm,Uint8* opcode,Uint32 flag,int* count)
{
    switch(opcode[0])
    {
        case 0:
            if (flag & REGISTER)
            {
                *count = 2;
                return REGs(vm->R,opcode[1]);
            }
            else
                goto end;
            break;

        case 1:
            if (flag & NUM_CONST)
            {
                *count = 4;
                return Uint24(opcode,1);
            }
            else
                goto end;
            break;

        case 2:
            if ((flag & REGISTER) && (flag & POINTER))
            {
                *count = 4;
                if (!opcode[1])
                    return Uint24(vm->RAM,REGs(vm->R,opcode[2]) +
                                  (char)opcode[3]);
                return vm->RAM[REGs(vm->R,opcode[2]) +
                                  (char)opcode[3]];
            }
            else
                goto end;
            break;

        case 3:
            if ((flag & POINTER) && (flag & NUM_CONST))
            {
                *count = 5;
                if (!opcode[1])
                    return Uint24(vm->RAM,Uint24(opcode,2));
                return vm->RAM[(int)Uint24(opcode,2)];
            }
            else
                goto end;
            break;

        default:
            end:
            fprintf(stderr,"Bad value ! (DEBUG : opcode[0] = %d)\n",opcode[0]);
            vm->state = 0;
            return 0;
    }
}

Uint32 lvalue(VM* vm,Uint8* opcode,Uint32 flag,int* count)
{
    switch(opcode[0])
    {
        case 0:
            if (flag & REGISTER)
            {
                *count = 2;
                return opcode[1];
            }
            else
                goto end;
            break;

        case 1:
            goto end;
            break;

        case 2:
            if ((flag & REGISTER) && (flag & POINTER))
            {
                *count = 3;
                return opcode[1];
            }
            else
                goto end;
            break;

        case 3:
            if ((flag & POINTER) && (flag & NUM_CONST))
            {
                *count = 4;
                return Uint24(opcode,1);
            }
            else
                goto end;
            break;

        default:
            end:
            fprintf(stderr,"Bad value ! (DBG : opcode[0] = %d)\n",opcode[0]);
            vm->R[15] = 0;
            return 0;
    }
}

int add       (int a,int b)
{
    return a + b;
}

int sub       (int a,int b)
{
    return a - b;
}

int mul       (int a,int b)
{
    return a * b;
}

int _div      (int a,int b)
{
    if (b == 0)
    {
        fprintf(stderr,"Error division by zero");
        exit(EXIT_FAILURE);
    }
    return a / b;
}

int mod       (int a,int b)
{
    return a % b;
}

int shl      (int a,int b)
{
    return a << b;
}

int shr      (int a,int b)
{
    return a >> b;
}

int or        (int a,int b)
{
    return a | b;
}

int and       (int a,int b)
{
    return a & b;
}

int xor      (int a,int b)
{
    return a ^ b;
}


int mode(void)
{
    FILE* m = fopen("config.txt","r");
    if (m == NULL)
    {
        fprintf(stderr,"Error config.txt does not exist !");
        exit(EXIT_FAILURE);
    }

    char mode[512];
    fscanf(m,"%s",mode);
    fclose(m);
    if (!strcmp(mode,"RELEASE"))
        return 1;
    else if (!strcmp(mode,"DEBUG"))
        return 2;
    else exit(EXIT_FAILURE);
}
