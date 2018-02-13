// Bash Shell Simulator in C
// COP4610 Intro to Operating Systems
// Amber Mickler and Benjamin Hybart

//DONE: Parse splits line into arguments, expand env vars, input & output redirection, execution (mostly)
//TODO: resolve pathfiles, background, pipes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

typedef enum {false, true} bool;

typedef struct {
    int process[100];
    int finished[100];
    int size;
    char command[256][256];
} ExQueue;


int isSpecial(char c);
void parse(char* line);
char* expand(char* envVar);
void execute(char** cmd);
int count(char* path);
char* resolve(char* path);
int isDir(const char *path);
char * catcat(char ** b, int elements);

ExQueue ExQ;

int main() {
    char input[256];            // user input will never be longer than 255 chars
    char *user = getenv("USER");
    char *machine = getenv("MACHINE");
    char *pwd;
    int status;

    char* test="../";                                                                      //REMOVE LATER
    char* c;
    c=resolve(test);
    printf("%s\n", c);

    int j;
    ExQ.size = 0;
    for (j = 0; j < 100; j++)   // initialize all spots in queue to "finished"
        ExQ.finished[j] = 2;

    do {
        // Grab PWD var here so it updates every loop
        pwd = getenv("PWD");
        printf("%s@%s::%s$ ", user, machine, pwd);
        fflush(stdin);
        fflush(stdout);
        fgets(input, 256, stdin);              // grabs whole line including \n

        parse(input);
        // do stuff in parse
        printf("FUCK");
        fflush(stdout);

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

    /* BUILT INS */
    if (strcmp(args[0], "exit") == 0) {
        int j;
        int status;
        while (ExQ.size != 0) {
            for (j = 0; j < ExQ.size; j++) {
                if (waitpid(ExQ.process[j], &status, WNOHANG) != 0) { // if background process has finished
                    printf("[%i]\t[%s]\n", j, ExQ.command[j]);
                    ExQ.finished[j] = 0;
                    // if finished move others up in Queue
                    int k = j+1;
                    while (ExQ.finished[k-1] == 0) {
                    // while the process before me is finished but I am not
                        ExQ.finished[k-1] = ExQ.finished[k];
                        strcpy(ExQ.command[k-1], ExQ.command[k]);
                        ExQ.process[k-1] = ExQ.process[k];
                        k++;
                    }
                    j--;
                    ExQ.size--;
                }
            }
        }
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        char* rpath = resolve(args[1]);
        if (index > 3)                                  // "cd" "path" "NULL"
            printf("Error: Too many arguments.\n");
        else if (isDir(rpath)) {
            setenv("PWD", rpath, 1);
            chdir(rpath);
        }
        else
            printf("Error: Please enter a valid directory.\n");
    }
    else if (strcmp(args[0], "echo") == 0) {
        int invalid = 0;
        for (i=1; i<index-1; i++) {
            if (args[i][0] == '$') {
                if (expand(args[i]) == NULL)
                    invalid = 1;
            }
        }
        if (!invalid) {
            for (i=1; i<index-1; i++) {
                if (args[i][0] == '$')
                    printf(expand(args[i]));
                else
                    printf(args[i]);
                printf(" ");
            }
        printf("\n");
        }
    }
    else if (strcmp(args[0], "etime") == 0) {
        struct timeval start, end;
        gettimeofday(&start,NULL);
        execute(args+1);
        gettimeofday(&end,NULL);
        time_t sec = end.tv_sec - start.tv_sec;
        time_t usec = end.tv_usec - start.tv_usec;
        printf("Elapsed time: %i.%06is\n", sec, usec);
    }

    else {
        execute(args);
    }

    //free memory
    for (i = 0; i < 256; i++)
        free(args[i]);
    free(args);

}  // end of parse function

// Returns nonzero value if path points to a valid directory
int isDir(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}


// Returns expanded environmental variable when given a string (ex: "$HOME")
// When executing run a for loop to check for $ then call this
char* expand(char* envVar) {
    envVar++;                        // Drops leading $
    if (getenv(envVar) == NULL)
        printf("%s: Undefined Variable.\n", envVar);
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
    int FLAG=0;
    i=0;

    if(path[0]=='~'){
        strcpy(temp, getenv("HOME"));
        i=1;
    }
    else{
        strcpy(temp, getenv("PWD"));
    }

    strcpy(cwd, temp);
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

                if(isalpha(path[i-2])){
                track=count(cwd);}
                else
                track=count(temp);   //will count slashes in current directory
                //copy one less directory
                k=0;
                for(j=0; j < strlen(cwd); j++){
                    cwd[j]=temp[j];
                    if(cwd[j]=='/'){
                        k++;
                    }
                    if(k==track){
                        FLAG=1;
                        cwd[j]='\0';
                        //i++;
                    break;
                    }
                }
            }
            //if going down a directory (e.g. cd directory)
            else if(path[i]!='.'){
                //add on the directory to the current working directory.
                //strcpy(cwd, temp);
                if(i==0 ||path[i]=='/'&& (cwd[strlen(cwd)-1]!='/')){
                    strcat(cwd, "/");
                }
                if(path[i]!='/'){
                    strcat(cwd, dir);
                }
            }
            if(FLAG==1){
                strcpy(temp, cwd);
                temp[strlen(cwd)]='\0';
                FLAG=0;
            }
            //increment
            dir[0]='0';
            i++;
        } //end while loop
    } //end else statement
    {
        while(cwd[strlen(cwd)-1]=='/')
        cwd[strlen(cwd)-1]='\0';
        return cwd;
    }

    free(temp);
    free(cwd);
    free(dir);

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

