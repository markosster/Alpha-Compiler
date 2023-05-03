#include <stdio.h>
#include <string.h>

#include "../utils/utils.h"
#include "../tcode/consts.h"
#include "../tcode/instructions.h"
#include "memory.h"

#define BUFFER_SIZE 1024

unsigned int magic_number = 0;

vmopcode extract_instr_opcode(char *token)
{
    if (!strcmp(token, "ASSIGN"))
        return assign_v;
    else if (!strcmp(token, "ADD"))
        return add_v;
    else if (!strcmp(token, "SUB"))
        return sub_v;
    else if (!strcmp(token, "MUL"))
        return mul_v;
    else if (!strcmp(token, "DIV"))
        return div_v;
    else if (!strcmp(token, "MOD"))
        return mod_v;
    else if (!strcmp(token, "JEQ"))
        return jeq_v;
    else if (!strcmp(token, "JNE"))
        return jne_v;
    else if (!strcmp(token, "JLE"))
        return jle_v;
    else if (!strcmp(token, "JGE"))
        return jge_v;
    else if (!strcmp(token, "JGT"))
        return jgt_v;
    else if (!strcmp(token, "JLT"))
        return jlt_v;
    else if (!strcmp(token, "JUMP"))
        return jump_v;
    else if (!strcmp(token, "CALL"))
        return call_v;
    else if (!strcmp(token, "PUSHARG"))
        return pusharg_v;
    else if (!strcmp(token, "FUNCENTER"))
        return funcenter_v;
    else if (!strcmp(token, "FUNCEXIT"))
        return funcexit_v;
    else if (!strcmp(token, "NEWTABLE"))
        return newtable_v;
    else if (!strcmp(token, "TABLEGETELEM"))
        return tablegetelem_v;
    else if (!strcmp(token, "TABLESETELEM"))
        return tablesetelem_v;
}

typedef enum extract_table_t
{
    numtable_t,
    strtable_t,
    userfunctable_t,
    libfunctable_t,
    instrtable_t,
} extract_table_t;

void init_table(unsigned int size, extract_table_t table)
{
    switch (table)
    {
    case numtable_t:
    {
        numConsts = malloc(sizeof(double) * size);
        memset(numConsts, 0, sizeof(double) * size);
        break;
    }
    case strtable_t:
    {
        strConsts = malloc(sizeof(char *) * size);
        memset(strConsts, 0, sizeof(char *) * size);
        break;
    }
    case userfunctable_t:
    {
        userFuncs = malloc(sizeof(UserFunc) * size);
        memset(userFuncs, 0, sizeof(UserFunc) * size);
        break;
    }
    case libfunctable_t:
    {
        libFuncs = malloc(sizeof(char *) * size);
        memset(libFuncs, 0, sizeof(char *) * size);
        break;
    }
    case instrtable_t:
    {
        instructions = malloc(sizeof(Instruction) * size);
        memset(instructions, 0, sizeof(Instruction) * size);
        break;
    }
    default:
        break;
    }
}

void add_numconst(char *token)
{
    assert(currNumConsts < totalNumConsts);
    numConsts[currNumConsts++] = atof(token);
}

void add_strconst(char *token)
{
    assert(currStrConsts < totalStrConsts);
    strConsts[currStrConsts++] = strdup(token);
}

void add_userfunc(unsigned int address, unsigned int args, unsigned int localsize, char *fid)
{
    assert(currUserFuncs < totalUserFuncs);
    UserFunc uf;
    uf.address = address;
    uf.localSize = localsize;
    uf.args = args;
    uf.id = strdup(fid);
    userFuncs[currUserFuncs++] = uf;
}

void add_libfunc(char *token)
{
    assert(currLibFuncs < totalLibFuncs);
    libFuncs[currLibFuncs++] = strdup(token);
}

void add_instruction(Instruction i)
{
    assert(currInst < totalInst);
    instructions[currInst++] = i;
}

void print_content(FILE *fout)
{
    fprintf(fout, "%u\n", magic_number);
    fprintf(fout, "G:%u\n", totalGlobals);
    Consts_display(fout);
    Instr_display(fout, 0);
}

