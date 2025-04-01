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

// Handle forground process execution
void forground_process(char **tokens)
{
	
    if (strcmp(tokens[0], "cd") == 0) {
        cd(tokens);
		return;
    }
    pid_t pid = fork();
    if (pid == 0) {
        execvp(tokens[0], tokens);
        fprintf(stdout, "command not found: %s\n", tokens[0]);
        exit(0);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);  // Wait for the foreground process to finish
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

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;       
	pid_t background_pids[MAX_BACKGROUND_PROCESSES];
    int bg_count = 0;       
	int i;


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
        while (tokens[tokenCount] != NULL) {
            tokenCount++;
        }
        
        if (tokenCount > 0 && strcmp(tokens[tokenCount - 1], "&") == 0) {
            isBackground = 1;
            tokens[tokenCount - 1] = NULL;  // Remove the '&' token
        }

        if (isBackground) {
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
