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

#include "can_adaptor.hpp"
#include "include/can_define.hpp"

using namespace std;

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
    typedef std::function<void(int,int,int)> func_callback; // Callback function pointer variable definition
    func_callback rpmCallback; // Callback function pointer variable definition
  
    bool system_endian = 0;
    CanAdaptor* canlib = NULL;
  
    map<int,int> magMap; // code , MSG_ID
    map<int,string> channelMap; // code , channel

  public:  

    DataRelayer() {
      system_endian = is_big_endian();    
    }

    virtual ~DataRelayer() {
      if( canlib != NULL ){
        canlib->release();
      }
    }
  
    void control_steering(float angle){
       sendMessageControlSteering(angle);
    }

    void control_vel(float vel){//iECU_Control_Accelerate, iECU_Control_Brake둘다 사용)
       sendMessageControlAccelerate(vel);
    }

    void control_hardware(bool horn, bool head_light, bool right_light, bool left_light){
      sendMessageControlHardware( horn,  head_light,  right_light,  left_light);
    }

/**
* @brief service <-> message id / channel mapping
* @details 
* @param 
* @return void
* @exception
*/
  void setmsgMap(int svcid,int msgid,string device){
    magMap.insert(make_pair(svcid,msgid));
    channelMap.insert(make_pair(svcid,device)); 
  }

/**
* @brief send API(ControlSteering)
* @details 
* @param 
* @return void
* @exception
*/
  void sendMessageControlSteering(float steering_angle_cmd){
  
    if ( steering_angle_cmd > MAX_STEERING || steering_angle_cmd < MIN_STEERING ){
      return;
    }

    iECU_Control_Steering dat_1;
    memset(&dat_1,0x00,8);

    dat_1.iecu_steering_angle_cmd = (steering_angle_cmd + OFFSET_STEERING) * RESOLUTION_STEERING_CTRL;
    dat_1.iecu_steering_valid = 1;

    canlib->postCanMessage<iECU_Control_Steering>(dat_1,IECU_CONTROL_STEERING,device_type[CAN0]);    
  };

/**
* @brief send API(ControlAccelerate)
* @details 
* @param 
* @return void
* @exception
*/
  void sendMessageControlAccelerate(float vel){
    
    unsigned char gear;
    if ( vel > 0 ) {
      gear = FORWARD;
    } else if ( vel < 0 ) {
      gear = REVERSE;
    } else {
      gear = NEUTRAL;
    }

    iECU_Control_Accelerate dat_1;    
    memset(&dat_1,0x00,8);
    dat_1.iecu_accelerate_gear = gear; // (vel >0) = 1, (vel = 0) = 2, (vel < 0) = 3
    dat_1.iecu_accelerate_valid = 1;
    dat_1.iecu_accelerate_work_mode = 1;
    dat_1.iecu_speed_control = convert_speed_units(vel);
    dat_1.iecu_torque_control = 0;    
    canlib->postCanMessage<iECU_Control_Accelerate>(dat_1,IECU_CONTROL_ACCELERATE,device_type[CAN0]);
    
    iECU_Control_Brake dat_2;
    memset(&dat_2,0x00,8);
    dat_2.iecu_brakepressure_cmd = 100;
    dat_2.iecu_dbs_valid = 1;
    canlib->postCanMessage<iECU_Control_Brake>(dat_2,IECU_CONTROL_BRAKE,device_type[CAN0]);
  };

/**
* @brief send API(ControlHardware)
* @details 
* @param 
* @return void
* @exception
*/
  void sendMessageControlHardware(bool Horn,bool HeadLight,bool Right_Turn_Light, bool Left_Turn_Light){
    iECU_Control_Hardware dat_1;    
    memset(&dat_1,0x00,8);
    dat_1.iecu_headlight = 1;
    dat_1.iecu_horn_control = 1;
    dat_1.iecu_left_turn_light = 1;
    dat_1.iecu_right_turn_light = 1;
    canlib->postCanMessage<iECU_Control_Hardware>(dat_1,IECU_CONTROL_HARDWARE,device_type[CAN0]);    
  };
