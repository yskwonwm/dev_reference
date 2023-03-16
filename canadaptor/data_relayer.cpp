
#include "can_adaptor.hpp"
#include "data_relayer.hpp"


DataRelayer::DataRelayer() {
      system_endian = is_big_endian();
    }

DataRelayer::~DataRelayer() {
      if( canlib != NULL ){
        canlib->release();
      }
    }

/**
* @brief Send control commands.(ControlSteering)
* @details
* @param steering_angle_cmd Steering angle
* @return void
* @exception
*/
void DataRelayer::control_steering(float angle){
    sendMessageControlSteering(angle);
}

/**
* @brief Send control commands.(ControlVel)
* @details
* @param vel  Accelerate speeed
* @return void
* @exception
*/
void DataRelayer::control_vel(float vel){//iECU_Control_Accelerate, iECU_Control_Brake둘다 사용)
    sendMessageControlAccelerate(vel);
}

/**
* @brief Send control commands.(ControlHardware)
* @details
* @param Horn true : on, false : 0ff
* @param HeadLight true : on, false : 0ff
* @param Right_Turn_Light true : on, false : 0ff
* @param Left_Turn_Light true : on, false : 0ff
* @return void
* @exception
*/
void DataRelayer::control_hardware(bool horn, bool head_light, bool right_light, bool left_light){
    sendMessageControlHardware( horn,  head_light,  right_light,  left_light);
}


/**
* @brief Register a RPM callback function
* @details Register a callback function (static funcion)
* @param pfunc function point
* @return void
* @exception
*/
void DataRelayer::RegistRpmCallback(void(*pfunc)(int,int,int)){
    rpmCallback = static_cast<void(*)(int,int,int)>(pfunc);
};

/**
* @brief Register a FAULT callback function
* @details Register a callback function (static funcion)
* @param pfunc function point
* @return void
* @exception
*/
void DataRelayer::RegistFaultCallback(void(*pfunc)(int,int)){
    faultCallback = static_cast<void(*)(int,int)>(pfunc);
};

