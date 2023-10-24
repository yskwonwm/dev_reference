package org.example;

import static java.lang.Math.*;

public class PositionConvertor{
    public static class Point {
        Point(double x, double y){
            this.x= x;
            this.y= y;
        }
        double x;
        double y;
    };

    static public double mappingMaWidth;
    static public double mappingMaHeight;
    static public double slamRotationAngle;
    static public Point areaOffset;

    public enum WorkType {
        SLAM,GPS
    }
    /**
     *  <p>
     *  Initialize driving area data
     *  The coordinate mapping relationship is as follows.
     *  Slam <-> Mapping Map <-> Geo Map
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : slamWidth  SLAM MAP width(slam coordinate system)
     * @param : slamHeight SLAM MAP height(slam coordinate system)
     * @param : IntersectionStartPoint  Intersection (starting point) between slam-map and geo map area(longitude latitude coordinate system)
     * @param : IntersectionEndPoint  Intersection (end point) between slam-map and geo map area(longitude latitude coordinate system)
     * @return : void
     *
     */
    static public void initArea(int slamWidth
            ,int slamHeight
            ,Point IntersectionStartPoint
            ,Point IntersectionEndPoint){
        //1) lon2,lat2와  lon1,lat2로 slam map 회전 각도
        slamRotationAngle = getAngle(IntersectionStartPoint.x,IntersectionStartPoint.y,IntersectionEndPoint.x,IntersectionEndPoint.y);

        if ( slamRotationAngle < 0 ){
            System.out.println("일단 다른 방위 케이스는 보류");
            return;

        }
        //2) slam map의 width,height
        //slamWidth,slamHeight

         //3) GPS 매핑 맵 크기
        mappingMaWidth = round(sin(slamRotationAngle)*slamHeight + cos(slamRotationAngle)*slamWidth) ;
        mappingMaHeight = round(cos(slamRotationAngle)*slamHeight + sin(slamRotationAngle)*slamWidth) ;

        //4) x offset
        areaOffset = new Point(round(sin(slamRotationAngle)*slamHeight),0.0);
    }

    /**
     *  <p>
     *  Convert longitude-latitude coordinate system to slam coordinate system
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : longitude  longitude to convert
     * @param : latitude latitude to convert
     * @param : lonlatLB  Bottom left coordinate of geo map area
     * @param : lonlatRT  Top right coordinates of geo map area
     * @return : converted slam coordinates
     *
     */
    static public  Point ConvertGPS2Slam(double longitude, double latitude,Point lonlatLB,Point lonlatRT) {
        //1. GPS to Mapping Map
        double mX = ((longitude-lonlatLB.x)/(lonlatRT.x-lonlatLB.x)) * mappingMaWidth;
        double mY = ((latitude-lonlatLB.y)/(lonlatRT.y-lonlatLB.y))* mappingMaHeight;

       // 2. Mapping Map to Slam Position
        return ConvertSlamPos((int)mX, (int)mY, WorkType.GPS);
    }

    /**
     *  <p>
     *  Convert slam coordinate system to longitude-latitude coordinate system
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : x  slam x to convert
     * @param : y slam y to convert
     * @param : lonlatLB  Bottom left coordinate of the entire map area
     * @param : lonlatRT  Top right coordinates of the entire map area)
     * @return : converted slam coordinates
     *
     */
    static public  Point ConvertSlam2GPS(int x, int y,Point lonlatLB,Point lonlatRT) {
        // 1. Slam Position to Mapping Map
        Point mPos = ConvertSlamPos(x, y, WorkType.SLAM);

        //2. Mapping Map to GPS
        double Longitude = lonlatLB.x+(lonlatRT.x-lonlatLB.x) * (mPos.x/mappingMaWidth);
        double latitude = lonlatLB.y+(lonlatRT.y-lonlatLB.y) * (mPos.y/mappingMaHeight);

        return new Point(Longitude,latitude);
    }

    /**
     *  <p>
     *  conversion between mapping maps and slams
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : x  slam x to convert
     * @param : y  slam y to convert
     * @param : WorkType  conversion type ( Slam -> Mapping Map or Mapping Map -> Slam )
     *          WorkType.SLAM : Convert from SLAM to mapping map coordinates
     *          WorkType.GPS : Convert from mapping map to slam coordinates
     * @return : converted coordinates
     *
     */
    static private Point ConvertSlamPos(int x, int y, WorkType type){

        if ( slamRotationAngle <= 0 ){
            System.out.println("일단 다른 방위 케이스는 보류");
            return null;
        }
        Point pos = null;

        if (type == WorkType.SLAM){ //  Slam -> Mapping map
            //1 . x0,y0과 x,y 각도 α
            double a = atan2(y,x);

            //2. x0,y0과 x,y 거리 len
            long len = round(sqrt(x*x+y*y));

            //3.  매핑 맵 x
            double mX = round(cos(slamRotationAngle+a)*len + areaOffset.x);

            //4.  매핑 맵 y
            double mY = round(sin(slamRotationAngle+a)*len);

            pos = new Point(mX,mY);

        }else if (type == WorkType.GPS) { // Mapping map -> slam
            //1 . xOffset,0 -> x0,y0
            double xStd = x-areaOffset.x;
            double yStd = y;

            //2.  x0,y0 과 x’,y’ 각도 β
            double b = atan2(yStd,xStd);
            //   x0,y0 과 x’,y’ 거리 len
            long len = round(sqrt(xStd*xStd+yStd*yStd));

            //3. slam x,y 좌표
            double sX = round(cos(b-slamRotationAngle)*len);
            double sY = round(sin(b-slamRotationAngle)*len);

            pos = new Point(sX,sY);

        }else{
            //Nothing
        }
        return pos;
    }
   static private  double getAngle(double lon1, double lat1, double lon2, double lat2) {

        double y1 =lat1 * PI / 180;
        double y2 =lat2 * PI / 180;
        double x1 =lon1 * PI / 180;
        double x2 =lon2 * PI / 180;

        double y = sin(x2 - x1) *cos(y2);
        double x = cos(y1) * sin(y2) - Math.sin(y1) * cos(y2) * cos(x2 - x1);
        double theta = atan2(y, x);
      //  double bearing = (theta * 180 / Math.PI + 360) % 360;
        return PI/2-theta;// 정북 기준 이므로 변환
    }
}










