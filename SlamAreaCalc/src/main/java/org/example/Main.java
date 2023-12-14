package org.example;

import java.awt.*;
import java.io.*;
import java.util.Scanner;
import java.util.Map;
import org.yaml.snakeyaml.Yaml;


public class Main {
    public static void main(String[] args) {


        Point testPos1 = new Point(0,0);
        Point testPos2 = new Point(1238,765);
        Point testPos3 = new Point(415,235);
        Point testPos4 = new Point(574,235);
        Point testPos5 = new Point(1210,235);
        Point testPos6 = new Point(1210,541);

        Integer slam_map_width = 1238;
        Integer slam_map_height = 765;
        Double intersection_start_point_lon = 128.858009083;
        Double intersection_start_point_lat = 35.157430158;
        Double intersection_end_point_lon = 128.858870603;
        Double intersection_end_point_lat = 35.158056682;
        Integer std_point_slamX1 = 415;
        Integer std_point_slamY1 =  235;
        Integer std_point_slamX2 = 1210;
        Integer std_point_slamY2 = 541;
        Double std_point_lon1 = 128.8579836;
        Double std_point_lat1 = 35.1576298;
        Double std_point_lon2 = 128.858333;
        Double std_point_lat2 =  35.15818;

        double range_from_x =  0;
        double range_to_x =  0;
        double range_from_y =  0;
        double range_to_y =  0;
        double degree4th =  0;
        double degree3rd =  0;
        double degree2nd =  0;
        double degree1st =  0;
        double y_intercept =  0;
        double std_height =  0;

        Yaml yaml = new Yaml();
        try (InputStream inputStream = new FileInputStream("map_coordinates.yaml")) {
            Map<String, Object> obj  = yaml.load(inputStream);

            Map<String, Object> obj2 = (Map<String, Object>)obj.get("/gps_slam_converter");
            Map<String, Object> obj3 = ( Map<String, Object> )obj2.get("ros__parameters");

            slam_map_width = (Integer) obj3.get("slam_map_width");
            slam_map_height = (Integer) obj3.get("slam_map_height");
            intersection_start_point_lon = (Double) obj3.get("intersection_start_point_lon");
            intersection_start_point_lat = (Double) obj3.get("intersection_start_point_lat");
            intersection_end_point_lon = (Double) obj3.get("intersection_end_point_lon");
            intersection_end_point_lat = (Double) obj3.get("intersection_end_point_lat");
            std_point_slamX1 = (Integer) obj3.get("std_point_slamX1");
            std_point_slamY1 =  (Integer) obj3.get("std_point_slamY1");
            std_point_slamX2 = (Integer) obj3.get("std_point_slamX2");
            std_point_slamY2 = (Integer) obj3.get("std_point_slamY2");
            std_point_lon1 = (Double) obj3.get("std_point_lon1");
            std_point_lat1 = (Double) obj3.get("std_point_lat1");
            std_point_lon2 = (Double) obj3.get("std_point_lon2");
            std_point_lat2 =  (Double) obj3.get("std_point_lat2");

            range_from_x =  (Double) obj3.get("range_from_x");
            range_to_x =  (Double) obj3.get("range_to_x");
            range_from_y =  (Double) obj3.get("range_from_y");
            range_to_y =  (Double) obj3.get("range_to_y");
            degree4th =  (Double) obj3.get("degree4th");
            degree3rd =  (Double) obj3.get("degree3rd");
            degree2nd =  (Double) obj3.get("degree2nd");
            degree1st =  (Double) obj3.get("degree1st");
            y_intercept =  (Double) obj3.get("y_intercept");
            std_height =  (Double) obj3.get("std_height");


        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        double x = 10.0;
        double y = 6.13;
        double displacement = 0;
        // gps2slam 은 변환 값에서 displacement 더해줌, slam2gps는 변환 값에서 displacement 빼준다.
        if ( PositionConvertor.isCorrectionRage(x,y,range_from_x,range_to_x,range_from_y,range_to_y) ) {
            displacement = PositionConvertor.getCorrectYPosition(x, std_height, y_intercept, degree4th, degree3rd, degree2nd, degree1st);
        }

        System.out.println("correctYPosition : " + displacement);

/*        Scanner sc = new Scanner(System.in);
        System.out.println("input parameter -s sllamx SlamY -g longitude latitude");
        String cmd = sc.next();*/


        int shift = 0;
        PositionConvertor.initialize(
                (int)(std_point_slamX1-shift), std_point_slamY1
                ,(int)(std_point_slamX2-shift), std_point_slamY2
                ,(int)(slam_map_width-shift), slam_map_height
                ,new PositionConvertor.Point(std_point_lon1,std_point_lat1)
                ,new PositionConvertor.Point(std_point_lon2,std_point_lat2)
                ,new PositionConvertor.Point(intersection_start_point_lon,intersection_start_point_lat)
                ,new PositionConvertor.Point(intersection_end_point_lon,intersection_end_point_lat));

        System.out.println("[1]" );
        PositionConvertor.Point gpsposTest = PositionConvertor.ConvertSlam2GPS(testPos1.x,testPos1.y);
        PositionConvertor.Point slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //PositionConvertor.Point gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("[2]" );
        gpsposTest = PositionConvertor.ConvertSlam2GPS((int)(testPos2.x-shift),testPos2.y);
        slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("[3]" );
        gpsposTest = PositionConvertor.ConvertSlam2GPS((int)(testPos3.x-shift),testPos3.y);
        slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("[4]" );
        gpsposTest = PositionConvertor.ConvertSlam2GPS((int)(testPos4.x-shift),testPos4.y);
        slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("[5]" );
        gpsposTest = PositionConvertor.ConvertSlam2GPS((int)(testPos5.x-shift),testPos5.y);
        slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("[6]" );
        gpsposTest = PositionConvertor.ConvertSlam2GPS((int)(testPos6.x-shift),testPos6.y);
        slamposTest = PositionConvertor.ConvertGPS2Slam(gpsposTest.x,gpsposTest.y);
        //gpsposTest2 = PositionConvertor.ConvertSlam2GPS((int)slamposTest.x, (int)slamposTest.y);
        System.out.println("GPS :  " + gpsposTest.y+", "+gpsposTest.x);
        System.out.println("SLAM:  " + Math.round(slamposTest.y*100)/100.0 +", "+Math.round(slamposTest.x*100)/100.0);

        System.out.println("Coordinate system conversion complete!");
    }
}

