#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc,char * argv[])
{
  char buf[512];
  FILE *cmd_pipe = popen("pidof -s noStarw", "r");
//  FILE *cmd_pipe = popen("pidof -s mateusz", "r");// pipe open komendy pidof
  fgets(buf, 512, cmd_pipe);//zczytanie z buffora wyniku komendy pipof
  pid_t pid = strtoul(buf, NULL, 10); //wczytanie samej liczby ze stringa
  pclose( cmd_pipe );//zamkniecie pipa
  if(argc>1)//sprawdzenie czy ktos podal paramentr
    pid  = atoi(argv[1]);//podmianka pida
  kill(pid, SIGUSR1);//wyslaie sygnalu do precesu 
  printf("Send SIGUSR1 to pid: %d\n",pid);
  return 0;
}
