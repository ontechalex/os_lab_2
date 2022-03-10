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
void recieveCommand(char **argv);
void tokenize(char *input,char *op,char *ext);
void StringConCat(char *op, char *ext, char *Combine);
void ShellInputErrorPrint();
int HelpWithFilter(char *HelpFilt);
int matchCharacters(char *input, char *pattern);
char cwd[PATH_MAX];//used for command line prompt 
char path[INPUT_SIZE]; //used for storing cwd for parent env set
int failedForks = 0; //used failed fork attempts in int main()
char *array[INPUT_SIZE + 1]; // allocated memeory for pointer to point
char *END_FLAG = "END"; // represents the end of an array

void ShellInputErrorPrint(){ // Used for printing out list of acceptable input types when wrong user input sent
	printf("NOT A VALID INPUT \nVALID CMD LINE INPUTS \n");
	printf("	<simple method> ./myshell\n	<batchfile method> ./myshell batchfile.txt\n");
	printf("op: cd,dir,help,quit,clr,echo,pause,env\n");
	printf("ext: examples 'hi' - where hi is a folder or something like 'hi there' for echo\n");
}

int main(int argc, char**argv){ // main uses argc to count num of argumments and argv is used to access each argumment 
	int NUMargs = argc; // setting var to number of argumments
	char args[ent][entL]; // used for file extraction with batchfile.txt
	int extract = 0; // recording the number of extracted lines from batchfile.txt
	if (argc == 4 && strcmp(argv[1],"^")==0){// can be used as a by pass to output a shell output
		recieveCommand(argv);
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
	getcwd(path,sizeof(path)); // save a copy of current cwd and then set a parent env that can only be seen once in the shell usign 'env' 
	setenv("parent",strcat(path,"/myshell"),1);
	int cmd = 0; // used mainly for batchfile, to only allow the while loop to loop up to the num of extracted lines  
	while(1){ // cont loop until a quit or up to the num of extracted lines 
		if (cmd > extract){return 0;}
		char input[INPUT_SIZE] = {};// these 3 arrays used for storing input and tokens
		char op[ent] = {};
		char ext[ent] = {};
		recieveInput(input,op,ext,NUMargs,cmd,args);// get the tokens for op ext 
		if (extract > 0){ printf("%s %s",op,ext); sleep(2); } // used with batchfile, print current cmd and sleep until for 2 sec before outputting
		if (strcmp(op,"quit") == 0 && strlen(ext) == 0) { 
		printf("\n"); return 0;} // quit shell
		else if (strcmp(op,"quit") == 0 && strlen(ext) != 0) { printf("\nNOT VALID\n"); continue;} // if quit and an ext is present state error and loop again
		if (strcmp(op, "cd") == 0){ // used for changing directory before forking so child takes new directory
			int chPWD = 0;
			if (extract > 0){ cmd ++; }
    			if(strlen(ext)==0){system("pwd"); }
    			else { 
    				chPWD = chdir(ext); 
    				if (chPWD == -1){ printf("\nUnsuccessful directory change, enivorment var unchanged");}
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
		    if (execv(getenv("parent"),tobeSent) == -1 ) { printf("\nExecv Has faild\n"); return 0; }; // see if execv() fails or not 
		} else if (pid > 0) {
		    // inside the parent process, wait for child to finish
		    if (extract > 0){ cmd++; }// increase cmd bc batchfile was used
		    // if an '&', do not wait
		    int ampersandExists = matchCharacters(ext, "&");// used for seeing if threading is used 
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
void recieveCommand(char **argv) {
    char *op = argv[2];
    char *ext = argv[3];
    // checks input for UNIX command style and converts to Linux command
    // ex. clr is coverted to clear
    if (strlen(ext) == 1 && ext[0] == '!'){ ext[0] = '\0'; } // if ext was null be set to ! beofre excv() was called change it back null 
    char Combine[INPUT_SIZE];// empty array used for concat of op and ext for echo, dir, and help 
    int success; // used for checking if success output or not (-1 being unsuccessful)
    printf("\n\n");
	// sel a op with some checking if ext is present for invalid or different output for that sel op
	// dir, echo, and help use concat function called StringConCat()  for when ext is present 
	// and help uses HelpWithFilter() for when more filter is present and if not returns -1 
    
    if (matchCharacters(ext, "<") == 1 || matchCharacters(ext, ">")) { // user is attempting to use some form of i/o redirection
    	char extCopy[strlen(ext) * sizeof(char)];
    	int extLength = strlen(ext);
    	strcpy(extCopy, ext);
		char *args[100]; // split up ext in args[] for easier parsing
		char *ptr = strtok(extCopy, " ");
		int i = 0;
		
		while (ptr != NULL) {
			args[i] = ptr;
			ptr = strtok(NULL, " ");
			i++;
		}   
		
		i--;
		
		char *inputFile;
		char *outputFile;
		char *file1;
		char *file2;
		
		int file1Exists = matchCharacters(args[i], ".txt"); // checks if there is a first file
		
		if (file1Exists == 1) {
			file1 = args[i];
			extLength -= strlen(file1);
			int outputRedirect = (matchCharacters(args[i-1], ">")); // check if file1 is for output
			int inputRedirect = (matchCharacters(args[i-1], "<")); // check if file1 is for input
			
			if (outputRedirect == 1) {
				outputFile = file1;
				args[i-1] = END_FLAG;
			} else if (inputRedirect == 1) {
				inputFile = file1;
				args[i-1] = END_FLAG;
			} else {
				puts("Must state either '>' or '<' to choose where file is directed");
				success = -1;
				args[i] = END_FLAG;
			}
		} else {
			args[i+1] = END_FLAG;
		}

		if (i >= 3) {
			int file2Exists = matchCharacters(args[i-2], ".txt"); // checks if there is a second file
			
			if (file2Exists == 1) {
				file2 = args[i-2];
				extLength -= strlen(file2);
				int outputRedirect = (matchCharacters(args[i-3], ">")); // check if file2 is for output
				int inputRedirect = (matchCharacters(args[i-3], "<")); // check if file2 is for input
				
				if (outputRedirect == 1) {
					outputFile = file2;
					args[i-3] = END_FLAG;
				} else if (inputRedirect == 1) {
					inputFile = file2;
					args[i-3] = END_FLAG;
				} else {
					puts("Must state either '>' or '<' to choose where file is directed");
					success = -1;
					args[i-2] = END_FLAG;
				}
			}
		}
		
		if (outputFile != NULL) { // change stdout to the specified output file
			if (matchCharacters(ext, ">>") == 1) {
				freopen(outputFile, "a+", stdout); // append if '>>' character
			} else if (matchCharacters(ext, ">") == 1) {
				freopen(outputFile, "w+", stdout); // truncate if '>' character
			}
		}

		
		char extStr[extLength + 60];
		strcpy(extStr, "");

		i = 0;
		
		while (1) {
			if (args[i] == END_FLAG) {
					break;
				}
				
			strcat(extStr, args[i]);
			strcat(extStr, " ");
			i++;
		}
		
		if (inputFile != NULL) { // reads from the file and appends it to the extStr
			char currentDir[100] = "./";
			strcat(currentDir, inputFile);
			
			FILE *fp; 
			fp = fopen(currentDir, "r"); // read from file 
			
			if (fp == NULL) {
				printf("Unable to open input file %s\n", currentDir);
			} else {
				char buf[60];
				
				if (fgets(buf, 60, fp) != NULL) {
					strcat(extStr, buf);
				} 
				
				fclose(fp);
			}
		}
		
		strcat(extStr, "\0");
		strcpy(ext, extStr); // fixes the ext string to only be commands
    }
	
	if (strcmp(op,"clr") == 0){ 
	if(strlen(ext) == 0) { success = system("clear");}
	else { success = -1; }
    }else if (strcmp(op, "dir") == 0) {
    	if(strlen(ext) == 0) { success = system("dir");}
    	else { StringConCat(op,ext,Combine); success = system(Combine);}
    } else if (strcmp(op, "echo") == 0) { 
    	if (strlen(ext) == 0) {success = system("echo");}
    	else{ StringConCat(op,ext,Combine); success = system(Combine);}
    } else if (strcmp(op, "help") == 0) {
    	if (strlen(ext) == 0) { success = HelpWithFilter(op);}
    	else { StringConCat(op,ext,Combine); success = HelpWithFilter(Combine);}
    } else if (strcmp(op, "pause") == 0) {
    	if (strlen(ext) != 0) { success = -1; }
    	else { // pause will wait for a enter key '\n' to continue
    		char resume;
    		while (1) {  
    			printf("\nPress Enter Key to continue..");
    			resume=fgetc(stdin); 
    			if(resume == '\n'){break;}
    		} success = 0;
    	}
    } else if (strcmp(op, "env") == 0) {
    	if(strlen(ext) == 0) { success = system("env");}
    	else { success = -1; }
    } else if (access(op, F_OK) == 0) { // checks if input op is a file
		int opLength = strlen(ext) + 2;
		
		char opStr[opLength];
		strcpy(opStr, "./");
		strcat(opStr, op);

		StringConCat(opStr, ext, Combine);
		
		success = system(Combine);
	
    } else { success = -1; } // if none of the op were selected then unsuccessful and print NOT VALID 
    
    int sig; // this portion is used to end child process that is acting as a parent after being excv() 
    if (success == -1) {
    	printf("\nNOT VALID\n");
        sig = SIGKILL; // exit child process 
    } else {
        sig = SIGTERM; // if not exiting then terminate child process 
    }
    
    freopen("/dev/tty", "w", stdout); // resets the stdout
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
    
int matchCharacters(char *input, char *pattern) {
	char *matchingChars = strstr(input, pattern);
	
	if (matchingChars != NULL) { return 1; } // if there's matching letters then matchingChars is not null, return 1
	
	return 0; // otherwise will return a 0 indicating the pattern cannot be found
}
