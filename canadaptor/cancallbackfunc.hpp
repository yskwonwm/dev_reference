 
#include <functional>

/**    
    @file    cancallbackfunc.h
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   
*/

using namespace std;

/**
    @class   CanCallbackFunc
    @date    2023-02-14
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   Callback function storage class mapped with Can Id
    @version 0.0.1
    @warning 
*/
typedef function<void(unsigned char*)> func_callback;

class CanCallbackFunc {    
  
  private:  

    int canid_; ///can id
    string channel_; /// Channel used
    func_callback handler_; /// call back function    

  public:
 
    CanCallbackFunc(int id,string ch,func_callback func){
       handler_ = move(func);
       canid_ = id;
       channel_ = ch;
    };
    
    void setHandler(func_callback func) {
        handler_ = move(func);
    };
    
    func_callback getHandler(){
      return handler_;
    }
    
    int getCanid(){
      return canid_;
    }
    
    string getChannel(){
      return channel_;
    }
};
