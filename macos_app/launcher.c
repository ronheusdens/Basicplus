#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    /* Get the directory of this executable */
    char exe_path[1024];
    if (realpath(argv[0], exe_path) == NULL)
    {
        perror("realpath");
        return 1;
    }

    /* Extract directory by finding the last slash */
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

    /* Build path to basicpp binary */
    char basicpp_path[1024];
    snprintf(basicpp_path, sizeof(basicpp_path), "%s/basicpp", dir);

    /* Build path to shell launcher script */
    char launcher_script[1024];
    snprintf(launcher_script, sizeof(launcher_script), "%s/basicpp_launcher.sh", dir);

    /* Create a temporary script file */
    char script_path[256];
    snprintf(script_path, sizeof(script_path), "/tmp/basicpp_launch_%d.sh", (int)getpid());

    FILE *script = fopen(script_path, "w");
    if (script == NULL)
    {
        perror("fopen");
        return 1;
    }

    /* Write the command to the script, preserving cwd */
    fprintf(script, "#!/bin/bash\n");
    fprintf(script, "cd \"");

    /* Get current working directory and write it */
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)))
    {
        /* Escape special chars in cwd */
        for (const char *p = cwd; *p; p++)
        {
            if (*p == '"' || *p == '\\' || *p == '$' || *p == '`')
                fprintf(script, "\\");
            fprintf(script, "%c", *p);
        }
    }
    fprintf(script, "\"\n");

    fprintf(script, "\"%s\"", launcher_script);

    /* Pass the file argument if provided */
    if (argc > 1)
    {
        fprintf(script, " \"");
        /* Escape special chars in filename */
        for (const char *p = argv[1]; *p; p++)
        {
            if (*p == '"' || *p == '\\' || *p == '$' || *p == '`')
                fprintf(script, "\\");
            fprintf(script, "%c", *p);
        }
        fprintf(script, "\"");
    }
    fprintf(script, "\n");
    fclose(script);

    /* Make it executable */
    chmod(script_path, 0755);

    /* Use osascript to open Terminal and run the script */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "osascript -e 'tell application \"Terminal\"' "
             "-e 'activate' "
             "-e 'do script \"bash %s\"' "
             "-e 'end tell'",
             script_path);

    system(cmd);

    /* Clean up: schedule removal after a delay */
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "sleep 2; rm -f %s &", script_path);
    system(cleanup_cmd);

    return 0;
}
