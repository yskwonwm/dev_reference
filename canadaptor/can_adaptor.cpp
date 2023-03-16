#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <string>

#include <sys/ioctl.h>
#include <sys/socket.h>

// #include <linux/can.h>
// #include <linux/can/raw.h>

#include "include/can_define.hpp"
#include "can_adaptor.hpp"
#include "can_dump.hpp"
#include "can_send.hpp"

CanAdaptor* CanAdaptor::instance = NULL;

/**
* @brief initialization task
* @details
* @param endian system's endian type
* @return result of processing, 0 if successful
* @exception
*/
int CanAdaptor::initialize(bool endian){
    isBigEndian = endian;

    ptr_can_dump = std::make_shared<CanDump>();
    ptr_can_send = std::make_shared<CanSend>();

    cout <<  "1) initialize"<< endl;
    return 0;
};

/**
* @brief release task
* @details
* @param
* @return void
* @exception
*/
void CanAdaptor::release(){
	//socketclose();
    if ( ptr_can_send != NULL ){
      ptr_can_send->socketclose();
    }
};

/**
* @brief Open a can channel.
* @details
* @param
* @return  Result of processing, 0 if successful
* @warning After registering all callback functions, call them.
* @exception
*/
int CanAdaptor::open(vector<string> device){
    if ( s_open(device) == 0 &&  r_open(device) == 0 ){
        return 0;
    }
    return -1;
}

/**
* @brief Open a can channel for send.
* @details
* @param device Channel name to open
* @return  Result of processing, 0 if successful
* @warning After registering all callback functions, call them.
* @exception
*/
int CanAdaptor::s_open(vector<string> device){
   
   cout << "2) send socket open" << endl;
   if ( ptr_can_send == NULL ){
        cerr << "invalid can send object" << endl;
        return -1;
   }
   if ( ptr_can_send->socketopen(device) < 0 ){
        return -1;
   }
   return 0;
}

/**
* @brief Open a can channel for reception.
* @details
* @param  device Channel name to open
* @return  Result of processing, 0 if successful
* @warning After registering all callback functions, call them.
* @exception
*/
int CanAdaptor::r_open(vector<string> device){
   
   int count = 0;
   map<string,string> parametermap;

   cout << "3) receive chnnel open" << endl;
   cout << "Number of messages waiting to be received : " << funcsmap.size() << endl;

   if ( funcsmap.size() <0 ){
     cerr << "not found messages waiting to be received"<< endl;
     return -1;
   }
   //map에서 channel,msgid,func을 조회->2번째 파라메터에 넣는다.
   for (auto iter = funcsmap.begin(); iter !=  funcsmap.end(); iter++){
     //cout << "iter->first : " <<  iter->first<< endl;
     CanCallbackFunc* obj = (CanCallbackFunc*)iter->second.get();
     string ch = obj->getChannel();
     cout << "<registered message>  channel ("<<obj->getChannel() << "), canid ("<< obj->getCanid() << ")" << endl;

     if (parametermap.count(ch)){
       auto val = parametermap.find(ch);
       string param = val->second; // chnnel open 파라메터
       param.append(",").append(std::to_string(obj->getCanid())).append(":").append(ONLY_SFF);
       //cout << "param2 : " << param << endl;
       val->second=param;
       //auto  ret = parametermap.insert(make_pair(ch,param));
     }else{
       string param;
       param.append(ch).append(",").append(std::to_string(obj->getCanid())).append(":").append(ONLY_SFF);
	   //cout << "param1 : " << param << endl;
       parametermap.insert(make_pair(ch,param));
     }
   }

   vector<string> argval;
   int idx = 0;
   for (auto iter = parametermap.begin(); iter !=  parametermap.end(); iter++){
     //cout << idx << " : "<< "iter->second.c_str() : " << iter->second.c_str() << ", map size : "<< parametermap.size() << endl;
     argval.push_back(iter->second);
     idx++;
   }
// thread 없이 호출
// can_dump->open(parametermap.size(), arg,this,&CanAdaptor::receive);
// thread 방법
   try{
     thread recvthread([&](int argc, vector<string> argv,CanAdaptor* pClassType,void(CanAdaptor::*func)(unsigned char* data,int canid)) {
	 					       this->canopen(argc,argv,pClassType,func);
                         }, parametermap.size(),argval,this,&CanAdaptor::receive);
     recvthread.detach();
    //recvthread.join();
    } catch (std::exception& e) {
       cerr << "Exception : " << e.what();
    } catch (...) {
       cerr << "default Exception : " << endl;
    }

 // async 방법
    // int (CanAdaptor::*pFunc)(int, char **,CanAdaptor*,void(CanAdaptor::*func)(unsigned char* data,int canid)) = &CanAdaptor::canopen;
    // function<int(int, char **,CanAdaptor*,void(CanAdaptor::*func)(unsigned char* data,int canid))> func_canopen
    //        = move(bind(pFunc, this, placeholders::_1,placeholders::_2,placeholders::_3,placeholders::_4));
    //  auto ret = std::async(std::launch::async, func_canopen,parametermap.size(), arg,this,&CanAdaptor::receive);
    //int n = ret.get();

    cout << "<Start detecting receive data>\n" << endl;

    return 0;
};

