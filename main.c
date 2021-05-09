#include "main.h"
#define file argv[1]

VM vm = {1,NULL,NULL,NULL,NULL,NULL,0,0,0,NULL,NULL};

int main(int argc,char** argv)
{
    if (argc <= 1)
    {
        fprintf(stderr,"Aucun executable fourni en paramètre !");
        return EXIT_SUCCESS;
    }

    char pic_characters[2048],pic_curseur[2048],current_dir[2048];
    strcpy(current_dir,argv[0]);
    current_dir[strlen(current_dir) - strlen("virtual machine.exe")] = 0;
    strcpy(pic_characters,current_dir);
    strcpy(pic_curseur,current_dir);
    strcat(pic_characters,"characters.bmp");
    strcat(pic_curseur,"curseur.bmp");

    SDL_Surface* images[NB_IMG] = {NULL,NULL,NULL};

    if (_mode_ == 2)
    {
        images[0] = init(640,300,argv[1]);
        images[1] = s_IMG_Load(pic_characters);
        images[2] = s_IMG_Load(pic_curseur);
    }

    SDL_Event event; event.key.keysym.sym = 0;
    size_t exe_size = sizeofFile(file);
    Uint32 R[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,exe_size,0};
    Uint8* RAM = s_malloc(2097152 * sizeof(Uint8));
    stack  callstack,paramstack; callstack.last = 0; paramstack.last = 0;
    FILE* exe  = s_fopen(file,"rb");
    fread(RAM,sizeof(Uint8),exe_size,exe);
    fclose(exe);
    vm.RAM        = RAM;
    vm.R          = R;
    vm.exe_size   = exe_size;
    vm.screen     = ecran;
    vm.charset    = images[1];
    vm.curseur    = images[2];
    vm.callstack  = &callstack;
    vm.paramstack = &paramstack;

    instruction JEU[] =
    {
        instr(off),
        instr(date),
        instr(calc),
        instr(mov),
        instr(jump),
        instr(cmp),
        instr(je_jne),
        instr(io),
        instr(regaff),
        instr(str),
        instr(charac),
        instr(movcurs),
        instr(aff_charac),
        instr(wait),
        instr(clear),
        instr(stackPush),
        instr(stackPop),
        instr(call),
        instr(ret),
        instr(nstr),
        instr(mem),
        instr(open),
        instr(close),
        instr(read),
        instr(write),
        instr(seek)
    };

    instruction2 OP[] =
    {
        instr2(add),
        instr2(sub),
        instr2(mul),
        instr2(_div),
        instr2(mod),
        instr2(shl),
        instr2(shr),
        instr2(or),
        instr2(and),
        instr2(xor)
    };

    while (vm.state != 0 && EIP_REG < 2097152) // ou EIP_REG < exe_size
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_KEYDOWN)
            EVENT_REG = event.key.keysym.sym;
        if (event.type == SDL_QUIT)
            vm.state = 0;
        exec(&vm,JEU,OP);
    }

    if (_mode_ == 2)
        cleanup(images);
    free(RAM);
    return EXIT_SUCCESS;
}
