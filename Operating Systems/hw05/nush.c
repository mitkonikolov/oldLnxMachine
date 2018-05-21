#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <alloca.h>
#include <ctype.h>
#include <fcntl.h>


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

  const char s[2] = " ";
  char *token;

  // get the command from cmd
  char* command = strtok(cmd, s);

  // command is the built-in cd
  if(strcmp(command, "cd")==0) {
    token = strtok(NULL, s);
    chdir(token);
  }
  else if (strcmp(command, "exit")==0) {
    exit(0);
  }
  else {
    // if opHUH is equal to 0, then there is no operator present
    int opHUH = 0;
    char *args[80];
    int p=0;
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

      if(strcmp(oper, "&")!=0) {
        int status;
        waitpid(cpid, &status, 0);

        if(opHUH) {
          if((strcmp(oper, "&&")==0) && (strcmp(args[0], "false")==0)) {
          }
          else if ((strcmp(oper, "||")==0) && (strcmp(args[0], "true")==0)) {
          }
          else {
            int status3;
            int cpid2;

            cpid2 = fork();
            int status2;
            if(cpid2) {
              waitpid(cpid2, &status2, 0);

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

    } //only older child (cpid) knows about this process
    else {
      // child that always runs
      if((strcmp(oper, "<")==0)) {
        int fd = open(args2[0], O_RDONLY);
        dup2(fd, STDIN_FILENO);
        int q=1;
        char cmd[256];
        while(!feof(stdin)) {

          if(fgets(cmd, 256, stdin)) {
            args[q]=cmd;
          }
        }
      }
      execvp(cmd, args);
      printf("Can't get here, exec only returns on error.");
    }
  }
}

int
main(int argc, char* argv[])
{

  char cmd[256];

  if (argc!=1) {
    int fd = open(argv[1], O_RDONLY);
    dup2(fd, STDIN_FILENO);
  }

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
