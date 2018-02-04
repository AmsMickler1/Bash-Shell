// Bash Shell Simulator in C
// COP4610 Intro to Operating Systems
// Amber Mickler and Benjamin Hybart

//DONE: Parse splits line into arguments, expand env vars, output redirection, execution (mostly)
//TODO: resolve pathfiles, input redirection, background, pipes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef enum {false, true} bool;


int isSpecial(char c);
void parse(char* line);
char* expand(char* envVar);
void execute(char** cmd/*, char* path = getenv("HOME")*/);
int count(char* path);
char* resolve(char* path);



int main() {
    char input[256];            // user input will never be longer than 255 chars
    char *user = getenv("USER");
    char *machine = getenv("MACHINE");
    char *pwd;

    char* test="../";
    char* c;
    c=resolve(test);
    printf("%s\n", c);

    do {
        // Grab PWD var here so it updates every loop
        pwd = getenv("PWD");
        printf("%s@%s::%s$ ", user, machine, pwd);
        fgets(input, 256, stdin);              // grabs whole line including \n

        parse(input);
        // do stuff in parse

    } while(1);     //if exit is entered parse will close the shell
}


int isSpecial(char c) {
    if (c == '<' || c == '>' || c == '|' || c == '&')
        return 1;
    else
        return 0;
}   // Returns 1 if char is a special char


void parse(char* line) {
    char** args;
    int index = 0;
    int flag = 0;
    int i;
    int pos = 0;

    // dynamic allocation of args**
    args = (char**)calloc(256, sizeof(char*));
    for (i=0;i<256;i++)
    args[i] = (char*)calloc(256,sizeof(char));

    // parsing into a list of arguments
    for (i=0;i<strlen(line);i++) {
        while (isspace(line[i])) {
            i++;
            pos = i;
        }
        while (!isspace(line[i]) && !isSpecial(line[i])) {
            i++;
            flag = 1;
        }
        if (flag) {
            strncpy(args[index], line+pos, i-pos);
            index++;
            flag = 0;
            i--;
        }
        if (isSpecial(line[i])) {
            strncpy(args[index], line+i, 1);
            index++;
            pos = i+1;
        }
    }
    // side effect: Last argument is always empty
    // solution: make last argument null arg for execv() later on
    strncpy(args[index-1],"NULL", 4);

    // prints list of args for debugging purposes.
    printf("Args: \'%s\'", args[0]);
    for (i=1;i<index;i++) {
        printf(", \'%s\'", args[i]);
    }
    printf("\n");


    /* BUILT INS */
    if (strncmp(args[0], "exit", 4) == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strncmp(args[0], "cd", 2) == 0) {
        //input += 2;     // gets rid of "cd"
        //char* rpath = resolve(input);
        //if ( valid directory )
        //    setenv("PWD", rpath);
    }
    else if (strncmp(args[0], "echo", 4) == 0) {

    }
    else if (strncmp(args[0], "etime", 5) == 0) {

    }
    else if (strncmp(args[0], "io", 2) == 0) {

    }
    else {
        execute(args);
    }


    // free memory
    for (i = 0; i < 256; i++)
    free(args[i]);
    free(args);

}  // end of parse function


// Returns expanded environmental variable when given a string (ex: "$HOME")
// When executing run a for loop to check for $ then call this
char* expand(char* envVar) {
    envVar++;                        // Drops leading $
    return getenv(envVar);
}


