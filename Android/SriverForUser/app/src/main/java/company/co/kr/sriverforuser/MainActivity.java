package company.co.kr.sriverforuser;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Point;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Display;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageButton;
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
import java.util.List;


public class MainActivity extends AppCompatActivity {

    List<ParkingPoint> parkingPoint;
    ArrayList<Integer> posX = new ArrayList<>();
    ArrayList<Integer> posY = new ArrayList<>();

    String ID;
    int width;
    int height;
    int fragTop = 100;
    int fragBottom = 100;
    int fragLeft = 10;
    int fragRight = 10;
    int maxWidth, maxHeight;
    int kx = 0, ky = 0;
    int totalStartX = 0, totalStartY = 0, totalLenX = 0, totalLenY = 0;
    int startX, startY, lenX, lenY;
    int maxIndexX = 0, maxIndexY = 0;
    int carX = 100, carY = 300;
    int dCarX = 0, dCarY = 0;
    int ddCarX = 0, ddCarY = 0;
    int revx = 0, revy = 0, revHy = 0;
    int dir = 0;//0=up, 1=right, 2=down, 3=left
    int pWidth, pHeight;
    int reserv = -1;
    ArrayList<Integer> rposX = new ArrayList<>();
    ArrayList<Integer> rposY = new ArrayList<>();
    ArrayList<TextView> tvList = new ArrayList<>();
    static public ArrayList<Point> Path = new ArrayList<>();
    static public ArrayList<CrossPoint> crossPoint = new ArrayList<>();

    RelativeLayout topLL;
    TextView tv_click = null;

    Dijkstra dijkstra;

    ImageView im;
    Context context;


    private ReservationDialog reservationDialog;
    private ParkingDialog parkingDialog;




    int check  = 0;
    boolean stop = true;
    boolean park = false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        context = this;
        ID = getIntent().getStringExtra("ID");
        parkingPoint = new ArrayList<>();
        new ParkingPointLoad().execute();//주차칸 내부 정보 수정됬을시 추가해해야됨/////////////////////////////////

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        width = size.x;
        height = size.y;

        topLL = (RelativeLayout)findViewById(R.id.layout_Main);
        ImageButton ib_userProperty = (ImageButton)findViewById(R.id.ib_userProperty);


