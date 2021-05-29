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
#include <stdlib.h>

// Buffer
#define MAXLEN 512
#define MAXTOKEN 10
#define MAXJOBS 10

// Redirection
int inRedirectPresentAt(char** args);
int outRedirectPresentAt(char** args);

//background
int ampersandPresentAt(char** args);

// main functions
char* readLine();
char** tokenize(char*);
int execute(char*);

int exitCode = 0;

// keep history
char history[100][100];
int history_counter = 0;

// job
struct job{
    pid_t pid;
    int jid;
    int state;
    char line[MAXLEN];
};

int nextjid = 1;

#define UNDEF 0 // undefined
#define FG 1    // running in foreground
#define BG 2    // running in background
#define ST 3    // stopped
#define DONE 4  // done

struct job jobList[MAXJOBS];

void clearjob(struct job *job) { // clear out jobList
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->line[0] = '\0';
}

void initjobs(struct job *jobs) { // initialize jobList
    int i;
    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

// jobs control helpers
int addjob(struct job *jobs, pid_t pid, int state, char *line);
void listjobs(struct job *jobList);
struct job *getjobpid(struct job *jobList, pid_t pid);
int maxjid(struct job *jobList); 
int deletejob(struct job *jobList, pid_t pid); 
struct job *getjobpid(struct job *jobList, pid_t pid);
int pid2jid(pid_t pid);
int getjobjid(struct job *jobList, int jid);
pid_t fgpid(struct job *jobList);

// signal
void sigint_handler(int signal) {
    printf("]^C found\n");
}

void child_handler(int sig){
    int child_status;
    pid_t pid;
    int id;
    while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0){       
        //printf("%d",jobList[i].state);
        id  = pid2jid(pid) - 1;
        //printf("jid: %d\n", id);
        //printf("job info: %d\n",jobList[id].jid);
        if( jobList[id].state != FG)
            printf("\njob done: [%d] %d %s",jobList[id].jid, jobList[id].pid, jobList[id].line);
        deletejob(jobList, pid);
    }
}

void sigstop_handler(int sig) {
    printf("\n");
}


