package company.co.kr.sriverforuser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.os.AsyncTask;
import android.support.constraint.solver.widgets.Rectangle;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

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
    int pCountX, pCountY;//x,y개수
    int step = 0, maxStep = 2;
    //new ParkingPointLoad().execute();//주차장 정보 받아옴 step++
    //new UserPosLoad(0).execute();//유저 위치 받아옴 0일때만 step++;

    int flag = -1;
    int carX = 0, carY = 0, dCarX = 0, dCarY = 0, reserveX = 0, reserveY = 0, dReserveX = 0, dReserveY = 0;
    boolean dij = false;
    int maxCrossPointIndexX, maxCrossPointIndexY;
    int parkingTime = 0;



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
                    Log.e("parkingTime", Integer.toString(parkingTime));
                    try {
                        if(flag != 2){
                            if(flag == -1){//맨처음 시작
                                new CheckFlag().execute();//flag확인

                                //test
                                flag = 2;
                                step = maxStep;

                            }
                            else if(flag == 0)//checkflag로 flag가 0일때->1로변경
                                new ChangeFlag(1).execute();
                            else if(flag == 1)
                                new CheckFlag().execute();//flag확인
                        }
                        else{//추적중임
                            if(step >= maxStep){//초기 print다함
                                //실시간 동기화
                                //new ParkingPointSync().execute();//주차칸 동기화
                                new UserPosLoad(1).execute();




                                //ChangeParkingPoint();//???
                                //new UserPosLoad().execute();
                                //new ParkingPointSync().execute();

                                //주차했는지 확인
                                //CheckPark();
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
            //tv.setTextColor(Color.alpha(0));
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
        if(dij){

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

        if(parkingTime > 50){
            //다이얼로그생성
            parkingDialog = new ParkingDialog(context, parkingDialog_OkClickListener);
            reservationDialog.setCancelable(true);
            reservationDialog.getWindow().setGravity(Gravity.CENTER);
            reservationDialog.show();
            return;
        }
        for(int i = 0; i<parkingPoint.size(); i++){
            if(parkingPoint.get(i).index == reserve){
                ParkingPoint pp = parkingPoint.get(i);
                if(carX > pp.startX && carX > pp.startX + rateLengthWidth() && carY > pp.startY && carY < pp.startY + rateLengthHeight()){
                    parkingTime++;
                }
                break;
            }
        }

    }

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

    CrossPoint cpNearCP(CrossPoint target, String option){
        switch(option){
            case "T":
                for(int i = 0; i<crossPoint.size(); i++){
                    if(crossPoint.get(i).yIndex == target.yIndex - 1){
                        if(crossPoint.get(i).xIndex == target.xIndex){
                            return crossPoint.get(i);
                        }
                    }
                }
            case "B":
                for(int i = 0; i<crossPoint.size(); i++){
                    if(crossPoint.get(i).yIndex == target.yIndex + 1){
                        if(crossPoint.get(i).xIndex == target.xIndex){
                            return crossPoint.get(i);
                        }
                    }
                }
            case "L":
                for(int i = 0; i<crossPoint.size(); i++){
                    if(crossPoint.get(i).xIndex == target.xIndex - 1){
                        if(crossPoint.get(i).yIndex == target.yIndex){
                            return crossPoint.get(i);
                        }
                    }
                }
            case "R":
                for(int i = 0; i<crossPoint.size(); i++){
                    if(crossPoint.get(i).xIndex == target.xIndex + 1){
                        if(crossPoint.get(i).yIndex == target.yIndex){
                            return crossPoint.get(i);
                        }
                    }
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
                                    //주차했다는 표시해야될듯.----------------------------------------------------------------
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
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    carX = rateLengthX(object.getInt("POSX"));
                    carY = rateLengthY(object.getInt("POSY"));
                    carX = rateLengthX(200);
                    carY = rateLengthY(290);
                    count++;
                }
                if(init == 0) step++;
                else{
                    setDCar();
                    SetCarImage(carX, carY);
                    if(reserve != -1){
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
            if(reserve != -1){//처음시작인데 이미 주차했었음

            }
            tv_click = tv;
            new SetParkingPointReservation(Integer.parseInt(tv_click.getText().toString().substring(2)), 1, ID).execute();
            tv_click.setText(Integer.toString(2) + tv_click.getText().toString().substring(1));
            tv_click.setBackgroundResource(R.drawable.edge_myreversation);

            reserve = Integer.parseInt(tv_click.getText().toString().substring(2));
            setDReserve();
            parkingTime = 0;
            reservationDialog.dismiss();
        }
    };

    private View.OnClickListener reservationDialog_CancelClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            reservationDialog.dismiss();
//            park = true;
        }
    };
    private View.OnClickListener parkingDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            //stop 예약


            parkingDialog.dismiss();
        }
    };
}














