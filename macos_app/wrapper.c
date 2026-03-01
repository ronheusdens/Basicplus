#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    FILE *dbg = fopen("/tmp/basicpp_wrapper_debug.log", "a");
    if (dbg)
    {
        fprintf(dbg, "wrapper called: argc=%d\n", argc);
        for (int i = 0; i < argc; i++)
            fprintf(dbg, "  argv[%d]=%s\n", i, argv[i]);
        fflush(dbg);
    }

    /* Get directory of this executable */
    char exe_path[1024];
    if (realpath(argv[0], exe_path) == NULL)
    {
        if (dbg)
            fprintf(dbg, "realpath failed\n");
        perror("realpath");
        if (dbg)
            fclose(dbg);
        return 1;
    }

    char dir[1024];
    strcpy(dir, exe_path);
    for (int i = strlen(dir) - 1; i >= 0; i--)
    {
        if (dir[i] == '/')
        {
            dir[i] = '\0';
            break;
        }
    }

    char basicpp_path[1024];
    snprintf(basicpp_path, sizeof(basicpp_path), "%s/basicpp", dir);

    if (dbg)
        fprintf(dbg, "basicpp_path=%s\n", basicpp_path);

    /* Build argument list: wrapper [file] becomes basicpp [file] */
    char *new_argv[3] = {basicpp_path, NULL, NULL};

    if (argc > 1)
    {
        /* File passed from command line or double-click */
        new_argv[1] = argv[1];
        if (dbg)
            fprintf(dbg, "Passing file: %s\n", new_argv[1]);
    }
    else
    {
        if (dbg)
            fprintf(dbg, "No arguments - launching interactive\n");
    }

    if (dbg)
        fclose(dbg);

    /* Execute basicpp with same arguments */
    execv(basicpp_path, new_argv);

    /* If execv fails */
    perror("execv");
    return 1;
}
