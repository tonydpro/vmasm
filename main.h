#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "instructions.h"

const char* keywords[] =
{
    "off","time",
    "add","sub","mul","div","mod",
    "shl","shr","or","and","xor",
    "mov",
    "goto","test","je","jne",
    "in","out","disp",
    "str","char",
    "movcurs",
    "putchar",
    "wait","clear",
    "push","pop",
    "call","ret",
    "numstr",
    "inc","dec",
    "mem",
    "open","close",
    "read","write",
    "seek"
};

const Uint8 keyfunct[] =
{
    0,1,
    2,2,2,2,2,
    2,2,2,2,2,
    3,
    4,5,6,6,
    7,7,8,
    9,10,
    11,
    12,
    13,14,
    15,16,
    17,18,
    19,
    2,2,
    20,
    21,22,
    23,24,
    25
};

void* s_malloc(size_t size)
{
    void* ptr = NULL;

    if ((ptr = malloc(size)) == NULL)
        exit(EXIT_FAILURE);

    return ptr;
}

unsigned int searchword (FILE* file,const char* rword)
{
    unsigned int count = 0;
    char word[1024];

    while (!feof(file))
    {
        fscanf(file,"%512s",word);

        if (!strcmp(word,rword))
            count++;
    }

    rewind(file);

    return count;
}

void goto_next (FILE* file,int c)
{
    int fc;

    do
    {
        fc = fgetc(file);
    }while (fc != c && fc != EOF);
}

size_t strinstr(const char* str1,const char* str2,size_t size)
{
    size_t i,c;

    for (i = 0 ; i < size ; i++)
    {
        if (str1[i] == str2[0])
        {
            for (c = 1 ; (str2[c] != '\0') && ((i+c) < size) ; c++)
            {
                if (str2[c] != str1[i+c])
                    break;
            }

            return i;
        }
    }

    return -1;
}

char* precompilation (char* args)
{
    FILE* source = s_fopen(args,"r"),*cible = s_fopen("pre1.o","w");
    unsigned int cdefine = searchword(source,"define"),count = 0;
    char** expressions = s_malloc(cdefine),**correspondance = s_malloc(cdefine);
    char word[1024],word2[1024];
    int last,ninstr;
    size_t num;
    char* target = s_malloc(512);

    fscanf(source,"target \"%[^\"]s\"",target);

    while (!feof(source))
    {
        fscanf(source,"%512s",word);

        if (!strcmp(word,"define"))
        {
            fscanf(source,"%512s",word2);
            expressions[count] = s_malloc(strlen(word2) + 1);
            strcpy(expressions[count],word2);
            fscanf(source,"%512s",word2);
            correspondance[count] = s_malloc(strlen(word2) + 1);
            strcpy(correspondance[count],word2);
            count++;
        }
    }

    rewind(source);

    count = 0;
    while (!feof(source))
    {
        fscanf(source,"%512s",word);
        last = fgetc(source);

        if (!strcmp(word,"define") || !strcmp(word,"target") || word[0] == ';')
        {
            goto_next(source,'\n');
            fputc('\n',cible);
        }
        else if (!feof(source))
            fprintf(cible,"%s%c",word,last);
    }

    fclose(cible);
    fclose(source);
    cible = s_fopen("pre2.o","w");
    source = s_fopen("pre1.o","r");

    while (!feof(source))
    {
        fscanf(source,"%s",word);
        last = fgetc(source);

        if (!feof(source))
        {
            for (num = 0 ; num < cdefine ; num++)
            {
                if ((ninstr = strinstr(word,expressions[num],strlen(word))) != -1)
                {
                    strcpy(word2,word);
                    strcpy(word + ninstr,correspondance[num]);
                    strcpy(word + ninstr + strlen(correspondance[num]),word2 + ninstr + strlen(expressions[num]));
                }
            }

            fprintf(cible,"%s%c",word,last);
        }
    }

    fprintf(cible,"off\n");

    for (count = 0 ; count < cdefine ; count++)
        free(expressions[count]);
    for (count = 0 ; count < cdefine ; count++)
        free(correspondance[count]);
    free(expressions);
    free(correspondance);

    fclose(source);
    fclose(cible);
    remove("pre1.o");

    return target;
}