//    List<ParkingPoint> parkingPoint;
//    ArrayList<Integer> posX = new ArrayList<>();
//    ArrayList<Integer> posY = new ArrayList<>();
//
//    String ID;
//    int width;
//    int height;
//    int fragTop = 200;
//    int fragBottom = 80;
//    int fragLeft = 10;
//    int fragRight = 10;
//    int maxWidth, maxHeight;
//    int kx = 0, ky = 0;
//    int totalStartX = 0, totalStartY = 0, totalLenX = 0, totalLenY = 0;
//    int startX, startY, lenX, lenY;
//    int maxIndexX = 0, maxIndexY = 0;
//    int carX = 100, carY = 300;
//    int dCarX = 0, dCarY = 0;
//    int ddCarX = 0, ddCarY = 0;
//    int revx = 0, revy = 0, revHy = 0;
//    int dir = 0;//0=up, 1=right, 2=down, 3=left
//    int pWidth, pHeight;
//    int reserv = -1;
//    ArrayList<Integer> rposX = new ArrayList<>();
//    ArrayList<Integer> rposY = new ArrayList<>();
//    ArrayList<TextView> tvList = new ArrayList<>();
//    static public ArrayList<Point> Path = new ArrayList<>();
//    static public ArrayList<CrossPoint> crossPoint = new ArrayList<>();
//
//    RelativeLayout topLL;
//    TextView tv_click = null;
//
//    Dijkstra dijkstra;
//
//    ImageView im;
//    Context context;
//
//
//    private ReservationDialog reservationDialog;
//    private ParkingDialog parkingDialog;
//
//
//
//
//    int check  = 0;
//    boolean stop = true;
//    boolean park = false;
//    boolean start = false;
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
//
//        context = this;
//        ID = getIntent().getStringExtra("ID");
//        parkingPoint = new ArrayList<>();
//
//
//
//        new ParkingPointLoad().execute();//주차칸 내부 정보 수정됬을시 추가해해야됨/////////////////////////////////
//
//        Display display = getWindowManager().getDefaultDisplay();
//        Point size = new Point();
//        display.getSize(size);
//        width = size.x;
//        height = size.y;
//
//        topLL = (RelativeLayout)findViewById(R.id.layout_Main);
//        ImageButton ib_userProperty = (ImageButton)findViewById(R.id.ib_userProperty);
//
//
//        ib_userProperty.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                startActivity(new Intent(MainActivity.this, UserProperty.class));
//            }
//        });
//        new UserPosLoad().execute();
//
//        parkingPoint = parkingPoint;
//        Thread myThread = new Thread(new Runnable() {
//            public void run() {
//                while (true) {
//                    try {
//
//                        //posy+=10;
//                        //new test().execute();
//                        new UserPosLoad().execute();
//                        new ParkingPointSync().execute();
//                        Thread.sleep(100);
//                        runOnUiThread(new Runnable(){
//                            @Override
//                            public void run() {
//                                CheckPark();
//                                reserv = reserv;
//                                ChangeParkingPoint();
//                            }
//                        });
//                        //차량이 주차완료됬는지 체크해야됨
//
//
//                        //데이터 받아와야됨
//                    } catch (Throwable t) {
//                    }
//                }
//            }
//        });
//
//        myThread.start();
//    }
//
//
//    void ChangeParkingPoint(){
//        tvList = tvList;
//        for(int i = 0; i<tvList.size(); i++){
//            for(int j = 0; j<parkingPoint.size(); j++){
//                if(parkingPoint.get(j).point == Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))){
//                    tvList.get(i).setText(parkingPoint.get(j).occupy+tvList.get(i).getText().toString().substring(1));
//                    break;
//                }
//            }
//        }
//        for(int j = 0; j<tvList.size(); j++){
//            switch(Integer.parseInt(tvList.get(j).getText().toString().substring(0, 1))){
//                case 0://empty
//                    tvList.get(j).setBackgroundResource(R.drawable.edge_empty);
//                    break;
//                case 1://reservation
//                    if(Integer.parseInt(tvList.get(j).getText().toString().substring(1, 3)) == reserv)
//                        if(stop)
//                            tvList.get(j).setBackgroundResource(R.drawable.edge_myparking);
//                        else
//                            tvList.get(j).setBackgroundResource(R.drawable.edge_myreversation);
//                    else
//                        tvList.get(j).setBackgroundResource(R.drawable.edge_noparking);
//                    break;
//                case 2://parking
//                    if(Integer.parseInt(tvList.get(j).getText().toString().substring(1, 3)) == reserv)
//                        tvList.get(j).setBackgroundResource(R.drawable.edge_myparking);
//                    else
//                        tvList.get(j).setBackgroundResource(R.drawable.edge_noparking);
//                    break;
//            }
//        }
//
//
//    }
//    void CheckPark(){
//
//        if(stop) return;
//        TextView target_tv = null;
//        reserv = reserv;
//        for(int i = 0; i<tvList.size(); i++){
//            if(Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3)) == reserv){
//                target_tv = tvList.get(i);
//                break;
//            }
//        }
//        target_tv = target_tv;
//
//        if(check >= 50){
//            stop = true;
//            //다이얼로그 생성
//
//            parkingDialog = new ParkingDialog(context, parkingDialog_OkClickListener);
//            parkingDialog.setCancelable(true);
//            parkingDialog.getWindow().setGravity(Gravity.CENTER);
//            parkingDialog.show();
//
//            SetParkingPointReservation a = new SetParkingPointReservation();
//            a.setOccupy(2);
//            a.setTargetPoint(reserv);
//            a.setTargetID(ID);
//            a.execute();
//
//
//            //path삭제
//            return;
//        }
//        if(target_tv == null) return;
//        String[] tmp = target_tv.getHint().toString().split("/");
//        kx = (Integer.parseInt(tmp[0])*(width-fragLeft-fragRight))/maxWidth + fragLeft;
//        ky = (Integer.parseInt(tmp[1])*(height-fragTop-fragBottom))/maxHeight + fragTop;
//        if(dCarX > kx && dCarX < kx + lenX && dCarY > ky && dCarY < ky + lenY)//차량위치
//            check++;
//        else
//            check = 0;
//    }
//
//
//
//    private View.OnClickListener parkingDialog_OkClickListener = new View.OnClickListener() {
//        public void onClick(View v) {
//            parkingDialog.dismiss();
//
//        }
//    };
//
//    void Print(){
//        posX = posX;
//
//        int c = 0;
//        for(int k = 0; k<rposX.size(); k++)
//            if(rposX.get(k) == 1) c++;
//        c = c;
//        rposY =rposY;
//
//        parkingPoint = parkingPoint;
//        for(int i = 0; i<parkingPoint.size(); i++) {
//            int pointp = parkingPoint.get(i).point - 1;//번호
//            int occupy = parkingPoint.get(i).occupy;//점유
//            int startX = parkingPoint.get(i).startX;//x
//            int startY = parkingPoint.get(i).startY;//y
//            final TextView tv = new TextView(MainActivity.this);
//
//            //point=> 안드로이드에서 드로우할 위치계산용
//            //카메라에서 보내는 index(point)
//
//            int xi = ApproachX(startX);
//            int yi = ApproachY(startY);
//            int point = xi + yi *c;
//            point = point;
//            parkingPoint.get(i).point = point;
//            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(lenX, lenY);
//            if(im == null){
//                im = new ImageView(MainActivity.this);
//            }
//            int x = point % maxIndexX;
//            int y = 0;
//            if(point != 0)
//                y = point / maxIndexX;
//            int px = posX.get(point % posX.size());
//            int py;
//            if(point == 0)
//                py = posY.get(0);
//            else
//                py = posY.get(point / posX.size());
//            parkingPoint.get(i).index = point;
//            layoutParams.setMargins((px *(width-fragLeft-fragRight))/maxWidth + fragLeft, (py *(height-fragTop-fragBottom))/maxHeight + fragTop, 0, 0);
//            tv.setLayoutParams(layoutParams);
//            if(point<9)
//                tv.setHint(Integer.toString(occupy) + "0" + Integer.toString(pointp+1));
//            else
//                tv.setHint(Integer.toString(occupy) + Integer.toString(pointp+1));
//            if(i<9)
//                tv.setText(tv.getHint().toString()+"0"+Integer.toString(i+1));
//            else
//                tv.setText(tv.getHint().toString()+Integer.toString(i+1));
//            //tv.setTextColor(Color.alpha(0));
//            tv.setHint(Integer.toString(px) + "/" + Integer.toString(py));
//            tv.setTextSize(20);
//            switch(occupy){
//                case 0://empty
//                    tv.setBackgroundResource(R.drawable.edge_empty);
//                    break;
//                case 1://reservation
//                    if(Integer.parseInt(tv.getText().toString().substring(1, 3)) == reserv)
//                        tv.setBackgroundResource(R.drawable.edge_myreversation);
//                    else
//                        tv.setBackgroundResource(R.drawable.edge_noparking);
//                    break;
//                case 2://parking
//                    if(Integer.parseInt(tv.getText().toString().substring(1, 3)) == reserv)
//                        tv.setBackgroundResource(R.drawable.edge_myparking);
//                    else
//                        tv.setBackgroundResource(R.drawable.edge_noparking);
//                    break;
//            }
//
//
//            tv.setOnClickListener(new View.OnClickListener() {
//                @Override
//                public void onClick(View view) {
//                    if((tv.getText().toString().substring(0, 1).equals("0")
//                            && Integer.parseInt(tv.getText().toString().substring(1, 3))!=reserv) || reserv == -1){
//
//
//
//                        //Dialog
//                        reservationDialog = new ReservationDialog(context, tv, reservationDialog_OkClickListener, reservationDialog_CancelClickListener);
//                        reservationDialog.setCancelable(true);
//                        reservationDialog.getWindow().setGravity(Gravity.CENTER);
//                        reservationDialog.show();
//
//                    }
//                }
//            });
//
//
//
//            //topTV1.setId(R.id.layout_Main + i);
//            //topTV1.setBackgroundColor(Color.parseColor("#000000"));
//            topLL.addView(tv);
//            tvList.add(tv);//tv리스트에 추가
//        }
//        tvList = tvList;
//    }
//
//    private View.OnClickListener reservationDialog_OkClickListener = new View.OnClickListener() {
//        public void onClick(View v) {
//            TextView tv = reservationDialog.getTextView();
//
//            //무조건 바뀌면 안됨
//            if(tv_click != null){
//                tv_click.setBackgroundResource(R.drawable.edge_empty);
//                tv_click.setText(Integer.toString(0) + tv_click.getText().toString().substring(1));
//
//                SetParkingPointReservation a = new SetParkingPointReservation();
//                a.setOccupy(0);
//                a.setTargetPoint(Integer.parseInt(tv_click.getText().toString().substring(1, 3)));
//                a.execute();
//            }
//            tv_click = tv;
//            reserv = Integer.parseInt(tv.getText().toString().substring(1, 3));
//
//
//            SetParkingPointReservation a = new SetParkingPointReservation();
//            a.setOccupy(1);
//            a.setTargetPoint(reserv);
//            a.setTargetID(ID);
//            a.execute();
//
//            reservationDialog.dismiss();
//
//
//            stop = false;
//            check = 0;
//        }
//    };
//
//    private View.OnClickListener reservationDialog_CancelClickListener = new View.OnClickListener() {
//        public void onClick(View v) {
//            reservationDialog.dismiss();
//            park = true;
//        }
//    };
//
//    class ChangeFlag extends AsyncTask<Void, Void, String> {
//        String target;
//        int flag;
//        void SetFlag(int f){
//            flag = f;
//        }
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/FlagMember.php?ID="+ID + "&FLAG=" + flag;
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result) {//공지사항리스트에 연결
//        }
//    }
//
//    class CheckFlag extends AsyncTask<Void, Void, String> {
//        String target;
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/ParkingPoint.php?ID="+ID;
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//                JSONObject jsonObject = new JSONObject(result);
//                JSONArray jsonArray = jsonObject.getJSONArray("response");
//                int count = 0;
//
//                int point, occupy, startX, startY;
//                String id;
//                while(count<jsonArray.length()){
//                    JSONObject object = jsonArray.getJSONObject(count);
//                    point = object.getInt("POINT");
//                    occupy = object.getInt("OCCUPY");
//                    startX = object.getInt("STARTX");
//                    startY = object.getInt("STARTY");
//                    id = object.getString("ID");
//                    if(count == 0) {
//                        maxWidth = startX;
//                        maxHeight = startY;
//                    }
//                    else {
//                        if(count != jsonArray.length()-1) {
//                            parkingPoint.add(new ParkingPoint(point, occupy, startX, startY));
//                            if(ID.equals(id)){
//                                reserv = point;
//                                start = true;
//                            }
//                        }
//                        else{
//                            pWidth = startX;
//                            pHeight = startY;
//                        }
//
//                    }
//                    count++;
//                }
//                SortParkingPoint();
//                Print();//출력함수
//                //Reservation(0);
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//
//    class ParkingPointLoad extends AsyncTask<Void, Void, String> {
//        String target;
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/ParkingPoint.php";
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//                JSONObject jsonObject = new JSONObject(result);
//                JSONArray jsonArray = jsonObject.getJSONArray("response");
//                int count = 0;
//
//                int point, occupy, startX, startY;
//                String id;
//                while(count<jsonArray.length()){
//                    JSONObject object = jsonArray.getJSONObject(count);
//                    point = object.getInt("POINT");
//                    occupy = object.getInt("OCCUPY");
//                    startX = object.getInt("STARTX");
//                    startY = object.getInt("STARTY");
//                    id = object.getString("ID");
//                    if(count == 0) {
//                        maxWidth = startX;
//                        maxHeight = startY;
//                    }
//                    else {
//                        if(count != jsonArray.length()-1) {
//                            parkingPoint.add(new ParkingPoint(point, occupy, startX, startY));
//                            if(ID.equals(id)){
//                                reserv = point;
//                                start = true;
//                            }
//                        }
//                        else{
//                            pWidth = startX;
//                            pHeight = startY;
//                        }
//
//                    }
//                    count++;
//                }
//                if(!start){
//                    ChangeFlag changeFlag = new ChangeFlag();
//                    changeFlag.SetFlag(1);
//                    changeFlag.execute();
//                }
//                SortParkingPoint();
//                Print();//출력함수
//                //Reservation(0);
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//
//
//    class ParkingPointSync extends AsyncTask<Void, Void, String> {
//        String target;
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/ParkingPoint.php";
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//                JSONObject jsonObject = new JSONObject(result);
//                JSONArray jsonArray = jsonObject.getJSONArray("response");
//                int count = 0;
//
//                int point, occupy;
//                while(count<jsonArray.length()){
//                    JSONObject object = jsonArray.getJSONObject(count);
//                    point = object.getInt("POINT");
//                    occupy = object.getInt("OCCUPY");
//                    if(count != 0) {
//                        if(count != jsonArray.length()-1){
//                            for(int i = 0; i<parkingPoint.size(); i++){
//                                if(parkingPoint.get(i).point == point){
//                                    if(parkingPoint.get(i).occupy != occupy){
//                                        parkingPoint.get(i).occupy = occupy;
//                                    }
//                                }
//                            }
//                        }
//                    }
//                    count++;
//                }
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//
//
//    class UserPosLoad extends AsyncTask<Void, Void, String> {
//        String target;
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/UserPosGet.php?ID="+ID;//ID추가해야됨
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//                JSONObject jsonObject = new JSONObject(result);
//                JSONArray jsonArray = jsonObject.getJSONArray("response");
//                int count = 0;
//                while(count<jsonArray.length()){
//                    JSONObject object = jsonArray.getJSONObject(count);
//                    carX = object.getInt("POSX");
//                    carY = object.getInt("POSY");
//                    dir = object.getInt("DIR");
//                    count++;
//                }
//
//
//                //차량이 화면내부에 없으면 return
//
//                if(!(carX >=0 && carX <= maxWidth && carY >= 0 && carY <= maxHeight)){
//
//                    if(im != null) im.setVisibility(View.INVISIBLE);
//                    return;
//                }
//
//                if(im != null) im.setVisibility(View.VISIBLE);
//
//
//                int point = 0;
//                reserv = reserv;
//                for(int i =0; i<tvList.size(); i++){
//                    if(Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))==reserv){
//                        point = Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))-1;
//                        if(reserv != -1 && start){
//                            tv_click = tvList.get(i);
//                            start = false;
//                        }
//                        break;
//                    }
//                }
//                int x = point % maxIndexX;
//                int y = 0;
//                if(point != 0)
//                    y = point / maxIndexX;
//                int px = posX.get(point % posX.size());
//                int py;
//                if(point == 0)
//                    py = posY.get(0);
//                else
//                    py = posY.get(point / posX.size());
//
//                revx = ChangeRate(0, px + (posX.get(1)-posX.get(0))/2);
//                revy = ChangeRate(1, py);
//                revHy = crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y;
//                dCarX = ChangeRate(0, carX);
//                dCarY = ChangeRate(1, carY);
//                Bitmap image = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_car);
//                int w = 96;//image.getWidth();
//                int h = 96;//image.getHeight();
//
//
//                //dd위치 조정 배치에 따라서 수선
//                if(
//                        (dCarX >= crossPoint.get(0).startX && dCarX <= crossPoint.get(0).startX + crossPoint.get(0).getWidth())
//                        || (dCarX >= crossPoint.get(3).startX && dCarX <= crossPoint.get(3).startX + crossPoint.get(3).getWidth())){
//                        if(
//                                (dCarY >= crossPoint.get(0).startY && dCarY <= crossPoint.get(0).startY + crossPoint.get(0).getHeight())
//                                        || (dCarY >= crossPoint.get(3).startX && dCarY <= crossPoint.get(3).startY + crossPoint.get(3).getHeight())){
//
//
//                            ddCarX = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().x;
//                            ddCarY = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().y;
//                        }
//                        else{
//                            ddCarX = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().x;
//                            ddCarY = dCarY;
//                        }
//                }
//                else{
//                    ddCarX = dCarX;
//                    ddCarY = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().y;
//                }
//                ddCarX = ddCarX;
//                ddCarY = ddCarY;
//                RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
//                layoutParams.setMargins(dCarX-w/2, dCarY-h/2, 0, 0);//차량좌표 제대로해야됨
//
//                im.setLayoutParams(layoutParams);
//                im.setImageResource(R.drawable.ic_car);
//                topLL.removeView(im);
//                topLL.addView(im);
//
//
//
//                //맨처음 시작할때
//                //주차 완료시
//
//                dCarX = dCarX;
//                dCarY = dCarY;
//                ddCarX = ddCarX;
//                ddCarY = ddCarY;
//                if (stop || (dCarX >= kx && dCarX <= kx + lenX && dCarY >= ky && dCarY <= ky + lenY)){//길찾기 그리는부분
//                    Path.clear();
//                    return;
//                }
//
//
//
//
//                dijkstra = new Dijkstra();
//                dijkstra.INIT(6);
//                if(DistanceInt(new Point(ddCarX, ddCarY), new Point(revx, revHy))< ChangeRateNo(0, pWidth * 2)){
//
//                    dijkstra.CONNECT(0, 5, DistanceInt(new Point(ddCarX, ddCarY), new Point(revx, revy)));
//                    PathToPoint(dijkstra.SEARCHPATH());
//                    return;
//                }
//                dijkstra.CONNECT(1, 2, DistanceInt(crossPoint.get(0).getPoint(), crossPoint.get(2).getPoint()));
//                dijkstra.CONNECT(2, 3, DistanceInt(crossPoint.get(2).getPoint(), crossPoint.get(3).getPoint()));
//                dijkstra.CONNECT(3, 4, DistanceInt(crossPoint.get(1).getPoint(), crossPoint.get(3).getPoint()));
//                dijkstra.CONNECT(4, 1, DistanceInt(crossPoint.get(1).getPoint(), crossPoint.get(0).getPoint()));
//
//                if(revx >= crossPoint.get(0).startX  + crossPoint.get(0).getWidth() && revx <= crossPoint.get(3).startX){
//                    //예약지점이 y기준으로 있을때
//                    //y가 0,2쪽인지 1,3쪽인지 판단해야됨
//                    if(crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y > height/2) {
//                        //밑(1,3)
//                        dijkstra.DONNECT(3, 4);
//                        dijkstra.CONNECT(5, 3, DistanceInt(new Point(revx, revHy), crossPoint.get(3).getPoint()));
//                        dijkstra.CONNECT(5, 4, DistanceInt(new Point(revx, revHy), crossPoint.get(1).getPoint()));
//                    }
//                    else{
//                        //위 (0,2)
//                        dijkstra.DONNECT(1, 2);
//                        dijkstra.CONNECT(5, 1, DistanceInt(new Point(revx, revHy), crossPoint.get(0).getPoint()));
//                        dijkstra.CONNECT(5, 2, DistanceInt(new Point(revx, revHy), crossPoint.get(2).getPoint()));
//
//                    }
//                }
//                else{
//                    //바깥쪽일때는 그냥 추가하면됨
//                    int dot = 0;
//                    switch(NearCrossPoint(new Point(revx, revHy))){
//                        case 0:
//                            dot = 1;
//                            break;
//                        case 1:
//                            dot = 4;
//                            break;
//                        case 2:
//                            dot = 2;
//                            break;
//                        case 3:
//                            dot = 3;
//                            break;
//                    }
//                    dijkstra.CONNECT(5, dot, DistanceInt(new Point(revx, revHy), crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint()));
//
//                }
//                if(
//                        (ddCarX >= crossPoint.get(0).startX + crossPoint.get(0).getWidth() && ddCarX <= crossPoint.get(3).startX)
//                                || (ddCarY >= crossPoint.get(0).startY + crossPoint.get(0).getHeight() && ddCarY <= crossPoint.get(3).startY)
//                        ){
//
//                    if (ddCarX >= crossPoint.get(0).startX + crossPoint.get(0).getWidth() && ddCarX <= crossPoint.get(3).startX){
//                        if(ddCarY == crossPoint.get(0).getPoint().y){
//                            dijkstra.DONNECT(1, 2);
//                            dijkstra.CONNECT(0, 1, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(0).getPoint()));
//                            dijkstra.CONNECT(0, 2, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(2).getPoint()));
//
//                            if(revx > ddCarX)
//                                dijkstra.DONNECT(0, 1);
//                            else
//                                dijkstra.DONNECT(0, 2);
//                        }
//                        else if(ddCarY == crossPoint.get(3).getPoint().y){
//                            dijkstra.DONNECT(3, 4);
//                            dijkstra.CONNECT(0, 3, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(1).getPoint()));
//                            dijkstra.CONNECT(0, 4, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(3).getPoint()));
//
//                            if(revx > ddCarX)
//                                dijkstra.DONNECT(0, 4);
//                            else
//                                dijkstra.DONNECT(0, 3);
//                        }
//                    }
//                    else if(dCarY >= crossPoint.get(0).startY + crossPoint.get(0).getHeight() && dCarY <= crossPoint.get(3).startY){
//                        if(ddCarX == crossPoint.get(0).getPoint().x){
//                            dijkstra.DONNECT(1, 4);
//                            dijkstra.CONNECT(0, 1, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(0).getPoint()));
//                            dijkstra.CONNECT(0, 4, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(1).getPoint()));
//
//                            if(revHy > dCarY)
//                                dijkstra.DONNECT(0, 1);
//                            else
//                                dijkstra.DONNECT(0, 4);
//                        }
//                        else if(ddCarX == crossPoint.get(3).getPoint().x){
//                            dijkstra.DONNECT(2, 3);
//                            dijkstra.CONNECT(0, 2, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(2).getPoint()));
//                            dijkstra.CONNECT(0, 3, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(3).getPoint()));
//
//                            if(revHy > dCarY)
//                                dijkstra.DONNECT(0, 2);
//                            else
//                                dijkstra.DONNECT(0, 3);
//                        }
//                    }
//                }
//                else{
//                    int k = 1;
//                    if(
//                            (ddCarX >= crossPoint.get(0).startX && ddCarX <= crossPoint.get(2).startX + crossPoint.get(2).getWidth() )
//                                    && (ddCarY >= crossPoint.get(0).startY && ddCarY <= crossPoint.get(2).startY + crossPoint.get(2).getHeight())
//                            ){
//                        int a = 10;
//                        if(crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint().y != crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint().y) {
//                            if (ddCarY > height / 2)
//                                dijkstra.DONNECT(3, 4);
//                            else
//                                dijkstra.DONNECT(1, 2);
//                        }
//                    }
//                    else{
//
//                        if(crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint().y != crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint().y) {
//                            if (ddCarY > height / 2)
//                                dijkstra.DONNECT(3, 4);
//                            else
//                                dijkstra.DONNECT(1, 2);
//                        }
//
//
//                    }
//                    int dot = 0;
//                    switch(NearCrossPoint(new Point(ddCarX, ddCarY))){
//                        case 0:
//                            dot = 1;
//                            break;
//                        case 1:
//                            dot = 4;
//                            break;
//                        case 2:
//                            dot = 2;
//                            break;
//                        case 3:
//                            dot = 3;
//                            break;
//                    }
//                    dijkstra.CONNECT(0, dot, DistanceInt(new Point(ddCarX, ddCarY), crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint()));
//
//                }
//
//
//                dijkstra = dijkstra;
//
//                PathToPoint(dijkstra.SEARCHPATH());
//
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//    class SetParkingPointReservation extends AsyncTask<Void, Void, String> {
//        String target;
//        int occupy = 0;
//        int targetPoint = 0;
//        String targetID = "NULL";
//
//        public void setTargetID(String str){
//            this.targetID = str;
//        }
//        public void setTargetPoint(int n){
//            this.targetPoint = n;
//        }
//        public void setOccupy(int n){
//            this.occupy = n;
//        }
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/ParkingPointReservation.php?POINT="+targetPoint + "&OCCUPY="+occupy + "&ID="+targetID;
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//
//
//
//    private void PathToPoint(ArrayList<Integer> p){
//        p = p;
//        Path.clear();
//        Path.add(new Point(dCarX, dCarY));
//        Path.add(new Point(ddCarX, ddCarY));
//        crossPoint = crossPoint;
//        for(int i = 0; i<p.size(); i++){
//            if(p.get(i) >= 1 && p.get(i) <= 4){
//                    int dot = 0;
//                    switch(p.get(i)){
//                        case 1:
//                            dot = 0;
//                            break;
//                        case 2:
//                            dot = 2;
//                            break;
//                        case 3:
//                            dot = 3;
//                            break;
//                        case 4:
//                            dot = 1;
//                            break;
//                    }
//                Path.add(new Point(crossPoint.get(dot).getPoint().x, crossPoint.get(dot).getPoint().y));
//            }
//        }
//        Path.add(new Point(revx, crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y));
//        Path.add(new Point(revx, revy));
//
//
//
//
//    }
//
//
//    private void SortParkingPoint(){
//        //X에 관해 정렬
//        Comparator<ParkingPoint> cmpAscX = new Comparator<ParkingPoint>() {
//
//            @Override
//            public int compare(ParkingPoint o1, ParkingPoint o2) {
//                return o2.getStartX() - o1.getStartX();
//            }
//        };
//        Collections.sort(parkingPoint, cmpAscX);
//
//        Comparator<ParkingPoint> cmpAscZ = new Comparator<ParkingPoint>() {
//
//            @Override
//            public int compare(ParkingPoint o1, ParkingPoint o2) {
//                return o1.getStartY() - o2.getStartY();
//            }
//        };
//        Collections.sort(parkingPoint, cmpAscZ);
//        parkingPoint = parkingPoint;
//        //중복(+-10% 외에 것들만 posX에 추가)제거
//        for(int i = 0; i<6; i++){
//           posX.add(parkingPoint.get(i).getStartX()-5);
//        }
//        for(int i = 0; i<4; i++){
//            posY.add(parkingPoint.get(i*6).getStartY());
//        }
////        for(int i = 0; i<parkingPoint.size(); i++){
////            if(i == 0){
////                posX.add(parkingPoint.get(i).getStartX());
////            }
////            else{
////                boolean check = false;
////                for(int j = 0; j<posX.size(); j++){
////                    if(posX.get(j) < parkingPoint.get(i).getStartX()+30 && posX.get(j) > parkingPoint.get(i).getStartX()-30){
////                        check = true;
////                        break;
////                    }
////                }
////                if(!check)
////                    posX.add(parkingPoint.get(i).getStartX());
////            }
////        }
//        int min = 999;
//        for(int i = 0; i<posX.size()-1; i++){
//            if(posX.get(i+1) - posX.get(i) < min)
//                min = posX.get(i+1) - posX.get(i);
//        }
//        posX = posX;
//        rposX.add(1);
//        //오차 10%이상 나면 중간에 길 삽입된거임
//        for(int i = 0; i<posX.size()-1; i++){
//            int dif = posX.get(i+1) - posX.get(i);
//            if(min < dif * 1.1 && min > dif*0.9){
//                rposX.add(1);
//            }
//            else{
//                rposX.add(0);
//                rposX.add(1);
//            }
//        }
//        if(posX.get(0) % 10 < 5)
//            startX = posX.get(0) - posX.get(0) % 10;
//        else
//            startX = posX.get(0) - posX.get(0) % 10 + 10;
//
//
//
////        //Y에 관해 정렬
////        Comparator<ParkingPoint> cmpAscY = new Comparator<ParkingPoint>() {
////
////            @Override
////            public int compare(ParkingPoint o1, ParkingPoint o2) {
////                return o1.getStartY() - o2.getStartY();
////            }
////        };
////        Collections.sort(parkingPoint, cmpAscY);
////        //중복(+-10% 외에 것들만 posY에 추가)제거
////        for(int i = 0; i<parkingPoint.size(); i++){
////            if(i == 0){
////                posY.add(parkingPoint.get(i).getStartY());
////            }
////            else{
////                boolean check = false;
////                for(int j = 0; j<posY.size(); j++){
////                    if(posY.get(j) < parkingPoint.get(i).getStartY()+20 && posY.get(j) > parkingPoint.get(i).getStartY()-20){
////                        check = true;
////                        break;
////                    }
////                }
////                if(!check)
////                    posY.add(parkingPoint.get(i).getStartY());
////            }
////        }
//        min = 999;
//        for(int i = 0; i<posY.size()-1; i++){
//            if(posY.get(i+1) - posY.get(i) < min)
//                min = posY.get(i+1) - posY.get(i);
//        }
//        rposY.add(1);
//        //오차 10%이상 나면 중간에 길 삽입된거임
//        for(int i = 0; i<posY.size()-1; i++){
//            int dif = posY.get(i+1) - posY.get(i);
//            if(min < dif * 1.1 && min > dif*0.9){
//                rposY.add(1);
//                rposY.add(1);
//            }
//            else{
//                rposY.add(0);
//            }
//        }
//        rposY.add(1);
//        if(posY.get(0) % 10 < 5)
//            startY = posY.get(0) - posY.get(0) % 10;
//        else
//            startY = posY.get(0) - posY.get(0) % 10 + 10;
//
//
//        //x인덱스
//        for(int i = 0; i<rposX.size(); i++)
//            if(rposX.get(i) == 1)
//                maxIndexX++;
//        //y인덱스
//        for(int i = 0; i<rposY.size(); i++)
//            if(rposY.get(i) == 1)
//                maxIndexY++;
//
//        for(int i = 0; i<posX.size(); i++){
//            if(posX.get(i) % 10 < 5)
//                posX.set(i, posX.get(i) - posX.get(i) % 10);
//            else
//                posX.set(i, posX.get(i) - posX.get(i) % 10 + 10);
//        }
//        for(int i = 0; i<posY.size(); i++){
//            if(posY.get(i) % 10 < 5)
//                posY.set(i, posY.get(i) - posY.get(i) % 10);
//            else
//                posY.set(i, posY.get(i) - posY.get(i) % 10 + 10);
//        }
//
//        lenX = ((width-fragLeft-fragRight)*pWidth) / maxWidth;
//        lenY = ((height-fragTop-fragBottom)*pHeight) / maxHeight;
//        posX = posX;
//        posY = posY;
//        rposX = rposX;
//        rposY = rposY;
//        startX = startX;
//        startY = startY;
//
//
//        SetCrossPoint();
//
//    }
//
//
//    private void SetCrossPoint(){
//        ArrayList<Integer> x = new ArrayList<>();
//        ArrayList<Integer> y = new ArrayList<>();
//        int count= 0;
//        for(int i = 0; i<rposX.size(); i++) {
//            if (rposX.get(i) == 0) {
//                x.add(i - count);
//                count++;
//            }
//        }
//        count = 0;
//        for(int i = 0; i<rposY.size(); i++) {
//            if (rposY.get(i) == 0) {
//                y.add(i - count);
//                count++;
//            }
//        }
//        for(int i = 0; i<x.size(); i++)
//            for(int j = 0; j<y.size(); j++)
//                crossPoint.add(new CrossPoint(new Point(
//                        ChangeRate(0, (posX.get(x.get(i)) + posX.get(x.get(i) - 1) + pWidth) / 2),
//                                ChangeRate(1, (posY.get(y.get(j)) + posY.get(y.get(j) - 1) + pHeight) / 2)),
//                        ChangeRate(0, posX.get(x.get(i)-1) + pWidth),
//                        ChangeRate(1, posY.get(y.get(j)-1) + pHeight),
//                        ChangeRateNo(0, posX.get(x.get(i)) - (posX.get(x.get(i) - 1) + pWidth)),
//                        ChangeRateNo(1, posY.get(y.get(j)) - (posY.get(y.get(j) - 1) + pHeight))
//                ));
//     crossPoint = crossPoint;
//    }
//
//
//    private void Reservation(int index){
//        if(index != 0){
//            reserv = index;
//            SetParkingPointReservation a = new SetParkingPointReservation();
//            a.setOccupy(1);
//            a.setTargetPoint(index);
//            a.setTargetID(ID);
//            a.execute();
//            return;
//        }
//        double min = 9999;
//        parkingPoint = parkingPoint;
//        for(int i = 0; i<parkingPoint.size(); i++){
//            if(parkingPoint.get(i).occupy == 0){
//                double dis = Distance(new Point(dCarX, dCarY), new Point(parkingPoint.get(i).startX, parkingPoint.get(i).startY));
//                dis = dis;
//                if(min > dis){
//                    min = dis;
//                    reserv = i;
//                }
//            }
//        }
//        tvList = tvList;
//        for(int i = 0; i<tvList.size(); i++){
//            if(tvList.get(i).getHint().toString().substring(1).equals(Integer.toString(reserv))){
//                tvList.get(i).setBackgroundResource(R.drawable.edge_myreversation);
//                tvList.get(i).setHint(Integer.toString(2) + tvList.get(i).getHint().toString().substring(1));
//                reserv = reserv;
//                if(tv_click != null){
//                    tv_click.setBackgroundResource(R.drawable.edge_empty);
//                    tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
//                    SetParkingPointReservation a = new SetParkingPointReservation();
//                    a.setOccupy(0);
//                    a.setTargetPoint(Integer.parseInt(tv_click.getHint().toString().substring(1)));
//                    a.execute();
//                }
//                tv_click = tvList.get(i);
//                SetParkingPointReservation a = new SetParkingPointReservation();
//                a.setOccupy(1);
//                a.setTargetPoint(reserv);
//                a.setTargetID(ID);
//                a.execute();
//                return;
//            }
//        }
//
//    }
//
//    private int NearCrossPoint(Point p1){
//
//        int index = 0;
//        double dis = 9999;
//        for(int i = 0; i<crossPoint.size(); i++){
//            double distance = Math.sqrt((Math.pow(p1.x-crossPoint.get(i).getPoint().x, 2) + Math.pow(p1.y-crossPoint.get(i).getPoint().y, 2)));
//            if(distance < dis){
//                index = i;
//                dis = distance;
//            }
//        }
//        return index;
//    }
//    private double Distance(Point p1, Point p2){
//
//        return Math.sqrt((Math.pow(p1.x-p2.x, 2) + Math.pow(p1.y-p2.y, 2)));
//    }
//    private int DistanceInt(Point p1, Point p2){
//
//        return (int)(Math.sqrt((Math.pow(p1.x-p2.x, 2) + Math.pow(p1.y-p2.y, 2))));
//    }
//
//    public int ChangeRate(int num, int change){
//        //num = 0 x
//        //num = 1 y
//        if(num == 0)
//            return ((width-fragLeft-fragRight)*change)/maxWidth + fragLeft;
//         else
//            return ((height-fragTop-fragBottom)*change)/maxHeight + fragTop;
//
//    }
//    public int ChangeRateNo(int num, int change){
//        //num = 0 x
//        //num = 1 y
//        if(num == 0)
//            return ((width-fragLeft-fragRight)*change)/maxWidth;
//        else
//            return ((height-fragTop-fragBottom)*change)/maxHeight;
//
//    }
//
//    int posx = 150;
//    int posy = 0;
//    class test extends AsyncTask<Void, Void, String> {
//        String target;
//        @Override
//        protected void onPreExecute(){//연결
//            try{
//                target = "http://nejoo97.cafe24.com/UserMove.php?ID="+ID + "&posx="+posx + "&posy="+posy;
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//
//        @Override
//        protected String doInBackground(Void... voids) {//데이터읽어오기
//            try{
//                URL url = new URL(target);
//                HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
//                InputStream inputStream = httpURLConnection.getInputStream();
//                BufferedReader bufferReader = new BufferedReader(new InputStreamReader(inputStream));
//                String temp;
//                StringBuilder stringBuilder = new StringBuilder();
//                while((temp = bufferReader.readLine())!=null){
//                    stringBuilder.append(temp+"\n");
//                }
//                bufferReader.close();
//                inputStream.close();
//                httpURLConnection.disconnect();
//                return stringBuilder.toString().trim();
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//            return null;
//        }
//
//
//        @Override
//        public void onProgressUpdate(Void... values){
//            super.onProgressUpdate();
//        }
//
//        @Override
//        public void onPostExecute(String result){//공지사항리스트에 연결
//            try{
//            }
//            catch(Exception e){
//                e.printStackTrace();
//            }
//        }
//    }
//
//
//
//    private int ApproachX(int n){
//        int index = 0;
//        int min = 99999;
//        for(int i = 0; i<posX.size(); i++){
//            int num = Math.abs(n-posX.get(i));
//            if(min > num){
//                min = num;
//                index = i;
//            }
//        }
//        return index;
//    }
//    private int ApproachY(int n){
//        int index = 0;
//        int min = 99999;
//        for(int i = 0; i<posY.size(); i++){
//            int num = Math.abs(n-posY.get(i));
//            if(min > num){
//                min = num;
//                index = i;
//            }
//        }
//        return index;
//    }
//}
