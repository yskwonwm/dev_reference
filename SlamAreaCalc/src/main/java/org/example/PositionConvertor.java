package org.example;

import java.util.ArrayList;
import java.util.Optional;

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

    static public double mappingMapWidth;
    static public double mappingMapHeight;
    static public double slamRotationAngle;
    static public Point areaOffset;

    static public Point lonlatLB;
    static public Point lonlatRT;
    public enum WorkType {
        SLAM,GPS
    }

    /**
     *  <p>
     * Initialize driving area data(include initArea)
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : x1 slam reference coordinate start x
     * @param : y1 slam reference coordinate start y
     * @param : x2 slam reference coordinate end x
     * @param : y2 slam reference coordinate end y
     * @param : width slam map width
     * @param : height slam map height
     * @param : mapPoint1 virtual map reference coordinate start lonlat
     * @param : mapPoint2 virtual map reference coordinate end lonlat
     * @param : startlonlat  Top left coordinate of the virtual map area(only used to find the slope.)
     * @param : endlonlat  Top right coordinates of the virtual map area(only used to find the slope.)
     * @return : void
     *
     */
    static public void initialize(int x1, int y1, int x2, int y2, int slamWidth, int slamHeight
            , Point mapPoint1,Point mapPoint2, Point startlonlat, Point endlonlat){

        ArrayList<PositionConvertor.Point> arr =  PositionConvertor.ConvertSlam2VirtualMapArea(
                x1, y1, x2, y2,slamWidth, slamHeight
                ,mapPoint1
                ,mapPoint2
                ,startlonlat
                ,endlonlat);


        PositionConvertor.initArea(slamWidth,slamHeight
                ,startlonlat
                ,endlonlat
                ,new PositionConvertor.Point(arr.get(0).x,arr.get(0).y)
                ,new PositionConvertor.Point(arr.get(1).x,arr.get(1).y));
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
     * @param : startlonlat  Intersection (starting point) between slam-map and geo map area(longitude latitude coordinate system)
     * @param : endlonlat  Intersection (end point) between slam-map and geo map area(longitude latitude coordinate system)
     * @param : longitudeLatitudeLB Bottom left coordinate of geo map area
     * @param : longitudeLatitudeRT Top right coordinates of geo map area
     * @return : void
     *
     */
    static public void initArea(int slamWidth
            ,int slamHeight
            ,Point startlonlat
            ,Point endlonlat
            ,Point longitudeLatitudeLB
            ,Point longitudeLatitudeRT){
        //1) lon2,lat2와  lon1,lat2로 slam map 회전 각도
        slamRotationAngle = getAngle(startlonlat.x,startlonlat.y,endlonlat.x,endlonlat.y);

        if ( slamRotationAngle < 0 ){
            System.out.println("일단 다른 방위 케이스는 보류");
            return;

        }
        //2) slam map의 width,height
        //slamWidth,slamHeight

         //3) GPS 매핑 맵 크기
        mappingMapWidth = round(sin(slamRotationAngle)*slamHeight + cos(slamRotationAngle)*slamWidth) ;
        mappingMapHeight = round(cos(slamRotationAngle)*slamHeight + sin(slamRotationAngle)*slamWidth) ;

        //4) x offset
        areaOffset = new Point(round(sin(slamRotationAngle)*slamHeight),0.0);

        //5)현장맵을 포함하는 지도의 사각영역(경위도 직교)
        lonlatLB = longitudeLatitudeLB;
        lonlatRT = longitudeLatitudeRT;
    }


    /**
     *  <p>
     *  Convert longitude-latitude coordinate system to slam coordinate system
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : longitude  longitude to convert
     * @param : latitude latitude to convert     *
     * @return : converted slam coordinates
     *
     */
    static public  Point ConvertGPS2Slam(double longitude, double latitude) {
        //1. GPS to Mapping Map
        double mX = ((longitude-lonlatLB.x)/(lonlatRT.x-lonlatLB.x)) * mappingMapWidth;
        double mY = ((latitude-lonlatLB.y)/(lonlatRT.y-lonlatLB.y))* mappingMapHeight;

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
     * @return : converted slam coordinates
     *
     */
    static public  Point ConvertSlam2GPS(int x, int y) {
        System.out.println("input :  " + y + " , " + x);
        // 1. Slam Position to Mapping Map
        Point mPos = ConvertSlamPos(x, y, WorkType.SLAM);
        if(Optional.ofNullable(mPos).isEmpty()){
            return new Point(0,0);
        }
        //2. Mapping Map to GPS
        double Longitude = lonlatLB.x+(lonlatRT.x-lonlatLB.x) * (mPos.x/mappingMapWidth);
        double latitude = lonlatLB.y+(lonlatRT.y-lonlatLB.y) * (mPos.y/mappingMapHeight);

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
            double len = (sqrt(x*x+y*y));

            //3.  매핑 맵 x
            double mX = (cos(slamRotationAngle+a)*len + areaOffset.x);

            //4.  매핑 맵 y
            double mY = (sin(slamRotationAngle+a)*len);

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
            double sX = (cos(b-slamRotationAngle)*len);
            double sY = (sin(b-slamRotationAngle)*len);

            pos = new Point(sX,sY);

        }else{
            //Nothing
        }
        return pos;
    }

    /**
     *  <p>
     * Extract virtual map area coordinates mapped to slam map for mock testing
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : x slam reference coordinate x
     * @param : y slam reference coordinate y
     * @param : width slam map width
     * @param : height slam map height
     * @param : distPerPix slam map distance per pixel
     * @param : mapPoint virtual map reference coordinate lonlat
     * @param : lonlatLT  Top left coordinate of the virtual map area
     * @param : lonlatRT  Top right coordinates of the virtual map area
     * @return : virtual map area bottom left,top right
     *
     */
    static public ArrayList<Point> ConvertSlam2VirtualMapArea(int x, int y, int width, int height, double distPerPix, Point mapPoint, Point lonlatLT, Point lonlatRT) {

        //1. 맵의 기울어진 각도 추출
        double slamRotationAngle = getAngle(lonlatLT.x,lonlatLT.y,lonlatRT.x,lonlatRT.y);
        //2. 슬램맵 원점과 우상단의 거리와  각도
        double yDistSlam = (height-y)*distPerPix;  // 슬램맵 원점 기준 y 축 거리
        double xDistSlam = (width-x)*distPerPix;   // 슬램맵 원점 기준  x 축 거리
        double rtPointAngle = atan2(height-y,width-x); // 슬램맵 우상단과 기준좌표와의 각도
        double distSlam = sqrt(pow((height-y),2)+pow((width-x),2))*distPerPix; // 슬램맵 우상단과 기준좌표와의 거리

        double diagonalAngle = atan2(height,width); // 슬램맵 우상단과 좌하단의 각도
        double diagonalDistance = sqrt(pow((height),2)+pow((width),2))*distPerPix; // 슬램맵 우상단과 조하단 거리
        double heightDistance = height*distPerPix;
        double widthDistance = width*distPerPix;

        //3. 슬램맵 거리와 각도에 해당하는 현장맵 우상단 좌표 추출(현장맵 원점 기준,맵 각도 반역)(상단 교점)
        Point rightTopPos= getMovingLonLat(mapPoint.x,mapPoint.y,distSlam,slamRotationAngle+rtPointAngle);

        //4. 슬램맵 width로 현장맵 좌상단 구하기(좌측 교점)
        Point leftTopPos= getMovingLonLat(rightTopPos.x,rightTopPos.y,widthDistance,slamRotationAngle+PI);

        //5. 슬램맵 대각선 길이 rtDistSlam 현장맵 좌하단 구하기(하단 교점)
        Point leftBottomPos= getMovingLonLat(rightTopPos.x,rightTopPos.y,diagonalDistance,slamRotationAngle+diagonalAngle+PI);

        //6. 슬램맵 height 로 현장맵 우하단 구하기(우측 교점)
        Point rightBottomPos= getMovingLonLat(rightTopPos.x,rightTopPos.y,heightDistance,slamRotationAngle+(PI+PI/2));

        ArrayList<Point> pointList = new ArrayList<>();
        pointList.add(new Point(leftTopPos.x,leftBottomPos.y));// 좌상 lat, 좌하 lon
        pointList.add(new Point(rightBottomPos.x,rightTopPos.y)); // 우하 lat, 우상 lon

        return pointList;
    }


    /**
     *  <p>
     * Extract map area coordinates for mapping slab maps and GIS maps with inaccurate distances
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : x1 slam reference coordinate start x
     * @param : y1 slam reference coordinate start y
     * @param : x2 slam reference coordinate end x
     * @param : y2 slam reference coordinate end y
     * @param : width slam map width
     * @param : height slam map height
     * @param : mapPoint1 virtual map reference coordinate start lonlat
     * @param : mapPoint2 virtual map reference coordinate end lonlat
     * @param : lonlatLT  Top left coordinate of the virtual map area(only used to find the slope.)
     * @param : lonlatRT  Top right coordinates of the virtual map area(only used to find the slope.)
     * @return : virtual map area bottom left,top right
     *
     */
    static public ArrayList<Point> ConvertSlam2VirtualMapArea(int x1, int y1, int x2, int y2, int width, int height
            , Point mapPoint1,Point mapPoint2, Point startlonlat, Point endlonlat) {

        //1. 맵의 기울어진 각도 추출
        double slamRotationAngle = getAngle(startlonlat.x,startlonlat.y,endlonlat.x,endlonlat.y);
        //double slamposStdAngle = atan2(y2-y1,x2-x1); // 슬램 기준 좌표의 각도 (비율 보정 안됨)
        double slamStdAngle = getAngle(mapPoint1.x,mapPoint1.y,mapPoint2.x,mapPoint2.y)-slamRotationAngle; // 슬랩 기준 좌표의 경위도의 각도

        //2. 두 기준 좌표간 거리
        double ptDistance4GPS = getDistanceInMeter(mapPoint1.y, mapPoint1.x, mapPoint2.y, mapPoint2.x);
        double xDistance4GPS = ptDistance4GPS*Math.cos(slamStdAngle);
        double yDistance4GPS = ptDistance4GPS*Math.sin(slamStdAngle);
        double ptDistance4SLAM = abs(x2-x1);
        double xDistance4SLAM = abs(x2-x1);
        double yDistance4SLAM = abs(y2-y1);

        if ( yDistance4SLAM == 0 || yDistance4GPS == 0){
            yDistance4GPS = xDistance4GPS;
            yDistance4SLAM = xDistance4SLAM;
        }

        //3. 슬램 픽셀당 길이
        double xDistPerPix = xDistance4GPS/xDistance4SLAM;
        double yDistPerPix = yDistance4GPS/yDistance4SLAM;

        //4. 슬램맵 원점과 우상단의 거리와  각도
        /*double yDistSlam = (height-y)*ydistPerPix;  // 슬램맵 원점 기준 y 축 거리
        double xDistSlam = (width-x)*xdistPerPix;   // 슬램맵 원점 기준  x 축 거리*/
        double rtPointAngle = atan2((height-y2)*yDistPerPix,(width-x2)*xDistPerPix); // 슬램맵 우상단과 우측기준좌표와의 각도
        double rtDistSlam = sqrt(pow((height-y2)*yDistPerPix,2)+pow((width-x2)*xDistPerPix,2)); // 슬램맵 우상단과 우측 기준좌표와의 거리

        double lbPointAngle = atan2(y1*yDistPerPix,x1*xDistPerPix); // 슬램맵 좌하단과 좌측기준좌표와의 각도
        double lbDistSlam = sqrt(pow((y1)*yDistPerPix,2)+pow((x1)*xDistPerPix,2)); // 슬램맵 좌하단과 좌측 기준좌표와의 거리

        double diagonalDistance = sqrt(pow((height*yDistPerPix),2)+pow((width*xDistPerPix),2)); // 슬램맵 우상단과 좌하단 거리
        double diagonalAngle = atan2(height*yDistPerPix,width*xDistPerPix); // 슬램맵 우상단과 좌하단의 각도
        double heightDistance = height*yDistPerPix;
       // double widthDistance = width*xDistPerPix;

        //3. 슬램맵 거리와 각도에 해당하는 현장맵 우상단 좌표 추출(현장맵 원점 기준,맵 각도 반역)(상단 교점)
        Point rightTopPos = getMovingLonLat(mapPoint2.x,mapPoint2.y,rtDistSlam,slamRotationAngle+rtPointAngle);

        //4. 슬램맵 거리와 각도에 해당하는 현장맵 좌하단 구하기(하단 교점)
        Point leftBottomPos = getMovingLonLat(mapPoint1.x,mapPoint1.y,lbDistSlam,slamRotationAngle+lbPointAngle+PI);
        Point leftBottomPos1= getMovingLonLat(rightTopPos.x,rightTopPos.y,diagonalDistance,slamRotationAngle+diagonalAngle+PI);

        //5. 슬램맵 height 로 현장맵 우하단 구하기(우측 교점)
        Point rightBottomPos = getMovingLonLat(rightTopPos.x,rightTopPos.y,heightDistance,slamRotationAngle+(PI+PI/2));

        //6. 슬램맵 width로 현장맵 좌상단 구하기(좌측 교점)
        Point leftTopPos= getMovingLonLat(leftBottomPos.x,leftBottomPos.y,heightDistance,slamRotationAngle+PI/2);

        ArrayList<Point> pointList = new ArrayList<>();
        pointList.add(new Point(leftTopPos.x,leftBottomPos.y));// 좌상 lat, 좌하 lon
        pointList.add(new Point(rightBottomPos.x,rightTopPos.y)); // 우하 lat, 우상 lon

        return pointList;
    }

    /**
     *  <p>
     * Calculate the coordinates corresponding to the distance and angle from the current longitude and latitude coordinates.
     *  </p>
     *
     * @version : 0.0.1
     * @author : wavem
     * @param : lat latitude
     * @param : long longitude
     * @param : distance length between two coordinates
     * @param : radian angle between coordinates
     * @return : Longitude and latitude coordinates moved
     *
     */
    static private Point getMovingLonLat(double lon, double lat, double distance, double radian) {

        // 현재 위도 의 1도당 거리 (m)
        double distPerLatDegree = distanceInMeterByHaversine((int)lat, lon, (int)lat+1, lon);
        double distPerLonDegree = distanceInMeterByHaversine(lat, (int)lon, lat, (int)lon+1);

        //  위도 35의 경도 1도의 길이(m)
        if ( distPerLatDegree <= 0 ) {
            distPerLonDegree = 110941;
        }
        if ( distPerLonDegree <= 0 ){
            distPerLonDegree= 91290;
        }
        //4분면 값
        double quadrant1 = 90 * PI / 180;
        double quadrant2 = 180 * PI / 180;
        double quadrant3 = 270 * PI / 180;

        double LongitudeMove = (sqrt(pow(distance,2)-pow(sin(radian)*distance,2)))/distPerLonDegree;
        double latitudeMove = (sqrt(pow(distance,2)-pow(cos(radian)*distance,2)))/distPerLatDegree;

        double latitude = 0;
        double Longitude = 0;

        if ( quadrant1 >= radian){
            Longitude = lon + LongitudeMove;
            latitude = lat + latitudeMove;
        }else if( quadrant2 >= radian){
            Longitude = lon - LongitudeMove;
            latitude = lat + latitudeMove;
        }else if( quadrant3 >= radian){
            Longitude = lon - LongitudeMove;
            latitude = lat - latitudeMove;
        }else {
            Longitude = lon + LongitudeMove;
            latitude = lat - latitudeMove;
        }

        return new Point(Longitude,latitude);
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

    static private double getDistanceInMeter(double lat1, double lon1, double lat2, double lon2){

        double theta, dist;
        double radius = 6371; // 평균 지구 반지름(km)

        theta = lon1 - lon2;
        dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2))
                + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));

        dist = radius * acos(dist);
        return dist * 1000.0;
        //  dist = acos(dist);
        //   dist = rad2deg(dist);
        //   dist = dist * 60 * 1.1515;    // statute miles. 단위는 기본 마일. 1해리는 위도의 1/60도 거리
        //   dist = dist * 1.609344;    // 단위 mile 에서 km 변환.
        //   return dist * 1000.0;  // 단위  km 에서 m 로 변환
    }
    static private double deg2rad(double deg){
        return (double)(deg * Math.PI / (double)180.0);
    }

    static private double rad2deg(double rad){
        return (double)(rad * (double)180 / PI);
    }
    static private double distanceInMeterByHaversine(double lat1, double lon1, double lat2, double lon2) {

        double distance;
        double radius = 6371; // 평균 지구 반지름(km)
        //double toRadian = M_PI / 180.0;

        double deltaLatitude = abs(deg2rad(lat1 - lat2));
        double deltaLongitude = abs(deg2rad(lon1 - lon2));

        double sinDeltaLat = sin(deltaLatitude / 2);
        double sinDeltaLng = sin(deltaLongitude / 2);
        double squareRoot = sqrt(
                sinDeltaLat * sinDeltaLat +
                        cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sinDeltaLng * sinDeltaLng);

        distance = 2 * radius * asin(squareRoot);

        return distance * 1000.0;
    }
}










