#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "calc_distance.hpp"
#include "geo_trans.hpp"

void getduration(clock_t start){

    clock_t finish = clock();
    double duration = (double)(finish - start);
    printf("run time : %f ms\n", duration);
}

int main(int argc, char* argv[]) {
    // Testing with one value
    //struct Result res = trans(40.99698, 46.0, 9.20127, 10.0);
    clock_t start, finish;
    double duration;
    double dist,dist_x1,dist_x2,dist_y;

    // double P1_latitude = 37.00;
    // double P1_longitude = 126.00;
    // double P2_latitude = 38.00;
    // double P2_longitude = 127.00;

    // double P1_latitude = 37.57972629472382;
    // double P1_longitude = 126.97703995428492;
    // double P2_latitude = 37.158853203095845;
    // double P2_longitude = 126.16041116961833; 

    double P1_latitude = 36.570175;
    double P1_longitude = 129.9771472;
    double P2_latitude = 37.473472;
    double P2_longitude = 130.6163861;

    resutlt_distance  result; 
    CalcDiatance calc;

    if ( argc == 5 ){ 
      P1_latitude = atof(argv[1]);
      P1_longitude = atof(argv[2]);
      P2_latitude = atof(argv[3]);
      P2_longitude = atof(argv[4]);
    }else if (argc != 1 ){       	    
        fprintf(stderr, "\nUsage: %s latitude1 longitude1 latitude2 longitude2\n", argv[0]);
        exit(-1);
    }    

    printf("\n*** Distance calculation result by  Vincenty's formula /  Haversine formula / Law of Cosines / Korea TM ***\n");  
    printf("latitude1 : %f, longitude1 : %f , latitude2 : %f, longitude2 :  %f\n\n",P1_latitude,P1_longitude,P2_latitude,P2_longitude);  

    start = clock();
    for ( int i = 0; i < 1000 ; i++){
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,VINCENTY);     
    }        
    getduration(start);
    printf("[Vincenty] x,y dist - %f ,  %f ,  %f \n\n", result.distance_x,result.distance_y,result.distance);  

    start = clock();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,HAVERSINE);    
    }
    getduration(start);
    printf("[Haversine] x,y dist - %f ,  %f ,  %f \n\n", result.distance_x,result.distance_y,result.distance);  

    start = clock();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,LAWCOSINES);    
    }    
    getduration(start);
    printf("[Law of Cosines] x,y dist - %f ,  %f ,  %f \n\n", result.distance_x,result.distance_y,result.distance);  

    start = clock();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,KTM);    
    }
    getduration(start);
    printf("[TM] x,y dist - %f ,  %f ,  %f \n\n", result.distance_x,result.distance_y,result.distance);  

    return 0;
}