/**
* @brief Register a RPM callback function
* @details Registering class member function callbacks (std:funcion)
* @param pfunc function point
* @return void
* @exception
*/
template<typename T>
void DataRelayer::RegistRpmCallback(T *pClassType,void(T::*pfunc)(int,int,int)){
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
void DataRelayer::RegistFaultCallback(T *pClassType,void(T::*pfunc)(int,int)){
    faultCallback = move(bind(pfunc, pClassType
    , placeholders::_1
    , placeholders::_2
    ));
}

/**
* @brief service <-> message id / channel mapping
* @details Mapping so that can channel and can id can be changed according to environment variables later
* @param
* @return void
* @exception
*/
void DataRelayer::setmsgMap(int svcid,int msgid,string device){
    magMap.insert(make_pair(svcid,msgid));
    channelMap.insert(make_pair(svcid,device));
}

/**
* @brief send API(ControlSteering)
* @details
* @param steering_angle_cmd steering angle
* @return void
* @exception
*/
void DataRelayer::sendMessageControlSteering(float steering_angle_cmd){

    if ( steering_angle_cmd > MAX_STEERING || steering_angle_cmd < MIN_STEERING ){
      return;
    }

    iECU_Control_Steering dat_1;
    memset(&dat_1,0x00,CAN_MAX_DLEN);

    dat_1.iecu_steering_angle_cmd = (steering_angle_cmd + OFFSET_STEERING) * RESOLUTION_STEERING_CTRL;
    dat_1.iecu_steering_valid = 1;

    canlib->postCanMessage<iECU_Control_Steering>(dat_1,IECU_CONTROL_STEERING,device_type[CAN1]);
};

/**
* @brief send API(ControlAccelerate)
* @details
* @param vel  Accelerate speeed
* @return void
* @exception
*/
void DataRelayer::sendMessageControlAccelerate(float vel){

    unsigned char gear;
    if ( vel > 0 ) {
      gear = FORWARD;
    } else if ( vel < 0 ) {
      gear = REVERSE;
    } else {
      gear = NEUTRAL;
    }

    iECU_Control_Accelerate dat_1;
    memset(&dat_1,0x00,CAN_MAX_DLEN);
    dat_1.iecu_accelerate_gear = gear; // (vel >0) = 1, (vel = 0) = 2, (vel < 0) = 3
    dat_1.iecu_accelerate_valid = 1;
    dat_1.iecu_accelerate_work_mode = 1;
    dat_1.iecu_speed_control = [](float v){return v * CNV_SPEED_FACTOR * RESOLUTION_SPEED_CTRL;}(vel) ;
    dat_1.iecu_torque_control = 0;
    canlib->postCanMessage<iECU_Control_Accelerate>(dat_1,IECU_CONTROL_ACCELERATE,device_type[CAN1]);

    iECU_Control_Brake dat_2;
    memset(&dat_2,0x00,CAN_MAX_DLEN);
    dat_2.iecu_brakepressure_cmd = 100;
    dat_2.iecu_dbs_valid = 1;
    canlib->postCanMessage<iECU_Control_Brake>(dat_2,IECU_CONTROL_BRAKE,device_type[CAN1]);
};

/**
* @brief send API(ControlHardware)
* @details
* @param Horn true : on, false : 0ff
* @param HeadLight true : on, false : 0ff
* @param Right_Turn_Light true : on, false : 0ff
* @param Left_Turn_Light true : on, false : 0ff
* @return void
* @exception
*/
void DataRelayer::sendMessageControlHardware(bool Horn,bool HeadLight,bool Right_Turn_Light, bool Left_Turn_Light){
    iECU_Control_Hardware dat_1;
    memset(&dat_1,0x00,CAN_MAX_DLEN);
    dat_1.iecu_headlight = 1;
    dat_1.iecu_horn_control = 1;
    dat_1.iecu_left_turn_light = 1;
    dat_1.iecu_right_turn_light = 1;
    canlib->postCanMessage<iECU_Control_Hardware>(dat_1,IECU_CONTROL_HARDWARE,device_type[CAN1]);
};
// 필요에 따라 추가 한다.외부 인터페이스 API 정의 필요

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::handler_VCU_EPS_Control_Request (VCU_EPS_Control_Request msg){
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
void DataRelayer::handler_Remote_Control_Shake (Remote_Control_Shake msg){
    cout << "[recv] Remote_Control_Shake : " << (int)msg.remote_y1_longitudinal_control <<","<<(int)msg.remote_x2_lateral_control << endl;
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::handler_Remote_Control_IO (Remote_Control_IO msg){
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
void DataRelayer::handler_DBS_Status (DBS_Status msg){
    cout << "[recv] DBS_Status : " << (int)msg.dbs_fault_code <<","<<(int)msg.dbs_hp_pressure <<","<<(int)msg.dbs_system_status << endl;

    faultCallback(CAN_NO_FAULT,msg.dbs_fault_code);
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::handler_VCU_DBS_Request (VCU_DBS_Request msg){
    cout << "[recv] VCU_DBS_Request : " << (int)msg.vcu_dbs_pressure_request <<","<<(int)msg.vcu_dbs_request_flag << endl;
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::setHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::handler_MCU_Torque_Feedback (MCU_Torque_Feedback msg){
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
void DataRelayer::run(){

    canlib = CanAdaptor::getInstance();
    canlib->initialize(system_endian);

    // 수신 핸들러 등록
    // canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_VCU_EPS_Control_Request,VCU_EPS_CONTROL_REQUEST,device_type[CAN1]);
    // canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_Remote_Control_Shake,REMOTE_CONTROL_SHAKE_2,device_type[CAN1]);
    // canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_Remote_Control_IO,REMOTE_CONTROL_IO,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_DBS_Status,DBS_STATUS,device_type[CAN1]);
    // canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_VCU_DBS_Request,VCU_DBS_REQUEST,device_type[CAN1]);
    canlib->setHandler<DataRelayer>(this,&DataRelayer::handler_MCU_Torque_Feedback,TORQUE_FEEDBACK,device_type[CAN0]);

    // 수신 리스너 오픈
    vector<string> device;
    //device.push_back(device_type[CAN1]);
    device.push_back(device_type[CAN0]);

    int ret = 0;
   

    while(canlib->open(device)){
      cout << "open fail" << endl;
      sleep(2);      
    }
    
    while(canlib-> runControlFlag(1,device_type[CAN1])){
      cout << "run config fail" << endl;
      sleep(2);
    }
    //포트 오픈 체크 스레드
    cout << "Start checking for can channel fault" << endl;
    canlib->checkSocketStatus(device,faultCallback);
    
}

/**
* @brief data send test
* @details
* @param
* @return void
* @exception
*/
void DataRelayer::sendtest(){

    // Mode_Control_Flag dat_5;
    // memset(&dat_5,0x00,8);
    // dat_5.mode_control_request_flag = 1;
    // canlib->postCanMessage<Mode_Control_Flag>(dat_5,MODE_CONTROL_FLAG,device_type[CAN1]);

  // 전송 테스트
    iECU_Control_Hardware dat_1;
    memset(&dat_1,0x00,CAN_MAX_DLEN);
    dat_1.iecu_headlight = 1;
    dat_1.iecu_horn_control = 0;
    dat_1.iecu_left_turn_light = 1;
    dat_1.iecu_right_turn_light = 1;
    canlib->postCanMessage<iECU_Control_Hardware>(dat_1,IECU_CONTROL_HARDWARE,device_type[CAN1]);

    // iECU_Control_Accelerate dat_2;
    // memset(&dat_2,0x00,8);
    // dat_2.iecu_accelerate_gear = 1;
    // dat_2.iecu_accelerate_valid = 1;
    // dat_2.iecu_accelerate_work_mode = 1;
    // dat_2.iecu_speed_control = 1;
    // dat_2.iecu_torque_control = 1;
    // canlib->postCanMessage<iECU_Control_Accelerate>(dat_2,IECU_CONTROL_ACCELERATE,device_type[CAN1]);

    // iECU_Control_Brake dat_3;
    // memset(&dat_3,0x00,8);
    // dat_3.iecu_brakepressure_cmd = 1;
    // dat_3.iecu_dbs_valid = 1;
    // canlib->postCanMessage<iECU_Control_Brake>(dat_3,IECU_CONTROL_BRAKE,device_type[CAN1]);

    // iECU_Control_Steering dat_4;
    // memset(&dat_4,0x00,8);
    // dat_4.iecu_steering_angle_cmd = 1;
    // dat_4.iecu_steering_valid = 1;
    // canlib->postCanMessage<iECU_Control_Steering>(dat_4,IECU_CONTROL_STEERING,device_type[CAN1]);


   /*
    //canlib->print_map_state("data_relayer");
    // memcpy(bydata,&dat_2,8);
    // for(unsigned char d : bydata){
    //   fprintf(stdout,"0x%02x ",d);
    // }
    // cout << endl;
  */
}