unsigned int extract_size(char *last_token)
{
    char *tmp_token = last_token;
    // char *tmp = tmp_token;
    unsigned int size = 0;

    tmp_token = strtok(tmp_token, ":");
    tmp_token = strtok(NULL, ":");
    size = atoi(tmp_token);

    // free(tmp);
    return size;
}

void extract_instr_vmarg(char *token, vmarg **arg)
{
    char *tmp_token = token;
    // char *tmp = tmp_token;
    char *context = NULL;

    tmp_token = strtok_r(tmp_token, ":", &context);
    (*arg)->type = atoi(tmp_token);
    tmp_token = strtok_r(NULL, ":", &context);
    if (tmp_token)
        (*arg)->val = atoi(tmp_token);
    // free(tmp);
}

void extract_table_content(FILE *file, unsigned int tablesize, extract_table_t table)
{
    char buff[BUFFER_SIZE];
    char *line, *token;
    unsigned int i = 0;
    char *delim = "\n";

    while (i < tablesize && (line = fgets(buff, BUFFER_SIZE, file)))
    {
        // printf("content: %s\n", line);
        switch (table)
        {
        case numtable_t:
        {
            add_numconst(line);
            break;
        }
        case strtable_t:
        {
            char *string_read = NULL;
            unsigned int size = 0;
            unsigned int j = 0;

            size = atoi(line);
            // printf("string size: %d\n", size);
            if (size)
            {
                string_read = calloc((size + 1), sizeof(char));

                while (j < size)
                {
                    char c = fgetc(file);
                    string_read[j] = c;
                    // printf("%c", c);
                    j++;
                }
                string_read[j] = '\0';
                fgetc(file);
                // printf("j:%d\n", j);
                // printf("sizee: %ld\n", strlen(string_read));
                // printf("\n\n%slala\n\n",string_read);
                add_strconst(string_read);
                free(string_read);
            }

            break;
        }
        case userfunctable_t:
        {
            unsigned int address;
            unsigned int args;
            unsigned int localsize;
            char *fid, *t;
            char *tmp_token = line;

            tmp_token = strtok(tmp_token, " ");
            address = atoi(tmp_token);
            tmp_token = strtok(NULL, " ");
            args = atoi(tmp_token);
            tmp_token = strtok(NULL, " ");
            localsize = atoi(tmp_token);
            tmp_token = strtok(NULL, " ");
            fid = tmp_token;
            // t = fid;
            token = strtok(fid, "\n");

            add_userfunc(address, args, localsize, token);

            // free(t);
            break;
        }
        case libfunctable_t:
        {
            token = strtok(line, "\n");
            add_libfunc(line);
            break;
        }
        case instrtable_t:
        {
            Instruction i;
            i.result = malloc(sizeof(vmarg));
            i.arg1 = malloc(sizeof(vmarg));
            i.arg2 = malloc(sizeof(vmarg));
            memset(i.result, 0, sizeof(vmarg));
            memset(i.arg1, 0, sizeof(vmarg));
            memset(i.arg2, 0, sizeof(vmarg));

            i.result->type = none_a;
            i.arg1->type = none_a;
            i.arg2->type = none_a;

            char *tmp_token = line;

            tmp_token = strtok(tmp_token, " ");
            i.opcode = extract_instr_opcode(tmp_token);
            // printf("opcode: %s\n", tmp_token);

            switch (i.opcode)
            {
            case assign_v:
            {
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.result));
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.arg1));
                break;
            }
            case add_v:
            case sub_v:
            case mul_v:
            case div_v:
            case mod_v:
            case jeq_v:
            case jne_v:
            case jle_v:
            case jge_v:
            case jlt_v:
            case jgt_v:
            case tablegetelem_v:
            case tablesetelem_v:
            {
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.result));
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.arg1));
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.arg2));
                break;
            }
            case newtable_v:
            case jump_v:
            case call_v:
            case pusharg_v:
            case funcenter_v:
            case funcexit_v:
            {
                tmp_token = strtok(NULL, " ");
                extract_instr_vmarg(tmp_token, &(i.result));
                break;
            }
            default:
                break;
            }

            tmp_token = strtok(NULL, " ");

            tmp_token = strtok(tmp_token, ":");
            tmp_token = strtok(NULL, ":");
            i.srcLine = atoi(tmp_token);

            add_instruction(i);
            break;
        }
        default:
            break;
        }
        ++i;
    }
}

