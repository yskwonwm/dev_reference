#ifndef WM_MOTION_CONTROLLER__ENTITY__DF_UGV_HPP_
#define WM_MOTION_CONTROLLER__ENTITY__DF_UGV_HPP_
#define WHEEL 0.5078
#define UGV_ID "UGV"
#define MAX_SPEED 40
#define GEAR_RATIO 20

namespace UGV{
    enum class BREAK{
        LED=1,
        STOP=100,
        GO=0
    };
}
#endif