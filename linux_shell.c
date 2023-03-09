#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 64
char command[MAX_COMMAND_LENGTH];
char *args[MAX_ARGUMENTS];
char *token;
int i, l = 0;
bool back_ground;
char *name, *value;
pid_t pid;
FILE *log_file;

void write_to_log_file()
{  int status;
    waitpid(-1,&status,WNOHANG);
   log_file = fopen("/home/mado/Documents/log.txt", "a");
    fprintf(log_file, "Child process was terminated %d\n",pid);
    fclose(log_file);

}
void setup_enviroment()
{
    chdir("/home/mado");
}
void execute_shell_bultin()
{
    if (strcmp(args[0], "cd") == 0)
    {
        if(args[1]==NULL || strcmp(args[1],"~")==0)
        {
            chdir(getenv("HOME"));
        }
        else if (chdir(args[1]) != 0)
        {
            printf("there is no such file or directory\n");
        }
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        for (int j = 1; args[j] != NULL; j++)
            printf("%s ", args[j]);
        printf("\n");
    }
    else if (strcmp(args[0], "export") == 0)
    {
        char string[50];
        strcpy(string, args[1]);
        char *temp = strtok(string, "=");
        name = temp;
        temp = strtok(NULL, "\"");
        value = temp;
        int result = setenv(name, value, 1);
        if (result != 0)
        {
            perror("setenv");
            exit(EXIT_FAILURE);
        }
    }
}
void executable()
{ // Execute the command
 pid = fork();
     if (pid == 0)
    { // child
        if (execvp(args[0], args) == -1)
        {
            perror("Failed to execute command");
            exit(EXIT_FAILURE);
        }
    }
    else if(!back_ground)
    { 
          waitpid(pid, 0, WUNTRACED);
    }
}
void shell()
{
    do
    {
        l = 0;
        bool dq = false;
        back_ground = false;
        // Prompt the user for input
        printf("\033[31mshell=>\033[0m");
        // Read the user's input
        if (!fgets(command, MAX_COMMAND_LENGTH, stdin))
        {
            perror("Failed to read input");
            exit(EXIT_FAILURE);
        }
        // repeat function if ENTER button is pressed
        if (command[0] == '\n')
        {
            shell();
        }
        // Remove the newline character
        command[strcspn(command, "\n")] = '\0';
        // check for double qoutes then setting the flag dq
        for (int j = 0; command[j] != '\0'; j++)
        {
            if (command[j] == '"')
            {
                dq = true;
                break;
            }
        }

        if (dq)
        {
            token = command;
            args[0] = strsep(&token, " ");
            args[1] = token;
            int len = strlen(args[1]);
            int k;
            if (args[1][0] == '"')
                k = 0;
            else
                k = 2;
            for (; k < len - 1; k++)
            {
                args[1][k] = args[1][k + 1];
            }
            args[1][len - 2] = '\0';
            // substituting the value of the env variable in the args[]
            if (args[1][0] == '$')
            {
                char temp[50];
                l = 0;
                for (k = 1; args[1][k] != '\0'; k++)
                {
                    temp[l++] = args[1][k];
                }
                temp[l] = '\0';
                char *g = getenv(temp);
                strcpy(temp, g);
                l = 1;
                token = strtok(temp, " ");
                while (token != NULL)
                {
                    args[l++] = token;
                    token = strtok(NULL, " ");
                }
                args[l] = NULL;
            }
            else
            {
                args[2] = NULL;
            }
        }
        else
        {
            // Parse the input into tokens
            token = strtok(command, " ");
            i = 0;
            //check for background process &
            while (token != NULL)
            {
                if (strcmp(token, "&") == 0)
                {
                    back_ground = true;
                }
                else
                {
                    args[i++] = token;
                }
                token = strtok(NULL, " ");
            }
            for (int j = 1; j < i; j++)
            {
                if (args[j][0] == '$')
                {
                    char temp[50];
                    l = 0;
                    for (int k = 1; args[1][k] != '\0'; k++)
                    {
                        temp[l++] = args[1][k];
                    }
                    temp[l] = '\0';
                    char *g = getenv(temp);
                    strcpy(temp, g);
                    l = 1;
                    token = strtok(temp, " ");
                    while (token != NULL)
                    {
                        args[l++] = token;
                        token = strtok(NULL, " ");
                    }
                    args[l] = NULL;
                }
            }
            if (l == 0)
            {
                args[i] = NULL;
            }
        }

        int input_type;
        if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "echo") == 0 || strcmp(args[0], "export") == 0)
        {

            input_type = 0;
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            kill(0,SIGKILL);
        }
        else
        {
            input_type = 1;
        }

        switch (input_type)
        {
        case 0:
            execute_shell_bultin();
            break;

        case 1:
            executable();
            break;
        }
    } while (1);
}
int main()
{

    signal(SIGCHLD, write_to_log_file);
    setup_enviroment();
    shell();
    return 0;
}