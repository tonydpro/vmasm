#include "instructions.h"

FILE* s_fopen(const char*,const char*);
void* s_malloc(size_t);

instruction instr(int (*ptrfonction) (VM*,instruction2* OP))
{
    instruction instruc = {ptrfonction};
    return instruc;
}

instruction2 instr2(int (*ptrfonction) (int a,int b))
{
    instruction2 instruc = {ptrfonction};
    return instruc;
}

Uint32 RAMs (Uint8* RAM,unsigned int n)
{
    int result =
        (RAM[n] * 65536) + (RAM[n + 1] * 256) + RAM[n + 2];

    if ((n - 3) >= 2097152 || n < 0)
    {
        fprintf(stderr,"RAM overflow error.\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

Uint32 REGs (Uint32* R,int r)
{
    if (r < 0 || r > 23)
    {
        fprintf(stderr,"Register %d does not exist.\n",r);
        exit(EXIT_FAILURE);
    }

    return R[r] & 0x00FFFFFF;
}

int off        (VM* vm,instruction2* OP)
{
    vm->state = 0;
    return 1;
}

int date       (VM* vm,instruction2* OP)
{
    int r,count = 0;
    r = lvalue(vm,vm->RAM + vm->EIP_REG + 1,REGISTER,&count);
    REGs(vm->R,r);
    vm->R[r] = clock()/CLOCKS_PER_SEC;
    return count + 1;
}

int calc       (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 2,*c0 = opcode;
    int operation,count = 0;
    Uint32 a,b,dest,res,d;

    operation = vm->RAM[vm->EIP_REG + 1];
    a = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    b = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    dest = lvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    d = opcode[0];
    opcode += count;

    res = OP[operation].ptrfonction(a,b);

    if (d == 0)
    {
        REGs(vm->R,dest);
        vm->R[dest] = res;
    }
    else if (d == 2)
    {
        RAMs(vm->RAM,vm->R[dest]);
        vm->RAM[vm->R[dest]]     = res >> 16;
        vm->RAM[vm->R[dest] + 1] = res >> 8  & 0xFF;
        vm->RAM[vm->R[dest] + 2] = res       & 0xFF;
    }
    else if (d == 3)
    {
        RAMs(vm->RAM,dest);
        vm->RAM[dest]     = res >> 16;
        vm->RAM[dest + 1] = res >> 8  & 0xFF;
        vm->RAM[dest + 2] = res       & 0xFF;
    }
    return 2 + opcode - c0;
}

int mov        (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode,d;
    int count = 0;
    Uint32 source,dest;

    dest = lvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    d = opcode[0];
    opcode += count;
    source = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;

    if (d == 0)
    {
        REGs(vm->R,dest);
        vm->R[dest] = source;
    }
    else if (d == 2)
    {
        RAMs(vm->RAM,vm->R[dest]);
        REGs(vm->R,dest);
        vm->RAM[vm->R[dest]]     = source >> 16;
        vm->RAM[vm->R[dest] + 1] = source >> 8  & 0xFF;
        vm->RAM[vm->R[dest] + 2] = source       & 0xFF;
    }
    else if (d == 3)
    {
        RAMs(vm->RAM,dest);
        vm->RAM[dest]     = source >> 16;
        vm->RAM[dest + 1] = source >> 8  & 0xFF;
        vm->RAM[dest + 2] = source       & 0xFF;
    }

    return 1 + opcode - c0;
}

int jump       (VM* vm,instruction2* OP)
{
    const int
        s = (vm->RAM[vm->EIP_REG + 1] >> 4) & 0x0F,
        d =  vm->RAM[vm->EIP_REG + 1]       & 0x0F,
        p = (vm->RAM[vm->EIP_REG + 2] * 256) + vm->RAM[vm->EIP_REG + 3];

    if (d == 0)
    {
        if (s == 0)
            return -1*p;
        else
           return p;
    }
    else
    {
        return p - vm->EIP_REG;
    }

    return 0;
}

int cmp        (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,R1,R2,o;

    R1 = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;
    R2 = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;
    o  = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;

    switch (o)
    {
        case 0:
            vm->TEST_REG = (R1 != R2);
            break;

        case 1:
            vm->TEST_REG = (R1 == R2);
            break;

        case 2:
            vm->TEST_REG = (R1 <= R2);
            break;

        case 3:
            vm->TEST_REG = (R1 >= R2);
            break;

        case 4:
            vm->TEST_REG = (R1 < R2);
            break;

        case 5:
            vm->TEST_REG = (R1 > R2);
            break;

        default:
            break;
    }

    return 1 + opcode - c0;
}

int je_jne     (VM* vm,instruction2* OP)
{
    const int
        s = (vm->RAM[vm->EIP_REG + 1] >> 4) & 0x0F,
        d =  vm->RAM[vm->EIP_REG + 1]       & 0x0F,
        p = (vm->RAM[vm->EIP_REG + 2] * 256) + vm->RAM[vm->EIP_REG + 3],
        A =  vm->RAM[vm->EIP_REG + 4];

    if (vm->TEST_REG != A)
        return 5;
    else
        return p - vm->EIP_REG;
}

int io         (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0;

    int c,y = opcode[0],a;
    opcode++;
    a = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;

    if (_mode_ != 2) return 1 + opcode - c0;

    SDL_Event input;
    SDL_Rect  pos;

    if (y == 0)
    {
        do
        {
            do
            {
                SDL_WaitEvent(&input);
                pos.x = 8 * vm->x; pos.y = 12 * vm->y;
                SDL_BlitSurface(vm->curseur,NULL,vm->screen,&pos);
                SDL_UpdateRect(vm->screen,pos.x,pos.y,8,12);

                if (input.key.keysym.sym == SDLK_RETURN
                    || input.key.keysym.sym == SDLK_KP_ENTER)
                    aff_char(vm,' ');

                if (input.type == SDL_QUIT)
                {
                    vm->state = 0;
                    return 0;
                }

            }while (input.type != SDL_KEYDOWN);

            if (a >= 2097151 || a < 0)
            {
                fprintf(stderr,"RAM overflow error.\n");
                exit(EXIT_FAILURE);
            }

            vm->RAM[a] = input.key.keysym.unicode;

            if (vm->RAM[a] == '\b' && vm->x > 0)
            {
                a--;
                vm->x--;
                aff_char(vm,' ');
                aff_char(vm,' ');
                vm->x -= 2;
            }
            else if (vm->RAM[a] != '\b' && vm->RAM[a] > 0 && vm->RAM[a] < 128)
            {
                aff_char(vm,vm->RAM[a]);
                a++;
            }

        }while (input.key.keysym.sym != SDLK_RETURN &&
                input.key.keysym.sym != SDLK_KP_ENTER);

        vm->RAM[a-1] = '\0';
        SDL_Delay(150);

    }
    else
    {
        for (c = a ; vm->RAM[c] != '\0' && c < 2097152 ; c++)
            aff_char(vm,vm->RAM[c]);
    }

    SDL_Flip(vm->screen);

    return 1 + opcode - c0;
}

int regaff     (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,c,n = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    char aff[256];

    opcode += count;

    if (_mode_ != 2) return 1 + opcode - c0;

    sprintf(aff,"%d",n);

    for (c = 0 ; aff[c] != '\0' && c < 256 ; c++)
        aff_char(vm,aff[c]);

    return 1 + opcode - c0;
}

int str        (VM* vm,instruction2* OP)
{
    int count = 0,i,
    n =
    getvalue(vm,vm->RAM + vm->EIP_REG + 1,REGISTER|POINTER|NUM_CONST,&count);
    char c = 1;

    for (i = n ; c != '\0' ; i++)
    {
        if (i >= 2097152 || i < vm->exe_size)
        {
            fprintf(stderr,"RAM overflow error.\n");
            exit(EXIT_FAILURE);
        }

        c = vm->RAM[vm->EIP_REG + count + 1 - n + i];

        vm->RAM[i] = c;
    }

    return count + 1 + i - n;
}

int charac     (VM* vm,instruction2* OP)
{
    char c = vm->RAM[vm->EIP_REG + 1];
    int count = 0,
    n =
    getvalue(vm,vm->RAM + vm->EIP_REG + 2,REGISTER|POINTER|NUM_CONST,&count);

    if (n >= 2097152 || n < vm->exe_size)
    {
        fprintf(stderr,"RAM overflow error.\n");
        exit(EXIT_FAILURE);
    }

    vm->RAM[n] = c;

    return 2 + count;
}

int movcurs    (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,x,y;

    x = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;
    y = getvalue(vm,opcode,REGISTER | POINTER | NUM_CONST,&count);
    opcode += count;

    if (_mode_ != 2) return 1 + opcode - c0;

    if (x < 80 && x >= 0)
        vm->x = x;
    else
        fprintf(stderr,"bad position for cursor (x = %d)\n",x);

    if (y < 25 && y >= 0)
        vm->y = y;
    else
        fprintf(stderr,"bad position for cursor (y = %d)\n",y);

    return 1 + opcode - c0;
}

int aff_charac (VM* vm,instruction2* OP)
{
    int count = 0,
    n =
    getvalue(vm,vm->RAM + vm->EIP_REG + 1,REGISTER|NUM_CONST|POINTER,&count);

    if (_mode_ != 2) return 1 + count;


    aff_char(vm,vm->RAM[n]);

    return 1 + count;
}

int wait       (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1;
    int count = 0;
    Uint32 _time = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);

    SDL_Delay(_time);

    return 1 + count;
}

int clear      (VM* vm,instruction2* OP)
{
    if (_mode_ != 2) return 1;


    vm->x = 0;
    vm->y = 0;
    SDL_FillRect(vm->screen,NULL,SDL_MapRGB(vm->screen->format,0,0,0));
    SDL_Flip(vm->screen);
    return 1;
}

int stackPush  (VM* vm,instruction2* OP)
{
    int count = 0,
    n =
    getvalue(vm,vm->RAM + vm->EIP_REG + 1,REGISTER|NUM_CONST|POINTER,&count);

    push(vm->paramstack,n);

    vm->R[8] = vm->paramstack->last;

    return 1 + count;
}

int stackPop   (VM* vm,instruction2* OP)
{
    int count = 0,d = vm->RAM[vm->EIP_REG + 1],
    n =
    lvalue(vm,vm->RAM + vm->EIP_REG + 1,REGISTER|NUM_CONST|POINTER,&count);
    Uint32 val,ad;

    if (d == 0)
        pop(vm->paramstack,vm->R + n);
    else if (d == 2)
    {
        ad = REGs(vm->R,n);
        goto id;
    }
    else if (d == 3)
    {
        ad = n;
        id:
        pop(vm->paramstack,&val);
        vm->RAM[ad]     = val >> 16;
        vm->RAM[ad + 1] = val >> 8  & 0xFF;
        vm->RAM[ad + 2] = val       & 0xFF;
    }

    vm->R[8] = vm->paramstack->last;

    return 1 + count;
}

int call       (VM* vm,instruction2* OP)
{
    const int
    n = (vm->RAM[vm->EIP_REG + 1] * 256) + vm->RAM[vm->EIP_REG + 2];

    push(vm->callstack,vm->EIP_REG + 3);

    vm->R[9] = vm->callstack->last;

    return n - vm->EIP_REG;
}

int ret        (VM* vm,instruction2* OP)
{
    Uint32 n;

    pop(vm->callstack,&n);

    vm->R[9] = vm->callstack->last;

    return n - vm->EIP_REG;
}

int nstr       (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,n,c,val,d;
    char* ptr = NULL;

    n = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    c =   lvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    d = opcode[0];
    opcode += count;

    ptr = (void*)vm->RAM + n;
    if (n < 0 || n > 2097152)
    {
        fprintf(stderr,"RAM overflow error.\n");
        exit(EXIT_FAILURE);
    }

    val = atoi(ptr);

    if (d == 0)
    {
        REGs(vm->R,c);
        vm->R[c] = val;
    }
    else if (d == 2)
    {
        RAMs(vm->RAM,vm->R[c]);
        vm->RAM[vm->R[c]]     = val >> 16;
        vm->RAM[vm->R[c] + 1] = val >> 8  & 0xFF;
        vm->RAM[vm->R[c] + 2] = val       & 0xFF;
    }
    else if (d == 3)
    {
        RAMs(vm->RAM,c);
        vm->RAM[c]     = val >> 16;
        vm->RAM[c + 1] = val >> 8  & 0xFF;
        vm->RAM[c + 2] = val       & 0xFF;
    }

    return 1 + opcode - c0;
}

int mem        (VM* vm,instruction2* OP)
{
    Uint32
        t =
        (vm->RAM[vm->EIP_REG + 1] * 256) +
         vm->RAM[vm->EIP_REG + 2];

    int count = 0,
    n = getvalue(vm,vm->RAM + vm->EIP_REG + 3,
    REGISTER | NUM_CONST | POINTER,&count);


    char c = 1;
    int i;

    for (i = 0 ; i < t ; i++)
    {
        if ((vm->EIP_REG + 6 + i + n) >= 2097152 ||
            (vm->EIP_REG + 6 + i + n) < vm->exe_size)
        {
            fprintf(stderr,"RAM overflow error.\n");
            exit(EXIT_FAILURE);
        }

        c = vm->RAM[vm->EIP_REG + count + 3 + i];

        vm->RAM[i + n] = c;
    }

    return 3 + count + t;
}

int open       (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;

    int count = 0,f,c,m;

    f = getvalue(vm,opcode,NUM_CONST,&count);
    opcode += count;
    m = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    c = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;

    vm->files[f] =
    s_fopen(
            (const char*)(vm->RAM + m),
            (const char*)(vm->RAM + c)
            );


    return 1 + opcode - c0;
}

int close      (VM* vm,instruction2* OP)
{
    const Uint8
            f  = vm->RAM[vm->EIP_REG + 1] & 7;

    fclose(vm->files[(int)f]);

    return 2;
}

int read       (VM* vm,instruction2* OP)
{
    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,n,t,f;

    f = getvalue(vm,opcode,NUM_CONST,&count);
    opcode += count;
    n = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    t = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;

    fread(vm->RAM + n,1,t,vm->files[f]);

    return 1 + opcode - c0;
}

int write      (VM* vm,instruction2* OP)
{

    Uint8* opcode = vm->RAM + vm->EIP_REG + 1,*c0 = opcode;
    int count = 0,n,t,f;
    char* str = NULL;

    f = getvalue(vm,opcode,NUM_CONST,&count);
    opcode += count;
    n = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;
    t = getvalue(vm,opcode,REGISTER | NUM_CONST | POINTER,&count);
    opcode += count;

    fread(vm->RAM + n,1,t,vm->files[f]);

    str = s_malloc(t + 1);
    memcpy(str,vm->RAM + n,t);

    fwrite(str,1,t,vm->files[f]);

    return 1 + opcode - c0;
}

int seek         (VM* vm,instruction2* OP)
{
    const Uint8
        f = vm->RAM[vm->EIP_REG + 1],
        s = vm->RAM[vm->EIP_REG + 2],
        r = vm->RAM[vm->EIP_REG + 3];

    const Uint16
        d = (vm->RAM[vm->EIP_REG + 4] * 256) +
             vm->RAM[vm->EIP_REG + 5];

    fseek(vm->files[(int)f],(s != 0 ? 1 : -1)*d,r);

    return 6;
}

