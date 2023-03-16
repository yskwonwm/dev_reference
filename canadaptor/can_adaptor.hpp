#ifndef CANADAPTOR_H
#define CANADAPTOR_H

#include <functional>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <cstring>
#include <vector>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "include/w1candbc.h"
#include "cancallbackfunc.hpp"

//typedef unsigned char* byte;
#define byte unsigned char
//#define ONLY_SFF "C00007FF"
#define ONLY_SFF "7FF"
#define CAN_ALIVE_CHECKTIME 2 // second
#define CAN_NO_FAULT 0x00 
#define CAN_DEVICE_FAULT 0x01 


class CanDump;
class CanSend;

/**
    @class   CanAdaptor
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   CAN network relaying class
    @version 0.0.1
    @warning 
*/
class CanAdaptor {
 
  public:
    
    CanAdaptor(){
       instance = NULL;
    } 
    virtual ~CanAdaptor(){this->release();};

  private:  
  
    map<int, std::shared_ptr<CanCallbackFunc>> funcsmap;
        
    static CanAdaptor* instance;

    typedef std::function<void(VCU_EPS_Control_Request)> func_VCU_EPS_Control_Request;
    typedef std::function<void(Remote_Control_Shake)> func_Remote_Control_Shake;
    typedef std::function<void(Remote_Control_IO)> func_Remote_Control_IO;
    typedef std::function<void(DBS_Status)> func_DBS_Status;
    typedef std::function<void(VCU_DBS_Request)> func_VCU_DBS_Request;
    typedef std::function<void(MCU_Torque_Feedback)> func_MCU_Torque_Feedback;

    func_VCU_EPS_Control_Request handler_vcr;
    func_Remote_Control_Shake handler_rcs;
    func_Remote_Control_IO handler_rci;
    func_DBS_Status handler_ds;
    func_VCU_DBS_Request handler_vdr;
    func_MCU_Torque_Feedback handler_mtf;

    bool isBigEndian = 0;

    std::shared_ptr<CanDump> ptr_can_dump = NULL;
    std::shared_ptr<CanSend> ptr_can_send = NULL;
    
//    map<string, int> sockmap;  //< device-socket    
    //int s; /* can raw socket */     
    //string send_devive; //< 전송용 채널

  private:
    //int socketopen(char* device );
    //void socketclose();
    int  send(vector<byte>  body, unsigned int msgid, string device ); //< can network 연동, cansend.c 참조
    void receive(byte* data,int canid);
    int  canopen(int arc,vector<string> argv,CanAdaptor*,void(CanAdaptor::*func)(unsigned char* data,int canid));
       
    void print_map_state(string name);

    byte* makeframebody(byte* body,iECU_Control_Hardware data); //< strunct를 byte형 body로 변환
    // can network 데이터 전송 함수  - 전송 데이터 타입 별로 생성......

    void postMessageByType(iECU_Control_Hardware body,int msgid,string device);
    void postMessageByType(iECU_Control_Accelerate body,int msgid,string device);
    void postMessageByType(iECU_Control_Brake body,int msgid,string device);
    void postMessageByType(iECU_Control_Steering body,int msgid,string device);
    void postMessageByType(Mode_Control_Flag body,int msgid,string device);
    void postMessageByType(byte* body, unsigned int canid, string device );
    void postMessageByType(byte* data, unsigned int canid, string device,int duration );
    
    int s_open(vector<string> device); 
    int r_open(vector<string> device); 
    

