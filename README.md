# os_lab_2

### 1.0 INTRODUCTION
Simple Shell is a shell made by Alexander Campbell, Atharsan Kennedy, and Joey Villafeurte. This shell can use most simple commands. This manual will outline the functionalities of the shell and how to use its various commands. This shell supports both command line input and batchfile input.

### 2.0 FEATURES

### 2.1 Special Characters

  '&' : Use the amerpsand symbol at the end of your command to have it run within
    the environment you are currently in.

  '>' : Used in I/O redirection, see section 2.3 for more information.

  '>>' : Used in I/O redirection, see section 2.3 for more information.

  '<' : Used in I/O redirection, see section 2.3 for more information.

### 2.2 Basic Commands
  NAME `cd <directory>`

  DESCRIPTION
    The cd command will change the current working directory to the specified directory specified by <directory>. This will change the shell's execution environment to <directory>.

  NAME `clr`

  DESCRIPTION
    The clr command will clear the current screen in the terminal. This includes
    all outputs, previous inputs, etc.

  NAME `dir <directory>`

  DESCRIPTION
    The dir command will list information associated with the directory specified
  by <directory>.

  NAME `environ`

  DESCRIPTION
    The environ command will list all the environment variables as strings in the terminal. Environment variables are similar to commands in that they invoke a process, such as node in Node.js. These are set outside of the terminal.

  NAME `echo <comment>`

  DESCRIPTION
    The echo command will simply output whatever is in <comment>. This command can
  also be leveraged using i/o redirection to print from the output of another
  command (see 2.3 for an example.)

  NAME `help`

  DESCRIPTION
    The help command will display options and commands that are available to the user.

  NAME `pause`

  DESCRIPTION
    The pause command will pause the operation of the shell until more input is
  recieved.

  NAME `quit`

  DESCRIPTION
    The quit command will end the operation of the shell and exit the process.

### 2.3 I/O Redirection
  This shell also supports I/O redirection from and to files. I/O redirection is the process of changing where a program recieves input and outputs the output. This can be done by utlilizing the >, >>, and < characters. > and >> are used to direct output to a specified file while < takes input from a specified file. All the commands specified in the example sections can be used to test the command line. The videos input_test.webm and output_test.webm show case this.

  #### 2.3.1 EXAMPLE

  `adder < in.txt`

  In this example the adder will read whatever is in in.txt and use it as command line arguements to the program adder. This will then output to whatever the total is to the command line.

  #### 2.3.2 EXAMPLE
  
  For context, adder.c is simply a program that adds whatever numbers you pass in as arguements. It is used to demonstrate input and output redirection.

  `adder 10 5 > out.txt`

  In this example, the adder will take in 10 and 5 as the command line arguements. The result of this will then be sent to the file out.txt.

  #### 2.3.3 EXAMPLE

  `adder 10 5 < in.txt > out.txt`

  In this example, the shell will run the program adder using 10 and 5 as the command line arguements. In addition to this, it will read whatever is in in.txt and use that as additional command line arguements. The outout will then be directed to out.txt. Because we used the > it will truncate and overwrite whatever is there, if we wanted to append we would use the >> instead.

### 2.4 Other features
  In some cases a programmer may want to use a batchfile in order to run commands automatically. This can be done by specifying commands in a text file named "Batchfile.txt". The shell will then read the contents of the text file and attempt to run the commands.