int main()
{
    char *input;
    char **args;
    char *prompt;
    int status = 1;

    //Signal stuff
    struct sigaction sa = {
        .sa_handler = &sigint_handler,
        .sa_flags = 0
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    //signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    //signal(SIGTSTP, SIG_IGN); //---------- SIGTSTP bugged

    struct sigaction sa2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART;
    sa2.sa_handler = child_handler;
    sigaction(SIGCHLD, &sa2, NULL);

    struct sigaction sa3;
    sigemptyset(&sa3.sa_mask);
    sa3.sa_flags = 0;
    sa3.sa_handler = sigstop_handler;
    sigaction(SIGTSTP, &sa3, NULL);

    initjobs(jobList); // innitialize jobList

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
        //args = tokenize(input);

    
        // execute
        status = execute(input);
        
        
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
int execute(char* line)

{
    char** args;
    int i;
    char* inFile;
    char* outFile;
    FILE* infp;
    FILE* outfp;
    int inRedirect;
    int outRedirect;
    int background;
    int status;

    char* cmdline = malloc(strlen(line) + 1);
    strcpy(cmdline,line);

    // tokenize
    args = tokenize(line);

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
        //args = tokenize(history[history_counter]);
        return execute(history[history_counter]);
    }

    // help
    if ( strcmp(args[0], "help") == 0){
        printf("Thanthong shell\n");
        printf("The folloowing are built in:\n");
        printf("     exit <num>\n");
        printf("     echo <message>\n");
        printf("     !!\n");
        printf("     cd");
        printf("The folloowing are supported feture:\n");
        printf("     I/O redirection with '<' & '>'\n");
        printf("     script mode:\n            ./icsh <filename>\n");
        printf("     background process:\n            <command> &\n");
    }

    // cd
    if ( strcmp(args[0],"cd") == 0){
        if (args[1] == NULL) {
            fprintf(stderr, "expected argument to \"cd\"\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("lsh");
            }
        }
        return 1;
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
            //char **token =  tokenize(line);
            stat = execute(line);
        }

        fclose(fp);
        if (line){
            free(line);
        }
        exitCode = stat;
        return 1;
         
    }

    // jobs
    if ( strcmp(args[0],"jobs") == 0){
        listjobs(jobList);
        return 1;
    }

    // fg
    if ( strcmp(args[0],"fg") == 0){
        struct job *job;
        int id;

        // check all parameter
        if ( args[1] == NULL ){
            printf("fd %%<jid>\n");
            return 1;
        }

        id = atoi(&args[1][1]) -1;
        //printf("input id: %d\n",id);

        // chack if jid is valid
        if (getjobjid(jobList, id) <= 0){
            printf("no such job\n");
            return 1;
        }

        jobList[id].state = FG;
        waitpid(jobList[id].pid, &status, WUNTRACED);
        
        return 1;
    }

    // bg
    

    // make child
    pid = fork();

    if( pid == 0)
    {
        // another signal stuff
        setpgid(0, getpid());
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

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
            int i = addjob(jobList,pid,FG,cmdline);

            setpgid(pid, pid);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, pid);
            tcsetpgrp(STDOUT_FILENO, pid);
            waitpid(pid, &status, WUNTRACED); // wait fg to terminate
            tcsetpgrp(STDOUT_FILENO, getpid());
            tcsetpgrp(STDIN_FILENO, getpid());

            //if
            if (WIFEXITED(status)) {
                jobList[i].state = DONE;
            } else if (WIFSIGNALED(status)) {
                jobList[i].state = DONE;
            } else if (WSTOPSIG(status)) {
                jobList[i].state = ST;
                printf("\n[%d] %d Stopped %s",jobList[i].jid, jobList[i].pid, jobList[i].line);

            }

            //clearjob(&jobList[i]);
            //deletejob(jobList, pid);
        }
        else{

            int i = addjob(jobList,pid,BG,cmdline);
            //printf("running %d in background\n", pid); //dont wait
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

// ---------- job helper ----------
int addjob(struct job *jobList, pid_t pid, int state, char *line){
    int i;
    if( pid == 0){
        return 0;
    }
    for(i= 0; i < MAXJOBS; i++){
        if(jobList[i].pid == 0){
            jobList[i].pid = pid;
            jobList[i].state = state;
            jobList[i].jid = nextjid++;
            if (nextjid > MAXJOBS){
                nextjid = 1;
            }
            strcpy(jobList[i].line, line);
            if( jobList[i].state == BG)
                printf("Added job [%d] %d %s", jobList[i].jid, jobList[i].pid, jobList[i].line);
            return i;
        }
    }
    printf("try to create too many jobs\n");
    return 0;
}

void listjobs(struct job *jobList) 
{
    int i;
    for (i = 0; i < MAXJOBS; i++) {
	if (jobList[i].pid != 0) {
	    printf("[%d] %d ", jobList[i].jid, jobList[i].pid);
	    switch (jobList[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobList[i].state);
	    }
	    printf("%s", jobList[i].line);
	}
    }
}

struct job *getjobpid(struct job *jobList, pid_t pid) {
    int i;

    if (pid < 1){
	    return NULL;}
    for (i = 0; i < MAXJOBS; i++)
	    if (jobList[i].pid == pid)
	        return &jobList[i];
    return NULL;
}

int maxjid(struct job *jobList) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobList[i].jid > max)
	    max = jobList[i].jid;
    return max;
}

int deletejob(struct job *jobList, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobList[i].pid == pid) {
	    clearjob(&jobList[i]);
	    nextjid = maxjid(jobList)+1;
	    return 1;
	}
    }
    return 0;
}

int pid2jid(pid_t pid) 
{
    int i;
    if (pid < 1){
	    return 0;
    }
    for (i = 0; i < MAXJOBS; i++){
	    if (jobList[i].pid == pid) {
            return jobList[i].jid;
        }
    }
    return 0;
}

int getjobjid(struct job *jobList, int jid) 
{
    int i;

    if (jid < 0){
	    return -1;
    }
    for (i = 0; i < MAXJOBS; i++){
        //printf("from list:%d\n input: %d", jobList[i].jid,jid);
	    if (jobList[i].jid == jid){
	        return 1;
        }
    }
    return -1;
}

pid_t fgpid(struct job *jobList) {
    int i;

    for (i = 0; i < MAXJOBS; i++){
	    if (jobList[i].state == FG){
	        return jobList[i].pid;
        }
    }
    return 0;
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