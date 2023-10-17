#ifndef CANDBC_H
#define CANDBC_H

#pragma pack(push, 1)

struct AD_Control_Body {
	unsigned char ad_left_turn_light:1;
	unsigned char ad_right_turn_light:1;
	unsigned char ad_horn_control:1;
	unsigned char ad_headlight:1;
	unsigned char ad_body_msgcntr:4;
	unsigned char ad_brake_light : 1;
	unsigned char reserved1 : 7;
	unsigned char reserved2[6];
};

struct AD_Control_Accelerate {
	unsigned char ad_accelerate_valid:4;
	unsigned char ad_accelerate_msgcntr : 4;
	unsigned char reserved1;
	unsigned char ad_accelerate_work_mode;
	unsigned char ad_accelerate_gear;
	unsigned char ad_acc_de;
	unsigned char ad_torque_control;
	unsigned short ad_speed_control;
};


struct AD_Control_Brake {
	unsigned char ad_dbs_valid:4;
	unsigned char ad_dbs_msgcntr:4;
	unsigned char ad_brakepressure_cmd;
	unsigned char reserved[6];
};

struct AD_Control_Steering {
	unsigned char ad_steering_valid:4;
	unsigned char ad_steering_msgcntr:4;
	unsigned short reserved1;
	unsigned short ad_steering_angle_cmd;
	unsigned char reserved[3];
};

struct AD_Control_Flag {
	unsigned char ad_control_request_flag:4;
	unsigned char ad_flag_msgcntr : 4;
	unsigned char reserved[7];
};

struct VCU_EPS_Control_Request{
	unsigned char vcu_eps_ctrlenable:1; // 1bit
	unsigned char vcu_eps_ctrlmode:1; //1bit
	unsigned char reserved2:6; // 6bit
	unsigned char reserved3; // 1Byte
	unsigned short vcu_eps_strangle_req;
	unsigned char reserved4[4];
};


struct Remote_Control_Shake{
  unsigned short remote_x2_lateral_control;
  unsigned short reserved1;
  unsigned short remote_y1_longitudinal_control;
  unsigned short reserved2;
};

struct Remote_Control_IO{
  unsigned char remote_e_gear;
  unsigned char remote_f_horn;
  unsigned char remote_a_mode_switch;
  unsigned char remote_b_motor_holding_brake;
  unsigned char remote_c_speed_torque;
  unsigned char remote_d_headlight;
  unsigned char reserved[2];
};

struct DBS_Status{
	unsigned char dbs_system_status : 2;
	unsigned char reserved1 : 4;
	unsigned char dbs_park_warning:2;
	unsigned char dbs_work_mode;
	unsigned char breakpressurereqack;
	unsigned char dbs_hp_pressure;
	unsigned char dbs_peadalopening;
	unsigned char dbs_req_iq;
	unsigned char dbs_rollingcounter :4;
	unsigned char reserved2 : 2;
	unsigned char dbs_estopflag : 1;
	unsigned char dbs_pedaiflag : 1;
	unsigned char dbs_checksum;
};
struct DBS_Status2 {
	unsigned long long dbs_fault_code:24;
	unsigned long long dbs_waring_code : 24;
	unsigned long long dbs_rollingcounter : 4;
	unsigned long long reserved : 4;
	unsigned long long dbs_checksum : 8;
};

struct VCU_DBS_Request {
	unsigned char vcu_dbs_request_flag;
	unsigned char vcu_dbs_work_mode;
	unsigned char vcu_dbs_pressure_request;
	unsigned char vcu_abs_active;
	unsigned char reserved[4];
};

struct VCU_MCU_Request{
  unsigned char mcu_vcu_motor_request_valid:1;
  unsigned char mcu_drivemode:2;
  unsigned char mcu_clamping_brake_req:1;
  unsigned char reserved:4;
  unsigned short mcu_torque_req;
  unsigned short mcu_speed_req;
  unsigned char reserved2[3];
};

struct MCU_Torque_Feedback{
  unsigned long mcu_shift:2;
  unsigned long mcu_speed:18;
  unsigned long mcu_torque:16;
  unsigned long mcu_current:12;
  unsigned char mcu_motortemp;
  unsigned char mcu_errorcode;
};

struct BMS_System_Info {
	unsigned short bms_battery_out_voltage;// Signal Name: BMS_Battery_Out_Voltage 2byte 16 bite start 0 
	unsigned short bms_battery_out_current;  // Signal Name: BMS_Battery_Out_Current 2byte 16 bite start 2
	unsigned char bms_sys_soc;// Signal Name: BMS_Sys_SOC 1byte 8bite start 4
	unsigned char bms_battery_max_temperature;// Signal Name: BMS_Battery_Max_Temperature start 5
	unsigned short bms_sys_status:3; // Signal Name: BMS_Sys_Status 2byte
	unsigned short bms_charge_status:2; // Signal Name: BMS_Charge_Status 2byte
	unsigned short bms_precharge_signal_status:2;// Signal Name: BMS_Precharge_Signal_Status 2byte
	unsigned short reserved:9;// Signal Name: BMS_Precharge_Signal_Status 2byte
};

#pragma pack(pop)

#endif
/*
struct VCU_DBS_Request{
unsigned long reserved1:17;
unsigned long vcu_dbs_request_flag:1;
unsigned long reserved2:22;
unsigned long vcu_dbs_pressure_request:16;
unsigned long reserved3:8;
};*/
