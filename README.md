# IC-Shell
Overview
In this project, you will implement a simple shell for Linux. This shell will be called “icsh” or “IC shell”. The functionality of this shell will be similar to other popular Linux shells such as bash, csh, zsh, but with a subset of  features. Basically, your icsh should have the following functionality:

-Interactive and batch mode
-Support built-in some commands 
-Allow the user to execute one or more programs from executable files as either background or foreground jobs
-Provide job-control, including a job list and tools for changing the foreground/background status of currently running jobs and job suspension/continuation/termination.
-Allow for input and output redirection to/from files.

There are a number of things that you need to implement in this project. We organize the objectives into milestones below.

## Before you begin 
To start the project, you should create a file called icsh.c and a Makefile. These two files will be your starting point. You must also create a git repository to source control these two files. We will ask you to `git tag` the commit for each of the milestones for grading. The tag name will be given in each milestone description below. 

The look and feel of icsh should be similar to that of other UNIX shells, such as bash, tcsh, csh, etc. For example, your shell’s work loop should produce a prompt, e.g., icsh $, accept input from the user, and then produce another prompt. Messages should be written to the screen as necessary, and the prompt should be delayed when user input shouldn’t be accepted, as necessary. Needless to say, your shell should take appropriate action in response to the user’s input.

 

## Milestone 1: Interactive command-line interpreter
Git tag:  0.1.0  (please tag the commit for this milestone)
Points: 10
 

As your first milestone, you should implement a command-line interpreter functionality. Basically, when you start your shell with `icsh` with no argument, your shell should go into a prompt as shown below:

$ ./icsh
Starting IC shell
icsh $ <waiting for command>
You can design your own prompt. In the examples, here we use `icsh $` as our prompt symbol so you know we're in our shell. Optionally, you could write a welcome message when the shell starts.

For this milestone, your shell must support 3 built-in commands:

 

1. echo <text> -- the echo command prints a given text (until EOL) back to the console.
```C
icsh $ echo hello world
hello world
icsh $
```

2. !!  --  Double-bang; repeat the last command given to the shell. 
```C
icsh $ echo hello world
hello world
icsh $ !!
echo hello world
hello world
icsh $
```
Note that the command first prints out the last command first before running it. When there is no last command, !! just gives you back the prompt.

 

3. exit <num> -- this command exits the shell with a given exit code.
```C
icsh $ exit 1
bye
$
```
Technically speaking, there are two approaches to do this. You can either return from the main function the exit code, or use a system call exit()  (Links to an external site.).  The exit code should be in the range of 0-255. If a larger number is given, you must truncate them to fit in 8 bits. Optionally, a goodbye message can be shown on screen.

 

Important notes: 

When users give any other command, your shell should print out `bad command` and return to a prompt.
When users give an empty command, your shell just give a new prompt.
 

## Milestone 2: Script mode
Git tag:  0.2.0
Points: 10
 

For the milestone, you will enhance your shell to support script mode. When your shell is executed with an argument filename, your shell should read commands from the given file and run them one by one. For example,
```
## test.sh

echo hello
echo world
!!
exit 5
```
 Say we have `test.sh` with the above content in the current directory, you should be able to run it through your shell as follows.
```C
$ ./icsh test.sh
hello
world
world
$ echo $?
5
$
```
Note the command "echo $?" in the example above, this command will print out the exit code of the previous command. Since we gave exit 5 to icsh,  we expect the bash shell to report exit code = 5 here. 

 

## Milestone 3: Running an external program in the foreground
Git tag: 0.3.0
Points: 10
In this milestone, you will implement the most important functionality of a shell: running another program. Specifically, when user is given a command that doesn't match with any built-in command, it is assumed to be an external command.  Your shell must spawn a new process, execute it and wait for the command to complete and resume control of the terminal. Here is an example of a user running `ls` command
```C
icsh $ ls
file_a file_b file_c
icsh $
```
An external command could also contain argument list. Your shell must support them. 
```C
icsh $ ls -la
total 8
drwxr-xr-x 2 tusr 1000 4096 May  2 07:04 .
drwxrwxrwt 1 root  root 4096 May  2 07:04 ..
-rw-r--r-- 1 tusr 1000    0 May  2 07:04 file_a
-rw-r--r-- 1 tusr 1000    0 May  2 07:04 file_b
-rw-r--r-- 1 tusr 1000    0 May  2 07:04 fille_c
icsh $ 
```
Check out the resources section below for how to create process / execute a program / wait for it to complete.

 

