// Bash Shell Simulator in C
// COP4610 Intro to Operating Systems
// Amber Mickler and Benjamin Hybart


//TODO: the whole thing :D

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
   char instr[256];                          // user input will never be longer than 255 chars
   char *user = getenv("USER");
   char *pwd = getenv("PWD");

   do {
      prompt();
      // fgets adds newline at end of string, you'll need to remove it
      // scanf


   } while(strncmp(instr, "exit", 4) != 0);  // run until exit command

   /*
   Part 1: Parsing
   Before the shell can begin executing commands, it needs to extract the command name, the arguments,input redirection (<), output redirection (>), piping (|), and background execution (&) indicators. Understand the following segments of the project prior to designing your parsing. The ordering of execution of many constructs may influence your parsing strategy. It is also critical that you understand how to parse arguments to a command and what delimits arguments.
   */
   char** parse(char* line) {

   }

   /*
   Part 2: Environmental Variables
   Every program runs in its own environment. One example of an environmental variable is $USER, which expands to the current username. For example, if my current username is 'dennis', typing: => echo $USER
   outputs: dennis
   In the bash shell, you can type 'env' to see a list of all your environmental variables. You will need to use and expand various environmental variables in your shell, and you may use the getenv() library call to do so. The getenv() procedure searchs the environment list for a string that matches the string pointed to by name. The strings are of the form: NAME = VALUE
   */

   char* expand(char* envVar) {

   }

   /*
   Part 3: Prompt
   The prompt should always indicate to the user the absolute working directory, who they are, and the machine name. Remember that cd can update the working directory. This is the format:
   USER@MACHINE :: PWD =>
   Example:
   dennis@linprog3 :: /home/grads/dennis/cop4610t =>
   */

   void prompt() {
      printf("%s::%s$ ", user, pwd);
   }

   /*
   Part 4: Path Resolution
   You will need to convert different file path naming conventions to absolute path names. You can assume that directories are separated with a single forward slash (/).

   • Directories that can occur anywhere in the path
   ◦ ..
      ▪ Expands to the parent of the current working directory
      ▪ Signal an error if used on root directory
   ◦ .
      ▪ Expands to the current working directory (the directory doesn't change)
   ◦ DIRNAME
      ▪ Expands to the child of the current working directory named DIRNAME
      ▪ Signal an error if DIRNAME doesn't exist
      ▪ Signal an error if DIRNAME occurs before the final item and is not a directory
   • Directories that can only occur at the start of the path
   ◦ ~
      ▪ Expands to $HOME directory
   ◦ /
      ▪ Root directory
   • Files that can only occur at the end of the path
   ◦ FILENAME
      ▪ Expands to the child of the current working directory named FILENAME
      ▪ Signal an error if FILENAME doesn't exist

   You will need to handle commands slightly differently. If the path contains a '/', the path resolution is handled as above, signaling an error if the end target does not exist or is not a file. Otherwise, if the path is just a single name, then you will need to prefix it with each location in the $PATH and search for file existence. The first file in the concatenated path list to exist is the path of the command. If none of the files exist, signal an error.
   */



   /*
   Part 5: Execution
   You will need to execute simple commands. First resolve the path as above. If no errors occur, you will need to fork out a child process and then use execv to execute the path within the child process.
   */



   /*
   Part 6: I/O Redirection
   Once the shell can handle simple execution, you'll need to add the ability to redirect input and output from and to files. The following rules describe the expected behavior, note there does not have to be whitespace between the command/file and the redirection symbol.
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
   After it can handle redirection, your shell is capable of emulating the functionality of pipes. Pipes should behave in the following manner (again, there does not have to be whitespace between the commands and the symbol):
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
   You will need to handle execution for background processes. There are several ways this can be encountered:
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
      ◦ Changes the present working directory according to the path resolution above
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
      ◦ Output the elapsed time in the format of s.us where s is the number of seconds and us is the number of micro seconds (0 padded)
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
   

}