void execute(char** cmd) {
    //char** is array of command and args (if any)
    // the command being executed should always be cmd[0]
    // char** cmd is a copy of args, so we can change it however we want

    int status;
    int count = 0;

    int inredir = 0;            // input redirect flag
    int oredir = 0;             // output redirect flag
    int pipe = 0;               // pipe flag
    int background = 0;         // background flag
    int error = 0;              // error flag
    int io = 0;                 // io command flag

    char* input = NULL;         // input redirection filepath
    char* output = NULL;        // output redirection filepath

    int i;
    for (i=0;i<256;i++) {
        count++;

        if (cmd[0][0] == '&') {   // ignores leading &
            cmd++;
            count--;
        }

        if (strcmp(cmd[0], "io") == 0) {
            io = 1;
            cmd++;
            //count--;
        }

        // if is isSpecial
        if (isSpecial(cmd[i][0])) {
            // Check Syntax
            if (error == 0 && cmd[i][0] != '&' && (i == 0 || strcmp(cmd[i+1],"NULL") == 0)) {
                printf("Error: Invalid syntax.\n");
                error = 1;
            }
            else if (error == 0 && isSpecial(cmd[i-1][0])) {
                printf("Error: Invalid syntax.\n");
                error = 1;
            }
            else if (cmd[i][0] == '&' && strcmp(cmd[i+1], "NULL") != 0) {
                printf("Error: Invalid syntax.\n");
                error = 1;
            }
            // set background flag
            if (cmd[i][0] == '&' && i != 0)
                background = 1;
            else {
                // set redirect flag input
                if (cmd[i][0] == '<') {
                    inredir = i;
                    input = resolve(cmd[i+1]);
                }
                // set redirect flag output
                if (cmd[i][0] == '>') {
                    oredir = i;
                    output = resolve(cmd[i+1]);
                }
            }
            // set pipe flag here, this will probably need more to it
            if (cmd[i][0] == '|') {
                pipe++;
                //cmd[i] = NULL; This will give a seg fault :D
            }

        } // end of IsSpecial() check
        if (cmd[i][0] == '~') {
            strncpy(cmd[i], resolve(cmd[i]), 255);
        }
        if (cmd[i][0] == '$') {
            if (expand(cmd[i]) != NULL)
                strncpy(cmd[i],expand(cmd[i]), 255);
        }
        if (strcmp(cmd[i],"NULL") == 0) {
            cmd[i] = NULL;
            break;          //NULL is last arg, so no need to keep looping
        }
    } // end of FOR loop
    // Put null after first redirect character to mark end of command and start of redirection
    // This is so execv doesn't try to grab the special chars & filepath as parameters
    char * str = catcat(cmd, count-2);  // for background processing

    if (inredir != 0 && oredir != 0) {
        if (inredir < oredir)
            cmd[inredir] = NULL;
        else
            cmd[oredir] = NULL;
        count -= 4;
    }
    else if (inredir != 0 && oredir == 0) {
        cmd[inredir] = NULL;
        count =- 2;
    }
    else if (oredir != 0 && inredir == 0) {
        cmd[oredir] = NULL;
        count -= 2;
    }

    if (background){
        cmd[count-2] = NULL;
        count-=2;
    }

    if (error)
        return;

    /* Fun with Forks */
    pid_t pid = fork();
    if (pid == -1) {
        //Error
        exit(1);
    }
    else if (pid == 0) {
        //Child

        // if input redirects
        if (inredir) {
            int fd = open(input, O_RDONLY);
            if (fd == -1) {
                printf("Error: File does not exist or is not a file.\n");
                exit(1);
            }
            close(STDIN_FILENO);
            dup(fd);
            close(fd);
        }

        // if output redirects
        if (oredir) {
            int fd = open(output, O_CREAT|O_RDWR|O_TRUNC, 0600);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
        }

        // do pipes here //



        //handle commands like ls, etc.
        char* test=getenv("PATH");
        char* temp;
        temp=(char*)malloc(strlen(test));
        int k=0;
        int x=0;
        for(k; k < strlen(test); k++){
            if(test[k]==':'){
                k++;
                strcat(temp, "/");
                strcat(temp, cmd[0]);
                printf("%s  ", temp);
                if(access(temp, F_OK)==0)
                    execv(temp, cmd);
                else{
                    x=0;
                    memset(temp, '\0', strlen(temp));

                    if(k==strlen(test)-1){
                    printf("Error: Command not found.\n");
                    exit(1);
                    }
                }
            }
            temp[x]=test[k];
            x++;
        }
        free(temp);

    } // end of child process
    else {
        //Parent

        // check if background stuff is done
        int j;
        for (j = 0; j < ExQ.size; j++) {
            if (waitpid(ExQ.process[j], &status, WNOHANG) != 0) { // if background process has finished
                printf("[%i]\t[%s]\n", j, ExQ.command[j]);
                ExQ.finished[j] = 0;
                // if finished move others up in Queue
                int k = j+1;
                while (ExQ.finished[k-1] == 0) {
                // while the process before me is finished but I am not
                    ExQ.finished[k-1] = ExQ.finished[k];
                    strcpy(ExQ.command[k-1], ExQ.command[k]);
                    ExQ.process[k-1] = ExQ.process[k];
                    k++;
                }
                j--;
                ExQ.size--;
            }
        }

        if (background == 0) {
            // Wait on Child

            char file[50];
            FILE *f;
            FILE *asdf;
            if (io) {
                sprintf(file, "/proc/%i/io", pid);
                pid_t pid2 = fork();
                if (pid2 == -1) {
                    //Error
                    exit(1);
                }
               else if (pid2 == 0) {
                   //Child
                   char ** argstemp;
                   argstemp = (char**)calloc(4, sizeof(char*));
                   for (i=0;i<4;i++)
                       argstemp[i] = (char*)calloc(10,sizeof(char));
                    strcpy(argstemp[0], "cat");
                    strcpy(argstemp[1], ">");
                    strcpy(argstemp[2], "procio");
                    argstemp[3] = NULL;
                    execute(argstemp);
                    exit(0);
               }
               else
                waitpid(pid2, &status, 0);


            }

            waitpid(pid, &status, 0);

            if (io) {
                pid_t pid2 = fork();
                if (pid2 == -1) {
                    //Error
                    exit(1);
                }
               else if (pid2 == 0) {
                    //Child
                    char ** argstemp;
                    argstemp = (char**)calloc(3, sizeof(char*));
                    for (i=0;i<3;i++)
                        argstemp[i] = (char*)calloc(10,sizeof(char));
                    strcpy(argstemp[0], "cat");
                    strcpy(argstemp[1],"procio");
                    strcpy(argstemp[2],NULL);
                    execute(argstemp);
                    exit(0);
                }
                else
                    waitpid(pid2, &status, 0);


            }


        }
        else { // Else keep going while child does stuff in the background
            printf("%i\n", count);
            // queue next background process
            ExQ.process[ExQ.size] = pid;
            ExQ.finished[ExQ.size] = 1;
            strncpy(ExQ.command[ExQ.size], str, 255);   // str was created above before I nuked things out of the cmd array
            free(str);

            printf("[%i]\t[%i]\n", ExQ.size, pid);
            ExQ.size++;
        }

    }
}

// concatenates char ** into one long string
char * catcat(char ** b, int elements){
    char *t;
    int i, size = 0;
    for(i=0; i<elements;i++) {
        size += strlen(b[i]) + 1;
    }
    t = (char *)malloc(size*sizeof(char));
    for(i = 0; i<elements; i++) {
        strcat(t,b[i]);
        if(i < elements -1)
            strcat(t," ");
    }
    printf("\n%s\n", t);
    return t;
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