int load_binary_file(char *filename)
{
    char buffer[BUFFER_SIZE];
    FILE *file;
    char *last_token;
    const char *delim = "\n";
    unsigned int i = 0;

    unsigned char checkMagicNumber = 0;

    for (i = 0; i < BUFFER_SIZE; ++i)
        buffer[i] = '\0';

    if (strcmp(filename + (strlen(filename) - 4), ".abc"))
    {
        PRINT_RED(stderr, "Wrong binary file extension.\n")
        return 0;
    }

    if (!(file = fopen(filename, "r")))
        return 0;

    /* Check for magic number */
    if (fgets(buffer, BUFFER_SIZE, file))
    {
        last_token = strtok(buffer, delim);
        // printf("%s\n", last_token);
        if (strcmp(last_token, "40434099"))
        {
            PRINT_RED(stderr, "Wrong magic number.\n")
            return 0;
        }
        magic_number = atoi(last_token);
    }

    while (fgets(buffer, BUFFER_SIZE, file))
    {
        // fputs( buffer, stdout );

        // Gets each token as a string and prints it
        last_token = strtok(buffer, delim);
        while (last_token)
        {
            if (last_token[0] == 'S' && last_token[1] == ':')
            {
                totalStrConsts = extract_size(last_token);
                if (totalStrConsts)
                {
                    init_table(totalStrConsts, strtable_t);
                    extract_table_content(file, totalStrConsts, strtable_t);
                    // printf("S content\n");
                }
            }
            else if (last_token[0] == 'N' && last_token[1] == ':')
            {
                totalNumConsts = extract_size(last_token);
                if (totalNumConsts)
                {
                    init_table(totalNumConsts, numtable_t);
                    extract_table_content(file, totalNumConsts, numtable_t);
                    // printf("N content\n");
                }
            }
            else if (last_token[0] == 'U' && last_token[1] == ':')
            {
                totalUserFuncs = extract_size(last_token);
                if (totalUserFuncs)
                {
                    init_table(totalUserFuncs, userfunctable_t);
                    extract_table_content(file, totalUserFuncs, userfunctable_t);
                    // printf("U content\n");
                }
            }
            else if (last_token[0] == 'L' && last_token[1] == ':')
            {
                totalLibFuncs = extract_size(last_token);
                if (totalLibFuncs)
                {
                    init_table(totalLibFuncs, libfunctable_t);
                    extract_table_content(file, totalLibFuncs, libfunctable_t);
                    // printf("L content\n");
                }
            }
            else if (last_token[0] == 'I' && last_token[1] == ':')
            {
                totalInst = extract_size(last_token);
                if (totalInst)
                {
                    init_table(totalInst, instrtable_t);
                    extract_table_content(file, totalInst, instrtable_t);
                    // printf("I content\n");
                }
            }
            else if (last_token[0] == 'G' && last_token[1] == ':')
            {
                totalGlobals = extract_size(last_token);
            }

            // printf("%s\n", last_token);
            last_token = strtok(NULL, delim);
        }
    }

    fclose(file);

    return 1;
}

int main(int argc, char **argv)
{
    FILE *diff_file = NULL;

    if (argc != 2)
    {
        PRINT_RED(stderr, "Wrong arguments! Exiting...\n");
        return 1;
    }

    if (!load_binary_file(argv[1]))
    {
        PRINT_RED(stderr, "Error loading Binary file.\n")
        return 1;
    }

    diff_file = fopen("binary_loaded.abc", "w");
    print_content(diff_file);
    fclose(diff_file);

    /*else
        print_content(stderr);*/

    /*if (!avm_initialize())
        return 1;

    while (!executionFinished)
        execute_cycle();

    avm_mem_free();*/
    // avm_mem_display(stderr);
    Consts_free();
    Instr_free();
    return 0;
}
