#include "instructions.h"

instruction instr(int (*ptrfonction)(char* param,int key,FILE* ftarget))
{
    instruction instruc = {ptrfonction};
    return instruc;
}

int ishexdigit (int c)
{
    return isdigit(c) || (c <= 'F' && c >= 'A') || (c <= 'f' && c >= 'a');
}

int dx        (char* word)
{
    size_t s1 = strlen(word);

    if (word[s1 - 1] == 'h')
    {
        word[s1 - 1] = '\0';
        return strtohex(word);
    }
    else
        return atoi(word);
}

FILE* s_fopen(const char* cfile,const char* mode)
{
    FILE* file = NULL;

    if ((file = fopen(cfile,mode)) == NULL)
    {
        printf("\nError : %s. ",cfile);
        perror("");
        remove("pre2.o");
        remove("adresses.ad");
        remove("goto.g");
        getchar();
        exit(EXIT_FAILURE);
    }
    return file;
}

int ispointer(const char* word,size_t len)
{
    return (word[0] == '[' && word[len-1] == ']');
}

int strtohex   (char* str)
{
    int hn;
    sscanf(str,"%x",&hn);
    return hn;
}

int dx2        (char* word,size_t s1)
{
    if (word[s1 - 1] == 'h')
    {
        word[s1 - 1] = '\0';
        return strtohex(word);
    }
    else
        return atoi(word);
}

int dx3         (char* word,size_t s1)
{
    if (word[s1 - 2] == 'h')
    {
        word[s1 - 2] = '\0';
        return strtohex(word + 1);
    }
    else
        return atoi(word + 1);
}

void set (char* str)
{
    int i;
    for (i = strlen(str) - 1 ; i != 0 ; i--)
    {
        if (str[i] == ' ' || str[i] == '\t')
        {
            str[i] = '\0';
            return;
        }
    }
}

int value(char* arg,Uint8 val[4],Uint32 flag,int type)
{
    set(arg);
    size_t len = strlen(arg);
    Uint32 n;
    char signe = 0,s[32] = "",r[8],byte = 0;

    if (arg[0] == '~')
    {
        byte = 1;
        arg++;
        len--;
    }

    if (ispointer(arg,len))
    {
        if (!(flag & POINTER))
            error("The value cannot be a pointer value.\n");

        if (arg[1] == 'R')
        {
            if (!(flag & REGISTER))
                error("The value cannot be a register.\n");

            if (strchr(arg,'+') || strchr(arg,'-'))
                sscanf(arg,"%s %c %s",r,&signe,s);

            if (type == GETVALUE)
            {
                val[0] = 2;
                val[1] = byte;
                val[2] = atoi(arg + 2);
                val[3] = (signe == '+' ? 1 : -1)*dx(s) & 0xFF;

                if (val[2] > 0x10)
                    error("The register does not exist !\n");
                return 4;
            }
            else
            {
                val[0] = 2;
                val[1] = atoi(arg + 2);
                val[2] = (signe == '+' ? 1 : -1)*dx(s) & 0xFF;

                if (val[1] > 0x10)
                    error("The register does not exist !\n");
                return 3;
            }
        }
        else
        {
            if (!(flag & NUM_CONST))
                error("The value cannot be a numeric constant.\n");

            n = dx3(arg,len);

            if (type == GETVALUE)
            {
                val[0] = 3;
                val[1] = byte;
                val[2] = n >> 16 & 0xFF;
                val[3] = n >> 8  & 0xFF;
                val[4] = n       & 0xFF;
                return 5;
            }
            else
            {
                val[0] = 3;
                val[1] = n >> 16 & 0xFF;
                val[2] = n >> 8  & 0xFF;
                val[3] = n       & 0xFF;
                return 4;
            }
        }
    }
    else
    {
        if (arg[0] == 'R')
        {
            if (!(flag & REGISTER))
                error("The value cannot be a register.\n");

            val[0] = 0;
            val[1] = atoi(arg + 1);

            if (val[1] > 0x10)
                error("Register %d does not exist !\n");

            return 2;
        }
        else
        {
            if (!(flag & NUM_CONST))
                error("The value cannot be a numeric constant.\n");

            n = dx2(arg,len);
            val[0] = 1;
            val[1] = n >> 16 & 0xFF;
            val[2] = n >> 8  & 0xFF;
            val[3] = n       & 0xFF;
            return 4;
        }
    }
}

void inscr      (char* arg,FILE* ftarget,Uint32 flag,int type)
{
    Uint8 opcode[8];
    size_t i = value(arg,opcode,flag,type),c;

    for (c = 0 ; c < i ; c++)
        fputc(opcode[c],ftarget);
}

int error      (const char* err)
{
    printf("Error : %s.\n",err);
    return -1;
}



int off        (char* param,int key,FILE* ftarget)
{
    if (strlen(param))
        return error("\'off\' has no paramter");

    return 0;
}

