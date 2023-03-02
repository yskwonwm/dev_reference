#ifndef DISTANCE_H
#define DISTANCE_H


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "geo_trans.hpp"


struct resutlt_distance{
  double distance;
  double distance_x;
  double distance_y;
};

enum {VINCENTY,HAVERSINE,LAWCOSINES,TM};
//enum {GEO,KATEC,TM,GRS80};

// struct GeoPoint {
// 	double x;
// 	double y;
// 	double z;	
// };

class CalcDiatance {
  public:
    resutlt_distance 
    getDistance(double lat1, double lon1, double lat2, double lon2,int type ) {
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

        case TM:
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
    double distanceInMeterByCosines(double lat1, double lon1, double lat2, double lon2){  
    
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
    double distanceInMeterByVincenty(double lat1, double lon1, double lat2, double lon2){
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


    double distanceInMeterByHaversine(double x1, double y1, double x2, double y2) {
      
	  double distance;
      double radius = 6371; // 평균 지구 반지름(km)
      double toRadian = M_PI / 180.0;

      double deltaLatitude = abs(x1 - x2) * toRadian;
      double deltaLongitude = abs(y1 - y2) * toRadian;

      double sinDeltaLat = sin(deltaLatitude / 2);
      double sinDeltaLng = sin(deltaLongitude / 2);
      double squareRoot = sqrt(
        sinDeltaLat * sinDeltaLat +
        cos(x1 * toRadian) * cos(x2 * toRadian) * sinDeltaLng * sinDeltaLng);
    
      distance = 2 * radius * asin(squareRoot);
    
      return distance * 1000.0;
   }

    // 주어진 도(degree) 값을 라디언으로 변환  
    double deg2rad(double deg){  
      return (double)(deg * M_PI / (double)180.0);  
    }  
  
    // 주어진 라디언(radian) 값을 도(degree) 값으로 변환  
    double rad2deg(double rad){  
      return (double)(rad * (double)180 / M_PI);  
    } 


    double distanceInMeterByTM(double lat1, double lon1, double lat2, double lon2){

      //printf("distanceInMeterByTM x : %f , y : %f , x2 : %f , y2 : %f \n" ,lat1,  lon1,  lat2,  lon2);
	  GeoTrans trans;
 	  double result = trans.getDistancebyTm( lat1,  lon1,  lat2,  lon2);

    
      return result;
    }

    // double getDistancebyGeo(GeoPoint pt1, GeoPoint pt2) {
	// 	double lat1 = deg2rad(pt1.y);
	// 	double lon1 = deg2rad(pt1.x);
	// 	double lat2 = deg2rad(pt2.y);
	// 	double lon2 = deg2rad(pt2.x);

	// 	double longitude = lon2 - lon1;
	// 	double latitude = lat2 - lat1;

	// 	double a = pow(sin(latitude / 2.0), 2) + cos(lat1) * cos(lat2) * pow(sin(longitude / 2.0), 2);
	// 	return 6376.5 * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
	// }


// double getDistancebyTm(GeoPoint pt1, GeoPoint pt2) {
// 		pt1 = convert(pt1);
// 		pt2 = convert(pt2);

// 		return getDistancebyGeo(pt1, pt2);
// }


// GeoPoint convert(GeoPoint in_pt) {
// 		GeoPoint tmpPt;
// 		GeoPoint out_pt;

// 		tmpPt.x = deg2rad(in_pt.x);
// 		tmpPt.y = deg2rad(in_pt.y);
// 		geo2tm(tmpPt, out_pt);
		
// 		return out_pt;
// 	}
//  void transform(int srctype, int dsttype, GeoPoint point) {
// 		if (srctype == dsttype)
// 			return;
		
// 		if (srctype != 0 || dsttype != 0) {
// 			// Convert to geocentric coordinates.
// 			geodetic_to_geocentric(srctype, point);
			
// 			// Convert between datums
// 			if (srctype != 0) {
// 				geocentric_to_wgs84(point);
// 			}
			
// 			if (dsttype != 0) {
// 				geocentric_from_wgs84(point);
// 			}
			
// 			// Convert back to geodetic coordinates
// 			geocentric_to_geodetic(dsttype, point);
// 		}
// 	}

// void geo2tm( GeoPoint in_pt, GeoPoint out_pt) {
// 		double x, y;
		
// 		transform(GEO, dsttype, in_pt);
// 		double delta_lon = in_pt.x - m_arLonCenter[dsttype];
// 		double sin_phi = Math.sin(in_pt.y);
// 		double cos_phi = Math.cos(in_pt.y);

// 		if (m_Ind[dsttype] != 0) {
// 			double b = cos_phi * Math.sin(delta_lon);

// 			if ((Math.abs(Math.abs(b) - 1.0)) < EPSLN) {
// 				//Log.d("무한대 에러");
// 				//System.out.println("무한대 에러");
// 			}
// 		} else {
// 			double b = 0;
// 			x = 0.5 * m_arMajor[dsttype] * m_arScaleFactor[dsttype] * Math.log((1.0 + b) / (1.0 - b));
// 			double con = Math.acos(cos_phi * Math.cos(delta_lon) / Math.sqrt(1.0 - b * b));

// 			if (in_pt.y < 0) {
// 				con = con * -1;
// 				y = m_arMajor[dsttype] * m_arScaleFactor[dsttype] * (con - m_arLatCenter[dsttype]);
// 			}
// 		}

// 		double al = cos_phi * delta_lon;
// 		double als = al * al;
// 		double c = m_Esp[dsttype] * cos_phi * cos_phi;
// 		double tq = Math.tan(in_pt.y);
// 		double t = tq * tq;
// 		double con = 1.0 - m_Es[dsttype] * sin_phi * sin_phi;
// 		double n = m_arMajor[dsttype] / Math.sqrt(con);
// 		double ml = m_arMajor[dsttype] * mlfn(e0fn(m_Es[dsttype]), e1fn(m_Es[dsttype]), e2fn(m_Es[dsttype]), e3fn(m_Es[dsttype]), in_pt.y);

// 		out_pt.x = m_arScaleFactor[dsttype] * n * al * (1.0 + als / 6.0 * (1.0 - t + c + als / 20.0 * (5.0 - 18.0 * t + t * t + 72.0 * c - 58.0 * m_Esp[dsttype]))) + m_arFalseEasting[dsttype];
// 		out_pt.y = m_arScaleFactor[dsttype] * (ml - dst_m[dsttype] + n * tq * (als * (0.5 + als / 24.0 * (5.0 - t + 9.0 * c + 4.0 * c * c + als / 30.0 * (61.0 - 58.0 * t + t * t + 600.0 * c - 330.0 * m_Esp[dsttype]))))) + m_arFalseNorthing[dsttype];
// 	}
};

#endif
