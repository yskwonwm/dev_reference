
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

  // register receive callback function
  obj.RegistRpmCallback(rpmCallback);
  obj.run();

	//sleep(1);
   // obj.sendtest();
  // send control command 
  obj.control_hardware(false, true, true, false); //bool horn, bool head_light, bool right_light, bool left_light
  obj.control_steering(20); // float angle
  obj.control_vel(4); // float vel

  while(state){

    //obj.sendtest();
    sleep(2);
  }

  cout << "***can test end!!!***" << endl;
  return 0;
}
