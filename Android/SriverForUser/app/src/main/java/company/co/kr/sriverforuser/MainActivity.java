package company.co.kr.sriverforuser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.graphics.Color;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class MainActivity extends AppCompatActivity {

    int marTop = 200, marBottom = 100, marLeft = 100, marRight = 100;

    int windowWidth = 0, windowHeight = 0;
    int deviceWindowWidth = 1080, deviceWindowHeight = 1794;
    double rateX, rateY;


    ArrayList<ParkingPoint> dummyparkingPoint = new ArrayList<>();
    ArrayList<ParkingPoint> parkingPoint = new ArrayList<>();
    ArrayList<Integer> posX = new ArrayList<>();//주차칸 x축기준
    ArrayList<Integer> posY = new ArrayList<>();//주차칸 y축기준

    ArrayList<Integer> crossX = new ArrayList<>();
    ArrayList<Integer> crossY = new ArrayList<>();
    static public ArrayList<CrossPoint> crossPoint = new ArrayList<>();//교차로
    static public ArrayList<CrossPoint> crossPointX = new ArrayList<>();//교차로X
    static public ArrayList<CrossPoint> crossPointY = new ArrayList<>();//교차로Y


    Dijkstra dijkstra;
    String ID;
    Context context;
    int reserve = -1;//예약
    int pMaxWidth, pMaxHeight;//제일큰 주차장
    int pWidth, pHeight;//평균 주차칸
    int pStartX, pStartY;//평균 주차칸
    int pCountX, pCountY;//x,y개수
    int step = 0, maxStep = 2;
    //new ParkingPointLoad().execute();//주차장 정보 받아옴 step++
    //new UserPosLoad(0).execute();//유저 위치 받아옴 0일때만 step++;

    int flag = -1;
    int carX = 0, carY = 0, dCarX = 0, dCarY = 0, reserveX = 0, reserveY = 0, dReserveX = 0, dReserveY = 0;
    int maxCrossPointIndexX, maxCrossPointIndexY;
    int parkingTime = 0;
    boolean park = false;
    int parkIndex;



    RelativeLayout topLL;
    private ReservationDialog reservationDialog;
    private ParkingDialog parkingDialog;
    TextView tv_click;
    ImageView im;
    Bitmap carImage;


    public static ArrayList<Point> path = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        context = this;
        ID = getIntent().getStringExtra("ID");
        topLL = (RelativeLayout)findViewById(R.id.layout_Main);

        carImage = BitmapFactory.decodeResource(getResources(), R.drawable.car);
        im = new ImageView(MainActivity.this);

        new ParkingPointLoad().execute();//주차장 정보 받아옴 step++
        new UserPosLoad(0).execute();//유저 위치 받아옴 0일때만 step++;


        Thread myThread = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {
                        if(flag != 2){
                            if(flag == -1)//맨처음 시작
                                new CheckFlag().execute();//flag확인
                            else if(flag == 0)//checkflag로 flag가 0일때->1로변경
                                new ChangeFlag(1).execute();
                            else if(flag == 1)
                                new CheckFlag().execute();//flag확인
                        }
                        else{//추적중임
                            if(step >= maxStep){//초기 print다함
                                //실시간 동기화
                                new ParkingPointSync().execute();//주차칸 동기화
                                new UserPosLoad(1).execute();
                            }
                        }
                        Thread.sleep(100);
                    } catch (Throwable t) {
                    }
                }
            }
        });
        myThread.start();
    }

    private void SortParkingPoint() {
        //X에 관해 정렬

        Comparator<ParkingPoint> cmpAscX = new Comparator<ParkingPoint>() {
            @Override
            public int compare(ParkingPoint o1, ParkingPoint o2) {
                return o1.startX - o2.startX;
            }
        };

        Comparator<ParkingPoint> cmpAscY = new Comparator<ParkingPoint>() {

            @Override
            public int compare(ParkingPoint o1, ParkingPoint o2) {
                return o1.startY - o2.startY;
            }
        };
        Collections.sort(dummyparkingPoint, cmpAscY);

        while (true) {
            int index = 1;
            ArrayList<ParkingPoint> pp = new ArrayList<>();
            while (Math.abs(dummyparkingPoint.get(0).startY - dummyparkingPoint.get(index).startY) < pHeight * 0.5) {
                index++;
                if (dummyparkingPoint.size() <= index)
                    break;
            }
            pCountX = index;
            for (int i = 0; i < index; i++) {
                pp.add(dummyparkingPoint.get(0));
                dummyparkingPoint.remove(0);
            }
            Collections.sort(pp, cmpAscX);
            int sumY = 0;
            for(int i = 0; i<pp.size(); i++)
                sumY += pp.get(i).startY;
            for(int i = 0; i<pp.size(); i++)
                pp.get(i).startY = sumY / pp.size();
            for (int i = 0; i < pp.size(); i++)
                parkingPoint.add(pp.get(i));
            if (dummyparkingPoint.size() == 0)
                break;
        }
        pCountY = parkingPoint.size()/pCountX;
        for(int i = 0; i<pCountX; i++){
            int sumX = 0;
            for(int j = 0; j<pCountY; j++)
                sumX += parkingPoint.get(j*pCountX + i).startX;
            for(int j = 0; j<pCountY; j++)
                parkingPoint.get(j*pCountX + i).startX = sumX / pCountY;
        }
    }

    void setCrossPointXY() {
        setPosX();
        setPosY();
        int index = 0;

        for(int i = 0; i<posX.size(); i++){
            if(posX.get(i) == 0) crossX.add(index);
            else index++;
        }
        index = 0;
        for(int i = 0; i<posY.size(); i++){
            if(posY.get(i) == 0) crossY.add(index);
            else index++;
        }
        int cx = 0;
        int cy = 0;
        for(int i = 0; i<crossX.size(); i++){
            for(int j = 0; j<crossY.size(); j++){
                crossPoint.add(new CrossPoint(cx, cy,
                        rateLengthX(parkingPoint.get(crossX.get(i)-1).startX + pWidth),
                        rateLengthY(parkingPoint.get((crossY.get(j) - 1) * pCountX).startY + pHeight),
                        rateLengthX(parkingPoint.get(crossX.get(i)).startX),
                        rateLengthY(parkingPoint.get(crossY.get(j) * pCountX).startY)));
                cy++;
                maxCrossPointIndexY = cy;
            }
            cy = 0;
            cx++;
        }
        maxCrossPointIndexX = cx;


        Comparator<CrossPoint> cmpAscX = new Comparator<CrossPoint>() {
            @Override
            public int compare(CrossPoint o1, CrossPoint o2) {
                return o1.startX - o2.startX;
            }
        };

        Comparator<CrossPoint> cmpAscY = new Comparator<CrossPoint>() {

            @Override
            public int compare(CrossPoint o1, CrossPoint o2) {
                return o1.startY - o2.startY;
            }
        };

        ArrayList<CrossPoint> tmp = new ArrayList<>();
        for (int i = 0; i < crossPoint.size(); i++)
            tmp.add(crossPoint.get(i));

        Collections.sort(tmp, cmpAscX);
        for(int i = 0; i<maxCrossPointIndexX; i++)
            crossPointX.add(tmp.get(i));
        Collections.sort(crossPointX, cmpAscY);


        tmp = new ArrayList<>();
        for (int i = 0; i < crossPoint.size(); i++)
            tmp.add(crossPoint.get(i));

        Collections.sort(tmp, cmpAscY);
        for(int i = 0; i<maxCrossPointIndexY; i++)
            crossPointY.add(tmp.get(i));
        Collections.sort(crossPointY, cmpAscX);

        ConnectDijCrossPoint();
    }

    void setPosX(){
        ArrayList<ParkingPoint> pp = new ArrayList<>();
        for(int i = 0; i<pCountX; i++){
            pp.add(parkingPoint.get(i));
        }
        posX.add(1);
        int index = 0;
        while(true){
            pWidth = pWidth;
            if(Math.abs(pp.get(index).startX - pp.get(index+1).startX) < pWidth * 1.5)
                posX.add(1);
            else{
                posX.add(0);
                posX.add(1);
            }
            index++;
            if(pp.size() <= index+1)
                break;
        }
    }

    void setPosY(){
        ArrayList<ParkingPoint> pp = new ArrayList<>();
        for(int i = 0; i<pCountY; i++){
            pp.add(parkingPoint.get(i*pCountX));
        }
        posY.add(1);
        int index = 0;
        while(true){
            if(Math.abs(pp.get(index).startY - pp.get(index+1).startY) < pHeight * 1.5)
                posY.add(1);
            else{
                posY.add(0);
                posY.add(1);
            }
            index++;
            if(pp.size() <= index+1)
                break;
        }
    }

    void Print(){
        DisplayMetrics metrics = getResources().getDisplayMetrics();
        windowWidth = metrics.widthPixels;
        windowHeight = metrics.heightPixels;
        rateX = (double)(windowWidth) / deviceWindowWidth;
        rateY = (double)(windowHeight) / deviceWindowHeight;

        int prWidth = rateLengthWidth();
        int prHeight = rateLengthHeight();
        for(int i = 0; i<parkingPoint.size(); i++) {
            int index = parkingPoint.get(i).index;//번호
            int occupy = parkingPoint.get(i).occupy;//점유
            int startX = parkingPoint.get(i).startX;//x
            int startY = parkingPoint.get(i).startY;//y
            final TextView tv = new TextView(MainActivity.this);
            tv.setText(Integer.toString(occupy) + "/" + Integer.toString(index));
            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(prWidth, prHeight);
            layoutParams.setMargins(rateLengthX(startX), rateLengthY(startY), 0, 0);
            tv.setLayoutParams(layoutParams);
            switch(occupy){
                case 0://empty
                    tv.setBackgroundResource(R.drawable.edge_empty);
                    break;
                case 1://reservation
                    if(Integer.parseInt(tv.getText().toString().substring(2)) == reserve)
                        tv.setBackgroundResource(R.drawable.edge_myreversation);
                    else
                        tv.setBackgroundResource(R.drawable.edge_noparking);
                    break;
                case 2://parking
                    if(Integer.parseInt(tv.getText().toString().substring(2)) == reserve)
                        tv.setBackgroundResource(R.drawable.edge_myparking);
                    else
                        tv.setBackgroundResource(R.drawable.edge_noparking);
                    break;
            }
            tv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if(tv.getText().toString().substring(0, 1).equals("0")){
                        reservationDialog = new ReservationDialog(context, tv, reservationDialog_OkClickListener, reservationDialog_CancelClickListener);
                        reservationDialog.setCancelable(true);
                        reservationDialog.getWindow().setGravity(Gravity.CENTER);
                        reservationDialog.show();
                    }
                }
            });
            parkingPoint.get(i).tv = tv;
            tv.setTextColor(Color.alpha(0));
            topLL.addView(tv);
        }
    }

    int rateLengthWidth(){
        return (int)((pWidth * (deviceWindowWidth - marLeft - marRight) / pMaxWidth) * rateX);
    }

    int rateLengthHeight(){
        return (int)((pHeight * (deviceWindowHeight - marTop - marBottom) / pMaxHeight) * rateY);
    }

    int rateLengthX(Object x){
        return (int)(((int)x * (deviceWindowWidth - marLeft - marRight) / pMaxWidth + marLeft) * rateX);
    }

    int rateLengthY(Object y){
        return (int)(((int)y * (deviceWindowHeight - marTop - marBottom) / pMaxHeight + marTop) * rateY);
    }

    void SetCarImage(int x, int y){
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(
                (int)(80 * rateX), (int)(80 * rateY));
        layoutParams.setMargins(x - (int)(80 * rateX)/2, y - (int)(80 * rateY)/2, 0, 0);//차량좌표 제대로해야됨
        im.setLayoutParams(layoutParams);
        im.setImageResource(R.drawable.car);
        topLL.removeView(im);
        topLL.addView(im);
    }

    void ConnectDijCrossPoint(){
        dijkstra = new Dijkstra();
        dijkstra.INIT(crossPoint.size() + 1);

        for(int i = 0;  i<crossPoint.size(); i++)
            crossPoint.get(i).dijID = i + 1;

        for(int i = 0;  i<crossPoint.size(); i++){
            CrossPoint target = crossPoint.get(i);
            int dijTargetIndex = crossPoint.get(i).dijID;
            int cx = target.xIndex;
            int cy = target.yIndex;

            if(cy - 1 >= 0){//상
                CrossPoint match = GetNearCrossPoint(cx, cy - 1);
                int dijMatchIndex = match.dijID;
                dijkstra.CONNECT(dijTargetIndex, dijMatchIndex, distance(target.centerX, target.centerY, match.centerX, match.centerY));
            }
            if(cy + 1 < maxCrossPointIndexY){//하
                CrossPoint match = GetNearCrossPoint(cx, cy + 1);
                int dijMatchIndex = match.dijID;
                dijkstra.CONNECT(dijTargetIndex, dijMatchIndex, distance(target.centerX, target.centerY, match.centerX, match.centerY));
            }
            if(cx - 1 >= 0){//좌
                CrossPoint match = GetNearCrossPoint(cx - 1, cy);
                int dijMatchIndex = match.dijID;
                dijkstra.CONNECT(dijTargetIndex, dijMatchIndex, distance(target.centerX, target.centerY, match.centerX, match.centerY));
            }
            if(cx + 1 < maxCrossPointIndexX){//우
                CrossPoint match = GetNearCrossPoint(cx + 1, cy);
                int dijMatchIndex = match.dijID;
                dijkstra.CONNECT(dijTargetIndex, dijMatchIndex, distance(target.centerX, target.centerY, match.centerX, match.centerY));
            }
        }
    }

    void SetDijkstra(){
        path.clear();
        Dijkstra dijkstraCopy = dijkstra.copy();
        CrossPoint ccp = NearCP("car");
        CrossPoint rcp = NearCP("reserve");

        if(checkPos("car") ==  checkPos("reserve")){
            switch(checkPos("car")){
                case 1:
                    if(ccp.dijID == rcp.dijID){//같은동네임//check
                        if(dCarY == dReserveY){//직선으로 연결되어있으면
                            //car-dcar-drsv-rsv

                            path.add(new Point(carX, carY));
                            if(!(Math.abs(dCarX - dReserveX) < rateLengthWidth()/2)
                                    && !(Math.abs(carY - reserveY) < crossPoint.get(0).height/2)){
                                path.add(new Point(dCarX, dCarY));
                                path.add(new Point(dReserveX, dReserveY));
                            }
                            carX = dCarX;
                            path.add(new Point(reserveX, reserveY));
                        }
                        else{//check
                            //car-dcar-ccp-drsv-rsv
                            path.add(new Point(carX, carY));
                            path.add(new Point(dCarX, dCarY));
                            path.add(new Point(ccp.centerX, ccp.centerY));
                            path.add(new Point(dReserveX, dReserveY));
                            path.add(new Point(reserveX, reserveY));
                        }
                    }
                    else{//check
                        //car-dcar-ccp-rcp-drsv-rsv
                        path.add(new Point(carX, carY));
                        path.add(new Point(dCarX, dCarY));
                        path.add(new Point(ccp.centerX, ccp.centerY));
                        path.add(new Point(rcp.centerX, rcp.centerY));
                        path.add(new Point(dReserveX, dReserveY));
                        path.add(new Point(reserveX, reserveY));
                    }
                    break;
                case 2:
                case 3://check
                    //car-dcar-drsv-rsv
                    path.add(new Point(carX, carY));
                    if(!(Math.abs(dCarX - dReserveX) < rateLengthWidth()/2)
                            && !(Math.abs(carY - reserveY) < crossPoint.get(0).height/2)){
                        path.add(new Point(dCarX, dCarY));
                        path.add(new Point(dReserveX, dReserveY));
                    }
                    carX = dCarX;
                    path.add(new Point(reserveX, reserveY));
                    break;
            }
        }
        else{//check
            //0커넥트하고
            dijkstraCopy.CONNECT(0, ccp.dijID, distance(dCarX, dCarY, ccp.centerX, ccp.centerY));
            dijkstraCopy.CONNECT(0, NearXCP(ccp).dijID, distance(dCarX, dCarY, NearXCP(ccp).centerX, NearXCP(ccp).centerY));
            dijkstraCopy.DONNECT(ccp.dijID, NearXCP(ccp).dijID);


            //길 car-dcar-0->~~->rcp-drsv-rsv
            ArrayList<Integer> dijPath = dijkstraCopy.SEARCHPATH(rcp.dijID);
            path.add(new Point(carX, carY));
            path.add(new Point(dCarX, dCarY));
            for(int i = 1; i<dijPath.size(); i++){
                CrossPoint cp = search(dijPath.get(i));
                if(i == dijPath.size() -1){
                    if(ccp.centerY != rcp.centerY
                            &&(EdgeCP("TL").centerX < dReserveX && EdgeCP("TR").centerX > dReserveX))
                        break;
                }
                path.add(new Point(cp.centerX, cp.centerY));
            }
            path.add(new Point(dReserveX, dReserveY));
            path.add(new Point(reserveX, reserveY));
        }
    }

    CrossPoint GetNearCrossPoint(int xIndex, int yIndex){
        for(int i = 0; i<crossPoint.size(); i++){
            if(crossPoint.get(i).xIndex == xIndex){
                if(crossPoint.get(i).yIndex == yIndex){
                    return crossPoint.get(i);
                }
            }
        }
        return null;
    }


    void checkParking(){

        if(parkingTime > 200 && !park){
            park = true;
            path.clear();
            parkingPoint.get(parkIndex).tv.setBackgroundResource(R.drawable.edge_myparking);
            if(parkIndex != reserve)
                new SetParkingPointReservation(reserve, 0, "NULL").execute();
            new SetParkingPointReservation(parkingPoint.get(parkIndex).index, 2, ID).execute();
            parkingDialog = new ParkingDialog(context, parkingDialog_OkClickListener);
            parkingDialog.setCancelable(true);
            parkingDialog.getWindow().setGravity(Gravity.CENTER);
            parkingDialog.show();
            return;
        }
        if(checkInParkingPoint() != -1){
            parkIndex = checkInParkingPoint();
            parkingTime++;
        }
    }

    int checkInParkingPoint(){
        for(int i = 0; i<parkingPoint.size(); i++){
            if(carX > rateLengthX(parkingPoint.get(i).startX) && carX < rateLengthX(parkingPoint.get(i).startX) + rateLengthWidth() && carY > rateLengthY(parkingPoint.get(i).startY) && carY < rateLengthY(parkingPoint.get(i).startY) + rateLengthHeight()){
                return i;
            }
        }
        return -1;
    }

