#ifndef DATA_RELAYER_H
#define DATA_RELAYER_H

#include <iostream>
#include <functional>
#include <map>
#include <memory.h>
#include <arpa/inet.h>

#include "include/w1candbc.h"


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

    bool system_endian_ = 0;
    CanAdaptor* canlib_ = NULL;

    map<int,int> magMap_; // code , MSG_ID
    map<int,string> channelMap_; // code , channel

  public:
    DataRelayer();
    virtual ~DataRelayer();

    void ControlSteering(float angle);
    void ControlVel(float vel);
    void ControlHardware(bool horn, bool head_light, bool right_light, bool left_light);
    void StopPostMessage(unsigned int id);

    /**
    * @brief Register a RPM callback function
    * @details Registering class member function callbacks (std:funcion)
    * @param pfunc function point
    * @return void
    * @exception
    */
    template<typename T>
    void RegistRpmCallback(T *pClassType,void(T::*pfunc)(int,int,int)){
      rpmCallback = move(bind(pfunc, pClassType
        , placeholders::_1
        , placeholders::_2
        , placeholders::_3
        ));
    }
    /**
    * @brief Register a FAULT callback function
    * @details Registering class member function callbacks (std:funcion)
    * @param pfunc function point
    * @return void
    * @exception
    */
    template<typename T>
    void RegistFaultCallback(T *pClassType,void(T::*pfunc)(int,int)){
      faultCallback = move(bind(pfunc, pClassType
        , placeholders::_1
        , placeholders::_2
        ));
    }

    template<typename T>
    void RegistRpmCallback(T *pClassType,void(T::*pfunc)(int,int,int));
    template<typename T>
    void RegistFaultCallback(T *pClassType,void(T::*pfunc)(int,int));

    void Run();
    void SendTest();

  private:
    void SetmsgMap(int svcid,int msgid,string device);

    void SendMessageControlSteering(float steering_angle_cmd);
    void SendMessageControlAccelerate(float vel);
    void SendMessageControlHardware(bool Horn,bool HeadLight,bool Right_Turn_Light, bool Left_Turn_Light);

    void Handler_VCU_EPS_Control_Request (VCU_EPS_Control_Request msg);
    void Handler_Remote_Control_Shake (Remote_Control_Shake msg);
    void Handler_Remote_Control_IO (Remote_Control_IO msg);
    void Handler_DBS_Status (DBS_Status msg);
    void Handler_VCU_DBS_Request (VCU_DBS_Request msg);
    void Handler_MCU_Torque_Feedback (MCU_Torque_Feedback msg);
    unsigned short ConvertSpeedUnits(float vel);

    bool is_big_endian(){
      char buf[2] = {0,1};
      unsigned short *val = reinterpret_cast<unsigned short*>(buf);
      return *val == 1;
    }

 };

#endif //FUNCTIONCALLBACK_PARENT_H
