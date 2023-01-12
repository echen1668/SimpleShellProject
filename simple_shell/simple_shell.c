#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

//function declartions and global variables
char *shell_read_line(); //read line
char **shell_parse(char *command); //parse input
int shell(); //main function of shell
int shell_execute(char **argument); //execute input command
int shell_exit(char **args, int *isnumber); //built-in exit function
int shell_proc(char **args); //built-in proc function
void split_string(char *s, int num, char *first, char *second); //string split by index
void free_arguments(char **args); //free memorey from arguments
char *commandline; //main command line
char **arguments; //arguments



int main(int argc, char **argv) {
    int exitcode = 0;
    if(argc > 1){ //checks if ./simple_shell was called with an argument
        fprintf(stderr, "Shell Cannot Accept Any Arguments.\n"); //error message occurs if ./simple_shell is called with an argument
        fprintf(stderr, "%s cannot be accepted.\n", argv[1]);
        exit(1);
    }else{
        exitcode = shell(); //enters shell if ./simple_shell has no argument
    }
    return exitcode; //returns exit code
}

//main loop for the shell
int shell(){
    int status = 1;
    do{
        commandline = shell_read_line(); //reads input entred by use on command line
        arguments = shell_parse(commandline); //parse input
        if (strcmp(arguments[0], "") == 0) { //if empty command is entered
            printf("Empty command was entered\n");
        }else{
            if (strcmp(arguments[0], "exit") == 0) { //if the user enters exit
                int isnumber = 0;
                int exitcode = shell_exit(arguments,&isnumber); //parse the exit command
                if(isnumber == 0){ //if exit command has a number from 0 to 255 after that
                    if(exitcode >=0 && exitcode <= 255){
                        free(commandline);
                        commandline = NULL;
                        free_arguments(arguments); //free up all memory and exits
                        return exitcode; //exit
                    }else{
                        fprintf(stderr, "Exit code not valid.\n"); //if argument after exit command is not a number 0 to 255
                    }
                }else{
                    fprintf(stderr, "Exit code not valid.\n");
                }
            }else{
                status = shell_execute(arguments); //execute the command
            }
        }
        free(commandline); //free memorey
        commandline = NULL;
        free_arguments(arguments);
    }while(status);
    return 0;
}

char *shell_read_line() {
    int size = 16;
    char *command = malloc(size * sizeof(char*));
    if(command != NULL){
        printf("Enter your command >>>> "); //asks user to enter command
        int a = EOF;
        int i =0;
        while((a = getchar() )!='\n' && a != EOF) { //user input is read and assigned to command
            command[i++] = (char)a;
            if(i >= size){ //resize if needed
                size = size*2;
                command = realloc(command, size);
                if (command == NULL) { //if allocation error occures
                    fprintf(stderr, "simple_shell: Allocation Error\n");
                    exit(1);
                }
            }
        }
        command[i] = '\0'; //null termatior at the end
    }else{
        fprintf(stderr, "simple_shell: Allocation Error\n");
        exit(1);
    }
    return command;
}

char **shell_parse(char *command) {
    size_t size = 16;
    size_t index = 0;
    char **parameters = malloc(size * sizeof(char*)); //array of arguments
    if (!parameters) { //if allocation error occures
        fprintf(stderr, "simple_shell: Allocation error\n");
        exit(1);
    }
    char commands2[strlen(command)];
    strcpy(commands2,command); //copies command into a non-pointer char commands2
    while(first_unquoted_space(commands2) != -1){ //splits commands2 by spaces and then delimits then by quotes and escape sequences
        char first[strlen(commands2)]; //first is word before the first space
        char second[strlen(commands2)]; //secon is the remainder of the string after the first space
        if(first_unquoted_space(commands2) != -1){
            split_string(commands2, first_unquoted_space(commands2), first, second); //splits string commands2 by spaces
            parameters[index] = malloc(strlen(first) + 1 * sizeof(char*));
            char *unescaped = unescape(first,stderr); //demlimits first by escape sequences and quotes
            strcpy(parameters[index],unescaped); //inputs first into parameters as an argument
            free(unescaped); //free memorey
            unescaped = NULL;
            index++;
            strcpy(commands2,second); //make remainder of string as new commands2
            if (index >= size) { //resize if needed
                size += size*2;
                parameters = realloc(parameters, size * sizeof(char*));
                if (!parameters) {
                    fprintf(stderr, "simple_shell: Allocation error\n");
                    exit(1);
                }
            }
            
        }
    }
    parameters[index] = malloc(strlen(commands2) + 1 * sizeof(char*)); //puts last word in commands2 into parameters array
    char *unescaped = unescape(commands2,stderr);
    strcpy(parameters[index],unescaped);
    free(unescaped);
    unescaped = NULL;
    index++;
    if (index >= size) { //resize if needed
        size += size*2;
        parameters = realloc(parameters, size * sizeof(char*));
        if (!parameters) {
            fprintf(stderr, "simple_shell: Allocation error\n");
            exit(1);
        }
    }
    parameters[index] = NULL; //null terminatior at the end of parameters
    return parameters;
}

