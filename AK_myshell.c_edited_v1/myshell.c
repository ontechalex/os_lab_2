//----------------v get rid of warnings when compiled
#define _CRT_SECURE_NO_WARNINGS
//----------------v Important includes used for the shell
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include<sys/wait.h>
#include <unistd.h>
#include <limits.h>
//----------------v setting a int main() fork failed limit, a input limit size for either user or batchfile input 
#define INPUT_SIZE 300
#define MAX_FORKS_FAILED 3
//----------------v ent & entL used for creating a 2d array for storing batchfile lines 
#define ent 20 
#define entL 20
//----------------v functions, arrays, var, and counter used for the shell
void recieveInput(char *input,char *op,char *ext,int argc,int cmd,char (*args)[entL]);
int getAmpersand(char *input);
void recieveCommand(char *op,char *ext);
void tokenize(char *input,char *op,char *ext);
bool Scmp(char *s1, char *s2);
void StringConCat(char *op, char *ext, char *Combine);
void ShellInputErrorPrint();
int HelpWithFilter(char *HelpFilt);
char cwd[PATH_MAX];//used for command line prompt 
char path[INPUT_SIZE]; //used for excv after fork has been called
char *envvar = "parent"; //setting the env var to retrive
int failedForks = 0; //used failed fork attempts in int main()
char *array[INPUT_SIZE + 1]; // allocated memeory for pointer to point

void ShellInputErrorPrint(){ // Used for printing out list of acceptable input types when wrong user input sent
	printf("NOT A VALID INPUT \nVALID CMD LINE INPUTS \n");
	printf("	./myshell\n	./myshell batchfile.txt\n	./myshell ^ op ext\n");
	printf("op: cd,dir,help,quit,clr,echo,pause,env\n");
	printf("ext: examples 'hi' - where hi is a folder or something like 'hi there' for echo\n");
	printf("for (^) input, ext filter with help you must type cmd line as \n ./myshell ^ help '| filter' \n");
}

