#include <vector>
#include <unistd.h>
#include <cmath>
#include "can_adaptor.hpp"
#include "data_relayer.hpp"

#include "include/can_define.hpp"
DataRelayer::DataRelayer() {
  system_endian_ = is_big_endian();
}

DataRelayer::~DataRelayer() {
  if( canlib_ != NULL ){
    canlib_->Release();
  }
}

/**
* @brief Send control commands.(ControlSteering)
* @details
* @param steering_angle_cmd Steering angle
* @return void
* @exception
*/
void DataRelayer::ControlSteering(float angle){
  SendMessageControlSteering(angle);
}

/**
* @brief Send control commands.(ControlVel)
* @details
* @param vel  Accelerate speeed
* @return void
* @exception
*/
void DataRelayer::ControlVel(float vel){//AD_CONTROL_ACCELERATE, AD_Control_Brake둘다 사용)
  SendMessageControlAccelerate(vel);
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
void DataRelayer::ControlHardware(bool horn, bool head_light, bool right_light, bool left_light){
  SendMessageControlHardware( horn,  head_light,  right_light,  left_light);
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
* @brief service <-> message id / channel mapping
* @details Mapping so that can channel and can id can be changed according to environment variables later
* @param
* @return void
* @exception
*/
void DataRelayer::SetmsgMap(int svcid,int msgid,string device){
  magMap_.insert(make_pair(svcid,msgid));
  channelMap_.insert(make_pair(svcid,device));
}

/**
* @brief send API(ControlSteering)
* @details
* @param steering_angle_cmd steering angle
* @return void
* @exception
*/
void DataRelayer::SendMessageControlSteering(float steering_angle_cmd){

  if ( steering_angle_cmd > MAX_STEERING || steering_angle_cmd < MIN_STEERING ){
    return;
  }

  AD_Control_Steering dat_1;
  memset(&dat_1,0x00,CAN_MAX_DLEN);
  dat_1.ad_steering_msgcntr = 15; // Heartbeat --> new can message
  dat_1.ad_steering_angle_cmd = (steering_angle_cmd + OFFSET_STEERING) * RESOLUTION_STEERING_CTRL;
  dat_1.ad_steering_valid = 1;
  canlib_->PostCanMessage<AD_Control_Steering>(dat_1,AD_CONTROL_ACCELERATE,device_type[CAN1]);
};

/**
* @brief send API(ControlAccelerate)
* @details
* @param vel  Accelerate speeed
* @return void
* @exception
*/
void DataRelayer::SendMessageControlAccelerate(float vel){

  unsigned char gear;
  if ( vel > 0 ) {
    gear = FORWARD;
  } else if ( vel < 0 ) {
    gear = REVERSE;
  } else {
    gear = PARKING;
    //gear = NEUTRAL;
  }

  //HeartBeat();  --> delete reserve
  AD_Control_Accelerate dat_1;
  memset(&dat_1,0x00,CAN_MAX_DLEN);
  dat_1.ad_accelerate_gear = gear; // (vel >0) = 1, (vel = 0) = 2, (vel < 0) = 3
  dat_1.ad_accelerate_valid = 1; // 1
  dat_1.ad_accelerate_work_mode = 1; 
  dat_1.ad_acc_de =0 ; // Increase or decrease in acceleration --> new can message
  dat_1.ad_accelerate_msgcntr = 15; // Heartbeat --> new can message 
  dat_1.ad_speed_control = [](float v){return v * CNV_SPEED_FACTOR * RESOLUTION_SPEED_CTRL;}(std::fabs(vel)) ;
 // std::cout<<"@@@@@@@ : "<<gear<<' '<<dat_1.iecu_speed_control<<'\n';
  dat_1.ad_torque_control = 0;
  canlib_->PostCanMessage<AD_Control_Accelerate>(dat_1,AD_CONTROL_ACCELERATE,device_type[CAN1]);
/*
  AD_Control_Brake dat_2;
  memset(&dat_2,0x00,CAN_MAX_DLEN);
  dat_2.iecu_brakepressure_cmd = 100; //origin 100
  (std::fabs(vel)<0.001)?  dat_2.iecu_dbs_valid = 1: dat_2.iecu_dbs_valid = 0;
  //dat_2.iecu_dbs_valid = 1;
  canlib_->PostCanMessage<AD_Control_Brake>(dat_2,AD_Control_Brake,device_type[CAN1]);
  */
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
void DataRelayer::SendMessageControlHardware(bool Horn,bool HeadLight,bool Right_Turn_Light, bool Left_Turn_Light){
  AD_Control_Body  dat_1;
  memset(&dat_1,0x00,CAN_MAX_DLEN);
  dat_1.ad_headlight = HeadLight?1:0;
  dat_1.ad_horn_control = Horn?1:0;
  dat_1.ad_left_turn_light = Left_Turn_Light?1:0;
  dat_1.ad_right_turn_light = Right_Turn_Light?1:0;
  dat_1.ad_brake_light=0; // break led --> new can message 
  dat_1.ad_body_msgcntr=15; // Heartbeat --> new can message 
  canlib_->PostCanMessage<AD_Control_Body >(dat_1,AD_CONTROL_BODY ,device_type[CAN1]);
};
// 필요에 따라 추가 한다.외부 인터페이스 API 정의 필요

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::Handler_VCU_EPS_Control_Request (VCU_EPS_Control_Request msg){
  //   unsigned char  bydata[8];
  //   memcpy(bydata,&msg,8);
  //   for(unsigned char d : bydata){
  //     fprintf(stdout,"0x%02x ",d);
  //  }
  //  cout << endl;
   // Motorola  LSB
  short vcu_eps_strangle = htons(msg.vcu_eps_strangle_req);//new change

  double strangle_value = (vcu_eps_strangle / RESOLUTION_STEERING_CTRL ) - OFFSET_STRANGLE ;

 // cout << "[recv] VCU_EPS_Control_Request : " << vcu_eps_strangle <<"("<<strangle_value<<"),"<< (int)msg.vcu_eps_ctrlenable <<","<< (int)msg.vcu_eps_ctrlmode << endl;

}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::Handler_Remote_Control_Shake (Remote_Control_Shake msg){
 // cout << "[recv] Remote_Control_Shake : " << (int)msg.remote_y1_longitudinal_control <<","<<(int)msg.remote_x2_lateral_control << endl;
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::Handler_Remote_Control_IO (Remote_Control_IO msg){
  cout << "[recv] Remote_Control_IO : " << (int)msg.remote_f_horn
    << "," << (int)msg.remote_d_headlight
    << "," << (int)msg.remote_b_motor_holding_brake
    << "," << (int)msg.remote_a_mode_switch
    << "," << (int)msg.remote_c_speed_torque
    << "," << (int)msg.remote_e_gear << endl;
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
// !!!!!!!!!!prediction
void DataRelayer::Handler_DBS_Status (DBS_Status2 msg){
 // cout << "[recv] DBS_Status : " << (int)msg.dbs_fault_code <<","<<(int)msg.dbs_hp_pressure <<","<<(int)msg.dbs_system_status << endl;

  faultCallback(CAN_NO_FAULT,msg.dbs_fault_code);
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::Handler_VCU_DBS_Request (VCU_DBS_Request msg){
//  cout << "[recv] VCU_DBS_Request : " << (int)msg.vcu_dbs_pressure_request <<","<<(int)msg.vcu_dbs_request_flag << endl;
}

/**
* @brief Callback function to register with CanAdaptor::
* @details Register a callback function using CanAdaptor::SetHandler.
* @param msg receive data
* @return void
* @exception
*/
void DataRelayer::Handler_MCU_Torque_Feedback (MCU_Torque_Feedback msg){
  /*
  cout << "[recv] MCU_Torque_Feedback : " << (int)msg.mcu_current
    <<","<<(int)msg.mcu_errorcode
    <<","<<(int)msg.mcu_motortemp
    <<","<<(int)msg.mcu_shift
    <<","<<(int)msg.mcu_speed
    <<","<<(int)msg.mcu_torque << endl;
*/
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
void DataRelayer::Run(){

  canlib_ = CanAdaptor::getInstance();
  canlib_->Initialize(system_endian_);

  // 수신 핸들러 등록
  canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_VCU_EPS_Control_Request,VCU_EPS_CONTROL_REQUEST,device_type[CAN1]);
  // canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_Remote_Control_Shake,REMOTE_CONTROL_SHAKE_2,device_type[CAN1]);
  // canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_Remote_Control_IO,REMOTE_CONTROL_IO,device_type[CAN1]);
  //std::cout << "can_test1"<<'\n';

  canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_DBS_Status,DBS_STATUS2,device_type[CAN1]); // changun 
    //std::cout << "can_test2"<< '\n';
  // canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_VCU_DBS_Request,VCU_DBS_REQUEST,device_type[CAN1]);1
  canlib_->SetHandler<DataRelayer>(this,&DataRelayer::Handler_MCU_Torque_Feedback,TORQUE_FEEDBACK,device_type[CAN0]);
  //std::cout << "can_test3"<< '\n';
  // 수신 리스너 오픈
  vector<string> device;
  device.push_back(device_type[CAN0]);
  //changun 0->1 230427
  device.push_back(device_type[CAN1]);

  int ret = 0;
   

  while(canlib_->Open(device) != 0 ){
    cout << "open fail" << endl;
    sleep(CAN_ALIVE_CHECKTIME);      
  }
    //changun 1->0 230427
  while(canlib_->RunControlFlag(1,device_type[CAN1]) != 0 ){
    cout << "run config flag fail" << endl;
    sleep(CAN_ALIVE_CHECKTIME);
  }
  //포트 오픈 체크 스레드
  cout << "Start checking for can channel fault" << endl;
  canlib_->CheckSocketStatus(device,faultCallback);    
}

void DataRelayer::StopPostMessage(unsigned int id){
  canlib_->StopPostMessage(id);
}

/**
 * @brief Use when you need a lower HeartBeat
 * @details Must be passed together when controlling the robot
 * @author changunAn(changun516@wavem.net)
 */
void DataRelayer::HeartBeat(){
  AD_Control_Flag dat_5;
  memset(&dat_5,0x00,8);
  dat_5.ad_control_request_flag = 1;
  dat_5.ad_flag_msgcntr = 15; // Heartbeat --> new can message 
	//while(true){
    std::cout << "***can run heartbeat!!!***" << std::endl;
    canlib_->PostCanMessage<AD_Control_Flag>(dat_5,AD_CONTROL_FLAG,device_type[CAN1]);
	//	sleep(1);
	//}
}

/**
* @brief data send test
* @details
* @param
* @return void
* @exception
*/
void DataRelayer::SendTest(){

    // AD_Control_Flag dat_5;
    // memset(&dat_5,0x00,8);
    // dat_5.mode_control_request_flag = 1;
    // canlib->PostCanMessage<AD_Control_Flag>(dat_5,AD_Control_Flag,device_type[CAN1]);

  // 전송 테스트
  AD_Control_Body  dat_1;
  memset(&dat_1,0x00,CAN_MAX_DLEN);
  dat_1.ad_headlight = 1;
  dat_1.ad_horn_control = 1;
  dat_1.ad_left_turn_light = 1;
  dat_1.ad_right_turn_light = 1;
  dat_1.ad_brake_light=0; // break led --> new can message 
  dat_1.ad_body_msgcntr=15; // Heartbeat --> new can message 
  canlib_->PostCanMessage<AD_Control_Body >(dat_1,AD_CONTROL_BODY ,device_type[CAN1]);

    // AD_CONTROL_ACCELERATE dat_2;
    // memset(&dat_2,0x00,8);
    // dat_2.iecu_accelerate_gear = 1;
    // dat_2.iecu_accelerate_valid = 1;
    // dat_2.iecu_accelerate_work_mode = 1;
    // dat_2.iecu_speed_control = 1;
    // dat_2.iecu_torque_control = 1;
    // canlib->PostCanMessage<AD_CONTROL_ACCELERATE>(dat_2,AD_CONTROL_ACCELERATE,device_type[CAN1]);

    // AD_Control_Brake dat_3;
    // memset(&dat_3,0x00,8);
    // dat_3.iecu_brakepressure_cmd = 1;
    // dat_3.iecu_dbs_valid = 1;
    // canlib->PostCanMessage<AD_Control_Brake>(dat_3,AD_Control_Brake,device_type[CAN1]);

    // AD_Control_Steering dat_4;
    // memset(&dat_4,0x00,8);
    // dat_4.iecu_steering_angle_cmd = 1;
    // dat_4.iecu_steering_valid = 1;
    // canlib->PostCanMessage<AD_Control_Steering>(dat_4,AD_Control_Steering,device_type[CAN1]);


   /*
    //canlib->print_map_state("data_relayer");
    // memcpy(bydata,&dat_2,8);
    // for(unsigned char d : bydata){
    //   fprintf(stdout,"0x%02x ",d);
    // }
    // cout << endl;
  */
}

    void DataRelayer::static_break(UGV::BREAK break_status){
        AD_Control_Brake dat_2;
        memset(&dat_2,0x00,CAN_MAX_DLEN);
        dat_2.ad_dbs_valid = 1;
        dat_2.ad_dbs_msgcntr = 15; // Heartbeat --> new can message  
        if(UGV::BREAK::LED==break_status){
          dat_2.ad_brakepressure_cmd = 15;
        }
        else if(UGV::BREAK::GO==break_status){
          dat_2.ad_dbs_valid = 0;
          dat_2.ad_brakepressure_cmd = 0;
        }
        else if(UGV::BREAK::STOP==break_status){
          dat_2.ad_brakepressure_cmd = 100; //origin 100
        }   
        //dat_2.iecu_dbs_valid = 1;
        canlib_->PostCanMessage<AD_Control_Brake>(dat_2,AD_CONTROL_BRAKE,device_type[CAN1]);
    }


