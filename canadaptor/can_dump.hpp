#ifndef CAN_DUMP_H
#define CAN_DUMP_H


#include <iostream>
#include <vector>

#define TIMESTAMPSZ 50 /* string 'absolute with date' requires max 49 bytes */

#define MAXCN 2    /* max. number of CAN channel */
#define MAXSOCK 16    /* max. number of CAN interfaces given on the cmdline */
#define MAXIFNAMES 30 /* size of receive name index to omit ioctls */
#define ANYDEV "any"  /* name of interface to receive from any CAN interface */
#define ANL "\r\n"    /* newline in ASC mode */

#define MAXANI 4

#define DEVICE_EXCEPTION -1


class CanAdaptor;

/**
    @class   if_info
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   collection group information per open socket
    @version 0.0.1
    @warning 
*/
struct if_info { /* bundled information per open socket */
	int s; /* socket */
	char *cmdlinename;
	__u32 dropcnt;
	__u32 last_dropcnt;
};

/**
    @class   CanDump
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   Can network data listening class
    @version 0.0.1
    @warning 
*/
class CanDump {

  private:	
    struct if_info sock_info[MAXSOCK];
    int fd_epoll;
    int currmax;

    char *progname;
    char devname[MAXIFNAMES][IFNAMSIZ+1];
    int dindex[MAXIFNAMES];
    int max_devname_len; /* to prevent frazzled device name output */
  
    //static const int canfd_on = 1;

  public:    
    int  Open(int argc, std::vector<std::string> argval,CanAdaptor* pClassType,void(CanAdaptor::*func)(unsigned char* data,int canid));
    int  idx2dindex(int ifidx, int socket);
    void SocketClose();
};

 #endif