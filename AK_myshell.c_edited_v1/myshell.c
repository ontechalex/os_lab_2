#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
//----------------^
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include<sys/wait.h>
#include <unistd.h>
#include <limits.h>

#define INPUT_SIZE 300
#define MAX_FORKS_FAILED 3
//----------------v
#define ent 20
#define entL 20

void recieveInput(char *input,char *op,char *ext,int argc,int cmd,char (*args)[entL]);
int getAmpersand(char *input);
void recieveCommand(char *op,char *ext);
void tokenize(char *input,char *op,char *ext);
bool Scmp(char *s1, char *s2);
char cwd[PATH_MAX];
char path[INPUT_SIZE];
char path2[INPUT_SIZE];
char *envvar = "parent";
int failedForks = 0;
char *array[INPUT_SIZE + 1]; // allocated memeory for pointer to point

int main(int argc, char*argv[]){
	char args[ent][entL]; // used for file extraction with batchfile.txt
	int extract = 0; // recording the number of extracted lines from batchfile.txt
	if (argc == 2){
		FILE *pToFile = fopen(argv[1],"r");
		while( fgets(args[extract],INPUT_SIZE,pToFile)){
			args[extract][strlen(args[extract]) -1] = '\0';
			extract++;
		}
		fclose(pToFile);
		if(!getenv(envvar)){
			printf("\nThe %s env var could not be obtained.\n", envvar);
			return 0;
		} 
		strcpy(path,getenv(envvar));
    		printf("PATH: %s\n", path);
	}
	int cmd = 0;
	while(1){
		if (cmd > extract){break;}
		char input[INPUT_SIZE] = {};
		char op[ent] = {};
		char ext[ent] = {};
		recieveInput(input,op,ext,argc,cmd,args);
		if (Scmp(op,"quit")== true && strlen(ext) == 0){return 0;}
		else if (Scmp(op,"quit")== true && strlen(ext) != 0) { printf("\nNOT VALID\n"); continue;}
		if (Scmp(op,"cd") == true){
			int chPWD = 0;
			if (extract > 0){ printf("%s %s",op,ext); cmd ++; sleep(2); }
    			if(strlen(ext)==0){system("pwd"); }
    			else { 
    				chPWD = chdir(ext); 
    				if (chPWD == -1){ perror("Unsuccessful directory chnage, enivorment var unchanged");}
    				else { printf("\n//Dir changed ||> %s\n\n", getcwd(cwd, sizeof(cwd))); }
    			} 
    			fflush(stdout);
    			continue;
    		} 
		if (strlen(op) == 0){continue;}
		fflush(stdout);
		pid_t pid = fork();
		if (pid == 0) {
		    // inside the child process, execute the command
		    //execv(path,argv[0]);
		    if (extract > 0){ printf("%s %s",op,ext); sleep(2); }
		    recieveCommand(op,ext);
		} else if (pid > 0) {
		    // inside the parent process, wait for child to finish
		    if (extract > 0){ cmd++; }
		    // if an '&', do not wait
		    int ampersandExists = getAmpersand(input);
		    if (ampersandExists == 1) {
		        wait(NULL);
		    } else { while(wait(NULL) > 0); }
		    fflush(stdout);
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

bool Scmp(char *s1, char *s2){
	if(strlen(s1) != strlen(s2)) {return false;}
	bool same = false;
	if(strlen(s1) == strlen(s2)) {same = true;
		for(int i = 1; i<strlen(s1) ;i++){
			if(s1[i] != s2[i]){ same = false; break;}
		}
	}
	return same;
}

void recieveCommand(char *op,char *ext) {
    // checks input for UNIX command style and converts to Linux command
    // ex. clr is coverted to clear
    int success;
    failedForks = 0;
    printf("\n");
    if (Scmp(op,"clr") == true){ success = system("clear");
    }else if (Scmp(op,"dir") == true){
    	if(strlen(ext) == 0){ success = system("dir");}
    	else{ strcat(op," "); strcat(op,ext); success = system(op);}
    } else if (Scmp(op,"echo") == true){
    	if (strlen(ext) == 0){success = system("echo");}
    	else{ strcat(op," "); strcat(op,ext); success = system(op);}
    } else if (Scmp(op,"help") == true){
    	if (strlen(ext) == 0){ success = system("bash -c help"); }
    	else { success = system("bash -c help"); }//success = -1; printf("\nNOT VALID\n"); }
    } else if (Scmp(op,"pause") == true){
    	if (strlen(ext) != 0) { success = -1; printf("\nNOT VALID\n"); }
    	else {
    		char resume;
    		while (1){  
    			printf("\nPress Enter Key to continue..");
    			resume=fgetc(stdin); 
    			if(resume == '\n'){break;}
    		}success = 0;
    	}
    } else if (Scmp(op,"env") == true){
    	if(strlen(ext) == 0){ success = system("env");}
    	else { success = -1; printf("\nNOT VALID\n"); }
    }else { success = -1; printf("\nNOT VALID\n");}
    

    //success = system(op);
    printf("\n");
    int sig;
    if (success == -1) {
        sig = SIGKILL;
    } else {
        sig = SIGTERM;
    }
	fflush(stdout);
    kill(getpid(), sig);
}

void tokenize(char*input,char *op,char *ext){
	char *text[2];
	bool space = false;
	for (int i = 0;i<strlen(input);i++){
		if (input[i] == ' '){ space = true; break;}
	}
	if (!space){  strcpy(op,input); memset(ext,'\0',sizeof(ext));  }
	else{
		char *splitPoint = strtok(input," ");//initial split 
		int i = 0;//indexing text array
		while (splitPoint != NULL){
			text[i++] = splitPoint;// capture input after split
			splitPoint = strtok(NULL,"\0"); //cont to give pieces of last string
		}
		strcpy(op,text[0]);
		strcpy(ext,text[1]);
	} 
	op[strlen(op)] = '\0'; ext[strlen(ext)] = '\0';
}

void recieveInput(char *input,char *op, char *ext,int argc,int cmd,char (*args)[entL]){
    // get input from stdin
	if(argc == 1){
		printf("shell = %s> ", getcwd(cwd, sizeof(cwd)));
		fgets(input, INPUT_SIZE-1, stdin);
	} else {
		printf("shell = %s> ", getcwd(cwd, sizeof(cwd))); 
		strcpy(input,args[cmd]);
		}
    // remove any whitespace after the command
    	if ((strlen(input) > 0) && (input[strlen(input)-1] == '\n')) {
        	input[strlen(input) - 1] = '\0';
    	}
	tokenize(input,op,ext);
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
