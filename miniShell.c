#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

#define MAX 80

int main(int argc, char **argv)
{
    char input[MAX];
    char *before;
    char *after;
    char *command[MAX];
    char *command1[MAX];
    char *command2[MAX];
    bool debug;
    bool mode;
    /* ------------------------------- debug mode ------------------------------- */
    if ((argc == 2) && (strcmp(argv[1], "-debug") == 0))
        debug = true;
    if (debug)
    {
        printf("INFO:Father started PID[%d]\n", getpid());
        printf("Welcome to my extended mini shell. Type \"exit\" to terminate\n");
    }
    /* ------------------ endless minishell run till exit code ------------------ */
    while (true)
    {
        mode = false;
        printf("\nminishell>");
        gets(input);
        /* ------------------------------- handle exit ------------------------------ */
        if (strcmp(input, "exit") == 0)
        {
            if (debug)
                printf("\nINFO: Father will terminate\n");
            return 0;
        }
        /* ---------------------------- if pipe charecter --------------------------- */
        if (strchr(input, '|'))
        {
            mode = true;
            /* ---------------------- handle both pipe sides tokens --------------------- */
            before = strtok(input, "|");
            after = strtok(NULL, "|");
            if (after == NULL)
            {
                mode = false;
                printf("Error: NO COMMAND AFTER PIPE\n");
                continue;
            }
            if (debug)
                printf("INFO: Pipe detected. Command 1: \"%s\" and Command 2: \"%s\"\n", before, after);
            /* ----------------------------- handle command1 ---------------------------- */
            command1[0] = strtok(before, " ");
            int cnt = 0;
            while (command1[cnt] != NULL)
                command1[++cnt] = strtok(NULL, " ");
            /* ----------------------------- handle command2 ---------------------------- */
            command2[0] = strtok(after, " ");
            cnt = 0;
            while (command2[cnt] != NULL)
                command2[++cnt] = strtok(NULL, " ");
        }
        /* -------------------------- pipe creating childs -------------------------- */
        if (mode)
        {
            int fd[2];
            int status;
            pid_t pid1, pid2;
            pipe(fd);
            if (debug)
                printf("INFO: Making pipe\n");
            /* ------------------------------- First fork ------------------------------- */
            pid1 = fork();
            if (pid1 == 0)
            {
                if (debug)
                    printf("INFO: Child started PID[%d] command \"%s\"\n", getpid(), before);
                dup2(fd[1], 1);
                //close pipe read end
                close(fd[0]);
                //exec
                execvp(command1[0], command1);
                fprintf(stderr, "%s: command not found\n", command1[0]);
                return 0;
            }
            close(fd[1]);
            /* ------------------------------- Second fork ------------------------------ */
            pid2 = fork();
            if (pid2 == 0)
            {
                if (debug)
                    printf("INFO: Child started PID[%d] command \"%s\"\n\n", getpid(), after);
                dup2(fd[0], 0);
                //exec
                execvp(command2[0], command2);
                printf("%s: command not found\n", command2[0]);
                return 0;
            }
            close(fd[0]);
            /* ------------------------- father Waits for childs ------------------------ */
            waitpid(pid1, NULL, 0);
            waitpid(pid2, &status, 0);
            if (debug)
            {
                printf("\nINFO: Child with PID[%d] terminated\n", pid1);
                printf("INFO: Child with PID[%d] terminated\n", pid2);
            }
        }
        /* --------------------- rno pipe - creating one child --------------------- */
        else
        {
            if (debug)
                printf("INFO: NO pipe detected, creating child for command \"%s\" \n", input);
            command[0] = strtok(input, " ");
            int i = 0;
            while (command[i])
                command[++i] = strtok(NULL, " ");
            /* ------------------------------- fork child ------------------------------- */
            pid_t childpid;
            childpid = fork();
            int s;
            /* ---------------------------------- handle child - no pipe --------------------------------- */
            if (childpid == 0)
            {
                if (debug)
                {
                    printf("INFO: Child started PID [%d] command \"%s\" \n\n", getpid(), input);
                }
                execvp(command[0], command);
                command[0] = strtok(input, " ");
                printf("%s: command not found\n", command[0]);
            }
            /* ------------------------------ handle father ----------------------------- */
            else
            {
                wait(&s);
                if (debug)
                    printf("\nINFO:Child with PID[%d] terminated, continue waiting commands\n", childpid);
            }
        }
    }
}