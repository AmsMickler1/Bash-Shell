// Bash Shell Simulator in C
// COP4610 Intro to Operating Systems
// Amber Mickler and Benjamin Hybart

//DONE: Parse splits line into arguments
//TODO: resolve pathfiles, expand env vars, etc

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {false, true} bool;


int isSpecial(char c);
void parse(char* line);
char* expand(char* envVar);


int main() {
   char input[256];            // user input will never be longer than 255 chars
   char *user = getenv("USER");
   char *pwd = getenv("PWD");

   do {

      printf("%s::%s$ ", user, pwd);
      fgets(input, 256, stdin);              // grabs whole line including \n
      parse(input);
      // do stuff in parse


   } while(strncmp(input, "exit", 4) != 0);  // run until exit command

   printf("Exiting shell...\n");
}

int isSpecial(char c) {
   if (c == '<' || c == '>' || c == '|' || c == '&')
      return 1;
   else
      return 0;
}

   /*
   Part 1: Parsing :: needs work
   Before the shell can begin executing commands, it needs to extract the
   command name, the arguments,input redirection (<), output redirection (>),
   piping (|), and background execution (&) indicators. Understand the
   following segments of the project prior to designing your parsing. The
   ordering of execution of many constructs may influence your parsing
   strategy. It is also critical that you understand how to parse arguments to
   a command and what delimits arguments.
   */
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
   // solution: make last argument null arg for execv later on
   strncpy(args[index-1],"NULL", 4);

   // prints list of args for debugging purposes.
   printf("Args: \'%s\'", args[0]);
   for (i=1;i<index;i++) {
      printf(", \'%s\'", args[i]);
   }
   printf("\n");

   // Expand environmental variables
   for (i=0;i<index;i++) {
      if (args[i][0] == '$') {
         args[i]++;
         printf("\n%s\n", args[i]);
         args[i] = *getenv(args[i]);
         printf("\n%s\n", args[i]);
      }
   }





   /* DO STUFF HERE */



   printf("Here!");
   fflush(stdout);
   // free memory - giving a seg fault sometimes?
   for (i = 0; i < 256; i++)
      free(args[i]);
   free(args);

   }  // end of parse function


   /*
   Part 2: Environmental Variables
   Every program runs in its own environment. One example of an environmental
   variable is $USER, which expands to the current username. For example, if my
   current username is 'dennis', typing: => echo $USER
   outputs: dennis
   In the bash shell, you can type 'env' to see a list of all your
   environmental variables. You will need to use and expand various
   environmental variables in your shell, and you may use the getenv() library
   call to do so. The getenv() procedure searchs the environment list for a
   string that matches the string pointed to by name. The strings are of the
   form: NAME = VALUE
   */

   /*
   Part 3: Prompt
   The prompt should always indicate to the user the absolute working
   directory, who they are, and the machine name. Remember that cd can update
   the working directory. This is the format:
   USER@MACHINE :: PWD =>
   Example:
   dennis@linprog3 :: /home/grads/dennis/cop4610t =>
   */

   // done as part of main function

   /*
   Part 4: Path Resolution
   You will need to convert different file path naming conventions to absolute
   path names. You can assume that directories are separated with a single
   forward slash (/).

   • Directories that can occur anywhere in the path
   ◦ ..
      ▪ Expands to the parent of the current working directory
      ▪ Signal an error if used on root directory
   ◦ .
      ▪ Expands to the current working directory (the directory doesn't change)
   ◦ DIRNAME
      ▪ Expands to the child of the current working directory named DIRNAME
      ▪ Signal an error if DIRNAME doesn't exist
      ▪ Signal an error if DIRNAME occurs before the final item and is not a
      directory
   • Directories that can only occur at the start of the path
   ◦ ~
      ▪ Expands to $HOME directory
   ◦ /
      ▪ Root directory
   • Files that can only occur at the end of the path
   ◦ FILENAME
      ▪ Expands to the child of the current working directory named FILENAME
      ▪ Signal an error if FILENAME doesn't exist

   You will need to handle commands slightly differently. If the path contains
   a '/', the path resolution is handled as above, signaling an error if the
   end target does not exist or is not a file. Otherwise, if the path is just a
   single name, then you will need to prefix it with each location in the $PATH
   and search for file existence. The first file in the concatenated path list
   to exist is the path of the command. If none of the files exist, signal an
   error.
   */



   /*
   Part 5: Execution
   You will need to execute simple commands. First resolve the path as above.
   If no errors occur, you will need to fork out a child process and then use
   execv to execute the path within the child process.
   */



   /*
   Part 6: I/O Redirection
   Once the shell can handle simple execution, you'll need to add the ability
   to redirect input and output from and to files. The following rules describe
   the expected behavior, note there does not have to be whitespace between the
   command/file and the redirection symbol.
      • CMD > FILE
         ◦ CMD redirects its output to FILE
         ◦ Create FILE if it does not exist
         ◦ Overwrite FILE if it does exist
      • CMD < FILE
         ◦ CMD receives input from FILE
         ◦ Signal an error if FILE does not exist or is not a file
      • Signal an error for the following
         ◦ CMD <
         ◦ < FILE
         ◦ <
         ◦ CMD >
         ◦ > FILE
         ◦ >
   */



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
   Part 9: Built-ins
   • exit
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
