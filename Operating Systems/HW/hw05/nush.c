#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <alloca.h>
#include <ctype.h>
#include <fcntl.h>

// returns 1 iff the token is equal to one of the operators below
int
strOper(char* token) {
  if((strcmp(token, "&&")==0) ||
  (strcmp(token, "||")==0) ||
  (strcmp(token, ";")==0)  ||
  (strcmp(token, "&")==0)  ||
  (strcmp(token, "<")==0)) {
    return 1;
  }
  else {
    return 0;
  }
}


void
execute(char* cmd)
{

  // remove new line character from the end of cmd
  char *newline = strchr( cmd, '\n' );
  if (newline) *newline = 0;

  // s is my delimiter
  const char s[2] = " ";
  char *token;

  // get the command from cmd
  char* command = strtok(cmd, s);

  // command is the built-in cd
  if(strcmp(command, "cd")==0) {
    token = strtok(NULL, s);
    chdir(token);
  } //command is exit
  else if (strcmp(command, "exit")==0) {
    exit(0);
  }
  else {
    // if opHUH is equal to 0, then there is no operator present
    int opHUH = 0;
    char *args[80];
    int p=0;
    // args2 is an array that will be used to store
    // data about the second process's arguments
    char *args2[80];
    int q=0;
    char *oper="";

    /* get the first token */
    token = command;

    /* walk through other tokens */
    while( token != NULL )
    {
      if(opHUH==0) {
        args[p] = alloca(256);
        args[p] = token;
        p++;
      }
      else {
        args2[q]=alloca(256);
        args2[q]=token;
        q++;
      }

      token = strtok(NULL, s);
      if(token!=NULL && strOper(token)) {
        opHUH=1;
        oper = token;
        token = strtok(NULL, s);
      }
    }

    // null terminate the arrays
    if(opHUH==0) {
      args[p]=NULL;
    }
    else {
      args[p]=NULL;
      args2[q]=NULL;
    }

    int cpid;
    cpid = fork();
    // only parent knows about this cpid
    if (cpid) {
      // if the process has to be performed in the
      // background, then we do not need to wait for it
      if(strcmp(oper, "&")!=0) {
        int status;
        waitpid(cpid, &status, 0);

        if(opHUH) {
          if((strcmp(oper, "&&")==0) && (strcmp(args[0], "false")==0)) {
          }
          else if ((strcmp(oper, "||")==0) && (strcmp(args[0], "true")==0)) {
          }
          else {
            int status2;
            int status3;
            int cpid2;

            cpid2 = fork();

            // parent has a second child now
            if(cpid2) {
              waitpid(cpid2, &status2, 0);

              // if the given operator was ";", then nothing has to be done
              // with the resultant statuses from the two operations
              if(strcmp(oper, ";")!=0) {
                int cpid3 = fork();
                if(cpid3) {
                  waitpid(cpid3, &status3, 0);
                }
                else {
                  char* args3[2];
                  args3[0] = (char *)&status;
                  args3[1] = (char *)&status2;
                  execvp(oper, args3);
                }
              }
            }
            else { // second child
              execvp(args2[0], args2);
            }
          }
        }

      }

    } // child that always runs
    else {
      execvp(cmd, args);
      printf("Can't get here, exec only returns on error.");

    }
  }
}

int
main(int argc, char* argv[])
{

  char cmd[256];

  // if a file was passed, make the file stdin
  if (argc!=1) {
    int fd = open(argv[1], O_RDONLY);
    dup2(fd, STDIN_FILENO);
  }

  // read until eof is seen in stdin
  while(!feof(stdin)) {
    if (argc==1) {
      printf("nush$ ");
      fflush(stdout);
    }

    if(fgets(cmd, 256, stdin)) {
      execute(cmd);
    }
  }

  return 0;
}
