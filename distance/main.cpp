#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "calc_distance.hpp"
#include "geo_trans.hpp"

void getduration(clock_t start){

    clock_t finish = clock();
    double duration = (double)(finish - start);
    printf("%f ms\n\n", duration);
}

int main() {
    // Testing with one value
    //struct Result res = trans(40.99698, 46.0, 9.20127, 10.0);
    clock_t start, finish;
    double duration;
    double dist,dist_x1,dist_x2,dist_y;

    double P1_latitude = 37.00;
     double P1_longitude = 126.00;
     double P2_latitude = 38.00;
     double P2_longitude = 127.00;

//     double P1_latitude = 37.57972629472382;
//     double P1_longitude = 126.97703995428492;
//     double P2_latitude = 35.158853203095845;
//     double P2_longitude = 129.16041116961833; 

    //double P1_latitude = 37.570175;
    //double P1_longitude = 126.9771472;
    //double P2_latitude = 37.473472;
    //double P2_longitude = 126.6163861;


    resutlt_distance  result; 
    CalcDiatance calc;

    start = clock();
    for ( int i = 0; i < 1000 ; i++){
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,VINCENTY);     
    }    
    printf("[Vincenty] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    getduration(start);

    start = clock();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,HAVERSINE);    
    }
    printf("[Haversine] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    getduration(start);

    start = clock();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,LAWCOSINES);    
    }
    printf("[Law of Cosines] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    getduration(start);

   start = clock();
   for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,TM);    
    }
    printf("[TM] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    getduration(start);

    return 0;
}