// 필요에 따라 추가 한다.외부 인터페이스 API 정의 필요



/**
* @brief Register a callback function
* @details Register a callback function (static funcion)
* @param pfunc function point
* @return void
* @exception
*/
  void RegistCallback(void(*pfunc)(int,int,int)){
    rpmCallback = static_cast<void(*)(int,int,int)>(pfunc);
  };

/**
* @brief Register a callback function
* @details Registering class member function callbacks (std:funcion)
* @param pfunc function point
* @return void
* @exception
*/
  template<typename T>
  void RegistCallback(T *pClassType,void(T::*pfunc)(int,int,int)){        
      rpmCallback = move(bind(pfunc, pClassType, placeholders::_1
      , placeholders::_2
      , placeholders::_3
      ));
  }

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_VCU_EPS_Control_Request (VCU_EPS_Control_Request msg){
  //   unsigned char  bydata[8];
  //   memcpy(bydata,&msg,8);    
  //   for(unsigned char d : bydata){
  //     fprintf(stdout,"0x%02x ",d);
  //  }
  //  cout << endl;
   // Motorola  LSB
   short vcu_eps_strangle = htons(msg.vcu_eps_strangle);    

   cout << "[recv] VCU_EPS_Control_Request : " << vcu_eps_strangle <<","<< (int)msg.vcu_eps_ctrlenable <<","<< (int)msg.vcu_eps_ctrlmode << endl;
 
  }

/**
* @brief Callback function to register with CanAdaptor:: 
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_Remote_Control_Shake (Remote_Control_Shake msg){
    cout << "[recv] Remote_Control_Shake : " << (int)msg.remote_y1_longitudinal_control <<","<<(int)msg.remote_x2_lateral_control << endl;
  }

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_Remote_Control_IO (Remote_Control_IO msg){
    cout << "[recv] Remote_Control_IO : " << (int)msg.remote_f_horn 
    << "," << (int)msg.remote_d_headlight
    << "," << (int)msg.remote_b_motor_holding_brake
    << "," << (int)msg.remote_a_mode_switch
    << "," << (int)msg.remote_c_speed_torque
    << "," << (int)msg.remote_e_gear << endl;
  }

/**
* @brief Callback function to register with CanAdaptor:: 
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_DBS_Status (DBS_Status msg){
    cout << "[recv] DBS_Status : " << (int)msg.dbs_fault_code <<","<<(int)msg.dbs_hp_pressure <<","<<(int)msg.dbs_system_status << endl;
  }

/**
* @brief Callback function to register with CanAdaptor:: 
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_VCU_DBS_Request (VCU_DBS_Request msg){
    cout << "[recv] VCU_DBS_Request : " << (int)msg.vcu_dbs_pressure_request <<","<<(int)msg.vcu_dbs_request_flag << endl;
  }

/**
* @brief Callback function to register with CanAdaptor:: 
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
  void handler_MCU_Torque_Feedback (MCU_Torque_Feedback msg){
    cout << "[recv] MCU_Torque_Feedback : " << (int)msg.mcu_current 
    <<","<<(int)msg.mcu_errorcode
    <<","<<(int)msg.mcu_motortemp 
    <<","<<(int)msg.mcu_shift 
    <<","<<(int)msg.mcu_speed  
    <<","<<(int)msg.mcu_torque << endl;

    rpmCallback((int)msg.mcu_shift
                    ,(int)msg.mcu_speed
                    ,(int)msg.mcu_torque);
  }

/**
* @brief run test
* @details 
* @param 
* @return void
* @exception
*/
  void run(){

    // cout << sizeof(iECU_Control_Hardware) << endl; 
    // cout << sizeof(iECU_Control_Accelerate) << endl; 
    // cout << sizeof(iECU_Control_Brake) << endl; 
    // cout << sizeof(iECU_Control_Steering) << endl; 
    // cout << sizeof(Mode_Control_Flag) << endl; 
    // cout << sizeof(VCU_EPS_Control_Request) << endl; 
    // cout << sizeof(Remote_Control_Shake) << endl; 
    // cout << sizeof(Remote_Control_IO) << endl; 
    // cout << sizeof(DBS_Status) << endl; 
    // cout << sizeof(VCU_DBS_Request) << endl; 
    // cout << sizeof(VCU_MCU_Request) << endl; 
    // cout << sizeof(MCU_Torque_Feedback) << endl;     
    canlib = CanAdaptor::getInstance();
    canlib->initialize(system_endian);

    // 수신 핸들러 등록   
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_VCU_EPS_Control_Request,VCU_EPS_CONTROL_REQUEST,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_Remote_Control_Shake,REMOTE_CONTROL_SHAKE_2,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_Remote_Control_IO,REMOTE_CONTROL_IO,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_DBS_Status,DBS_STATUS,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_VCU_DBS_Request,VCU_DBS_REQUEST,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_MCU_Torque_Feedback,0x010,device_type[CAN0]);

    // 수신 리스너 오픈    
    vector<string> device;
    device.push_back(device_type[CAN0]);
    device.push_back(device_type[CAN1]);
    
    int ret = canlib->open(device);            
  }

