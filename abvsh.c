#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define NO_OF_TOKENS 15
#define DELIMS " \a\t\n\r"


//day - 1 phrasing the input
//day - 2 fork and execute 
//day - 3 fixing the termination bug 
//day - 4 adding the built in commands

void loop();
char **parser(char *line);
char *readline();
int executer(char **tokens);
int abvsh_cd(char **tokens);
int abvsh_help(char **tokens);
int abvsh_exit(char **tokens);
int main(){
	loop();

	return EXIT_SUCCESS;
}


void loop(){
	int status;
	char *usr_name = getenv("USER");
	char **tokens;
	char *line;
	do{
    printf("(%s)>",usr_name);
    fflush(stdout);
		line = readline();
		tokens = parser(line);
    status = executer(tokens);
    free(line);
		free(tokens);
	}while(status);
}

char *readline(){
  //ch is int becuase the getc() returns EOF which is -1 mostly
  int ch;
	char *buffer;
	int index = 0;
  int adder = 1;

	buffer = (char *)malloc(BUFFER_SIZE*adder*sizeof(char));
  
  // in case of memory allocation error
  if(!buffer){
    fprintf(stderr,"Error: abvsh memory allocation error");
    exit(EXIT_FAILURE);
  }

  while((ch = getc(stdin)) != EOF && ch != '\n'){
    buffer[index] = ch;
    index++;
    if(index >= BUFFER_SIZE * adder){
      buffer = (char *)realloc(buffer,BUFFER_SIZE*(++adder)*sizeof(char));
    }
    // in case memory re-allocation error 
    if(!buffer){
      fprintf(stderr,"Error: abvsh memory allocation error");
      exit(EXIT_FAILURE);
    }
  }
  buffer[index] = '\0';
  return buffer;
}

char **parser(char *line){

  char **tokens;
  int adder = 1;
  int index = 0;
  tokens = (char **)malloc(NO_OF_TOKENS*adder*sizeof(char *));
  if(!tokens){
    fprintf(stderr,"Error: abvsh memory allocation error");
    exit(EXIT_FAILURE);
  } 

  tokens[index] = strtok(line,DELIMS);

  for(;;){
    if((tokens[++index] = strtok(NULL,DELIMS)) == NULL){
      tokens[++index] = NULL; //for identifying the end ot the tokens array 
      return tokens;
    }
    if(index > NO_OF_TOKENS*adder){
      tokens = (char **)realloc(tokens,NO_OF_TOKENS*(++adder)*sizeof(char*));
    }
    if(!tokens){
      fprintf(stderr,"Error: abvsh memory allocation error");
      exit(EXIT_FAILURE);
    }
  }
}

int create(char **tokens){
  pid_t id = fork();
  int stat;
  if(id == -1){ // fork error
    perror("abvsh");
  }
  else if (id == 0){ // child process
    int exe = execvp(tokens[0],tokens);
    if(exe == -1){
      perror("abvsh");
      exit(EXIT_FAILURE);//to kill the child process not the shell
    }
    exit(EXIT_FAILURE);//to kill the child process not the shell
  }
  else{ // parent process 
    do{
      waitpid(id,&stat,WUNTRACED);//it will wait until the child process is terminated by a signal or until it is completed normally (by return 0 or exit in its code )
    }while(!WIFEXITED(stat) && !WIFSIGNALED(stat));//wifexited for normal termination & wifsignaled for termination by signals like ctrl + c and other keys
  }
  return 1;
}

char *builtin_func[]= {
  "cd","help","exit"
};

int no_of_builtin(){
  return (sizeof(builtin_func) / sizeof(char*));
}

//name the builtin fuction with shell name before to avoid fuction name conflict 
int abvsh_cd(char **tokens){
  if(tokens[1] == NULL){
    fprintf(stderr,"abvsh: expected argument to \"cd\" \n");
  }
  else{
    if(chdir(tokens[1]) != 0){
      perror("abvsh");
    }
  }
  return 1;
}

int abvsh_help(char **tokens){
  printf("\n");
  printf("Abhinav's Basic shell : abvsh \n");
  printf("type the program nmae and the argumnets and press enter\n");
  printf("The Builtin fucntions are:\n");
  for (int i = 0 ; i < no_of_builtin() ; i++){
    printf("\t%s\n",builtin_func[i]);
  }
  printf("\n");
  return 1;
}

int abvsh_exit(char **tokens){
  return 0; 
}

int (*farr[])(char **) = { abvsh_cd , abvsh_help , abvsh_exit };

int executer(char **tokens){
  if (tokens[0] == NULL){
    return 1;
  }
  for(int i = 0 ; i < no_of_builtin() ; i++){
    if(strcmp(tokens[0],builtin_func[i]) == 0 ){
     return (*farr[i])(tokens);
    }
  }
  return create(tokens);
}