int main(int argc, char**argv){ // main uses argc to count num of argumments and argv is used to access each argumment 
	int NUMargs = argc; // setting var to number of argumments
	char args[ent][entL]; // used for file extraction with batchfile.txt
	int extract = 0; // recording the number of extracted lines from batchfile.txt
	if (argc == 4 && strcmp(argv[1],"^")==0){// can be used as a by pass to output a shell output
		recieveCommand(argv[2],argv[3]);
		return 0;
	} else if (argc == 2){ // used for batchfile extraction
		if (strcmp(argv[1],"batchfile.txt")==0){ // if batchfile found preform the extracton 
			FILE *pToFile = fopen(argv[1],"r");
			while( fgets(args[extract],INPUT_SIZE,pToFile)){ // extract each line from file
				args[extract][strlen(args[extract]) -1] = '\0';
				extract++;
			}
			extract = extract - 1;
			fclose(pToFile);
    	} else { ShellInputErrorPrint(); return 0; }// if batchfile not found state what is acceptable user input 
	} else if (argc != 1) { ShellInputErrorPrint(); return 0; }
	if(!getenv(envvar)){ // get the path for the parent env var and see if false for not found  
		printf("\nThe %s env var could not be obtained.\n", envvar);
		return 0;
	} 
	strcpy(path,getenv(envvar));// copy the paretn env to path global array
	int cmd = 0; // used mainly for batchfile, to only allow the while loop to loop up to the num of extracted lines  
	while(1){ // cont loop until a quit or up to the num of extracted lines 
		if (cmd > extract){return 0;}
		char input[INPUT_SIZE] = {};// these 3 arrays used for storing input and tokens
		char op[ent] = {};
		char ext[ent] = {};
		recieveInput(input,op,ext,NUMargs,cmd,args);// get the tokens for op ext 
		if (extract > 0){ printf("%s %s",op,ext); sleep(2); } // used with batchfile, print current cmd and sleep until for 2 sec before outputting
		if (Scmp(op,"quit")== true && strlen(ext) == 0){ printf("\n"); return 0;} // quit shell
		else if (Scmp(op,"quit")== true && strlen(ext) != 0) { printf("\nNOT VALID\n"); continue;} // if quit and an ext is present state error and loop again
		if (Scmp(op,"cd") == true){ // used for changing directory before forking so child takes new directory
			int chPWD = 0;
			if (extract > 0){ cmd ++; }
    			if(strlen(ext)==0){system("pwd"); }
    			else { 
    				chPWD = chdir(ext); 
    				if (chPWD == -1){ printf("\nUnsuccessful directory chnage, enivorment var unchanged");}
    				else { printf("\n//Dir changed ||> %s\n\n", getcwd(cwd, sizeof(cwd))); }
    			} 
    			fflush(stdout); // make sure to flush stdout to remove repetiton that can cause input problems in the next iter
    			continue;
    		} 
		if (strlen(op) == 0){continue;}// if enter was hit with no op loop again and wait for user input
		fflush(stdout);// make sure to flush stdout to remove repetiton that can cause input problems in the next iter
		pid_t pid = fork();// used for forking before execv() to the same program in child
		if (pid == 0) {
		    // inside the child process, execute the command
		    if (strlen(ext) == 0) { ext[0] = '!'; } // used if ext has nothing, makes sure execv() only takes one null
		    char *tobeSent[] = {"./myshell","^",op,ext,NULL};
		    if (execv(path,tobeSent) == -1 ) { printf("\nExecv Has faild\n"); return 0; }; // see if execv() fails or not 
		} else if (pid > 0) {
		    // inside the parent process, wait for child to finish
		    if (extract > 0){ cmd++; }// increase cmd bc batchfile was used
		    // if an '&', do not wait
		    int ampersandExists = getAmpersand(input);// used for seeing if threading is used 
		    if (ampersandExists == 1) {
		        wait(NULL);
		    } else { while(wait(NULL) > 0); } // if threading or not the parent will wait for child to finish
		    fflush(stdout); // make sure to flush stdout to remove repetiton that can cause input problems in the next iter
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
// used for checking if string op matches op that was sent into recieveCommand()
bool Scmp(char *s1, char *s2){ // if length is not the same then op's are not same, do not cont checking
	if(strlen(s1) != strlen(s2)) {return false;}
	bool same = false; 
	if(strlen(s1) == strlen(s2)) {same = true; // initially think that the string arrays are the same 
		for(int i = 1; i<strlen(s1) ;i++){ // go through both string arrays to see if something is diff
			if(s1[i] != s2[i]){ same = false; break;} // if diff then set same to false and stop checking
		}
	}
	return same;
}
// used for combining the op and ext for dir,echo,and help
// sent the empty combined string array with the filled op and ext string arrays 
void StringConCat(char *op, char *ext, char *Combine){
	int L = strlen(op) + strlen(ext) + 1; // used for total lenght limiter for for loop 
	memset(Combine,'\0',L); // make sure to set Combine to null 
	int i = 0; // used for indexing for op and ext
	for(int i = 0; i<L ;i++){ // putting info one character at a time into combine string array 
		if (i < strlen(op)){ Combine[i] = op[i]; continue; } // i will go up to len of op string array
		else if (i == strlen(op)) { Combine[i] = ' '; continue; } // once i is done with op set a space bf ext
		Combine[i] = ext[i - (strlen(op) + 1)]; // make sure indexing in ext starts at the correct place   
	}
	Combine[L] = '\0'; // once everything has been extracted make sure a null is at the end of the combined  string array 
}
// used foe help | more 
int HelpWithFilter(char *HelpFilt){
	pid_t pidH = fork(); // fork and execl to bash file that will allow for help  | more to work
	if (pidH == 0){ // child process will execl to allow user to use more filter with help or for just help  with no filter 
		if (execl("/bin/bash", "bash", "-c", HelpFilt, NULL) == -1){ // check if execl fails or not return -1 
			printf("\nExecl for Help with filters Has faild\n"); return -1;
		}
	} else if (pidH > 0){ // parent will wait for child to finish
		while(wait(NULL) > 0);
	} else { // for when fork will fail a -1 is returned
		printf("Fork failed, child process not created.\n"); return -1;
	}
	return 0; // if fork and execl worked send a 0 back
}
// send op and ext to recieveCommand() for desired output from either user input or batchfile
void recieveCommand(char *op,char *ext) {
    // checks input for UNIX command style and converts to Linux command
    // ex. clr is coverted to clear
    if (strlen(ext) == 1 && ext[0] == '!'){ ext[0] = '\0'; } // if ext was null be set to ! beofre excv() was called change it back null 
    char Combine[INPUT_SIZE];// empty array used for concat of op and ext for echo, dir, and help 
    int success; // used for checking if success output or not (-1 being unsuccessful)
    printf("\n\n");
	// sel a op with some checking if ext is present for invalid or different output for that sel op
	// dir, echo, and help use concat function called StringConCat()  for when ext is present 
	// and help uses HelpWithFilter() for when more filter is present and if not returns -1 
    if (Scmp(op,"clr") == true){ success = system("clear");
    }else if (Scmp(op,"dir") == true){
    	if(strlen(ext) == 0){ success = system("dir");}
    	else { StringConCat(op,ext,Combine); success = system(Combine);}
    } else if (Scmp(op,"echo") == true){
    	if (strlen(ext) == 0){success = system("echo");}
    	else{ StringConCat(op,ext,Combine); success = system(Combine);}
    } else if (Scmp(op,"help") == true){
    	if (strlen(ext) == 0){ success = HelpWithFilter(op);}
    	else { StringConCat(op,ext,Combine); success = HelpWithFilter(Combine);}
    } else if (Scmp(op,"pause") == true){
    	if (strlen(ext) != 0) { success = -1; printf("\nNOT VALID\n"); }
    	else { // pause will wait for a enter key '\n' to continue
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
    }else { success = -1; printf("\nNOT VALID\n");} // if non of the op were sel then unsuccessful and print NOT VALID 
    
    printf("\n");
    int sig; // this portion is used to end child process that is acting as a parent after being excv() 
    if (success == -1) {
        sig = SIGKILL; // exit child process 
    } else {
        sig = SIGTERM; // if not exiting then terminate child process 
    }
	fflush(stdout);
    kill(getpid(), sig);// end child process to go back to orginal parent
}
// tokenize() used for sperating input into two string arrays op and ext 
void tokenize(char*input,char *op,char *ext){ // sent filled input array, and empty op,ext array to tokenize()
	char *text[2]; // create a 2d array for storing op and ext before setting them to op and ext array 
	bool space = false; // space used for detcting if ext or no ext is present, no ext is set to null
	for (int i = 0;i<strlen(input);i++){
		if (input[i] == ' '){ space = true; break;}
	}
	if (!space){  strcpy(op,input); memset(ext,'\0',sizeof(ext));  } // set op to input and make sure ext is just null
	else{ // if space found perform tokenize on input
		char *splitPoint = strtok(input," ");//initial split 
		int i = 0;//indexing text array
		while (splitPoint != NULL){ 
			text[i++] = splitPoint;// capture input after split
			splitPoint = strtok(NULL,"\0"); //cont to give pieces of last string, this will go up to the end of input for ext 
		}
		strcpy(op,text[0]); // set op and ext with stored info from text 2d array
		strcpy(ext,text[1]);
	} 
	op[strlen(op)] = '\0'; ext[strlen(ext)] = '\0'; // make sure null is at the end of the op and ext 
}
// send:[empty: input array , op array, ext array], Numargs, which line extracted the main while loop is on, and the 2d array extracted from batchfile if used to recieveInput()
void recieveInput(char *input,char *op, char *ext,int argc,int cmd,char (*args)[entL]){
    // get input from stdin
	if(argc == 1){ // used when argc is just 1 for ./myshell
		printf("shell = %s> ", getcwd(cwd, sizeof(cwd)));
		fgets(input, INPUT_SIZE-1, stdin);// get user input bf tokenizen for op and ext
	} else { // used for batchfile bf tokenize for op and ext 
		printf("shell = %s> ", getcwd(cwd, sizeof(cwd))); 
		strcpy(input,args[cmd]);// copy the line entry that was extracted from batchfile 
		}
    // remove any whitespace after the command
    	if ((strlen(input) > 0) && (input[strlen(input)-1] == '\n')) {
        	input[strlen(input) - 1] = '\0';// make sure that null is in the correct place bf tokenize
    	}
	tokenize(input,op,ext); // send input to be tokenize for op and ext of the input
}
//used for threading, checks if '&' is at the end of user cmd line input  
int getAmpersand(char *input) {
    // check if there is an ampersand in the command
    for (int i = 0; i < INPUT_SIZE-1; i++) {
        if (input[i] == '&') {
            return 1;
        }
    }

    return 0;
}
