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

// Redirection
int inRedirectPresentAt(char** args);
int outRedirectPresentAt(char** args);
int ampersandPresentAt(char** args);

// signal
void sigint_handler(int signal) {
    printf("]^C found\n");
}

void sigstop_handler(int sig) {
    printf("]^Z found\n");
}

// main functions
char* readLine();
char** tokenize(char*);
int execute(char**);

int exitCode = 0;

// keep history
char history[100][100];
int history_counter = 0;


int main()
{
    char *input;
    char **args;
    char *prompt;
    int status = 1;

    //using_history();
    

    //Signal stuff
    struct sigaction sa = {
        .sa_handler = &sigint_handler,
        .sa_flags = 0
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while(status == 1)
    {
        //set up
        char* cwd = getcwd(NULL, 0); // current directory
        prompt = malloc( (strlen(cwd) + 3) * sizeof(char) ); // allocate prompt
        strcpy(prompt, cwd);
        strcat(prompt, " >>> ");
        free(cwd);

        printf("%s", prompt);

        // read user input
        input = readLine();

        //printf("%s", input);
        if(strcmp(input,"!!\n")){
            // record history
            history_counter++;
            strcpy(history[history_counter], input);
            //printf("keep: %s\n", history[history_counter]);
        }
        
        // split input
        args = tokenize(input);

        
       
        // execute
        status = execute(args);
        
        
        //free
        free(args);
        free(input);
        free(prompt);
        
    }


    return status;
}

// function for reading input
char* readLine()
{
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin)) {  
            exit(EXIT_SUCCESS);
        } else  {
            perror("readline");
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
    char* inFile;
    char* outFile;
    FILE* infp;
    FILE* outfp;
    int inRedirect;
    int outRedirect;
    int background;

    pid_t pid = getpid();

    // check for redirection
    inRedirect = inRedirectPresentAt(args);
    outRedirect = outRedirectPresentAt(args);

    // check background process
    background = ampersandPresentAt(args);

    // if null
    if (args[0] == NULL) {
        return 1;
    }

    // echo
    if ( strcmp(args[0], "echo") == 0 ){
        if ( strcmp(args[1], "$?") == 0){
            printf("%d\n", exitCode);
            return 1;
        }
        for( i = 1; args[i] != NULL; i++){
            printf("%s ", args[i]);
        }
        printf("\n");
        return 1;
    }

    // exit
    if ( strcmp(args[0], "exit") == 0 ){
        if( args[1] == NULL){
            return 0;
        }
        return atoi(args[1]);
        }

    // !!
    if ( strcmp(args[0], "!!") == 0){
        //printf("%s",history[history_counter]);
        args = tokenize(history[history_counter]);
        return execute(args);
    }

    // Script mode
    if ( strcmp(args[0], "./icsh") == 0){
        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        const char* name = args[1];
        int stat = 1;

        fp = fopen(name, "r");
        if (fp == NULL){
            return 1;
        }

        while ((read = getline(&line, &len, fp)) != -1) {
            //printf("%s", line);
            if(strcmp(line,"!!\n")){
                // record history
                history_counter++;
                strcpy(history[history_counter], line);
                //printf("keep: %s\n", history[history_counter]);
            }
            char **token =  tokenize(line);
            stat = execute(token);
        }

        fclose(fp);
        if (line){
            free(line);
        }
        exitCode = stat;
        return 1;
         
    }

    //bg ----------------------------------bugged
    if (strcmp(args[0], "bg")==0){
      pid_t pidnumber;
      pidnumber=atoi(args[1]);
      printf("PID: %d\n", pidnumber);
      kill(pidnumber, SIGCONT);
      return 0;
    }

    //fg ----------------------------------bugged
    // if (strcmp(args[0], "fg")==0){
    //     pid_t pidnumber;
    //     pidnumber=atoi(args[1]);
    //     tcsetpgrp(0, getpgid(pidnumber));
    //     waitpid(getpgid(pidnumber), NULL, WUNTRACED);
    //     tcsetpgrp(0, getpgid(shellpid)); shellpid is placeholder

    //   return 0;
    // }

    // make child
    int status;
    pid = fork();

    if( pid == 0)
    {
        // another signal stuff
        setpgid(0, getpid());
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // redirection
        if (inRedirect >= 0) {
            inFile = args[inRedirect + 1];
            args[inRedirect] = NULL;

            infp = freopen(inFile, "r", stdin);
        }

        if (outRedirect >= 0) {
            outFile = args[outRedirect + 1];
            args[outRedirect] = NULL;

            outfp = freopen(outFile, "w", stdout);
        }

        // background
        if (background >= 0) {
            args[background] = NULL;
        }

        if (execvp(args[0], args) == -1) {
            perror("bad command");
        }
        exit(EXIT_FAILURE);

    } else if( pid < 0){ // fail
        perror("fork fail");

    } else{
        if (background < 0) {
            setpgid(pid, pid);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, pid);
            tcsetpgrp(STDOUT_FILENO, pid);
            //int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDOUT_FILENO, getpid());
            tcsetpgrp(STDIN_FILENO, getpid());

            //waitpid(pid, &status, 0);
        }
        else{
            printf("running %d in background\n", pid);
        }
    }

    return 1;
}

// redirection stuff
int outRedirectPresentAt(char** args)
{
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            return i;
        }
    }

    return -1;
}

int inRedirectPresentAt(char** args)
{
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            return i;
        }
    }

    return -1;
}

// background
int ampersandPresentAt(char** args)
{
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            return i;
        }
    }

    return -1;
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
https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
*/