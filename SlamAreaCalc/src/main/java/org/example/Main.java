package org.example;

import java.util.ArrayList;

public class Main {
    public static void main(String[] args) {
        //현장 맵 중심 : 128.8582196,35.15794665

        //slam area : 2096,1205
        //slam 중심 : 1048,602

        //좌측 교점:128.8575686,35.15783502
        //상단 교점:128.858427988,35.158463143
        //우측 교점:128.858870603,35.158056682
        //하단 교점:128.858009083,35.157430158

        // 전제 영역
        // LB : 128.8575686,35.157430158
        // RT : 128.858870603,35.158463143

        // LB : 128.8575686001505289,35.1574301543503580


        // 1. 슬램맵을 현장맵과 매핑 테스트
        // 슬램맵 1048 X 602 의 기준점 520,300  ( 원래 현장슬램맵의 사이즈는 2096 X 1205 )
        // 현장 맵 의 기준점 128.858427988,35.158463143
        // 현장 맵의 좌하단~우하단(기울기을 얻기 위함) 128.858009083,35.1574301588 ~ 128.858870603,35.158056682
        // return : 매핑 가상 현장 맵의 우상단~좌상단 , 좌표 매핑시 가상 현장맵의 기준 좌표로 활용 한다.
        ArrayList<PositionConvertor.Point> arr =  PositionConvertor.ConvertSlam2VirtualMapArea(520, 300, 1048, 602, 0.05
                ,new PositionConvertor.Point(128.858427988,35.158463143)
                ,new PositionConvertor.Point(128.858009083,35.157430158)
                ,new PositionConvertor.Point(128.858870603,35.158056682));

        PositionConvertor.initArea(2080,1200
                ,new PositionConvertor.Point(128.858009083,35.157430158)
                ,new PositionConvertor.Point(128.858870603,35.158056682)
                ,new PositionConvertor.Point(arr.get(0).x,arr.get(0).y)
                ,new PositionConvertor.Point(arr.get(1).x,arr.get(1).y));

        // 2. GPS 슬램맵 간 좌표 변환
        // 초기화
        // 슬램맵 크기 2080 X 1200
        // 현장 맵의 좌하단~우하단(기울기을 얻기 위함) 128.858009083,35.1574301588 ~ 128.858870603,35.158056682
        // 현장맵을 포함하는 지도의 사각영역(LB~RT) 128.8575686,35.157430158 ~ 128.858870603,35.158463143
        PositionConvertor.initArea(2080,1200
                ,new PositionConvertor.Point(128.858009083,35.157430158)
                ,new PositionConvertor.Point(128.858870603,35.158056682)
                ,new PositionConvertor.Point(128.8575686,35.157430158)
                ,new PositionConvertor.Point(128.858870603,35.158463143));
    /*  PositionConvertor.Point pos1 = PositionConvertor.ConvertSlamPos(0,0, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos2 = PositionConvertor.ConvertSlamPos(399,0, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos3 = PositionConvertor.ConvertSlamPos(1040,0, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos4 = PositionConvertor.ConvertSlamPos(1176,691, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos5 = PositionConvertor.ConvertSlamPos(1040,600, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos6 = PositionConvertor.ConvertSlamPos(777,1140, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos7 = PositionConvertor.ConvertSlamPos(0,600, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos8 = PositionConvertor.ConvertSlamPos(0,448, PositionConvertor.WorkType.GPS);*/

        // GPS좌표를 SLAM 좌표로 변환
        PositionConvertor.Point slampos1 = PositionConvertor.ConvertGPS2Slam(128.8575686,35.15783502);// 좌상

        PositionConvertor.Point slampos2 = PositionConvertor.ConvertGPS2Slam(128.858427988,35.158463143); //우상

        PositionConvertor.Point slampos3 = PositionConvertor.ConvertGPS2Slam(128.858870603,35.158056682); // 우하

        PositionConvertor.Point slampos4 = PositionConvertor.ConvertGPS2Slam(128.858009083,35.157430158); //좌하

        PositionConvertor.Point slampos5 = PositionConvertor.ConvertGPS2Slam(128.85821960150002,35.1579457443728); //중심


        // SLAM 좌표를 GPS좌표로 변환
        PositionConvertor.Point gpspos1 = PositionConvertor.ConvertSlam2GPS(0,0);
        System.out.println(gpspos1.y+","+gpspos1.x);

        PositionConvertor.Point gpspos2 = PositionConvertor.ConvertSlam2GPS(2080,0);
        System.out.println(gpspos2.y+","+gpspos2.x);

        PositionConvertor.Point gpspos3 = PositionConvertor.ConvertSlam2GPS(2080,1200);
        System.out.println(gpspos3.y+","+gpspos3.x);

        PositionConvertor.Point gpspos4 = PositionConvertor.ConvertSlam2GPS(0,1200);
        System.out.println(gpspos4.y+","+gpspos4.x);

        PositionConvertor.Point gpspos5 = PositionConvertor.ConvertSlam2GPS(1040,600);
        System.out.println(gpspos5.y+","+gpspos5.x);

        System.out.println("Coordinate system conversion complete!");
    }
}