  public:   
    int  initialize(bool endian); //< 초기화
    void release(); //< 종료
    int  open(vector<string> device); //< open can channel, warning : callback function을 전부 등록후 호출한다.
    int  runControlFlag(int flag, string device);
    bool isConnected(string device);           
    void checkSocketStatus(vector<string> device,std::function<void(int,int)> func);
   /**
    * @brief Returns a singleton object.
    * @details 
    * @param 
    * @return CanAdaptor* singleton object.
    * @exception
    */
    static CanAdaptor* getInstance(){
      if ( instance == NULL ){
         std::shared_ptr<CanAdaptor> object = std::make_shared<CanAdaptor>();
         instance=object.get();        
      }    
      return instance;
    };

    
    /**
    * @brief Register a callback function.(VCU_EPS_Control_Request)
    * @details Register a callback function that receives VCU_EPS_Control_Request as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(VCU_EPS_Control_Request),int canid,string device){
      //void CanAdaptor::setHandler(T *pClassType,void(T::*pfunc)(VCU_EPS_Control_Request,std::string msgid,CHANNEL_TYPE type)){  
      //setHandler(bind(pfunc, pClassType, placeholders::_1));    
      handler_vcr = move(bind(pfunc, pClassType, placeholders::_1));
      //int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                    // data를 VCU_EPS_Control_Request에 맞춰서 넣는다.
                    VCU_EPS_Control_Request r;                                        
                    memcpy((void*)&r,data,CAN_MAX_DLEN);
                    //cout<< "call VCU_EPS_Control_Request" << endl;
                    handler_vcr((VCU_EPS_Control_Request)r);
                    //cout<< "end handler_vcr" << endl;
                }
                );                
     cout << "setHandler(VCU_EPS_Control_Request) : " + device << ", canid : "<< canid << endl;          
     funcsmap.insert(make_pair(canid,object));     
//     print_map_state("VCU_EPS_Control_Request");
   };

    /**
    * @brief Register a callback function.(Remote_Control_IO)
    * @details Register a callback function that receives Remote_Control_IO as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(Remote_Control_IO),int canid,string device){        
      handler_rci = move(bind(pfunc, pClassType, placeholders::_1));
      //int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                  // data를 Remote_Control_IO 맞춰서 넣는다.
                  Remote_Control_IO r;
                  memcpy((void*)&r,data,CAN_MAX_DLEN);
                  //this->handler_h(r);
                  //cout<< "call Remote_Control_IO" << endl;
                  handler_rci((Remote_Control_IO)r);
                  //cout<< "end handler_rci" << endl;
                }
                );
      
       cout << "setHandler(Remote_Control_IO) : " + device << ", canid : "<< canid << endl;          
       funcsmap.insert(make_pair(canid,object));           
  //    print_map_state("Remote_Control_IO");
   };

   /**
    * @brief Register a callback function.(Remote_Control_Shake_2)
    * @details Register a callback function that receives Remote_Control_Shake_2 as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(Remote_Control_Shake),int canid,string device){        
      handler_rcs = move(bind(pfunc, pClassType, placeholders::_1));
      //int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                  // data를 Remote_Control_Shake_2 맞춰서 넣는다.
                  Remote_Control_Shake r;
                  memcpy((void*)&r,data,CAN_MAX_DLEN);
                  //this->handler_h(r);
                  //cout<< "call Remote_Control_Shake_2" << endl;
                  handler_rcs((Remote_Control_Shake)r);
                  //cout<< "end handler_rcs" << endl;
                }
                );
      
       cout << "setHandler(Remote_Control_Shake_2) : " + device << ", canid : "<< canid << endl;          
       funcsmap.insert(make_pair(canid,object));           
//      print_map_state("Remote_Control_Shake_2");
   };

   /**
    * @brief Register a callback function.(DBS_Status)
    * @details Register a callback function that receives DBS_Status as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(DBS_Status),int canid,string device){        
      handler_ds = move(bind(pfunc, pClassType, placeholders::_1));
      //int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                  // data를 DBS_Status 맞춰서 넣는다.
                  DBS_Status r;
                  memcpy((void*)&r,data,CAN_MAX_DLEN);
                  //this->handler_h(r);
                  //cout<< "call DBS_Status" << endl;
                  handler_ds((DBS_Status)r);
                 // cout<< "end DBS_Status" << endl;
                }
                );
      
       cout << "setHandler(DBS_Status) : " + device << ", canid : "<< canid << endl;          
       funcsmap.insert(make_pair(canid,object));           
//    print_map_state("DBS_Status");
   };

   /**
    * @brief Register a callback function.(VCU_DBS_Request)
    * @details Register a callback function that receives VCU_DBS_Request as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(VCU_DBS_Request),int canid,string device){        
      handler_vdr = move(bind(pfunc, pClassType, placeholders::_1));
      //int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                  // data를 VCU_DBS_Request 맞춰서 넣는다.
                  VCU_DBS_Request r;
                  memcpy((void*)&r,data,CAN_MAX_DLEN);
                  //this->handler_h(r);
                  //cout<< "call VCU_DBS_Request" << endl;
                  handler_vdr((VCU_DBS_Request)r);
                  //cout<< "end handler_vdr" << endl;
                }
                );
      
       cout << "setHandler(VCU_DBS_Request) : " + device << ", canid : "<< canid << endl;          
       funcsmap.insert(make_pair(canid,object));           
//      print_map_state("VCU_DBS_Request");
   };
   
   
  /**
    * @brief Register a callback function.(MCU_Torque_Feedback)
    * @details Register a callback function that receives MCU_Torque_Feedback as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @exception
    */
    template<typename T>
    void setHandler(T *pClassType,void(T::*pfunc)(MCU_Torque_Feedback),int canid,string device){        
      handler_mtf = move(bind(pfunc, pClassType, placeholders::_1));
     // int canid = string2hex(msgid);
      
      std::shared_ptr<CanCallbackFunc> object = std::make_shared<CanCallbackFunc>(
                 canid
                ,device
                ,[&](byte* data) { 
                  // data를 MCU_Torque_Feedback 맞춰서 넣는다.
                  MCU_Torque_Feedback r;
                  memcpy((void*)&r,data,CAN_MAX_DLEN);
                  //this->handler_h(r);
                  //cout<< "call MCU_Torque_Feedback" << endl;
                  handler_mtf((MCU_Torque_Feedback)r);
                  //cout<< "end handler_mtf" << endl;
                }
                );
      
       cout << "setHandler(MCU_Torque_Feedback) : " + device << ", canid : "<< canid << endl;          
       funcsmap.insert(make_pair(canid,object));           
//      print_map_state("MCU_Torque_Feedback");
   };