## Milestone 4: Signal Handler
Git tag: 0.4.0
Points: 10
Through an interaction with the terminal driver, certain combinations of keystrokes will generate signals to your shell instead of appearing within stdin. Your shell should respond appropriately to these signals.

1. Control-Z generates a SIGSTOP. This should not cause your shell to be suspended. Instead, it should cause your shell to suspend the processes in the current foreground job. If there is no foreground job, it should have no effect.

2. Control-C generates a SIGINT. This should not kill your shell. Instead it should cause your shell to kill the processes in the current foreground job. If there is no foreground job, it should have no effect.

 

Support additional build-in commands:

echo $?  -- this prints out the exit status code of the previous command. You may assume that all build-in commands exits with exit code 0
 

Check out the resources section below and read more about signal handling.

 

## Milestone 5: I/O redirection
Git tag: 0.5.0
Points: 10
A program's execution may be followed by the meta-character <  or > which is in turn followed by a file name. In the case of <, the input of the program will be redirected from the specified file name. In the case of >, the output of the program will be redirected to the specified file name. If the output file does not exist, it should be created. If the input file does not exist, this is an error.

For example,

icsh $ la -l > some_file
This above command starts a new process, executes ls, but instead of outputting to the STDOUT, the output is redirected to a file. 

The goal for this milestone is the support both input redirection and output redirection. You can read about how to do this in the resource section below.
 

## Milestone 6: Background jobs and job control
Git tag: 0.6.0
Points: 50
 

Each process executed by itself from the command line is called a job.  When executing backgrounds jobs, the shell should not wait for the job to finish before prompting, reading, and processing the next command. When a background job finally terminates a message to that effect must be printed, by the shell, to the terminal. This message should be printed as soon as the job terminates.

To start a background job, user will issue a command and followed by & symbol. Here is an example:

```C
1: icsh $ sleep 5 &
2: [1] 843
3: icsh $
```
Explanation:

Line 1, user issues a command "sleep 5" to be ran in the background (indicated by trailing &).  The shell starts a new process, runs the command, prints out the job id and its PID to the console (Line 2) and returns to the prompt immediately (Line 3).  This allows user to run another command while the sleep command is ran in the background.
```C
icsh $ 
[1]+  Done                    sleep 5
icsh $ 
```
When the job is completed, the shell will asynchronously print out the job information stating the job id, Done status, the original command back to notify the user that one of the background has been completed.

As we have more than jobs running at a time, your shell must support the following commands for job control.

1. jobs - list current jobs 
  
```C
icsh $ sleep 100 &
[1] 855
icsh $ sleep 200 &
[2] 856
icsh $ jobs
[1]-  Running                 sleep 100 &
[2]+  Running                 sleep 200 &
icsh $ 
  ```
The jobs command shows a table of uncompleted job information (job ids, process status, commands). 

2. fg %<job_id>: Brings the job identified by <job_id> into the foreground. If this job was previously stopped, it should now be running. Your shell should wait for a foreground child to terminate before returning a command prompt or taking any other action.

  ```C
icsh $ sleep 100 &
[1] 862
icsh $ fg %1
sleep 100
```
`sleep 100` (job id=1) was started as a background job. Then, it was brought up to the foreground by the fg command. The shell is waiting for the command to complete.
3. bg %<job_id>: Execute the suspended job identified by <job_id> in the background.

```C
icsh $ sleep 100
^Z
[1]+  Stopped                 sleep 100
icsh $ bg %1
[1]+ sleep 100 &
icsh $ 
```
This time `sleep 100` was started a a foreground job.  While it is running, the user presses 'Ctrl+Z' to suspend it (SIGSTOP). The shell detects that, print out the status line.  Then, the user gives the bg command to put job id =1 to resume executing in the background (SIGCONT) and the shell gives the prompt right away. 

 

Important notes:

This milestone is the most complicated part of the project. Start early!
  
## Milestone 7: Extra features, Extra credits
Git tag: 0.7.0
Points: 10
Congratulations for reaching this milestone. We know you're probably having a lot of fun writing your own shell. This part is for you to throw in any extra features for extra credits.

The points will be awarded proportional to how awesome your features are.