//    void checkParking(){
//
//        if(parkingTime > 50 && !park){
//            park = true;
//            for(int i = 0; i<parkingPoint.size(); i++){
//                if(parkingPoint.get(i).index == reserve){
//                    parkingPoint.get(i).tv.setBackgroundResource(R.drawable.edge_myparking);
//                    break;
//                }
//            }
//            new SetParkingPointReservation(reserve, 2, ID).execute();
//            parkingDialog = new ParkingDialog(context, parkingDialog_OkClickListener);
//            parkingDialog.setCancelable(true);
//            parkingDialog.getWindow().setGravity(Gravity.CENTER);
//            parkingDialog.show();
//            return;
//        }
//        for(int i = 0; i<parkingPoint.size(); i++){
//            if(parkingPoint.get(i).index == reserve){
//                ParkingPoint pp = parkingPoint.get(i);
//                if(carX > rateLengthX(pp.startX) && carX < rateLengthX(pp.startX) + rateLengthWidth() && carY > rateLengthY(pp.startY) && carY < rateLengthY(pp.startY) + rateLengthHeight()){
//                    parkingTime++;
//                }
//                break;
//            }
//        }
//
//    }

    int distance(int x1, int y1, int x2, int y2){
        return (int)(Math.sqrt(Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2)));
    }

    CrossPoint NearCP(String str){
        int x=0, y=0;
        if(str.equals("reserve")){//도착지점
            x = reserveX;
            y = reserveY;
        }
        else if(str.equals("car")){//차량
            x = carX;
            y = carY;
        }
        int minDis = 999999;
        int index = 0;
        for(int i = 0; i<crossPoint.size(); i++){
            CrossPoint target = crossPoint.get(i);
            int dis = distance(target.centerX, target.centerY, x, y);
            if(minDis > dis){
                minDis = dis;
                index = i;
            }
        }

        return crossPoint.get(index);
    }

    CrossPoint NearXCP(CrossPoint cp){
        for(int i = 0; i<crossPoint.size(); i++){
            if(cp.centerY == crossPoint.get(i).centerY){
                if(crossPoint.get(i) != cp)
                    return crossPoint.get(i);
            }
        }
        return null;
    }

    CrossPoint EdgeCP(String option){

        ArrayList<CrossPoint> tmp = new ArrayList<>();
        for(int i = 0; i<crossPoint.size(); i++)
            tmp.add(crossPoint.get(i));

        Comparator<CrossPoint> cmpAscX = new Comparator<CrossPoint>() {
            @Override
            public int compare(CrossPoint o1, CrossPoint o2) {
                return o1.startX - o2.startX;
            }
        };

        Comparator<CrossPoint> cmpAscY = new Comparator<CrossPoint>() {

            @Override
            public int compare(CrossPoint o1, CrossPoint o2) {
                return o1.startY - o2.startY;
            }
        };

        ArrayList<CrossPoint> xy = new ArrayList<>();
        Collections.sort(tmp, cmpAscX);
        switch(option){
            case "TL":
                for(int i = 0; i<maxCrossPointIndexX; i++)
                    xy.add(tmp.get(i));
                Collections.sort(xy, cmpAscY);
                return xy.get(0);
            case "TR":
                for(int i = 0; i<maxCrossPointIndexX; i++)
                    xy.add(tmp.get(tmp.size() - 1 - i));
                Collections.sort(xy, cmpAscY);
                return xy.get(0);
            case "BL":
                for(int i = 0; i<maxCrossPointIndexX; i++)
                    xy.add(tmp.get(i));
                Collections.sort(xy, cmpAscY);
                return xy.get(xy.size() - 1);
            case "BR":
                for(int i = 0; i<maxCrossPointIndexX; i++)
                    xy.add(tmp.get(tmp.size() - 1 - i));
                Collections.sort(xy, cmpAscY);
                return xy.get(xy.size() - 1);
        }
        return null;
    }

    int inCrossPointX(int x, CrossPoint cp){
        if(cp.startX <= x && cp.endX >= x){
            return 1;
        }
        return 0;
    }

    int inCrossPointY(int y, CrossPoint cp){
        if(cp.startY <= y && cp.endY >= y){
            return 1;
        }
        return 0;
    }

    void setDCar(){
        CrossPoint cp = NearCP("car");
        if(inCrossPointX(carX, cp) == 1){//x기준으로 안쪽일때
            if(inCrossPointY(carY, cp) == 1){//y기준으로 안쪽일때
                dCarX = cp.centerX;
                dCarY = cp.centerY;
            }
            else{//y기준 밖일때
                dCarX = cp.centerX;
                dCarY = carY;
            }
        }
        else{//아예밖일때
            dCarX = carX;
            dCarY = cp.centerY;
        }
    }

    void setDReserve(){
        ParkingPoint pp = null;
        for(int i = 0; i<parkingPoint.size(); i++){
            if(parkingPoint.get(i).tv == tv_click){
                pp = parkingPoint.get(i);
                reserveX = rateLengthX(pp.startX) + rateLengthWidth()/2;
                reserveY = rateLengthY(pp.startY) + rateLengthHeight()/2;
                break;
            }
        }
        CrossPoint cp = NearCP("reserve");
        dReserveX = reserveX;
        dReserveY = cp.centerY;
    }

    int checkPos(String str){
        int x=0, y=0;
        if(str.equals("reserve")){//도착지점
            x = dReserveX;
            y = dReserveY;
        }
        else if(str.equals("car")){//차량
            x = dCarX;
            y = dCarY;
        }
        //모퉁이면 1
        //가운데 기둥이면 위쪽 2
        //옆쪽이면 3
        CrossPoint tl = EdgeCP("TL");
        CrossPoint tr = EdgeCP("TR");
        CrossPoint bl = EdgeCP("BL");
        CrossPoint br = EdgeCP("BR");
        if((x < tl.endX && y < tl.endY)
            || (x > tr.startX && y < tr.endY)
                ||(x < bl.endX && y > bl.startY)
                || (x > br.startX && y > br.startY)){
            return 1;
        }
        else if(x > tl.endX && x < tr.startX){
            if(y < tl.endY)
                return 2;
            else
                return 3;
        }
        else{
            return 4;
        }
    }

    CrossPoint search(int dijID){
        for(int i = 0; i<crossPoint.size(); i++){
            if(crossPoint.get(i).dijID == dijID){
                return crossPoint.get(i);
            }
        }
        return null;
    }

    //PHP-----------------------------------------------------------------------------------------------------------------------

    class CheckFlag extends AsyncTask<Void, Void, String> {
        String target;

        @Override
        protected void onPreExecute() {//연결
            try {
                target = "http://nejoo97.cafe24.com/CheckFlag.php?ID=" + ID;
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try {
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection) url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while ((temp = bufferReader.readLine()) != null) {
                    stringBuilder.append(temp + "\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        public void onProgressUpdate(Void... values) {
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result) {//공지사항리스트에 연결
            try {
                JSONObject jsonObject = new JSONObject(result);
                JSONArray jsonArray = jsonObject.getJSONArray("response");
                int count = 0;
                int f = -1;
                while (count < jsonArray.length()) {
                    JSONObject object = jsonArray.getJSONObject(count);
                    f = object.getInt("FLAG");
                    count++;
                }
                if(flag == -1 || f == 2) flag = f;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }//flag값 가져옴

    class ChangeFlag extends AsyncTask<Void, Void, String> {
        String target;
        ChangeFlag(int f){
            flag = f;
        }
        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/FlagMember.php?ID="+ID + "&FLAG=" + flag;
            }
            catch(Exception e){
                e.printStackTrace();
            }

        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try{
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while((temp = bufferReader.readLine())!=null){
                    stringBuilder.append(temp+"\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            }
            catch(Exception e){
                e.printStackTrace();
            }
            return null;
        }


        @Override
        public void onProgressUpdate(Void... values){
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result) {//공지사항리스트에 연결
        }
    }//flag값 변경

    class ParkingPointLoad extends AsyncTask<Void, Void, String> {
        String target;

        @Override
        protected void onPreExecute() {//연결
            try {
                target = "http://nejoo97.cafe24.com/ParkingPoint.php";
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try {
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection) url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while ((temp = bufferReader.readLine()) != null) {
                    stringBuilder.append(temp + "\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        public void onProgressUpdate(Void... values) {
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result) {//공지사항리스트에 연결
            try {
                JSONObject jsonObject = new JSONObject(result);
                JSONArray jsonArray = jsonObject.getJSONArray("response");
                int count = 0;

                int index, occupy, startX, startY, lenX, lenY;
                int sumWidth = 0, sumHeight = 0;
                String id;
                while (count < jsonArray.length()) {
                    JSONObject object = jsonArray.getJSONObject(count);
                    index = object.getInt("POINT");
                    occupy = object.getInt("OCCUPY");
                    startX = object.getInt("STARTX");
                    startY = object.getInt("STARTY");
                    lenX = object.getInt("LENX");
                    lenY = object.getInt("LENY");
                    id = object.getString("ID");
                    if (count == 0) {//제일큰 사각형 가로 세로길이
                        pStartX = startX;
                        pStartY = startY;
                        pMaxWidth = lenX;
                        pMaxHeight = lenY;
                    } else {
                        if (count != jsonArray.length()) {
                            if(ID.equals(id)){//ID일치함
                                if(occupy == 1){//예약했는데 재접속함
                                    id = "NULL";
                                    occupy = 0;
                                    new SetParkingPointReset(index).execute();
                                }
                                else if(occupy == 2){//주차되어있음
                                    reserve = index;
                                    park = true;
                                }
                            }
                            dummyparkingPoint.add(new ParkingPoint(index, occupy, startX, startY, id));
                            sumWidth += lenX;
                            sumHeight += lenY;
                        }
                    }
                    count++;
                }
                pWidth = sumWidth / (dummyparkingPoint.size() - 1);
                pHeight = sumHeight / (dummyparkingPoint.size() - 1);
                SortParkingPoint();
                Print();
                setCrossPointXY();
                step++;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }//주차장 정보 받아옴 step++

    class SetParkingPointReset extends AsyncTask<Void, Void, String> {
        String target;
        int index;

        public SetParkingPointReset(int index) {
            this.index = index;
        }

        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/ParkingPointReset.php?INDEX="+index;
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try{
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while((temp = bufferReader.readLine())!=null){
                    stringBuilder.append(temp+"\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            }
            catch(Exception e){
                e.printStackTrace();
            }
            return null;
        }


        @Override
        public void onProgressUpdate(Void... values){
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result){}
    }//주차칸 occupy, ID리셋

    class SetParkingPointReservation extends AsyncTask<Void, Void, String> {
        String target;
        int occupy = 0;
        int targetPoint = 0;
        String targetID = "NULL";

        public SetParkingPointReservation(int targetPoint, int occupy, String targetID) {
            this.targetPoint = targetPoint;
            this.occupy = occupy;
            this.targetID = targetID;
        }

        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/ParkingPointReservation.php?POINT="+targetPoint + "&OCCUPY="+occupy + "&ID="+targetID;
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try{
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while((temp = bufferReader.readLine())!=null){
                    stringBuilder.append(temp+"\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            }
            catch(Exception e){
                e.printStackTrace();
            }
            return null;
        }


        @Override
        public void onProgressUpdate(Void... values){
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result){}
    }//주차칸 occupy, ID변경

    class ParkingPointSync extends AsyncTask<Void, Void, String> {
        String target;
        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/ParkingPoint.php";
            }
            catch(Exception e){
                e.printStackTrace();
            }

        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try{
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while((temp = bufferReader.readLine())!=null){
                    stringBuilder.append(temp+"\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            }
            catch(Exception e){
                e.printStackTrace();
            }
            return null;
        }


        @Override
        public void onProgressUpdate(Void... values){
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result){//공지사항리스트에 연결
            try{
                JSONObject jsonObject = new JSONObject(result);
                JSONArray jsonArray = jsonObject.getJSONArray("response");
                int count = 0;

                int index, occupy;
                TextView tv;
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    index = object.getInt("POINT");
                    occupy = object.getInt("OCCUPY");
                    if(count != 0) {
                        for(int i = 0; i<parkingPoint.size(); i++){
                            if(parkingPoint.get(i).index == index){//수정해야됨,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
                                if(parkingPoint.get(i).occupy != occupy){
                                    parkingPoint.get(i).occupy = occupy;
                                    tv = parkingPoint.get(i).tv;
                                    switch(occupy){
                                        case 0://empty
                                            tv.setBackgroundResource(R.drawable.edge_empty);
                                            break;
                                        case 1://reservation
                                            if(Integer.parseInt(tv.getText().toString().substring(2)) == reserve)
                                                tv.setBackgroundResource(R.drawable.edge_myreversation);
                                            else
                                                tv.setBackgroundResource(R.drawable.edge_noparking);
                                            break;
                                        case 2://parking
                                            if(Integer.parseInt(tv.getText().toString().substring(2)) == reserve)
                                                tv.setBackgroundResource(R.drawable.edge_myparking);
                                            else
                                                tv.setBackgroundResource(R.drawable.edge_noparking);
                                            break;
                                    }

                                }
                                break;
                            }
                        }
                    }
                    count++;
                }
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }//주차칸 동기화

    class UserPosLoad extends AsyncTask<Void, Void, String> {
        String target;
        int init;
        UserPosLoad(int init){
            this.init = init;
        }

        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/UserPosGet.php?ID="+ID;
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }

        @Override
        protected String doInBackground(Void... voids) {//데이터읽어오기
            try{
                URL url = new URL(target);
                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
                InputStream inputStream = httpURLConnection.getInputStream();
                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
                String temp;
                StringBuilder stringBuilder = new StringBuilder();
                while((temp = bufferReader.readLine())!=null){
                    stringBuilder.append(temp+"\n");
                }
                bufferReader.close();
                inputStream.close();
                httpURLConnection.disconnect();
                return stringBuilder.toString().trim();
            }
            catch(Exception e){
                e.printStackTrace();
            }
            return null;
        }


        @Override
        public void onProgressUpdate(Void... values){
            super.onProgressUpdate();
        }

        @Override
        public void onPostExecute(String result){//공지사항리스트에 연결
            try{
                JSONObject jsonObject = new JSONObject(result);
                JSONArray jsonArray = jsonObject.getJSONArray("response");
                int count = 0;
                int x = 0, y = 0;
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    x = object.getInt("POSX");
                    y = object.getInt("POSY");
                    carX = rateLengthX(x);
                    carY = rateLengthY(y);
                    count++;
                }
                if(init == 0) step++;
                else{
                    if(x<=0 || y <= 0)return;
                    SetCarImage(carX, carY);
                    if(reserve != -1 && !park){
                        setDCar();
                        SetDijkstra();
                        checkParking();
                    }
                }
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }

    //DIALOG---------------------------------------------------------------------------


    private View.OnClickListener reservationDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            TextView tv = reservationDialog.getTextView();

            if(tv_click != null){//이전에 클릭한적이 있음
                new SetParkingPointReservation(Integer.parseInt(tv_click.getText().toString().substring(2)), 0, "NULL").execute();
                tv_click.setText(Integer.toString(0) + tv_click.getText().toString().substring(1));
                tv_click.setBackgroundResource(R.drawable.edge_empty);
            }
            if(park){
                for(int i = 0; i<parkingPoint.size(); i++){
                    if(parkingPoint.get(i).index == reserve){
                        tv_click = parkingPoint.get(i).tv;
                        break;
                    }
                }
                new SetParkingPointReservation(Integer.parseInt(tv_click.getText().toString().substring(2)), 0, "NULL").execute();
                tv_click.setText(Integer.toString(0) + tv_click.getText().toString().substring(1));
                tv_click.setBackgroundResource(R.drawable.edge_empty);
            }

            tv_click = tv;
            new SetParkingPointReservation(Integer.parseInt(tv_click.getText().toString().substring(2)), 1, ID).execute();
            tv_click.setText(Integer.toString(2) + tv_click.getText().toString().substring(1));
            tv_click.setBackgroundResource(R.drawable.edge_myreversation);
            reserve = Integer.parseInt(tv_click.getText().toString().substring(2));
            setDReserve();
            park = false;
            parkingTime = 0;
            reservationDialog.dismiss();
        }
    };

    private View.OnClickListener reservationDialog_CancelClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            reservationDialog.dismiss();
        }
    };
    private View.OnClickListener parkingDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            parkingDialog.dismiss();
        }
    };
}