//resolve function definition---needs to account for cascading ../ with itself and other directories
char* resolve(char* path) {
    //copies working directory
    char* temp;
    temp=(char*)malloc(200);

    char * cwd;
    cwd = (char*)malloc(200);

    char* dir;
    dir=(char*)malloc(1);

    int i, j, k;
    int track;

    i=0;

    /*if command that must be located $PATH do stuff here*/

    if(path[0]=='~'){
        strcpy(temp, getenv("HOME"));
        i=1;
    }
    else{
        strcpy(temp, getenv("PWD"));
    }

    //if absolute path (e.g. pathname begins with /) update PWD to change directory
    if(path[0]=='/'){
        strcpy(cwd, path);
        return cwd;
    }

    else{
        while(i<strlen(path)){
            //copy path over until first /
            if(path[i]!='/'){
                dir[0]=path[i];
            }
            if(dir[0]=='.' && path[i+1]=='.'){
                track=count(temp);    //will count slashes in current directory
                //copy one less directory
                k=0;
                for(j=0; j < strlen(temp); j++){
                    cwd[j]=temp[j];
                    if(cwd[j]=='/'){
                        k++;
                    }
                    if(k==track)
                    break;

                }
            }

            //if going down a directory (e.g. cd directory)
            else if(path[i]!='.'){
                //add on the directory to the current working directory.

                //strcpy(cwd, temp);
                if(i==0||path[i]=='/'){
                    strcat(cwd, "/");
                }
                if(path[i]!='/')
                    strcat(cwd, dir);
            }
            //increment i
            i++;
        } //end while loop
    } //end else statement

    if(strcmp(path, "../") && strcmp(path, "..")){
        strcat(temp, cwd);  //copy final temp into current working directory
        return temp;
    }
    else{
        while(cwd[strlen(cwd)-1]=='/')
        cwd[strlen(cwd)-1]='\0';
        return cwd;
    }
}  //end function


//count the number of /'s in the path---WORKS PROPERLY
int count(char* path) {
    int i;
    int j=0;
    for(i=0; i < strlen(path); i++){
        if(path[i]=='/')
        j++;
    }

    return j;
}


void execute(char** cmd/*, char* path*/) {
    //char** is array of command and args (if any)
    // the command being executed should always be cmd[0]
    // char** cmd is a copy of args, so we can change it however we want

    int status;
    int count = 0;

    pid_t pid = fork();
    if (pid == -1) {
        //Error
        exit(1);
    }
    else if (pid == 0) {
        //Child
        int inredir = 0;    // input redirect flag
        int oredir = 0;     // output redirect flag
        int pipe = 0;       // pipe flag
        int background = 0; // background flag

        char* input = NULL;        // input redirection filepath
        char* output = NULL;       // output redirection filepath

        int i;
        for (i=0;i<256;i++) {
            count++;
            // If path can be resolved, resolve path and update cmd array

            //char* temp = resolve(cmd[i]);
            //if (temp != NULL) {
            //    strncpy(cmd[i],temp,strlen(temp));
            //}

            // if is isSpecial
            if (isSpecial(cmd[i][0])) {
                // Check Syntax
                if (cmd[i][0] != '&' && (i == 0 || strcmp(cmd[i+1],"NULL") == 0)) {
                    printf("Error: Invalid syntax.\n");
                    exit(1);
                }
                // set background flag
                if (cmd[i][0] == '&' && i != 0)
                    background = 1;
                else {
                    // set redirect flag input
                    if (cmd[i][0] == '<') {
                        inredir = 1;
                        input = resolve(cmd[i+1]);
                    }
                    // set redirect flag output
                    if (cmd[i][0] == '>') {
                        oredir = 1;
                        output = resolve(cmd[i+1]);
                    }
                    cmd[i] = NULL;  // NULL terminates command, otherwise execv will try to execute > as a param
                    break;
                }
                // set pipe flag here, this will probably need more to it
                if (cmd[i][0] == '|') {
                    pipe = 1;
                }

            } // end of IsSpecial() check

            if (cmd[i][0] == '$')
                strncpy(cmd[i],expand(cmd[i]), 255);
            if (strcmp(cmd[i],"NULL") == 0) {
                cmd[i] = NULL;
                break;          //NULL is last arg, so no need to keep looping
            }
        } // end of FOR loop

        // print new args and redirects for debugging
        printf("New args: \'%s\'", cmd[0]);
        for (i=1;i<count;i++) {
            printf(", \'%s\'", cmd[i]);
        }
        printf("\nRedirect to/from: %s/%s\n", output, input);

        // if input redirects
        if (inredir) {
            int fd = open(input, O_RDONLY);
            close(STDIN_FILENO);
            dup(fd);
            close(fd);
        }

        // if output redirects
        if (oredir) {
            int fd = open(output, O_CREAT|O_WRONLY, 0600);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
        }

        execv(cmd[0], cmd);
        printf("Problem executing %s\n", cmd[0]);
        exit(1);
    } // end of child process
    else {
        //Parent
        waitpid(pid, &status, 0);
    }
}