    /**
    * @brief Register a callback function.
    * @details Register a callback function that receives Remote_Control_IO as a parameter.
    * @param *pClassType an channel to open
    * @param T::*pfunc callback function
    * @param  msgid  MSG ID (same value as can id)
    * @param  device can channel
    * @return void
    * @warning structType parameter must be assigned a value after initialization.
    * @exception
    */
    template<typename T>
    void postCanMessage(T structTypeData,int msgid,string device){ 
      //cout <<  "post msg "<< endl;
      string msg("[send]<");
      msg.append(std::to_string(msgid)).append("> ").append(typeid(structTypeData).name()).append(" : ").append(device);
      cout << msg << endl;
      //byte* body = makeframebody(temp,data);
      //1) 타입별로 별고 처리가 필요하지 않은 경우 아래 사용
      byte body[CAN_MAX_DLEN];	         
	    memcpy(body,(void*)&structTypeData,CAN_MAX_DLEN);                
      postMessageByType(body,msgid,device);      
      //2) 중간에 타입별로 처리가 필요한 경우 아래 사용
      //postMessageByType(structType,msgid,device);      
   };

    template<typename T>
    void postCanMessage(T structTypeData,int msgid,string device,int duration){ 
      //cout <<  "post msg "<< endl;
      string msg("[send]<");
      msg.append(std::to_string(msgid)).append("> ").append(typeid(structTypeData).name());
      cout << msg << endl;
      //byte* body = makeframebody(temp,data);
      //1) 타입별로 별고 처리가 필요하지 않은 경우 아래 사용
      byte body[CAN_MAX_DLEN];	         
	    memcpy(body,(void*)&structTypeData,CAN_MAX_DLEN);                
      postMessageByType(body,msgid,device,duration);      

      //2) 중간에 타입별로 처리가 필요한 경우 아래 사용
      //postMessageByType(structType,msgid,device);      
   };
    
  private:
  //   inline unsigned int string2hex(string msgid){
  //      unsigned int hexValue;
  //      std::stringstream conv(msgid);
  //      conv >> std::hex >> hexValue;	
  //      return hexValue;
  // }
};

#endif //FUNCTIONCALLBACK_CHILD_H
