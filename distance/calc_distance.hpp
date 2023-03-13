#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory>

struct resutlt_distance{
  double distance;
  double distance_x;
  double distance_y;
  double degree;
};

enum {VINCENTY,HAVERSINE,LAWCOSINES,KTM};

class GeoTrans;

/**
    @class   CalcDistance
    @date    2023-03-02
    @author  ys.kwon(ys.kwon@wavem.net)
    @brief   longitude and Latitude coordinate distance calculation class
    @version 0.0.1
    @warning 
*/
class CalcDistance {
  private :
    std::shared_ptr<GeoTrans> ptr_geo_trans = NULL;

    double deg2rad(double deg);
    double rad2deg(double rad);

  public:
    CalcDistance();
    virtual ~CalcDistance(){};

    resutlt_distance getDistance(double lat1, double lon1, double lat2, double lon2,int type );
    double distanceInMeterByCosines(double lat1, double lon1, double lat2, double lon2);    
    double distanceInMeterByVincenty(double lat1, double lon1, double lat2, double lon2);
    double distanceInMeterByHaversine(double x1, double y1, double x2, double y2);
    double distanceInMeterByTM(double lat1, double lon1, double lat2, double lon2);
    double getBearingBetweenPoints(double lat1, double lon1, double lat2, double lon2);
};

#endif