int date       (char* param,int key,FILE* ftarget)
{

    char word[32];
    int lu =
    sscanf(param,"%s",word);

    if (!strlen(param) || lu != 1)
        return error("\'date\' invalid paramter.");

    inscr(word,ftarget,REGISTER,GETVALUE);

    return 0;
}

int calc       (char* param,int key,FILE* ftarget)
{
    int lu;
    char word1[32],word2[32],word3[32];

    if (key == 31 || key == 32)
    {
        lu = sscanf (param,"%[^\n]",word1);

        if (lu != 1)
            return error("\'inc\' or \'dec\' invalid paramter");

        sprintf(param,"%s,1 : %s",word1,word1);

        key -= 29;
    }

    lu = sscanf(param,"%[^,^\n],%[^:^\n] : %[^\n]",word1,word2,word3);

    if (lu != 3)
        return error("operation invalid parameters");

    fputc(key - 2,ftarget);
    inscr(word1,ftarget,NUM_CONST | REGISTER | POINTER,GETVALUE);
    inscr(word2,ftarget,NUM_CONST | REGISTER | POINTER,GETVALUE);
    inscr(word3,ftarget,NUM_CONST | REGISTER | POINTER,LVALUE);

    return 0;
}

int mov        (char* param,int key,FILE* ftarget)
{
    char word1[32],word2[32];
    int lu;

    lu = sscanf(param,"%[^,^\n],%[^\n]",word1,word2);

    if (lu != 2)
        return error("\'mov\' invalid paramters");

    inscr(word1,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);
    inscr(word2,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int jump       (char* param,int key,FILE* ftarget)
{
    char bloc[3] = {48,48,48};

    FILE* g = s_fopen("goto.g","a");
    fprintf(g,"%ld\n",ftell(ftarget));
    fclose(g);

    fwrite(bloc,1,3,ftarget);
    return 0;
}

int test        (char* param,int key,FILE* ftarget)
{
    char word1[32],word2[32],word3[32];
    int lu;

    lu = sscanf(param,"%[^,^\n],%[^,^\n],%[^,^\n]",word1,word2,word3);

    if (lu != 3)
        return error("\'test\' invalid parameters");

    inscr(word1,ftarget,REGISTER | NUM_CONST | POINTER,GETVALUE);
    inscr(word2,ftarget,REGISTER | NUM_CONST | POINTER,GETVALUE);
    inscr(word3,ftarget,REGISTER | NUM_CONST | POINTER,GETVALUE);

    return 0;
}

int jmpif       (char* param,int key,FILE* ftarget)
{
    char bloc[4] = {48,48,48,48};

    FILE* g = s_fopen("goto.g","a");
    fprintf(g,"%ld\n",ftell(ftarget));
    fclose(g);

    fwrite(bloc,1,4,ftarget);

    return 0;
}

int io          (char* param,int key,FILE* ftarget)
{
    char word[32];

    sscanf(param,"%s",word);

    fputc((key == 18),ftarget);

    inscr(word,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);

    return 0;
}

int disp     (char* param,int key,FILE* ftarget)
{
    char word[32];
    int lu;

    lu = sscanf(param,"%[^,^\n]",word);

    if (lu != 1)
        error("\'disp\' invalid parameter");

    inscr(word,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int str        (char* param,int key,FILE* ftarget)
{
    int lu,i;
    char adresse[32],chaine[4096];

    lu = sscanf(param,"%s \"%[^\"]s\"",adresse,chaine);

    if (lu != 2)
        return error("\'str\' invalid paramters");

    inscr(adresse,ftarget,NUM_CONST | REGISTER | POINTER,LVALUE);

    for (i = 0 ; chaine[i] != '\0' ; i++)
    {
        if (chaine[i] == '\\')
        {
            switch(chaine[i + 1])
            {
                case 'n':
                    fputc(13,ftarget);
                    break;

                case 't':
                    fputc(9,ftarget);
                    break;

                case 'r':
                    fputc('\r',ftarget);
                    break;

                case '0':
                    fputc('\0',ftarget);
                    break;

                case '\\':
                    fputc('\\',ftarget);
                    break;

                default:
                    break;
            }
            i++;
        }
        else
            fputc(chaine[i],ftarget);
    }

    fputc('\0',ftarget);

    return 0;
}

int charac     (char* param,int key,FILE* ftarget)
{
    int c,lu,s;
    char schar[512],adresse[512];

    lu = sscanf(param,"%s %[^\n]",adresse,schar);

    if (lu != 2)
        return error("\'char\' invalid paramters");

    s = strlen(schar);

    if (schar[0] == '\'')
    {
        if (schar[s - 1] != '\'')
            return error("last \' for character is missing");

        c = schar[1];

        if (c == '\\')
        {
            switch(schar[2])
            {
                case 'n':
                    c = 13;
                    break;

                case 't':
                    c = 9;
                    break;

                case 'r':
                    c = '\r';
                    break;

                case '0':
                    c = '\0';
                    break;

                default:
                    return error("instruction \'char\' : unknow escape sequence.\n");
                    break;
            }
        }
    }
    else
        c = dx(schar);

    fputc(c,ftarget);

    inscr(adresse,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);

    return 0;
}

int movcurs    (char* param,int key,FILE* ftarget)
{
    char sx[32],sy[32];
    int lu;

    lu = sscanf(param,"%[^,^\n],%[^\n]",sx,sy);

    if (lu != 2)
        return error("\'movcurs\' invalid paramters");

    inscr(sx,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);
    inscr(sy,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int aff_charac (char* param,int key,FILE* ftarget)
{
    int lu;
    char word[256];

    lu = sscanf(param,"%[^\n]",word);

    if (lu != 1)
        return error("\'putchar\' invalid paramter");

    inscr(word,ftarget,REGISTER | NUM_CONST | POINTER,GETVALUE);

    return 0;
}

int wait       (char* param,int key,FILE* ftarget)
{
    int lu;
    char stime[1024];

    lu = sscanf(param,"%[^\n]",stime);

    if (lu != 1)
        return error("\'wait\' invalid paramter");

    inscr(stime,ftarget,REGISTER | NUM_CONST | POINTER,GETVALUE);

    return 0;
}

int clear      (char* param,int key,FILE* ftarget)
{
    if (strlen(param))
        return error("\'clear\' has no paramter");

    return 0;
}

int stackPush  (char* param,int key,FILE* ftarget)
{
    int lu;
    char word[32];

    lu = sscanf(param,"%[^\n]",word);

    if (lu != 1)
        return error("\'push\' invalid paramter");

    inscr(word,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int stackPop   (char* param,int key,FILE* ftarget)
{
    int lu;
    char word[32];

    lu = sscanf(param,"%[^\n]",word);

    if (lu != 1)
        return error("\'pop\' invalid paramter");

    inscr(word,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);

    return 0;
}

int call       (char* param,int key,FILE* ftarget)
{
    char bloc[3] = {48,48};

    FILE* g = s_fopen("goto.g","a");
    fprintf(g,"%ld\n",ftell(ftarget));
    fclose(g);

    fwrite(bloc,1,2,ftarget);
    return 0;
}

int ret        (char* param,int key,FILE* ftarget)
{
    if (strlen(param))
        return error("\'ret\' has no paramter");

    return 0;
}

int nstr       (char* param,int key,FILE* ftarget)
{
    char word1[32],word2[32];
    int lu;

    lu = sscanf(param,"%[^,^\n],%[^\n]",word1,word2);

    if (lu != 2)
        return error("\'numstr\' invalid paramters");

    inscr(word1,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);
    inscr(word2,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);

    return 0;
}

int mem        (char* param,int key,FILE* ftarget)
{
    Uint32 s,i;
    char word1[32],word2[2048];
    char hex[3] = {0,0,0};

    sscanf(param,"%s {%[^}]s}",word1,word2);

    s = strlen(word2);

    fputc((s / 2) >> 8 & 0xFF,ftarget);
    fputc((s / 2)      & 0xFF,ftarget);

    inscr(word1,ftarget,REGISTER | POINTER | NUM_CONST,LVALUE);

    for (i = 0 ; i < s ; i += 2)
    {
        hex[0] = word2[i];
        hex[1] = word2[i + 1];
        fputc(strtohex(hex),ftarget);
    }

    return 0;
}

int open       (char* param,int key,FILE* ftarget)
{
    char word1[32],word2[32],word3[32];

    sscanf(param,"%[^,^\n],%[^,^\n],%[^\n]",word1,word2,word3);

    inscr(word1,ftarget,NUM_CONST,GETVALUE);
    inscr(word2,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);
    inscr(word3,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int close      (char* param,int key,FILE* ftarget)
{
    Uint32 f;

    sscanf(param,"%d",&f);

    fputc(f,ftarget);

    return 0;
}

int write      (char* param,int key,FILE* ftarget)
{
    char word1[32],word2[32],word3[32];

    sscanf(param,"%[^,^\n],%[^,^\n],%[^\n]",word1,word2,word3);

    inscr(word1,ftarget,NUM_CONST,GETVALUE);
    inscr(word2,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);
    inscr(word3,ftarget,REGISTER | POINTER | NUM_CONST,GETVALUE);

    return 0;
}

int read       (char* param,int key,FILE* ftarget)
{
    write(param,key,ftarget);
    return 0;
}

int seek       (char* param,int key,FILE* ftarget)
{
    int file,value,ch;

    sscanf(param,"%d,%d,%d",&file,&value,&ch);

    fputc(file,ftarget);
    fputc((value < 0 ? 0 : 1),ftarget);
    fputc(ch,ftarget);
    fputc((abs(value) >> 8)  & 0x0000FF,ftarget);
    fputc( abs(value)        & 0x0000FF,ftarget);

    return 0;
}
