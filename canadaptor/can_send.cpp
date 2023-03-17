#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <cstring>
#include <net/if.h>

#include "can_send.hpp"


using namespace std;
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
int CanSend::Send(std::vector<unsigned char> data, unsigned int msgid, char* device ){
    
    struct ifreq ifr;
    //byte* body = makeframebody(temp,data);
    unsigned char body[CAN_MAX_DLEN];	
    int idx = 0;
    for (auto iter = data.begin(); iter !=  data.end(); iter++)	{
    	body[idx++] = *iter;
    }
	
    auto item = sockmap.find(device);
    if (item == sockmap.end()) {
        printf("device does not exist! ( %s )\r\n", device) ;
        return -1;
    } 

    int s = item->second;
    int required_mtu = CAN_MTU;	
    struct canfd_frame frame;
	
    frame.can_id = msgid;
    frame.len = CAN_MAX_DLEN;
    memcpy(frame.data,body,CAN_MAX_DLEN);
    
    printf("  <channel> %s, <can_id> = 0x%X, %d <can_dlc> = %d\r\n",device, frame.can_id,frame.can_id,frame.len);
    
    /* send frame */
    if (write(s, &frame, required_mtu) != required_mtu) {
	    perror("write");
	    return -1;
    }
	
    //printf("  <data> = ");
    string msg("  <data> = ");        
    char buf[10];
    memset(buf,0x00,10);
    for(int i = 0; i < CAN_MAX_DLEN; i++){
       sprintf(buf,"0x%02x ",frame.data[i]);
       msg.append(buf);
       msg.append(" ");
    }
    msg.append(" <data transfer success>\n");
    printf("%s",msg.c_str());
    
    return 0;
}

/**
* @brief open the socket
* @details 
* @param device channel names
* @return 
* @warning
* @exception
*/
int CanSend::SocketOpen(std::vector<std::string> device){

  for (vector<string>::iterator iter = device.begin(); iter != device.end(); ++iter){
    if ( SocketOpen((char*)iter->c_str()) != 0 ){
		  perror("can device open fail");        
		  return -1;
    }   
  }
  return 0;
}

/**
* @brief open the socket
* @details 
* @param device channel name
* @return 
* @warning
* @exception
*/
int CanSend::SocketOpen(char* device )
{    
    int ret;
    int s; /* can raw socket */ 
    int required_mtu = CAN_MTU;
    int mtu;
    int enable_canfd = 1;
    struct sockaddr_can addr;
    struct canfd_frame frame;	
    struct ifreq ifr;

    auto item = sockmap.find(device);

    if (item != sockmap.end()) {
        printf("device exists! -  %s ; %d", item->first.c_str(),item->second) ;
        return -1;
    } 

    /* open socket */
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
	    perror("socket");
	    return -1;
    }

    strcpy(ifr.ifr_name, device);
    ret = ioctl(s, SIOCGIFINDEX, &ifr);
    if (ret < 0) {
      perror("ioctl interface index failed!");
      return -1;
    }

    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) {
	    perror("if_nametoindex");
	    return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* disable default receive filter on this RAW socket */
    /* This is obsolete as we do not read from the socket at all, but for */
    /* this reason we can remove the receive list in the Kernel to save a */
    /* little (really a very little!) CPU usage.                          */
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	    perror("bind");
	    return -1;
    }

    sockmap.insert(make_pair(device,s));

    printf("can device open ( %s ) , sockfd : %d\n",device,s );
    
    return 0;
}

/**
* @brief close the socket
* @details close the socketof all channels stored in the this object
* @param 
* @return 
* @warning
* @exception
*/
void CanSend::SocketClose(){

  for (auto iter = sockmap.begin(); iter !=  sockmap.end(); iter++){
     close(iter->second);
     
     printf("socket close(%s,%d)\n",iter->first.c_str(),iter->second);     
  }	
  sockmap.clear();
}

/**
* @brief Check can device connection status
* @details
* @param device channel name
* @return  true if successful, false otherwise
* @warning
* @exception
*/
bool CanSend::IsConnected(char* device){
    
  auto item = sockmap.find(device);
  if (item == sockmap.end()) {
    printf("device does not exist! ( %s )\r\n", device) ;
    return false;
  }     
  
  int s = item->second;
 
  struct ifreq ifr;   
  memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
  strcpy(ifr.ifr_name,device);
  ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
  if (!ifr.ifr_ifindex) {
     perror("if_nametoindex");
     return false;
  }

  if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
    perror("SIOCGIFINDEX");
    return false;
  }

  if (ioctl(s, SIOCGIFMTU, &ifr) < 0){
    perror("SIOCGIFMTU");
    return false;
  }

  return true;
}

/**
* @brief Check can device connection status
* @details Check the sockets of all channels stored in the this object
* @param device channel name
* @return  true if all were successful, false otherwise
* @warning
* @exception
*/
bool CanSend::IsConnect(){
    
  for (auto iter = sockmap.begin(); iter !=  sockmap.end(); iter++){
    if ( IsConnected((char*)iter->first.c_str()) == false ){
      return false;
    }
  }
  return true;
}
