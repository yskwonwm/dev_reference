#pragma pack(push, 1)

struct iECU_Control_Hardware{
  unsigned char  iecu_left_turn_light;
  unsigned char  iecu_right_turn_light;
  unsigned char  iecu_headlight;
  unsigned char  iecu_horn_control;
  unsigned char reserved[4]; 
};

struct iECU_Control_Accelerate{
  unsigned char iecu_accelerate_valid;
  unsigned char reserved1;
  unsigned char iecu_accelerate_work_mode;
  unsigned char iecu_accelerate_gear;
  unsigned char reserved2;
  unsigned char iecu_torque_control;
  unsigned short iecu_speed_control; // Resolution 0.1,  
};


struct iECU_Control_Brake{
  unsigned char iecu_brakepressure_cmd;
  unsigned char iecu_dbs_valid;
  unsigned char reserved[6];
};

struct iECU_Control_Steering{
  unsigned char iecu_steering_valid;
  unsigned char reserved1[3];
  unsigned short iecu_steering_angle_cmd;  // Resolution 0.1, offset -30.0
  unsigned char reserved2[2];
};

struct Mode_Control_Flag{
  unsigned char mode_control_request_flag;
  unsigned char reserved[7];	
};

struct VCU_EPS_Control_Request{
  unsigned char vcu_eps_ctrlenable:1;
  unsigned char reserved1:1; 
  unsigned char vcu_eps_ctrlmode:2;
  unsigned char reserved2:4; 
  unsigned short vcu_eps_strangle; // Resolution 0.1, offset -3000.0
  unsigned char reserved3[5]; 
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
  unsigned short dbs_system_status:2;
  unsigned short reserved1:14;
  unsigned short dbs_hp_pressure;// Resolution 0.01
  unsigned char dbs_fault_code;
  unsigned char reserved2[3];
};

struct VCU_DBS_Request{
unsigned short reserved1;
unsigned char reserved:1;
unsigned char vcu_dbs_request_flag:1;
unsigned short reserved2;
unsigned short vcu_dbs_pressure_request;
unsigned char reserved3;
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

#pragma pack(pop)

/*
struct VCU_DBS_Request{
unsigned long reserved1:17;
unsigned long vcu_dbs_request_flag:1;
unsigned long reserved2:22;
unsigned long vcu_dbs_pressure_request:16;
unsigned long reserved3:8;
};*/