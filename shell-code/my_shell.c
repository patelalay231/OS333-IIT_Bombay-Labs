#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BACKGROUND_PROCESSES 64

pid_t fg_pid = -1;  // Track the foreground process ID
int stop_execution = 0;  // Flag to stop execution after SIGINT
pid_t parallel_pids[MAX_NUM_TOKENS];  // Track PIDs for parallel processes
int parallel_pid_count = 0;  // Track the number of parallel processes

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
		tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		strcpy(tokens[tokenNo++], token);
		tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void cd(char **tokens){
	if (tokens[1] == NULL) {
		fprintf(stderr, "cd: expected argument\n");
	} else {
		if (chdir(tokens[1]) != 0) {
			perror("cd failed");
		}
	}
}

void handle_sigint(int sig) {
    if (fg_pid != -1) {
        // Stop the current foreground process in serial mode
        kill(fg_pid, SIGINT);
        stop_execution = 1;  // Stop further serial commands
    } else {
        // Stop all running parallel processes
        for (int i = 0; i < parallel_pid_count; i++) {
            kill(parallel_pids[i], SIGINT);
        }
        stop_execution = 1;  // Stop parallel execution
    }
}

// Reap any background processes that have terminated
void reap_background_processes()
{
    pid_t pid;
    int status;
    // Use non-blocking wait to reap any terminated background processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Shell: Background process finished\n");
    }
}

// Send SIGTERM to all background processes
void terminate_background_processes(pid_t *background_pids, int bg_count)
{
    for (int i = 0; i < bg_count; i++) {
        kill(background_pids[i], SIGTERM);  // Send SIGTERM to terminate the background process
    }
}

// Handle forground process execution
void forground_process(char **tokens)
{
    if (strcmp(tokens[0], "cd") == 0) {
        cd(tokens);
		return;
    }
    pid_t pid = fork();
    if (pid == 0) {
        setpgid(0, 0);
        execvp(tokens[0], tokens);
        fprintf(stdout, "command not found: %s\n", tokens[0]);
        exit(0);
    } else if (pid > 0) {
        fg_pid = pid;  // Set the foreground process ID
        waitpid(pid, NULL, 0);  // Wait for the foreground process to finish
        fg_pid = -1;  // Reset the foreground process ID
    } else {
        perror("fork failed");
        exit(1);
    }
}

// Handle background process execution
void background_process(char **tokens,pid_t *background_pids, int *bg_count)
{
    if (strcmp(tokens[0], "cd") == 0) {
        cd(tokens);
		return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        setpgid(0, 0);
        execvp(tokens[0], tokens);
        fprintf(stdout, "command not found: %s\n", tokens[0]);
        exit(0);
    } else if (pid > 0) {
        if (*bg_count < MAX_BACKGROUND_PROCESSES) {
            background_pids[*bg_count] = pid;
            (*bg_count)++;
        } else {
            fprintf(stderr, "Too many background processes\n");
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

void serial_process(char** tokens){
    int tokenCount = 0;
    int command_index = 0;

    while(tokens[tokenCount] !=  NULL && !stop_execution){
        if(strcmp(tokens[tokenCount],"&&") == 0){
            tokens[tokenCount] = NULL;
            pid_t pid = fork();
            if(pid == 0){
                setpgid(0,0);
                execvp(tokens[command_index],&tokens[command_index]);
                fprintf(stdout, "command not found: %s\n", tokens[command_index]);
                exit(0);
            } else if (pid > 0) {
                fg_pid = pid;  // Set the foreground process ID
                waitpid(pid, NULL, 0);  // Wait for the foreground process to finish
                fg_pid = -1;  // Reset the foreground process ID
                command_index = tokenCount + 1;
            } else {
                perror("fork failed");
                exit(1);
            }
        }
        tokenCount++;
    }

    if (command_index < tokenCount && !stop_execution) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, 0);
            execvp(tokens[command_index], &tokens[command_index]);
            fprintf(stdout, "command not found: %s\n", tokens[command_index]);
            exit(0);
        } else if (pid > 0) {
            fg_pid = pid;  // Set the foreground process ID
            waitpid(pid, NULL, 0);  // Wait for the foreground process to finish
            fg_pid = -1;  // Reset the foreground process ID
        } else {
            perror("fork failed");
            exit(1);
        }
    }
}

void parallel_process(char **tokens) {
    int tokenCount = 0;
    int command_index = 0;

    // Loop over the tokens and split commands based on "&&&"
    while (tokens[tokenCount] != NULL && !stop_execution) {
        if (strcmp(tokens[tokenCount], "&&&") == 0) {
            tokens[tokenCount] = NULL;  // Null-terminate the current command

            // Fork and execute the command in parallel
            pid_t pid = fork();
            if (pid == 0) {
                setpgid(0, 0);  // Set process group
                execvp(tokens[command_index], &tokens[command_index]);
                fprintf(stdout, "command not found: %s\n", tokens[command_index]);
                exit(0);
            } else if (pid > 0) {
                parallel_pids[parallel_pid_count] = pid;  // Store the PID for later reaping
                parallel_pid_count++;
                command_index = tokenCount + 1;  // Update command index for the next command
            } else {
                perror("fork failed");
                exit(1);
            }
        }
        tokenCount++;
    }

    // If there is one last command to execute
    if (command_index < tokenCount && !stop_execution) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, 0);  // Set process group
            execvp(tokens[command_index], &tokens[command_index]);
            fprintf(stdout, "command not found: %s\n", tokens[command_index]);
            exit(0);
        } else if (pid > 0) {
            parallel_pids[parallel_pid_count] = pid;  // Store the PID
            parallel_pid_count++;
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    // Wait for all child processes to finish (reap them)
    for (int i = 0; i < parallel_pid_count; i++) {
        waitpid(parallel_pids[i], NULL, 0);  // Wait for each process to terminate
    }
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;       
	pid_t background_pids[MAX_BACKGROUND_PROCESSES];
    int bg_count = 0;       
	int i;

    // Set up the signal handler for SIGINT
    signal(SIGINT, handle_sigint);

	while(1) {			
		reap_background_processes();
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		// Handle exit command
        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting the shell...\n");
            terminate_background_processes(background_pids, bg_count);  // Terminate all background processes
            // Free dynamically allocated memory for tokens
            for (i = 0; tokens[i] != NULL; i++) {
                free(tokens[i]);
            }
            free(tokens);
            break;  // Exit the loop and terminate the shell
        }
		
		int isBackground = 0;
        int tokenCount = 0;
        int isSerial = 0;
        int isParallel = 0;
        stop_execution = 0;
        while (tokens[tokenCount] != NULL) {
            if(strcmp(tokens[tokenCount],"&&") == 0){
                isSerial = 1;
            }
            else if(strcmp(tokens[tokenCount],"&&&") == 0){
                isParallel = 1;
            }
            tokenCount++;
        }
        
        if (tokenCount > 0 && strcmp(tokens[tokenCount - 1], "&") == 0) {
            isBackground = 1;
            tokens[tokenCount - 1] = NULL;  // Remove the '&' token
        }

        if(isSerial) {
            serial_process(tokens);
        }
        else if(isParallel){
            parallel_process(tokens);
        }
        else if (isBackground) {
            background_process(tokens,background_pids, &bg_count);
        } else {
            forground_process(tokens);
        }
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