/*
Part 7: Pipes
After it can handle redirection, your shell is capable of emulating the
functionality of pipes. Pipes should behave in the following manner (again,
there does not have to be whitespace between the commands and the symbol):
• CMD1 | CMD2
    ◦ CMD1 redirects its standard output to CMD2's standard input
• CMD1 | CMD2 | CMD3
    ◦ CMD1 redirects its standard output to CMD2's standard input
    ◦ CMD2 redirects its standard output to CMD3's standard input
• CMD1 | CMD2 | CMD3 | CMD4
    ◦ CMD1 redirects its standard output to CMD2's standard input
    ◦ CMD2 redirects its standard output to CMD3's standard input
    ◦ CMD3 redirects its standard output to CMD4's standard input
• Signal an error for the following
    ◦ |
    ◦ CMD |
    ◦ | CMD
*/




/*
Part 8: Background Processing
You will need to handle execution for background processes. There are
several ways this can be encountered:
• CMD &
    ◦ Execute CMD in the background
    ◦ When execution starts, print
        [position of CMD in the execution queue] [CMD's PID]
    ◦ When execution completes, print
        [position of CMD in the execution queue]+ [CMD's command line]
• & CMD
    ◦ Executes CMD in the foreground
    ◦ Ignores &
• & CMD &
    ◦ Behaves the sames as CMD &
    ◦ Ignores first &
• CMD1 | CMD2 &
    ◦ Execute CMD1 | CMD2 in the background
    ◦ When execution starts, print
        [position in the background execution queue] [CMD1's PID] [CMD2's PID]
    ◦ When execution completes, print
        [position in the background execution queue]+ [CMD1 | CMD2 command line]
• CMD > FILE &
    ◦ Follow rules for output redirection and background processing
• CMD < FILE &
    ◦ Follow rules for input redirection and background processing
• Signal an error for anything else
    ◦ Examples includes
        ▪ CMD1 & | CMD2 &
        ▪ CMD1 & | CMD2
        ▪ CMD1 > & FILE
        ▪ CMD1 < & FILE

*/




/*
Part 9: Built-ins -- currently in parse(), may need to be moved
• exit  -- works!
    ◦ Terminates your running shell process and prints “Exiting Shell...”
    ◦ Example
        dennis@linprog3 :: /home/grads/dennis/cop4610t => exit
        Exiting Shell....
        (shell terminates)
• cd PATH
    ◦ Changes the present working directory according to the path resolution
        above
    ◦ If no arguments are supplied, it behaves as if $HOME is the argument
    ◦ Signal an error if more than one argument is present
    ◦ Signal an error if the target is not a directory
• echo
    ◦ Outputs whatever the user specifies
    ◦ For each argument passed to echo
        ▪ If the argument does not begin with “$”
            • Output the argument without modification
        ▪ If the argument begins with “$”
            • Look up the argument in the list of environment variables
            • Print the value if it exists
            • Signal an error if it does not exist
• etime COMMAND
    ◦ Record the start time using gettimeofday()
    ◦ execute the rest of the arguments as per typical execution
        ▪ You don’t have to worry about nesting with other built-ins
    ◦ Record the end time using gettimeofday()
    ◦ Output the elapsed time in the format of s.us where s is the number of
        seconds and us is the number of micro seconds (0 padded)
    ◦ Example
        dennis@linprog3 :: /home/grads => etime sleep 1
        Elapsed Time: 1.000000s
• io COMMAND
    ◦ Execute the supplied commands
        ▪ Again you don’t have to worry about nesting with other built-ins
    ◦ Record /proc/<pid>/io while it executes
    ◦ When it finishes, output each of the recorded values
        ▪ Your output comes after the command finishes
        ▪ Your output needs to be in a table format (unlike the io file)
    ◦ Example
        dennis@linprog3 :: /home/grads => limits sleep 1
        rchar: 0
        wchar: 0
        syscr: 0
        syscw: 0
        read_bytes: 0
        write_bytes: 0
        cancelled_write_bytes: 0

*/
