#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

const int PATH_MAX = 1024;

void handleChangeDir(char* parsed_command[]){
  if(parsed_command[1][0] == '~'){
    chdir(getenv("HOME")); // Home directory
  } else{
  DIR* dir = opendir(parsed_command[1]); // checks for directory existence
  if(dir)
    chdir(parsed_command[1]); // Absolute and relative work for chdir
  else
    printf("cd: %s: No such file or directory\n", parsed_command[1]);
  }
} 

void handleTypeCommand(char* parsed_command[], char* path){ // pass by value, do not change parsed command
  if((strcmp(parsed_command[1],"echo") == 0) || (strcmp(parsed_command[1],"type") == 0) || 
      (strcmp(parsed_command[1], "exit") == 0) ||(strcmp(parsed_command[1],"pwd") == 0) ||
      (strcmp(parsed_command[1],"cd") == 0)){
    printf("%s is a shell builtin\n", parsed_command[1]);
  }else{
    char* path_copy = strdup(path);
    char* token = strtok(path_copy, ":");
    char* filepath;
    FILE* file;

    while(token != NULL){
      snprintf(filepath, PATH_MAX, "%s/%s", token, parsed_command[1]); // string concat max environ length = 1024

      if((file = fopen(filepath, "r"))){
        fclose(file);
        printf("%s is %s\n", parsed_command[1], filepath);
        free(path_copy);
        return;
      }
      token = strtok(NULL, ":");
    }
    printf("%s: not found\n", parsed_command[1]);
  }
}

int main() {
  char input[100];
  char* parsed_command[2];
  char* path = getenv("PATH");

  while(1){
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    fgets(input, 100, stdin);
    input[strlen(input) - 1] = '\0'; // make it string terminator instead of new line

    if(strcmp(input, "exit 0") == 0){
      return 0;
    }
    parsed_command[0] = input;
    //get the command using int iterator and find first space
    int com_pos = 0;
    for(com_pos; input[com_pos] != '\0'; com_pos++){
      if(input[com_pos] == ' ')
        break;
    }
    
    // parse the arguments
    if(com_pos == strlen(input)){
      parsed_command[1] = NULL;
    }else {
      input[com_pos] = '\0';
      parsed_command[1] = input + com_pos + 1;
    }
    
    if(strcmp(parsed_command[0], "cd") == 0){
      handleChangeDir(parsed_command);
    } else if(strcmp(parsed_command[0],"echo") == 0){
      printf("%s\n", parsed_command[1]);
    } else if(strcmp(parsed_command[0],"type") == 0){
      handleTypeCommand(parsed_command, path);
    }else if(strcmp(parsed_command[0], "pwd")==0){
      char cwd[PATH_MAX];
      if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s\n", cwd);
      }
    } else {
      char* path_copy = strdup(path);
      char* token = strtok(path_copy, ":");
      char* filepath = malloc(PATH_MAX);
      FILE* file;
      char filefound = 0;
      while(token != NULL){
        snprintf(filepath, PATH_MAX, "%s/%s", token, parsed_command[0]);//string concat max environ length = 1024
        if((file = fopen(filepath, "r"))){
          fclose(file);
          //execute program
          pid_t exepid;
          if(fork()==0){
            //child executes so main shell doesnt fail
            char *arg[3] = {parsed_command[0], parsed_command[1], NULL};
            execvp(parsed_command[0], arg);
            exit(0);
          }
          else
            exepid = wait(NULL);
          free(path_copy);
          free(filepath);
          filefound = 1;
          break;
        }
        token = strtok(NULL, ":");
      }
      if(filefound != 1)
        printf("%s: command not found\n", parsed_command[0]);
    }

  }

  return 0;
}
