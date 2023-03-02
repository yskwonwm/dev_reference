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

   //map에서 channel,msgid,func을 조회->2번째 파라메터에 넣는다.
   int count = 0;
   map<string,string> parametermap;
   
   cout << "2) send socket open" << endl;   
   if ( ptr_can_send == NULL ){
        cerr << "invalid can send object" << endl;   
        return -1;
   }
   if ( ptr_can_send->socketopen(device) < 0 ){
        return -1;
   }
   
//    for (vector<string>::iterator iter = device.begin(); iter != device.end(); ++iter){
//      if ( socketopen((char*)iter->c_str()) != 0 ){
// 		cout << "socke open fail" << endl;
// 		return -1;
//      }   
//    }
   
   cout << "3) receive chnnel open" << endl;
   cout << "Number of messages waiting to be received : " << funcsmap.size() << endl;

   if ( funcsmap.size() <0 ){
     cerr << "not found messages waiting to be received"<< endl;
     return -1;
   }

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

int CanAdaptor::canopen(int argc, vector<string> arg,CanAdaptor* pClassType,void(CanAdaptor::*func)(unsigned char* data,int canid)){
    if ( ptr_can_dump == NULL ){
        return -1;
    }

    ptr_can_dump->open(argc, arg, this, func); 
    
    return 0;
}

// int CanAdaptor::socketopen(char* device )
// {
//     int ret;
//     int s; /* can raw socket */ 
//     int required_mtu = CAN_MTU;
//     int mtu;
//     int enable_canfd = 1;
//     struct sockaddr_can addr;
//     struct canfd_frame frame;	
//     struct ifreq ifr;

//     auto item = sockmap.find(device);

//     if (item != sockmap.end()) {
//         printf("device exists! -  %s ; %d", item->first.c_str(),item->second) ;
//         return 0;
//     } 
	
//     /* open socket */
//     if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
// 	perror("socket");
// 	return 1;
//     }

//     strcpy(ifr.ifr_name, device);
//     ret = ioctl(s, SIOCGIFINDEX, &ifr);
//     if (ret < 0) {
//         perror("ioctl interface index failed!");
//         return 1;
//     }

//     ifr.ifr_name[IFNAMSIZ - 1] = '\0';
//     ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
//     if (!ifr.ifr_ifindex) {
// 	perror("if_nametoindex");
// 	return 1;
//     }

//     memset(&addr, 0, sizeof(addr));
//     addr.can_family = AF_CAN;
//     addr.can_ifindex = ifr.ifr_ifindex;

//     /* disable default receive filter on this RAW socket */
//     /* This is obsolete as we do not read from the socket at all, but for */
//     /* this reason we can remove the receive list in the Kernel to save a */
//     /* little (really a very little!) CPU usage.                          */
//     setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

//     if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
// 	  perror("bind");
// 	  return 1;
//     }

//     sockmap.insert(make_pair(device,s));

//     cout << "socke device open (" << device << "," << s << " )"<<endl;
//     return 0;
// }

// void CanAdaptor::socketclose(){

//   for (auto iter = sockmap.begin(); iter !=  sockmap.end(); iter++){
//      close(iter->second);
//      printf("socket close(%s,%d)\n",iter->first.c_str(),iter->second);
//   }	
// }

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

    byte temp[CAN_MAX_DLEN];
    //byte* body = makeframebody(temp,data);
    memcpy(temp,(void*)data,CAN_MAX_DLEN);    
    vector<byte> body; 
    for (byte value : temp){
         body.emplace_back(value);
    }

    int (CanAdaptor::*pFunc)(vector<byte>, unsigned int, char*) = &CanAdaptor::send;
    function<void(vector<byte>, unsigned int, char*)> postMessagefunc = move(bind(pFunc, this, placeholders::_1, placeholders::_2, placeholders::_3));

    //auto result = std::async(std::launch::async, postMessagefunc,body, canid, (char*)device.c_str());
    thread sendthread(postMessagefunc, body, canid, (char*)device.c_str());
    sendthread.detach();
    //sendthread.join();
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
    cout <<  "send iECU_Control_Hardware msg "<< endl;
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
    cout <<  "send iECU_Control_Hardware msg "<< endl;
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
     cout <<  "send iECU_Control_Hardware msg "<< endl;
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
     cout <<  "send iECU_Control_Hardware msg "<< endl;
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
int CanAdaptor::send(vector<byte> data, unsigned int msgid, char* device ){
    
    if( ptr_can_send->send(data,msgid,device) != 0 ){
        return -1;
    }
    return 0;
    // //byte* body = makeframebody(temp,data);
    // byte body[CAN_MAX_DLEN];	
    // int idx = 0;
    // for (auto iter = data.begin(); iter !=  data.end(); iter++)	{
    // 	body[idx++] = *iter;
    // }
	
    // auto item = sockmap.find(device);
    // if (item == sockmap.end()) {
    //     printf("device does not exist! ( %s )\r\n", device) ;
    //     return 1;
    // } 

    // int s = item->second;
    // int required_mtu = CAN_MTU;	
    // struct canfd_frame frame;
	
    // frame.can_id = msgid;
    // frame.len = CAN_MAX_DLEN;
    // memcpy(frame.data,body,CAN_MAX_DLEN);
    
    // printf("  <channel> %s, <can_id> = 0x%X, %d <can_dlc> = %d\r\n",device, frame.can_id,frame.can_id,frame.len);
    
    // /* send frame */
    // if (write(s, &frame, required_mtu) != required_mtu) {
	// perror("write");
	// return 1;
    // }
	
    // //printf("  <data> = ");
    // string msg("  <data> = ");        
    // char buf[10];
    // memset(buf,0x00,10);
    // for(int i = 0; i < CAN_MAX_DLEN; i++){
    //    sprintf(buf,"0x%02x ",frame.data[i]);
    //    msg.append(buf);
    //    msg.append(" ");
    // }
    // msg.append(" <data transfer success>\n");
    // printf("%s",msg.c_str());
    
    // return 0;
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