/**
* @brief convert speed units(m/s -> km/h)
* @details 
* @param 
* @return unsigned int
* @exception
*/
  unsigned short convert_speed_units(float vel){
    unsigned short speed_ctrl;
    speed_ctrl = vel * CNV_SPEED_FACTOR * RESOLUTION_SPEED_CTRL;     
    return  speed_ctrl;
  }

/**
* @brief data send test
* @details 
* @param 
* @return void
* @exception
*/
 void sendtest(){

  // 전송 테스트
    iECU_Control_Hardware dat_1;    
    memset(&dat_1,0x00,8);
    dat_1.iecu_headlight = 1;
    dat_1.iecu_horn_control = 1;
    dat_1.iecu_left_turn_light = 1;
    dat_1.iecu_right_turn_light = 1;
    canlib->postCanMessage<iECU_Control_Hardware>(dat_1,IECU_CONTROL_HARDWARE,device_type[CAN1]);

    iECU_Control_Accelerate dat_2;
    memset(&dat_2,0x00,8);
    dat_2.iecu_accelerate_gear = 1;
    dat_2.iecu_accelerate_valid = 1;
    dat_2.iecu_accelerate_work_mode = 1;
    dat_2.iecu_speed_control = 1;
    dat_2.iecu_torque_control = 1;    
    canlib->postCanMessage<iECU_Control_Accelerate>(dat_2,IECU_CONTROL_ACCELERATE,device_type[CAN1]);

    iECU_Control_Brake dat_3;
    memset(&dat_3,0x00,8);
    dat_3.iecu_brakepressure_cmd = 1;
    dat_3.iecu_dbs_valid = 1;
    canlib->postCanMessage<iECU_Control_Brake>(dat_3,IECU_CONTROL_BRAKE,device_type[CAN1]);

    iECU_Control_Steering dat_4;
    memset(&dat_4,0x00,8);
    dat_4.iecu_steering_angle_cmd = 1;
    dat_4.iecu_steering_valid = 1;
    canlib->postCanMessage<iECU_Control_Steering>(dat_4,IECU_CONTROL_STEERING,device_type[CAN1]);
   
    Mode_Control_Flag dat_5;
    memset(&dat_5,0x00,8);
    dat_5.mode_control_request_flag = 1;
    canlib->postCanMessage<Mode_Control_Flag>(dat_5,MODE_CONTROL_FLAG,device_type[CAN1]);
        
    //canlib->print_map_state("data_relayer");
    // memcpy(bydata,&dat_2,8);    
    // for(unsigned char d : bydata){
    //   fprintf(stdout,"0x%02x ",d);
    // }
    // cout << endl;
 }
};

#endif //FUNCTIONCALLBACK_PARENT_H