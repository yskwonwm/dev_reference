
#include <iostream>
#include <unistd.h> 
#include <signal.h>

#include "data_relayer.hpp"

//extern int optind, opterr, optopt;
//static char *progname;
static volatile int state = 1;

static void sigterm(int signo)
{
  fprintf(stdout, "SIGNAL %d  in main\n", signo);
	state = 0;
}

static void rpmCallback(int remote_f_horn
                    ,int remote_d_headlight
                    ,int remote_b_motor_holding_brake
                    ){

  cout << "[main] callback Remote_Control_IO : " << (int)remote_f_horn 
  << "," << (int)remote_d_headlight
  << "," << (int)remote_b_motor_holding_brake
  << endl;
}

int main(int argc, char **argv){


	signal(SIGINT, sigterm);  

  DataRelayer obj;

  obj.RegistCallback(rpmCallback);
  obj.run();

	//sleep(1);
    while(state){      
       
       obj.sendtest();
       sleep(2);
    }
  
  cout << "***can test end!!!***" << endl;
  return 0;
}