        ib_userProperty.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this, UserProperty.class));
            }
        });
        new UserPosLoad().execute();

        Thread myThread = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {

                        //posy+=10;
                        //new test().execute();
                        new UserPosLoad().execute();
                        new ParkingPointSync().execute();
                        Thread.sleep(100);
                        runOnUiThread(new Runnable(){
                            @Override
                            public void run() {
                                CheckPark();
                                reserv = reserv;
                                ChangeParkingPoint();
                            }
                        });
                        //차량이 주차완료됬는지 체크해야됨


                        //데이터 받아와야됨
                    } catch (Throwable t) {
                    }
                }
            }
        });

        myThread.start();
    }


    void ChangeParkingPoint(){
        tvList = tvList;
        for(int i = 0; i<tvList.size(); i++){
            for(int j = 0; j<parkingPoint.size(); j++){
                if(parkingPoint.get(j).point == Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))){
                    tvList.get(i).setText(parkingPoint.get(j).occupy+tvList.get(i).getText().toString().substring(1));
                    break;
                }
            }
        }
        for(int j = 0; j<tvList.size(); j++){
            switch(Integer.parseInt(tvList.get(j).getText().toString().substring(0, 1))){
                case 0://empty
                    tvList.get(j).setBackgroundResource(R.drawable.edge_empty);
                    break;
                case 1://reservation
                    if(Integer.parseInt(tvList.get(j).getText().toString().substring(1, 3)) == reserv)
                        if(stop)
                            tvList.get(j).setBackgroundResource(R.drawable.edge_myparking);
                        else
                            tvList.get(j).setBackgroundResource(R.drawable.edge_myreversation);
                    else
                        tvList.get(j).setBackgroundResource(R.drawable.edge_noparking);
                    break;
                case 2://parking
                    if(Integer.parseInt(tvList.get(j).getText().toString().substring(1, 3)) == reserv)
                        tvList.get(j).setBackgroundResource(R.drawable.edge_myparking);
                    else
                        tvList.get(j).setBackgroundResource(R.drawable.edge_noparking);
                    break;
            }
        }


    }
    void CheckPark(){

        if(stop) return;
        TextView target_tv = null;
        reserv = reserv;
        for(int i = 0; i<tvList.size(); i++){
            if(Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3)) == reserv){
                target_tv = tvList.get(i);
                break;
            }
        }
        target_tv = target_tv;

        if(check >= 50){
            stop = true;
            //다이얼로그 생성

            parkingDialog = new ParkingDialog(context, parkingDialog_OkClickListener);
            parkingDialog.setCancelable(true);
            parkingDialog.getWindow().setGravity(Gravity.CENTER);
            parkingDialog.show();




            //path삭제
            return;
        }
        if(target_tv == null) return;
        String[] tmp = target_tv.getHint().toString().split("/");
        kx = (Integer.parseInt(tmp[0])*(width-fragLeft-fragRight))/maxWidth + fragLeft;
        ky = (Integer.parseInt(tmp[1])*(height-fragTop-fragBottom))/maxHeight + fragTop;
        if(dCarX > kx && dCarX < kx + lenX && dCarY > ky && dCarY < ky + lenY)//차량위치
            check++;
        else
            check = 0;
    }



    private View.OnClickListener parkingDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            parkingDialog.dismiss();
            SetParkingPointReservation a = new SetParkingPointReservation();
            a.setOccupy(2);
            a.setTargetPoint(reserv);
            a.setTargetID(ID);
            a.execute();
        }
    };

    void Print(){
        posX = posX;

        int c = 0;
        for(int k = 0; k<rposX.size(); k++)
            if(rposX.get(k) == 1) c++;
        c = c;
        rposY =rposY;

        parkingPoint = parkingPoint;
        for(int i = 0; i<parkingPoint.size(); i++) {
            int pointp = parkingPoint.get(i).point - 1;//번호
            int occupy = parkingPoint.get(i).occupy;//점유
            int startX = parkingPoint.get(i).startX;//x
            int startY = parkingPoint.get(i).startY;//y
            final TextView tv = new TextView(MainActivity.this);

            //point=> 안드로이드에서 드로우할 위치계산용
            //카메라에서 보내는 index(point)

            int xi = ApproachX(startX);
            int yi = ApproachY(startY);
            int point = xi + yi *c;
            point = point;
            parkingPoint.get(i).point = point;
            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(lenX, lenY);
            if(im == null){
                im = new ImageView(MainActivity.this);
            }
            int x = point % maxIndexX;
            int y = 0;
            if(point != 0)
                y = point / maxIndexX;
            int px = posX.get(point % posX.size());
            int py;
            if(point == 0)
                py = posY.get(0);
            else
                py = posY.get(point / posX.size());
            parkingPoint.get(i).index = point;
            layoutParams.setMargins((px *(width-fragLeft-fragRight))/maxWidth + fragLeft, (py *(height-fragTop-fragBottom))/maxHeight + fragTop, 0, 0);
            tv.setLayoutParams(layoutParams);
            if(point<9)
                tv.setHint(Integer.toString(occupy) + "0" + Integer.toString(point+1));
            else
                tv.setHint(Integer.toString(occupy) + Integer.toString(point+1));
            if(i<9)
                tv.setText(tv.getHint().toString()+"0"+Integer.toString(i+1));
            else
                tv.setText(tv.getHint().toString()+Integer.toString(i+1));
            //tv.setTextColor(Color.alpha(0));
            tv.setHint(Integer.toString(px) + "/" + Integer.toString(py));
            tv.setTextSize(10);
            switch(occupy){
                case 0://empty
                    tv.setBackgroundResource(R.drawable.edge_empty);
                    break;
                case 1://reservation
                    if(Integer.parseInt(tv.getText().toString().substring(1, 3)) == reserv)
                        tv.setBackgroundResource(R.drawable.edge_myreversation);
                    else
                        tv.setBackgroundResource(R.drawable.edge_noparking);
                    break;
                case 2://parking
                    if(Integer.parseInt(tv.getText().toString().substring(1, 3)) == reserv)
                        tv.setBackgroundResource(R.drawable.edge_myparking);
                    else
                        tv.setBackgroundResource(R.drawable.edge_noparking);
                    break;
            }


            tv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if((tv.getText().toString().substring(0, 1).equals("0")
                            && Integer.parseInt(tv.getText().toString().substring(1, 3))!=reserv) || reserv == -1){



                        //Dialog
                        reservationDialog = new ReservationDialog(context, tv, reservationDialog_OkClickListener, reservationDialog_CancelClickListener);
                        reservationDialog.setCancelable(true);
                        reservationDialog.getWindow().setGravity(Gravity.CENTER);
                        reservationDialog.show();

                    }
                }
            });



            //topTV1.setId(R.id.layout_Main + i);
            //topTV1.setBackgroundColor(Color.parseColor("#000000"));
            topLL.addView(tv);
            tvList.add(tv);//tv리스트에 추가
        }
        tvList = tvList;
    }

    private View.OnClickListener reservationDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            TextView tv = reservationDialog.getTextView();

            //무조건 바뀌면 안됨
            if(tv_click != null){
                tv_click.setBackgroundResource(R.drawable.edge_empty);
                tv_click.setText(Integer.toString(0) + tv_click.getText().toString().substring(1));

                SetParkingPointReservation a = new SetParkingPointReservation();
                a.setOccupy(0);
                a.setTargetPoint(Integer.parseInt(tv_click.getText().toString().substring(1, 3)));
                a.execute();
            }
            tv_click = tv;
            reserv = Integer.parseInt(tv.getText().toString().substring(1, 3));


            SetParkingPointReservation a = new SetParkingPointReservation();
            a.setOccupy(1);
            a.setTargetPoint(reserv);
            a.setTargetID(ID);
            a.execute();

            reservationDialog.dismiss();


            stop = false;
            check = 0;
        }
    };

    private View.OnClickListener reservationDialog_CancelClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            reservationDialog.dismiss();
            park = true;
        }
    };

    class ParkingPointLoad extends AsyncTask<Void, Void, String> {
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

                int point, occupy, startX, startY;
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    point = object.getInt("POINT");
                    occupy = object.getInt("OCCUPY");
                    startX = object.getInt("STARTX");
                    startY = object.getInt("STARTY");
                    if(count == 0) {
                        maxWidth = startX;
                        maxHeight = startY;
                    }
                    else {
                        if(count != jsonArray.length()-1)
                            parkingPoint.add(new ParkingPoint(point, occupy, startX, startY));
                        else{
                            pWidth = startX;
                            pHeight = startY;
                        }

                    }
                    count++;
                }
                SortParkingPoint();
                Print();//출력함수
                //Reservation(0);
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }


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

                int point, occupy;
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    point = object.getInt("POINT");
                    occupy = object.getInt("OCCUPY");
                    if(count != 0) {
                        if(count != jsonArray.length()-1){
                            for(int i = 0; i<parkingPoint.size(); i++){
                                if(parkingPoint.get(i).point == point){
                                    if(parkingPoint.get(i).occupy != occupy){
                                        parkingPoint.get(i).occupy = occupy;
                                    }
                                }
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
    }


    class UserPosLoad extends AsyncTask<Void, Void, String> {
        String target;
        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/UserPosGet.php?ID="+ID;//ID추가해야됨
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
                    carX = object.getInt("POSX");
                    carY = object.getInt("POSY");
                    dir = object.getInt("DIR");
                    count++;
                }


                //차량이 화면내부에 없으면 return

                if(!(carX >=0 && carX <= maxWidth && carY >= 0 && carY <= maxHeight)){

                    if(im != null) im.setVisibility(View.INVISIBLE);
                    return;
                }

                if(im != null) im.setVisibility(View.VISIBLE);


                int point = 0;
                reserv = reserv;
                for(int i =0; i<tvList.size(); i++){
                    if(Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))==reserv){
                        point = Integer.parseInt(tvList.get(i).getText().toString().substring(1, 3))-1;
                        break;
                    }
                }
                int x = point % maxIndexX;
                int y = 0;
                if(point != 0)
                    y = point / maxIndexX;
                int px = posX.get(point % posX.size());
                int py;
                if(point == 0)
                    py = posY.get(0);
                else
                    py = posY.get(point / posX.size());

                revx = ChangeRate(0, px + (posX.get(1)-posX.get(0))/2);
                revy = ChangeRate(1, py);
                revHy = crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y;
                dCarX = ChangeRate(0, carX);
                dCarY = ChangeRate(1, carY);
                Bitmap image = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_car);
                int w = 96;//image.getWidth();
                int h = 96;//image.getHeight();


                //dd위치 조정 배치에 따라서 수선
                if(
                        (dCarX >= crossPoint.get(0).startX && dCarX <= crossPoint.get(0).startX + crossPoint.get(0).getWidth())
                        || (dCarX >= crossPoint.get(3).startX && dCarX <= crossPoint.get(3).startX + crossPoint.get(3).getWidth())){
                        if(
                                (dCarY >= crossPoint.get(0).startY && dCarY <= crossPoint.get(0).startY + crossPoint.get(0).getHeight())
                                        || (dCarY >= crossPoint.get(3).startX && dCarY <= crossPoint.get(3).startY + crossPoint.get(3).getHeight())){


                            ddCarX = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().x;
                            ddCarY = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().y;
                        }
                        else{
                            ddCarX = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().x;
                            ddCarY = dCarY;
                        }
                }
                else{
                    ddCarX = dCarX;
                    ddCarY = crossPoint.get(NearCrossPoint(new Point(dCarX, dCarY))).getPoint().y;
                }
                ddCarX = ddCarX;
                ddCarY = ddCarY;
                RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
                layoutParams.setMargins(dCarX-w/2, dCarY-h/2, 0, 0);//차량좌표 제대로해야됨

                im.setLayoutParams(layoutParams);
                im.setImageResource(R.drawable.ic_car);
                topLL.removeView(im);
                topLL.addView(im);



                //맨처음 시작할때
                //주차 완료시

                dCarX = dCarX;
                dCarY = dCarY;
                ddCarX = ddCarX;
                ddCarY = ddCarY;
                if (stop || (dCarX >= kx && dCarX <= kx + lenX && dCarY >= ky && dCarY <= ky + lenY)){//길찾기 그리는부분
                    Path.clear();
                    return;
                }




                dijkstra = new Dijkstra();
                dijkstra.INIT(6);
                if(DistanceInt(new Point(ddCarX, ddCarY), new Point(revx, revHy))< ChangeRateNo(0, pWidth * 2)){

                    dijkstra.CONNECT(0, 5, DistanceInt(new Point(ddCarX, ddCarY), new Point(revx, revy)));
                    PathToPoint(dijkstra.SEARCHPATH());
                    return;
                }
                dijkstra.CONNECT(1, 2, DistanceInt(crossPoint.get(0).getPoint(), crossPoint.get(2).getPoint()));
                dijkstra.CONNECT(2, 3, DistanceInt(crossPoint.get(2).getPoint(), crossPoint.get(3).getPoint()));
                dijkstra.CONNECT(3, 4, DistanceInt(crossPoint.get(1).getPoint(), crossPoint.get(3).getPoint()));
                dijkstra.CONNECT(4, 1, DistanceInt(crossPoint.get(1).getPoint(), crossPoint.get(0).getPoint()));

                if(revx >= crossPoint.get(0).startX  + crossPoint.get(0).getWidth() && revx <= crossPoint.get(3).startX){
                    //예약지점이 y기준으로 있을때
                    //y가 0,2쪽인지 1,3쪽인지 판단해야됨
                    if(crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y > height/2) {
                        //밑(1,3)
                        dijkstra.DONNECT(3, 4);
                        dijkstra.CONNECT(5, 3, DistanceInt(new Point(revx, revHy), crossPoint.get(3).getPoint()));
                        dijkstra.CONNECT(5, 4, DistanceInt(new Point(revx, revHy), crossPoint.get(1).getPoint()));
                    }
                    else{
                        //위 (0,2)
                        dijkstra.DONNECT(1, 2);
                        dijkstra.CONNECT(5, 1, DistanceInt(new Point(revx, revHy), crossPoint.get(0).getPoint()));
                        dijkstra.CONNECT(5, 2, DistanceInt(new Point(revx, revHy), crossPoint.get(2).getPoint()));

                    }
                }
                else{
                    //바깥쪽일때는 그냥 추가하면됨
                    int dot = 0;
                    switch(NearCrossPoint(new Point(revx, revHy))){
                        case 0:
                            dot = 1;
                            break;
                        case 1:
                            dot = 4;
                            break;
                        case 2:
                            dot = 2;
                            break;
                        case 3:
                            dot = 3;
                            break;
                    }
                    dijkstra.CONNECT(5, dot, DistanceInt(new Point(revx, revHy), crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint()));

                }
                if(
                        (ddCarX >= crossPoint.get(0).startX + crossPoint.get(0).getWidth() && ddCarX <= crossPoint.get(3).startX)
                                || (ddCarY >= crossPoint.get(0).startY + crossPoint.get(0).getHeight() && ddCarY <= crossPoint.get(3).startY)
                        ){

                    if (ddCarX >= crossPoint.get(0).startX + crossPoint.get(0).getWidth() && ddCarX <= crossPoint.get(3).startX){
                        if(ddCarY == crossPoint.get(0).getPoint().y){
                            dijkstra.DONNECT(1, 2);
                            dijkstra.CONNECT(0, 1, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(0).getPoint()));
                            dijkstra.CONNECT(0, 2, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(2).getPoint()));

                            if(revx > ddCarX)
                                dijkstra.DONNECT(0, 1);
                            else
                                dijkstra.DONNECT(0, 2);
                        }
                        else if(ddCarY == crossPoint.get(3).getPoint().y){
                            dijkstra.DONNECT(3, 4);
                            dijkstra.CONNECT(0, 3, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(1).getPoint()));
                            dijkstra.CONNECT(0, 4, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(3).getPoint()));

                            if(revx > ddCarX)
                                dijkstra.DONNECT(0, 4);
                            else
                                dijkstra.DONNECT(0, 3);
                        }
                    }
                    else if(dCarY >= crossPoint.get(0).startY + crossPoint.get(0).getHeight() && dCarY <= crossPoint.get(3).startY){
                        if(ddCarX == crossPoint.get(0).getPoint().x){
                            dijkstra.DONNECT(1, 4);
                            dijkstra.CONNECT(0, 1, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(0).getPoint()));
                            dijkstra.CONNECT(0, 4, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(1).getPoint()));

                            if(revHy > dCarY)
                                dijkstra.DONNECT(0, 1);
                            else
                                dijkstra.DONNECT(0, 4);
                        }
                        else if(ddCarX == crossPoint.get(3).getPoint().x){
                            dijkstra.DONNECT(2, 3);
                            dijkstra.CONNECT(0, 2, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(2).getPoint()));
                            dijkstra.CONNECT(0, 3, DistanceInt(new Point(ddCarX, dCarY), crossPoint.get(3).getPoint()));

                            if(revHy > dCarY)
                                dijkstra.DONNECT(0, 2);
                            else
                                dijkstra.DONNECT(0, 3);
                        }
                    }
                }
                else{
                    int k = 1;
                    if(
                            (ddCarX >= crossPoint.get(0).startX && ddCarX <= crossPoint.get(2).startX + crossPoint.get(2).getWidth() )
                                    && (ddCarY >= crossPoint.get(0).startY && ddCarY <= crossPoint.get(2).startY + crossPoint.get(2).getHeight())
                            ){
                        int a = 10;
                        if(crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint().y != crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint().y) {
                            if (ddCarY > height / 2)
                                dijkstra.DONNECT(3, 4);
                            else
                                dijkstra.DONNECT(1, 2);
                        }
                    }
                    else{

                        if(crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint().y != crossPoint.get(NearCrossPoint(new Point(revx, revHy))).getPoint().y) {
                            if (ddCarY > height / 2)
                                dijkstra.DONNECT(3, 4);
                            else
                                dijkstra.DONNECT(1, 2);
                        }


                    }
                    int dot = 0;
                    switch(NearCrossPoint(new Point(ddCarX, ddCarY))){
                        case 0:
                            dot = 1;
                            break;
                        case 1:
                            dot = 4;
                            break;
                        case 2:
                            dot = 2;
                            break;
                        case 3:
                            dot = 3;
                            break;
                    }
                    dijkstra.CONNECT(0, dot, DistanceInt(new Point(ddCarX, ddCarY), crossPoint.get(NearCrossPoint(new Point(ddCarX, ddCarY))).getPoint()));

                }


                dijkstra = dijkstra;

                PathToPoint(dijkstra.SEARCHPATH());

            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }
    class SetParkingPointReservation extends AsyncTask<Void, Void, String> {
        String target;
        int occupy = 0;
        int targetPoint = 0;
        String targetID = "NULL";

        public void setTargetID(String str){
            this.targetID = str;
        }
        public void setTargetPoint(int n){
            this.targetPoint = n;
        }
        public void setOccupy(int n){
            this.occupy = n;
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
        public void onPostExecute(String result){//공지사항리스트에 연결
            try{
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }



    private void PathToPoint(ArrayList<Integer> p){
        p = p;
        Path.clear();
        Path.add(new Point(dCarX, dCarY));
        Path.add(new Point(ddCarX, ddCarY));
        crossPoint = crossPoint;
        for(int i = 0; i<p.size(); i++){
            if(p.get(i) >= 1 && p.get(i) <= 4){
                    int dot = 0;
                    switch(p.get(i)){
                        case 1:
                            dot = 0;
                            break;
                        case 2:
                            dot = 2;
                            break;
                        case 3:
                            dot = 3;
                            break;
                        case 4:
                            dot = 1;
                            break;
                    }
                Path.add(new Point(crossPoint.get(dot).getPoint().x, crossPoint.get(dot).getPoint().y));
            }
        }
        Path.add(new Point(revx, crossPoint.get(NearCrossPoint(new Point(revx, revy))).getPoint().y));
        Path.add(new Point(revx, revy));




    }


    private void SortParkingPoint(){
        //X에 관해 정렬
        Comparator<ParkingPoint> cmpAscX = new Comparator<ParkingPoint>() {

            @Override
            public int compare(ParkingPoint o1, ParkingPoint o2) {
                return o1.getStartX() - o2.getStartX();
            }
        };
        Collections.sort(parkingPoint, cmpAscX);
        //중복(+-10% 외에 것들만 posX에 추가)제거
        for(int i = 0; i<parkingPoint.size(); i++){
            if(i == 0){
                posX.add(parkingPoint.get(i).getStartX());
            }
            else{
                boolean check = false;
                for(int j = 0; j<posX.size(); j++){
                    if(posX.get(j) < parkingPoint.get(i).getStartX()+20 && posX.get(j) > parkingPoint.get(i).getStartX()-20){
                        check = true;
                        break;
                    }
                }
                if(!check)
                    posX.add(parkingPoint.get(i).getStartX());
            }
        }
        int min = 999;
        for(int i = 0; i<posX.size()-1; i++){
            if(posX.get(i+1) - posX.get(i) < min)
                min = posX.get(i+1) - posX.get(i);
        }
        posX = posX;
        rposX.add(1);
        //오차 10%이상 나면 중간에 길 삽입된거임
        for(int i = 0; i<posX.size()-1; i++){
            int dif = posX.get(i+1) - posX.get(i);
            if(min < dif * 1.1 && min > dif*0.9){
                rposX.add(1);
            }
            else{
                rposX.add(0);
                rposX.add(1);
            }
        }
        if(posX.get(0) % 10 < 5)
            startX = posX.get(0) - posX.get(0) % 10;
        else
            startX = posX.get(0) - posX.get(0) % 10 + 10;



        //Y에 관해 정렬
        Comparator<ParkingPoint> cmpAscY = new Comparator<ParkingPoint>() {

            @Override
            public int compare(ParkingPoint o1, ParkingPoint o2) {
                return o1.getStartY() - o2.getStartY();
            }
        };
        Collections.sort(parkingPoint, cmpAscY);
        //중복(+-10% 외에 것들만 posY에 추가)제거
        for(int i = 0; i<parkingPoint.size(); i++){
            if(i == 0){
                posY.add(parkingPoint.get(i).getStartY());
            }
            else{
                boolean check = false;
                for(int j = 0; j<posY.size(); j++){
                    if(posY.get(j) < parkingPoint.get(i).getStartY()+20 && posY.get(j) > parkingPoint.get(i).getStartY()-20){
                        check = true;
                        break;
                    }
                }
                if(!check)
                    posY.add(parkingPoint.get(i).getStartY());
            }
        }
        min = 999;
        for(int i = 0; i<posY.size()-1; i++){
            if(posY.get(i+1) - posY.get(i) < min)
                min = posY.get(i+1) - posY.get(i);
        }
        rposY.add(1);
        //오차 10%이상 나면 중간에 길 삽입된거임
        for(int i = 0; i<posY.size()-1; i++){
            int dif = posY.get(i+1) - posY.get(i);
            if(min < dif * 1.1 && min > dif*0.9){
                rposY.add(1);
                rposY.add(1);
            }
            else{
                rposY.add(0);
            }
        }
        rposY.add(1);
        if(posY.get(0) % 10 < 5)
            startY = posY.get(0) - posY.get(0) % 10;
        else
            startY = posY.get(0) - posY.get(0) % 10 + 10;


        //x인덱스
        for(int i = 0; i<rposX.size(); i++)
            if(rposX.get(i) == 1)
                maxIndexX++;
        //y인덱스
        for(int i = 0; i<rposY.size(); i++)
            if(rposY.get(i) == 1)
                maxIndexY++;

        for(int i = 0; i<posX.size(); i++){
            if(posX.get(i) % 10 < 5)
                posX.set(i, posX.get(i) - posX.get(i) % 10);
            else
                posX.set(i, posX.get(i) - posX.get(i) % 10 + 10);
        }
        for(int i = 0; i<posY.size(); i++){
            if(posY.get(i) % 10 < 5)
                posY.set(i, posY.get(i) - posY.get(i) % 10);
            else
                posY.set(i, posY.get(i) - posY.get(i) % 10 + 10);
        }

        lenX = ((width-fragLeft-fragRight)*pWidth) / maxWidth;
        lenY = ((height-fragTop-fragBottom)*pHeight) / maxHeight;
        posX = posX;
        posY = posY;
        rposX = rposX;
        rposY = rposY;
        startX = startX;
        startY = startY;


        SetCrossPoint();

    }


    private void SetCrossPoint(){
        ArrayList<Integer> x = new ArrayList<>();
        ArrayList<Integer> y = new ArrayList<>();
        int count= 0;
        for(int i = 0; i<rposX.size(); i++) {
            if (rposX.get(i) == 0) {
                x.add(i - count);
                count++;
            }
        }
        count = 0;
        for(int i = 0; i<rposY.size(); i++) {
            if (rposY.get(i) == 0) {
                y.add(i - count);
                count++;
            }
        }
        for(int i = 0; i<x.size(); i++)
            for(int j = 0; j<y.size(); j++)
                crossPoint.add(new CrossPoint(new Point(
                        ChangeRate(0, (posX.get(x.get(i)) + posX.get(x.get(i) - 1) + pWidth) / 2),
                                ChangeRate(1, (posY.get(y.get(j)) + posY.get(y.get(j) - 1) + pHeight) / 2)),
                        ChangeRate(0, posX.get(x.get(i)-1) + pWidth),
                        ChangeRate(1, posY.get(y.get(j)-1) + pHeight),
                        ChangeRateNo(0, posX.get(x.get(i)) - (posX.get(x.get(i) - 1) + pWidth)),
                        ChangeRateNo(1, posY.get(y.get(j)) - (posY.get(y.get(j) - 1) + pHeight))
                ));
     crossPoint = crossPoint;
    }


    private void Reservation(int index){
        if(index != 0){
            reserv = index;
            SetParkingPointReservation a = new SetParkingPointReservation();
            a.setOccupy(1);
            a.setTargetPoint(index);
            a.setTargetID(ID);
            a.execute();
            return;
        }
        double min = 9999;
        parkingPoint = parkingPoint;
        for(int i = 0; i<parkingPoint.size(); i++){
            if(parkingPoint.get(i).occupy == 0){
                double dis = Distance(new Point(dCarX, dCarY), new Point(parkingPoint.get(i).startX, parkingPoint.get(i).startY));
                dis = dis;
                if(min > dis){
                    min = dis;
                    reserv = i;
                }
            }
        }
        tvList = tvList;
        for(int i = 0; i<tvList.size(); i++){
            if(tvList.get(i).getHint().toString().substring(1).equals(Integer.toString(reserv))){
                tvList.get(i).setBackgroundResource(R.drawable.edge_myreversation);
                tvList.get(i).setHint(Integer.toString(2) + tvList.get(i).getHint().toString().substring(1));
                reserv = reserv;
                if(tv_click != null){
                    tv_click.setBackgroundResource(R.drawable.edge_empty);
                    tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
                    SetParkingPointReservation a = new SetParkingPointReservation();
                    a.setOccupy(0);
                    a.setTargetPoint(Integer.parseInt(tv_click.getHint().toString().substring(1)));
                    a.execute();
                }
                tv_click = tvList.get(i);
                SetParkingPointReservation a = new SetParkingPointReservation();
                a.setOccupy(1);
                a.setTargetPoint(reserv);
                a.setTargetID(ID);
                a.execute();
                return;
            }
        }

    }

    private int NearCrossPoint(Point p1){

        int index = 0;
        double dis = 9999;
        for(int i = 0; i<crossPoint.size(); i++){
            double distance = Math.sqrt((Math.pow(p1.x-crossPoint.get(i).getPoint().x, 2) + Math.pow(p1.y-crossPoint.get(i).getPoint().y, 2)));
            if(distance < dis){
                index = i;
                dis = distance;
            }
        }
        return index;
    }
    private double Distance(Point p1, Point p2){

        return Math.sqrt((Math.pow(p1.x-p2.x, 2) + Math.pow(p1.y-p2.y, 2)));
    }
    private int DistanceInt(Point p1, Point p2){

        return (int)(Math.sqrt((Math.pow(p1.x-p2.x, 2) + Math.pow(p1.y-p2.y, 2))));
    }

    public int ChangeRate(int num, int change){
        //num = 0 x
        //num = 1 y
        if(num == 0)
            return ((width-fragLeft-fragRight)*change)/maxWidth + fragLeft;
         else
            return ((height-fragTop-fragBottom)*change)/maxHeight + fragTop;

    }
    public int ChangeRateNo(int num, int change){
        //num = 0 x
        //num = 1 y
        if(num == 0)
            return ((width-fragLeft-fragRight)*change)/maxWidth;
        else
            return ((height-fragTop-fragBottom)*change)/maxHeight;

    }

    int posx = 150;
    int posy = 0;
    class test extends AsyncTask<Void, Void, String> {
        String target;
        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/UserMove.php?ID="+ID + "&posx="+posx + "&posy="+posy;
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
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }



    private int ApproachX(int n){
        int index = 0;
        int min = 99999;
        for(int i = 0; i<posX.size(); i++){
            int num = Math.abs(n-posX.get(i));
            if(min > num){
                min = num;
                index = i;
            }
        }
        return index;
    }
    private int ApproachY(int n){
        int index = 0;
        int min = 99999;
        for(int i = 0; i<posY.size(); i++){
            int num = Math.abs(n-posY.get(i));
            if(min > num){
                min = num;
                index = i;
            }
        }
        return index;
    }
}
