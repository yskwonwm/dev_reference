#ifndef FUNCTIONCALLBACK_PARENT_H
#define FUNCTIONCALLBACK_PARENT_H

#include <iostream>
#include <functional>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <typeinfo>
#include <unistd.h>

#include "include/w1candbc.h"
#include "include/can_define.hpp"

using namespace std;

class CanAdaptor;
/**
    @class   DataRelayer
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   Wrapping classes for CAN data conversion and relaying
    @version 0.0.1
    @warning
*/
class DataRelayer {

  private:
    //void(*fpoint)(int,int,int);
    typedef std::function<void(int,int)> func_fault_callback; // Callback function pointer variable definition
    typedef std::function<void(int,int,int)> func_rpm_callback; // Callback function pointer variable definition
    //typedef std::function<void(int,int,int)> func_other_callback; // Callback function pointer variable definition

    func_fault_callback faultCallback;
    func_rpm_callback rpmCallback; // Callback function pointer variable definition
    //func_other_callback otherCallback; // Callback function pointer variable definition

    bool system_endian = 0;
    CanAdaptor* canlib = NULL;

    map<int,int> magMap; // code , MSG_ID
    map<int,string> channelMap; // code , channel

  public:

    DataRelayer(); 
    virtual ~DataRelayer();

    void control_steering(float angle);
    void control_vel(float vel);
    void control_hardware(bool horn, bool head_light, bool right_light, bool left_light);

    void RegistRpmCallback(void(*pfunc)(int,int,int));
    void RegistFaultCallback(void(*pfunc)(int,int));

    template<typename T> 
    void RegistRpmCallback(T *pClassType,void(T::*pfunc)(int,int,int));
    template<typename T> 
    void RegistFaultCallback(T *pClassType,void(T::*pfunc)(int,int));

  private:
    void setmsgMap(int svcid,int msgid,string device);

    void sendMessageControlSteering(float steering_angle_cmd);
    void sendMessageControlAccelerate(float vel);
    void sendMessageControlHardware(bool Horn,bool HeadLight,bool Right_Turn_Light, bool Left_Turn_Light);
  
    void handler_VCU_EPS_Control_Request (VCU_EPS_Control_Request msg);
    void handler_Remote_Control_Shake (Remote_Control_Shake msg);
    void handler_Remote_Control_IO (Remote_Control_IO msg);
    void handler_DBS_Status (DBS_Status msg);
    void handler_VCU_DBS_Request (VCU_DBS_Request msg);
    void handler_MCU_Torque_Feedback (MCU_Torque_Feedback msg);
    unsigned short convert_speed_units(float vel);

    bool is_big_endian(){
      char buf[2] = {0,1};
      unsigned short *val = reinterpret_cast<unsigned short*>(buf);
      return *val == 1;
    }

public:
    void run();
    void sendtest();
 };

#endif //FUNCTIONCALLBACK_PARENT_H
