package org.example;

public class Main {
    public static void main(String[] args) {
       //slam area : 1040,600

        //좌측 교점:128.8575686,35.15783502
        //상단 교점:128.858427988,35.158463143
        //우측 교점:128.858870603,35.158056682
        //하단 교점:128.858009083,35.157430158

        // 전제 영역
        // LB : 128.8575686,35.157430158
        // RT : 128.858870603,35.158463143

        // LB : 128.8575686001505289,35.1574301543503580
        // RT : 128.8588706034498728,35.1584631433985280

        PositionConvertor.initArea(2080,1200,new PositionConvertor.Point(128.858009083,35.157430158),new PositionConvertor.Point(128.858870603,35.158056682));
    /*    PositionConvertor.Point pos1 = PositionConvertor.ConvertSlamPos(0,0, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos2 = PositionConvertor.ConvertSlamPos(399,0, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos3 = PositionConvertor.ConvertSlamPos(1040,0, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos4 = PositionConvertor.ConvertSlamPos(1176,691, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos5 = PositionConvertor.ConvertSlamPos(1040,600, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos6 = PositionConvertor.ConvertSlamPos(777,1140, PositionConvertor.WorkType.GPS);

        PositionConvertor.Point pos7 = PositionConvertor.ConvertSlamPos(0,600, PositionConvertor.WorkType.SLAM);
        PositionConvertor.Point pos8 = PositionConvertor.ConvertSlamPos(0,448, PositionConvertor.WorkType.GPS);*/

        PositionConvertor.Point slampos1 = PositionConvertor.ConvertGPS2Slam(128.8575686,35.15783502
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));

        PositionConvertor.Point slampos2 = PositionConvertor.ConvertGPS2Slam(128.858427988,35.158463143
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));

        PositionConvertor.Point slampos3 = PositionConvertor.ConvertGPS2Slam(128.858870603,35.158056682
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));

        PositionConvertor.Point slampos4 = PositionConvertor.ConvertGPS2Slam(128.858009083,35.157430158
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));

        PositionConvertor.Point slampos5 = PositionConvertor.ConvertGPS2Slam(128.85821960150002,35.1579457443728
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));



        PositionConvertor.Point gpspos1 = PositionConvertor.ConvertSlam2GPS(0,0
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));
        System.out.println(gpspos1.y+","+gpspos1.x);

        PositionConvertor.Point gpspos2 = PositionConvertor.ConvertSlam2GPS(2080,0
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));
        System.out.println(gpspos2.y+","+gpspos2.x);

        PositionConvertor.Point gpspos3 = PositionConvertor.ConvertSlam2GPS(2080,1200
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));
        System.out.println(gpspos3.y+","+gpspos3.x);

        PositionConvertor.Point gpspos4 = PositionConvertor.ConvertSlam2GPS(0,1200
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));
        System.out.println(gpspos4.y+","+gpspos4.x);

        PositionConvertor.Point gpspos5 = PositionConvertor.ConvertSlam2GPS(1040,600
                ,new PositionConvertor.Point(128.8575686,35.157430158),new PositionConvertor.Point(128.858870603,35.158463143));
        System.out.println(gpspos5.y+","+gpspos5.x);

        System.out.println("Coordinate system conversion complete!");
    }
}