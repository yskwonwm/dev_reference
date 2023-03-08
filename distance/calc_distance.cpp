#include "geo_trans.hpp"
#include "calc_distance.hpp"

CalcDistance::CalcDistance(){  
  ptr_geo_trans = std::make_shared<GeoTrans>();
};

/**
* @brief distance calculation function called
* @details 
* @param lat1 start latitude
* @param lon1 start longitude
* @param lat2 finish latitude
* @param lon2 finish longitude
* @param type calculation formula type
* @return resutlt_distance
* @exception
*/
resutlt_distance CalcDistance::getDistance(double lat1, double lon1, double lat2, double lon2,int type ) {
      
      resutlt_distance result;
  
      switch (type){
        case VINCENTY:
          result.distance = distanceInMeterByVincenty(lat1, lon1, lat2, lon2);     
          result.distance_x = distanceInMeterByVincenty(lat2, lon1, lat2, lon2); 
          result.distance_y = distanceInMeterByVincenty(lat1, lon1, lat2, lon1); 
          break;

        case HAVERSINE:
          result.distance = distanceInMeterByHaversine(lat1, lon1, lat2, lon2);     
          result.distance_x = distanceInMeterByHaversine(lat2, lon1, lat2, lon2); 
          result.distance_y = distanceInMeterByHaversine(lat1, lon1, lat2, lon1); 
          break;
		  
        case LAWCOSINES:
          result.distance = distanceInMeterByCosines(lat1, lon1, lat2, lon2);     
          result.distance_x = distanceInMeterByCosines(lat2, lon1, lat2, lon2); 
          result.distance_y = distanceInMeterByCosines(lat1, lon1, lat2, lon1); 
          break;

        case KTM:
          result.distance = distanceInMeterByTM(lat1, lon1, lat2, lon2);     
          result.distance_x = distanceInMeterByTM(lat2, lon1, lat2, lon2); 
          result.distance_y = distanceInMeterByTM(lat1, lon1, lat2, lon1); 
          break;

        default:
          printf("not found parameter type err : %d", type);
          break;  
      }
      return result;
    };

//Spheical Law of Cosines (구면 코사인 법칙)
/**
* @brief distance calculation by Spheical Law of Cosines (구면 코사인 법칙)
* @details 
* @param lat1 start latitude
* @param lon1 start longitude
* @param lat2 finish latitude
* @param lon2 finish longitude
* @return double
* @exception
*/
double CalcDistance::distanceInMeterByCosines(double lat1, double lon1, double lat2, double lon2){  
    
      double theta, dist;  
      theta = lon1 - lon2;  
      dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) 
          + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));  
      dist = acos(dist);  
      dist = rad2deg(dist);  
      
      dist = dist * 60 * 1.1515;   
      dist = dist * 1.609344;    // 단위 mile 에서 km 변환.  
  
      return dist * 1000.0;  // 단위  km 에서 m 로 변환  
    } 

// Vincenty 공식 http://www.movable-type.co.uk/scripts/latlong-vincenty.html
/**
* @brief distance calculation by Vincenty formula
* @details 
* @param lat1 start latitude
* @param lon1 start longitude
* @param lat2 finish latitude
* @param lon2 finish longitude
* @return double
* @exception
*/
double CalcDistance::distanceInMeterByVincenty(double lat1, double lon1, double lat2, double lon2){
		double a = 6378137;  // 장축
		double b = 6356752.314245; // 단출
		double f = 1 / 298.257223563; // 편평률
		double L = deg2rad(lon2 - lon1);
		double U1 = atan((1 - f) * tan(deg2rad(lat1)));
		double U2 = atan((1 - f) * tan(deg2rad(lat2)));
		double sinU1 = sin(U1), cosU1 = cos(U1);
		double sinU2 = sin(U2), cosU2 = cos(U2);
		double cosSqAlpha;
		double sinSigma;
		double cos2SigmaM;
		double cosSigma;
		double sigma;

		double lambda = L, lambdaP, iterLimit = 100;
		do 
		{
			double sinLambda = sin(lambda), cosLambda = cos(lambda);
			sinSigma = sqrt(	(cosU2 * sinLambda)
									* (cosU2 * sinLambda)
									+ (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda)
									* (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda)
								);
			if (sinSigma == 0) 
			{
				return 0;
			}

			cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
			sigma = atan2(sinSigma, cosSigma);
			double sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
			cosSqAlpha = 1 - sinAlpha * sinAlpha;
			cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;

			double C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));
			lambdaP = lambda;
			lambda = 	L + (1 - C) * f * sinAlpha	
						* 	(sigma + C * sinSigma	
								* 	(cos2SigmaM + C * cosSigma
										* 	(-1 + 2 * cos2SigmaM * cos2SigmaM)
									)
							);
		
		} while (abs(lambda - lambdaP) > 1e-12 && --iterLimit > 0);

		if (iterLimit == 0) 
		{
			return 0;
		}

		double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
		double A = 1 + uSq / 16384
				* (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
		double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
		double deltaSigma = 
					B * sinSigma
						* (cos2SigmaM + B / 4
							* (cosSigma 
								* (-1 + 2 * cos2SigmaM * cos2SigmaM) - B / 6 * cos2SigmaM
									* (-3 + 4 * sinSigma * sinSigma)
										* (-3 + 4 * cos2SigmaM * cos2SigmaM)));
		
		double s = b * A * (sigma - deltaSigma);
		
		return s;
	}

/**
* @brief distance calculation by Haversine formula
* @details 
* @param lat1 start latitude
* @param lon1 start longitude
* @param lat2 finish latitude
* @param lon2 finish longitude
* @return double
* @exception
*/
double CalcDistance::distanceInMeterByHaversine(double lat1, double lon1, double lat2, double lon2) {
      
    double distance;
    double radius = 6371; // 평균 지구 반지름(km)
    double toRadian = M_PI / 180.0;

    double deltaLatitude = abs(lat1 - lat2) * toRadian;
    double deltaLongitude = abs(lon1 - lon2) * toRadian;

    double sinDeltaLat = sin(deltaLatitude / 2);
    double sinDeltaLng = sin(deltaLongitude / 2);
    double squareRoot = sqrt(
    sinDeltaLat * sinDeltaLat +
        cos(lat1 * toRadian) * cos(lat2 * toRadian) * sinDeltaLng * sinDeltaLng);
    
    distance = 2 * radius * asin(squareRoot);
    
    return distance * 1000.0;
}

// 주어진 도(degree) 값을 라디언으로 변환      
/**
* @brief Convert a given degree value to radians
* @details 
* @param deg degree
* @return double
* @exception
*/
double CalcDistance::deg2rad(double deg){  
      return (double)(deg * M_PI / (double)180.0);  
}  
  
// 주어진 라디언(radian) 값을 도(degree) 값으로 변환  
/**
* @brief Convert a given radian value to a degree value
* @details 
* @param deg degree
* @return double
* @exception
*/
double CalcDistance::rad2deg(double rad){  
      return (double)(rad * (double)180 / M_PI);  
} 

/**
* @brief distance calculation by Korea TM 
* @details 
* @param lat1 start latitude
* @param lon1 start longitude
* @param lat2 finish latitude
* @param lon2 finish longitude
* @return double
* @exception
*/
double CalcDistance::distanceInMeterByTM(double lat1, double lon1, double lat2, double lon2){

 	  double result = ptr_geo_trans->getDistancebyTm( lat1,  lon1,  lat2,  lon2);
    
      return result;
}
