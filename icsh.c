/*
***************** Note to self *****************
This project is 15% of total grade, dont fuck up!
checkpoint 1:1.5%
checkpoint 2:1.5%
checkpoint 3:1.5% use execvp()
checkpoint 4:1.5% check lecture4
checkpoint 5:1.5%
checkpoint 6:7.5%!! ---very important---
checkpoint 7:1.5%
************************************************
*/

// C Program to design a shell in Linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include<readline/history.h>


// Buffer
#define MAXLEN 512
#define MAXTOKEN 10

// main functions
char* readLine();
char** tokenize(char*);
int execute(char**);

// functions for checkpoints
int inRedirectAt(char** args);
int outRedirectAt(char** args);
int checkBackground(char** args);


int main()
{
    char *input;
    char **args;
    char *prompt;
    int status = 1;

    using_history();

    while(status == 1)
    {

        //set up
        char* cwd = getcwd(NULL, 0); // current directory
        prompt = malloc( (strlen(cwd) + 3) * sizeof(char) ); // allocate prompt
        strcpy(prompt, cwd);
        strcat(prompt, ">>> ");
        free(cwd);

        printf("%s", prompt);

        // read user input
        input = readLine();
        
        // split input
        args = tokenize(input);

        // exit
        if ( strcmp(args[0], "exit") == 0 ){
            return (int) args[1];
        }

        // !!
        if ( strcmp(args[0], "!!") == 0){
            HIST_ENTRY *entry = history_get(where_history());
            printf("%s\n", entry->line);
            args = tokenize((char*) entry->line);
            status = execute(args);
            

        }
        else{
            // execute
            status = execute(args);
        }
        
        //free
        free(args);
        free(input);
        free(prompt);
        
    }


    return 0;
}

// function for reading input
char* readLine()
{
    char* line = (char*) malloc(MAXLEN + 1); // create line

    // read the input
    if (fgets(line, MAXLEN + 1, stdin) == NULL) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }

        else {
            perror("fgets error\n");
            exit(EXIT_FAILURE);
        }
    }
    if( strcmp("!!", line) != 0){
        add_history(line);
    }
    //printf("%d", where_history());

    return line;
}

// function for spliting input
char** tokenize(char* line)
{
    int i = 0;

    char** tokenList = malloc(MAXTOKEN * sizeof(char*));
    char* token;

    if (!tokenList){
        perror("allocation error\n");
        exit(EXIT_FAILURE);
    }

    // spliting
    token = strtok(line, " \t\r\n");
    while(token != NULL){
        tokenList[i] = token;
        //printf("%s ", token);
        i++;

        if (i >= ( MAXTOKEN -1 )){
            break;
        }

        token = strtok(NULL, " \t\r\n");
    }
    tokenList[i] = NULL;

    return tokenList;

}

// function for execution
int execute(char** args)
{
    int i;

    // if null
    if (args[0] == NULL) {
        return 1;
    }

    // echo
    if ( strcmp(args[0], "echo") == 0 ){
        for( i = 1; args[i] != NULL; i++){
            printf("%s ", args[i]);
        }
        printf("\n");
        return 1;
    }

  

    // make chile
    pid_t pid, wpid;
    int status;
    pid = fork();

    if( pid == 0){
        if (execvp(args[0], args) == -1) {
            perror("basic shell");
        }
        exit(EXIT_FAILURE);
    } else if( pid < 0){
        perror("lsh");
    } else{
        waitpid(pid, &status, 0);
    }

    return 1;

}



/* references

https://www.geeksforgeeks.org/making-linux-shell-c/
https://stackoverflow.com/questions/27541910/how-to-use-execvp
https://web.mit.edu/gnu/doc/html/rlman_2.html
https://tiswww.case.edu/php/chet/readline/history.html
https://www.codementor.io/@sandesh87/how-and-why-i-built-a-mini-linux-shell-using-c-1dqk5olxgw
https://brennan.io/2015/01/16/write-a-shell-in-c/
https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c
https://github.com/brenns10/lsh/issues/14
https://stackoverflow.com/questions/38792542/readline-h-history-usage-in-c
http://www.math.utah.edu/docs/info/hist_2.html

*/