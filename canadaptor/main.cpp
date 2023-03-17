
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

  cout << "[main] callback RPM Status : " << (int)remote_f_horn
  << "," << (int)remote_d_headlight
  << "," << (int)remote_b_motor_holding_brake
  << endl;
}

// can_falut
// 0x0: no fault
// 0X1: invalid connection

// dbs_fault 
// 0x0: no fault
// 0X1: oil pressure sensor failure
// 0X2: Insufficient system power supply
// 0X4: Controller temperature is too high
// 0X8: Motor failure
// 0X10: The controller itself is faulty
// 0X20: Oil pressure command message lost
// 0X40: CAN BUS OFF
static void faultCallback(int can_falut
                         ,int dbs_fault){

  cout << "[main] callback DBS_Status : " << (int)can_falut
  << "," << (int)dbs_fault
  << endl;
}

int main(int argc, char **argv){


  signal(SIGINT, sigterm);

  //std::system("~/setup.sh");

  DataRelayer obj;

  // register receive callback function
  obj.RegistFaultCallback(faultCallback);
  obj.RegistRpmCallback(rpmCallback);
  obj.Run();

	// sleep(1);
  // send control command 
  obj.ControlHardware(false, true, true, false); //bool horn, bool head_light, bool right_light, bool left_light
  obj.ControlSteering(20); // float angle
  obj.ControlVel(4); // float vel

  while(state){
    //obj.SendTest();
    sleep(5);
    //obj.StopPostMessage(1286);
  }

  cout << "***can test end!!!***" << endl;
  return 0;
}
