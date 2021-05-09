#include "main.h"

int main(int argc,char** argv)
{

    if (argc < 2)
    {
        printf("No file to compile !\n");
        getchar();
        return EXIT_SUCCESS;
    }

    instruction JEU[] =
    {
        instr(off),
        instr(date),
        instr(calc),
        instr(mov),
        instr(jump),
        instr(test),
        instr(jmpif),
        instr(io),
        instr(disp),
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


    FILE* g = fopen("goto.g","r");
    if (g != NULL)
    {
        fclose(g);
        remove("goto.g");
    }

    printf("Source code : %s\n\n",argv[1]);

    puts("Precompilation...");
    char* target = precompilation(argv[1]);

    puts("Compilation...");
    if (compile(target,JEU) == 0)
        printf("Output size for \"%s\" : %d bytes.",target,sizeofFile(target));

    free(target);
    getchar();
    return 0;
}