int shell_launch(char **parameters){
    pid_t pid; //process id
    int status; //status code
    pid = fork();
    if(pid < 0) { //if an errors occures
        // Error forking
        fprintf(stderr, "simple_shell: Command not found..\n");
    }else if(pid == 0) {
        // Child process
        if (execvp(parameters[0], parameters) == -1) { //executes commands, error occures if command is not valid
            fprintf(stderr, "simple_shell: Command not found.\n");
        }
        free(commandline); //free memorey
        commandline = NULL;
        free_arguments(arguments);
        exit(1);
    }else{
        // Parent process
        pid_t wpid = waitpid(pid, &status, 0x00000002);
        while(((*(int *) &(status)) & 0177) != 0 && !(((*(int *) &(status)) & 0177) != 0177 && ((*(int *) &(status)) & 0177) != 0)){
            wpid = waitpid(pid, &status, 0x00000002); //wait for child to exit
            printf("simple_shell: Waiting for child pid of %d.\n", wpid);
        }
    }
    
    return 1;
    
}

int shell_execute(char **argument){
    if (strcmp(argument[0], "proc") == 0) { //calls proc built-in function if user input first argument if proc
        return (&shell_proc)(argument);
    }
    return shell_launch(argument); //else call function with fork() execvp()
}



int shell_exit(char **args, int *isnumber){
    if(!args[1]){ //if exit is entred with no argument, exit normally
        return 0;
    }else{
        if(strcmp(args[1], "0") == 0){ //if exit is entred with 0, exit normally with 0
            return 0;
        }
        int num = atoi(args[1]); //if exit is entred with any argument, parse argument, if number, exit with that number
        if(num != 0){
            return num;
        }else{
            *isnumber = 1; //isnumber turns 1 if if argument is not a number and will not exit
            return -1;
        }
    }
}

int shell_proc(char **args) {
    char procfilename[100] = "/proc/"; //base file name to open
    char finalname[100]; //file name for proc
    int c = 0;
    if (args[1] != NULL && strcmp(args[1], "") != 0) {
        if(args[1][0] == 'f' && args[1][1] == 'i' && args[1][2] == 'l' && args[1][3] == 'e' && args[1][4] == 's' && args[1][5] == 'y' && args[1][6] == 's' && args[1][7] == 't' && args[1][8] == 'e' && args[1][9] == 'm' && args[1][10] == 's'){
            strcpy(finalname,"/proc/filesystems"); //parse "proc filesystems" if entred (For some reason, my code reads an extra empty char in the end of "proc filesystems" command)
        }else if (args[1][strlen(args[1]) - 2] == 's' && args[1][strlen(args[1]) - 3] == 'u' && args[1][strlen(args[1]) - 4] == 't' && args[1][strlen(args[1]) - 5] == 'a' && args[1][strlen(args[1]) - 6] == 't' && args[1][strlen(args[1]) - 7] == 's' && args[1][strlen(args[1]) - 8] == '/'){
            strcat(procfilename, args[1]); //parse "proc id/status" if entred (For some reason, my code reads an extra empty char in the end of "proc id/status" command)
            char placeholder[20];
            split_string(procfilename,strlen(procfilename) - 1,finalname,placeholder);
        }else{
            strcat(procfilename, args[1]); //copies argument after proc into procfilename
            strcpy(finalname,procfilename); //procfilename is added to string "/proc/"
        }
        FILE *procfile = fopen(finalname, "r"); //open and read file
        if (procfile != NULL) {
            do { //print out every charatcher in the proc file
                c = fgetc(procfile);
                if( feof(procfile) ) {
                    break ;
                }
                printf("%c", c);
            } while(1);
            printf("%c", '\n'); //print newline
            fclose(procfile); //close file
        } else {
            fprintf(stderr, "Cannot Open proc file\n"); //if proc is entred with invalid argument
        }
    } else {
        fprintf(stderr, "No command after proc found\n"); //if proc is entred with no argument
    }
    return 1;
}

// splits a string such that first will contain the substring from 0-index
// second is substring index-end
// index is where the first unquoted space is in the string
void split_string(char *s, int num, char *first, char *second){
    int length = strlen(s);
    if (num < length){ //reads string until first space
        for (int i = 0; i < num; i++){
            first[i] = s[i];
        }
        first[num] = '\0'; //null termatior, split
        int j = 0;
        for (int i = num + 1; i <= length; i++){ //remainder of string
            second[j] = s[i];
            j++;
        }
        second[j]  = '\0';
    }
}

void free_arguments(char **args){ //free memorey
    int index = 0;
    while(args[index] != NULL){ //free every paramter in the argument array
        free(args[index]);
        args[index] = NULL;
        index++;
    }
    free(args); //free args
    args = NULL;
    
}



