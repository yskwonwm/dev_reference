#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <chrono>

#include "calc_distance.hpp"
#include "geo_trans.hpp"

using namespace std::chrono;
using std::cout;
using std::endl;


int main(int argc, char* argv[]) {
    // Testing with one value
    //struct Result res = trans(40.99698, 46.0, 9.20127, 10.0);
    
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
    CalcDistance calc;

    if ( argc == 5 ){ 
      P1_latitude = atof(argv[1]);
      P1_longitude = atof(argv[2]);
      P2_latitude = atof(argv[3]);
      P2_longitude = atof(argv[4]);
    }else if (argc != 1 ){       	    
        fprintf(stderr, "\nUsage: %s latitude1 longitude1 latitude2 longitude2\n", argv[0]);
        exit(-1);
    }else{
        printf("\nUsage: %s latitude1 longitude1 latitude2 longitude2\n", argv[0]);
    }    

    printf("\n*** Distance calculation result by  Vincenty's formula /  Haversine formula / Law of Cosines / Korea TM ***\n");  
    printf("latitude1 : %f, longitude1 : %f , latitude2 : %f, longitude2 :  %f\n\n",P1_latitude,P1_longitude,P2_latitude,P2_longitude);  

    auto start = std::chrono::high_resolution_clock::now();
    //steady_clock::time_point startTime = steady_clock::now(); // 물리적 고정 시간
    system_clock::time_point startTime = system_clock::now();// 컴퓨터 시스템 시간
    
    for ( int i = 0; i < 1000 ; i++){
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,VINCENTY);     
    }        

    //steady_clock::time_point endTime = steady_clock::now();
    system_clock::time_point endTime = system_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    
    auto currentTime = endTime - startTime;    
    auto int_s = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);                   
    printf("[Vincenty] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    printf("run time : %ld ms , %ld ns \n\n", int_s.count(),currentTime.count()) ;

    start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,HAVERSINE);    
    }    
    finish = std::chrono::high_resolution_clock::now();
    int_s = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);                   
    printf("[Haversine] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    printf("run time : %ld ms\n\n", int_s.count());

    start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,LAWCOSINES);    
    }    
    finish = std::chrono::high_resolution_clock::now();
    int_s = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);                   
    printf("[Law of Cosines] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    printf("run time : %ld ms\n\n", int_s.count());

    start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000 ; i++){    
        result = calc.getDistance(P1_latitude, P1_longitude, P2_latitude, P2_longitude,KTM);    
    }
    finish = std::chrono::high_resolution_clock::now();
    int_s = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);                   
    printf("[TM] x,y dist - %f ,  %f ,  %f \n", result.distance_x,result.distance_y,result.distance);  
    printf("run time : %ld ms\n\n", int_s.count());

    return 0;
}
