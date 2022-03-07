#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include<sys/wait.h>
#include <unistd.h>
#include <limits.h>

#define INPUT_SIZE 300
#define MAX_FORKS_FAILED 3

void recieveInput(char *input);
int getAmpersand(char *input);
void recieveCommand(char input[INPUT_SIZE]);
int failedForks = 0;
char cwd[PATH_MAX];

int main() {
    // reads input from stdin, if valid it will convert, execute, and loop until 'quit' is typed
    while (1) {
        char input[INPUT_SIZE] = {};
        recieveInput(input);

        if (strcmp("quit", input) == 0) {
            break;
        }

        if (strlen(input) == 0) {
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            // inside the child process, execute the command
            recieveCommand(input);
        } else if (pid > 0) {
            // inside the parent process, wait for child to finish
            // if an '&', do not wait
            int ampersandExists = getAmpersand(input);
            if (ampersandExists == 1) {
                wait(NULL);
            }
        } else {
            // fork failed
            failedForks += 1;
            printf("Fork failed, child process not created.\n");
            // if more than the maximum failed forks failed, break out of the program so no infinite loop occurs
            if (failedForks > MAX_FORKS_FAILED) {
                printf("Maximum forks failed, exiting automatically.\n");
                break;
            }
        }
    }
}

void recieveCommand(char input[INPUT_SIZE]) {
    // checks input for UNIX command style and converts to Linux command
    // ex. clr is coverted to clear
    failedForks = 0;
    printf("\n");
    int success = system(input);
    printf("\n");
    int sig;

    if (success == -1) {
        sig = SIGKILL;
    } else {
        sig = SIGTERM;
    }

    kill(getpid(), sig);
}

void recieveInput(char *input) {
    printf("shell = %s> ", getcwd(cwd, sizeof(cwd)));
    // get input from stdin
    fgets(input, INPUT_SIZE-1, stdin);

    // remove any whitespace after the command
    if ((strlen(input) > 0) && (input[strlen(input)-1] == '\n')) {
        input[strlen(input) - 1] = '\0';
    }
}

int getAmpersand(char *input) {
    // check if there is an ampersand in the command
    for (int i = 0; i < INPUT_SIZE-1; i++) {
        if (input[i] == '&') {
            return 1;
        }
    }

    return 0;
}