/**
* @brief Function to open can channel
* @details
* @param
* @return  Result of processing, 0 if successful
* @warning 
* @exception
*/
int CanAdaptor::canopen(int argc, vector<string> arg,CanAdaptor* pClassType,void(CanAdaptor::*func)(unsigned char* data,int canid)){
    if ( ptr_can_dump == NULL ){
        return -1;
    }

    ptr_can_dump->open(argc, arg, this, func);

    return 0;
}

/**
* @brief Check whether the can channel is activated.
* @details
* @param device Channel name to check
* @param callbackfunc Function pointer to be called when a fault occurs
* @return  
* @warning 
* @exception
*/
void CanAdaptor::checkSocketStatus(vector<string> device,std::function<void(int,int)> callbackfunc){

    int (CanAdaptor::*pFunc1)(vector<string>) = &CanAdaptor::s_open;
    function<int(vector<string>)> openfunc = move(bind(pFunc1, this, placeholders::_1));
    //cout << "thread check socket OK!!! : " << endl;        
    std::thread ([&](vector<string> dev,std::function<void(int,int)> func) {
          
        while (true){

          sleep(CAN_ALIVE_CHECKTIME);
          
          bool isConn = true;  
          for (vector<string>::iterator iter = dev.begin(); iter != dev.end(); ++iter){
            if ( isConnected((char*)iter->c_str()) == false ){
		          cerr << "[ERR]Socket check result :  invalid can device - " << iter->c_str() << endl;        
              // call function (can device fault) 
              func(CAN_DEVICE_FAULT,0x00);
              release();             
              isConn = false; 

              cout << "try reopen socket : " << endl;        
              if ( s_open(dev) == 0 ){
                // call function (can device normal)
                func(CAN_NO_FAULT,0x00);                
              }                         
              break;
           }              
          }           
          // if ( isConn ){
          //   cout << "send socket OK!!! : " << endl;        
          // }
        };
     },device,callbackfunc).detach();
}

/**
* @brief Receive data from can network
* @details Search and call a function mapped with canid in the function map.
* @param data received data body
* @param canid received data can id
* @return void
* @exception
*/
void CanAdaptor::receive(byte* data,int canid) {

    //function map에서 canid에 해당하는 callback을 조회하여 호출한다.
    auto value =funcsmap.find(canid);
    //cout << " map key  : "<< value->first << endl;
    std::shared_ptr<CanCallbackFunc> object = value->second;
    //cout << " count  : "<< object.use_count() << "," << object << endl;
    CanCallbackFunc* lpCls = (CanCallbackFunc*)value->second.get();
    cout << "[recv] canid :"<< lpCls->getCanid() << ", channel :" << lpCls->getChannel() << endl;
    std::function<void(byte*)> func = lpCls->getHandler();
    //data 파싱 및 등록된 callback 함수를 호출한다.
    //std::future<void> ret = std::async(std::launch::async, func,data);
    func(data);
};