long int search(FILE* source,char* str)
{
    long int tell;
    char word[1024],err[1024];

    rewind(source);

    while (!feof(source))
    {
        fscanf(source,"%s",word);


        if (!strcmp(word,str))
        {
            fscanf(source,"%lx",&tell);
            return tell;
        }

    }

    sprintf(err,"%s undefined label",str);
    fclose(source);
    remove("pre2.o");
    remove("adresses.ad");
    remove("goto.g");
    return error(err);
}

int compile(char* target,instruction* JEU)
{
    FILE* ftarget = s_fopen(target,"wb"),*source = s_fopen("pre2.o","r"),
    *adjump = s_fopen("adresses.ad","w");
    char line[1024],word[1024],word2[1024];
    unsigned int key;
    int read;

    while (!feof(source))
    {
        fgets(line,512,source);
        read = sscanf(line,"%s %[^\n]",word,word2);

        if (read == 1)
            strcpy(word2,"");

        if (feof(source)) break;

        if (word[0] != '.')
        {
            for (key = 0 ; key < 39 ; key++)
            {
                if (!strcmp(word,keywords[key]))
                {
                    fputc(keyfunct[key],ftarget);
                    JEU[keyfunct[key]].ptrfonction(word2,key,ftarget);
                }
            }
        }
        else
            fprintf(adjump,"%s %lx\n",word+1,ftell(ftarget));
    }

    FILE* g_goto = fopen("goto.g","r");

    if (g_goto == NULL)
        goto g_release;

    fclose(adjump);
    fclose(source);
    source = s_fopen("pre2.o","r");
    adjump = s_fopen("adresses.ad","r");
    int pos,lbl;

    while (!feof(source))
    {
        fscanf(source,"%512s",word);

        if (!strcmp(word,"goto"))
        {
            fscanf(source,"%512s",word2);
            fscanf(g_goto,"%d",&pos);
            lbl = search(adjump,word2);
            fseek(ftarget,pos - 1,SEEK_SET);
            fputc(4,ftarget);
            fputc(1,ftarget);
            fputc((lbl >> 8) & 0x00FF,ftarget);
            fputc(lbl & 0x00FF,ftarget);
        }
        else if (!strcmp(word,"je"))
        {
            fscanf(source,"%512s",word2);
            fscanf(g_goto,"%d",&pos);
            lbl = search(adjump,word2);
            fseek(ftarget,pos,SEEK_SET);
            fputc(1,ftarget);
            fputc((lbl >> 8) & 0x00FF,ftarget);
            fputc(lbl & 0x00FF,ftarget);
            fputc(1,ftarget);
        }
        else if (!strcmp(word,"jne"))
        {
            fscanf(source,"%512s",word2);
            fscanf(g_goto,"%d",&pos);
            lbl = search(adjump,word2);
            fseek(ftarget,pos,SEEK_SET);
            fputc(1,ftarget);
            fputc((lbl >> 8) & 0x00FF,ftarget);
            fputc(lbl & 0x00FF,ftarget);
            fputc(0,ftarget);
        }
        else if (!strcmp(word,"call"))
        {
            fscanf(source,"%512s",word2);
            fscanf(g_goto,"%d",&pos);
            lbl = search(adjump,word2);
            fseek(ftarget,pos,SEEK_SET);
            fputc((lbl >> 8) & 0x00FF,ftarget);
            fputc(lbl & 0x00FF,ftarget);

        }
    }

        fclose(g_goto);
    g_release:
        fclose(ftarget);
        fclose(source);
        fclose(adjump);
        remove("pre2.o");
        remove("adresses.ad");
        remove("goto.g");

    return 0;
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

#endif // MAIN_H_INCLUDED