/**
* @brief  iECU mode, need to remote control
* @details see can dbc file , In case of success, the next command is valid and processed synchronously.
* @param flag Mode_Control_Request_Flag ( 0, 1, 2)
* @param device Channel name to command
* @return Result of processing, 0 if successful, -1 otherwise
* @exception
*/
int CanAdaptor::runControlFlag(int flag, string device ){
    
    Mode_Control_Flag data;
    memset(&data,0x00,CAN_MAX_DLEN);
    data.mode_control_request_flag = (unsigned char)flag;
    //postCanMessage<Mode_Control_Flag>(data,MODE_CONTROL_FLAG,device_type[CAN1]);
    //for synchronously
    byte temp[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(temp,(void*)&data,CAN_MAX_DLEN);
    vector<byte> body;
    for (byte value : temp){
         body.emplace_back(value);
    }

    int (CanAdaptor::*pFunc)(vector<byte>, unsigned int, string) = &CanAdaptor::send;
    function<void(vector<byte>, unsigned int, string)> postMessagefunc = move(bind(pFunc, this, placeholders::_1, placeholders::_2, placeholders::_3));

    int ret = send(body, MODE_CONTROL_FLAG, (char*)device.c_str());   

    return ret;
}

/**
* @brief Each data type is transmitted through the CAN network.
* @details
* @param body transmission body
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(byte* data, unsigned int canid, string device ){
//printf("postMessageByType device ( %s )\r\n", device.c_str()) ;
    if ( isConnected(device) == false ){
      cerr << "[ERR]The socket is invalid and does not transmit." << endl;
        return;     
    }
  
    byte temp[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(temp,(void*)data,CAN_MAX_DLEN);
    vector<byte> body;
    for (byte value : temp){
         body.emplace_back(value);
    }

    int (CanAdaptor::*pFunc)(vector<byte>, unsigned int, string) = &CanAdaptor::send;
    function<void(vector<byte>, unsigned int, string)> postMessagefunc = move(bind(pFunc, this, placeholders::_1, placeholders::_2, placeholders::_3));

    //auto result = std::async(std::launch::async, postMessagefunc,body, canid, (char*)device.c_str());
    thread sendthread(postMessagefunc, body, canid, device);
    sendthread.detach();
    //sendthread.join();
};

/**
* @brief Each data type is transmitted through the CAN network.
* @details
* @param body transmission body
* @param canid can id
* @param device can channel
* @param duration can sandingduration (milliseconds)
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(byte* data, unsigned int canid, string device,int duration){

    byte temp[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(temp,(void*)data,CAN_MAX_DLEN);
    vector<byte> body;
    for (byte value : temp){
         body.emplace_back(value);
    }

    int (CanAdaptor::*pFunc)(vector<byte>, unsigned int, string) = &CanAdaptor::send;
    function<void(vector<byte>, unsigned int, string)> postMessagefunc = move(bind(pFunc, this, placeholders::_1, placeholders::_2, placeholders::_3));

     std::thread([postMessagefunc, duration,body, canid, device]() {
        while (true){
            //auto start = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::steady_clock::now() + std::chrono::milliseconds(duration);
            postMessagefunc(body, canid, (char*)device.c_str());

            if ( duration <= 0 ){
              return; // 반복 없음
            }

            std::this_thread::sleep_until(ms);
            //std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            // auto finish = std::chrono::high_resolution_clock::now();
            // auto int_s = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
            // std::cout << "elapsed time is " << int_s.count() << " ms )" << std::endl;
        }
    }).detach();
};

/**
* @brief iECU_Control_Hardware type is transmitted through the CAN network.
* @details
* @param body iECU_Control_Hardware data
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(iECU_Control_Hardware data,int msgid,string device){
    cout <<  "send iECU_Control_Hardware msg "<< endl;
    byte body[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(body,(void*)&data,CAN_MAX_DLEN);
    postMessageByType(body, msgid, device );
    //send( body, canid, (char*)device.c_str());
};

/**
* @brief iECU_Control_Accelerate type is transmitted through the CAN network.
* @details
* @param body iECU_Control_Hardware data
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(iECU_Control_Accelerate data,int msgid,string device){
    cout <<  "send iECU_Control_Accelerate msg "<< endl;
    byte body[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(body,(void*)&data,CAN_MAX_DLEN);
    postMessageByType(body, msgid, device );
};

/**
* @brief iECU_Control_Brake type is transmitted through the CAN network.
* @details
* @param body iECU_Control_Hardware data
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(iECU_Control_Brake data,int msgid,string device){
    cout <<  "send iECU_Control_Brake msg "<< endl;
    byte body[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(body,(void*)&data,CAN_MAX_DLEN);
    postMessageByType(body, msgid, device );
};

/**
* @brief iECU_Control_Steering type is transmitted through the CAN network.
* @details
* @param body iECU_Control_Hardware data
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(iECU_Control_Steering data,int msgid,string device){
     cout <<  "send iECU_Control_Steering msg "<< endl;
     byte body[CAN_MAX_DLEN];
     //byte* body = makeframebody(temp,data);
     memcpy(body,(void*)&data,CAN_MAX_DLEN);
    postMessageByType(body, msgid, device );
};

/**
* @brief Mode_Control_Flag type is transmitted through the CAN network.
* @details
* @param body iECU_Control_Hardware data
* @param canid can id
* @param device can channel
* @return  void
* @warning
* @exception
*/
void CanAdaptor::postMessageByType(Mode_Control_Flag data,int msgid,string device){
     cout <<  "send Mode_Control_Flag msg : "<< device << endl;
     byte body[CAN_MAX_DLEN];
     //byte* body = makeframebody(temp,data);
     memcpy(body,(void*)&data,CAN_MAX_DLEN);
     postMessageByType(body, msgid, device );
};

/**
* @brief Convert structure data to 8-byte byte array
* @details
* @param body transmission body
* @param data command data
* @return  unsigned char* byte array
* @warning
* @exception
*/
byte * CanAdaptor::makeframebody(byte* body,iECU_Control_Hardware data){

    memcpy(body,(void*)&data,CAN_MAX_DLEN);
    // data를 body로 변환
    return body;
};


/**
* @brief Send data to the CAN network.
* @details
* @param body transmission body
* @param msgid can id
* @param device channel
* @return  int
* @warning
* @exception
*/
int CanAdaptor::send(vector<byte> data, unsigned int msgid, string device ){
    //cout <<  "send device : "<< device << endl;

    if( ptr_can_send->send(data,msgid,(char*)device.c_str()) != 0 ){
        return -1;
    }
    return 0;    
}

/**
* @brief Check can device connection status
* @details
* @param device channel
* @return  true if successful, false otherwise
* @warning
* @exception
*/
bool CanAdaptor::isConnected(string device){

    if ( ptr_can_send == NULL ){
        cerr << "[ERR]invalid can send object" << endl;
        return false;
    }

    if ( ptr_can_send->isConnected((char*)device.c_str()) == false){
		  cerr << "[ERR]Not currently connected to CAN network" << endl;        
		  return false;
    } 

    return true;
}

void CanAdaptor::print_map_state(string name){

    if ( funcsmap.size() <= 0 ){
	    cout << "("<< name << ")funcsmap.size() is  "<< funcsmap.size() << endl;
	    return;
    }

    for (auto iter = funcsmap.begin(); iter !=  funcsmap.end(); iter++)	{
  	  cout << "("<< name << ")iter->first : " <<  iter->first<< endl;
		  cout << "("<< name << ")iter->second : "<< iter->second << endl;
	//iter->first;
		  CanCallbackFunc* obj = (CanCallbackFunc*)iter->second.get();
		  cout << "("<< name << ")iter->second channel : "<<obj->getChannel() << ", canid : "<< obj->getCanid() << endl;
    }
}

