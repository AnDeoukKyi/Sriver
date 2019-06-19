package company.co.kr.sriverforuser;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Point;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Display;
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
import java.util.List;


public class MainActivity extends AppCompatActivity {

    List<ParkingPoint> parkingPoint;
    ArrayList<Integer> posX = new ArrayList<>();
    ArrayList<Integer> posY = new ArrayList<>();

    String ID = "a";
    int width;
    int height;
    int fragTop = 200;
    int fragBottom = 200;
    int fragLeft = 100;
    int fragRight = 100;
    int maxWidth, maxHeight;

    int totalStartX = 0, totalStartY = 0, totalLenX = 0, totalLenY = 0;
    int startX, startY, lenX, lenY;
    int maxIndexX = 0, maxIndexY = 0;
    int carX = 100, carY = 300;
    int pWidth, pHeight;
    int reserv = 0;
    ArrayList<Integer> rposX = new ArrayList<>();
    ArrayList<Integer> rposY = new ArrayList<>();
    ArrayList<TextView> tvList = new ArrayList<>();
    static public ArrayList<Point> Path = new ArrayList<>();
    static public ArrayList<Point> crossPoint = new ArrayList<>();

    RelativeLayout topLL;
    TextView tv_click = null;

    Dijkstra dijkstra;

    ImageView im;
    Context context;


    private ReservationDialog reservationDialog;


    int c = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        context = this;

        parkingPoint = new ArrayList<>();
        new ParkingPointLoad().execute();

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        width = size.x;
        height = size.y;
        dijkstra = new Dijkstra();
        topLL = (RelativeLayout)findViewById(R.id.layout_Main);

        new UserPosLoad().execute();

        Thread myThread = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {
                        Thread.sleep(1000);
                        new UserPosLoad().execute();
                        c+=20;
                        Path.set(0, new Point(0, c));
                        //데이터 받아와야됨
                    } catch (Throwable t) {
                    }
                }
            }
        });

        myThread.start();
    }

    void Print(){
        for(int i = 0; i<parkingPoint.size(); i++) {
            int point = parkingPoint.get(i).point - 1;//번호
            int occupy = parkingPoint.get(i).occupy;//점유
            int startX = parkingPoint.get(i).startX;//x
            int startY = parkingPoint.get(i).startY;//y
            final TextView tv = new TextView(MainActivity.this);

            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(lenX, lenY);
            if(im == null){
                im = new ImageView(MainActivity.this);
            }
            int x = point % maxIndexX;
            int y = 0;
            if(point != 0)
                y = point / maxIndexX;
            int countX = 0;//x위치조정용
            int countY = 0;//y위치조정용


            int count = 0;
            for(int j = 0; j<rposX.size(); j++){
                if(rposX.get(j) == 1){
                    count++;
                    if(count == x + 1) break;
                    countX++;
                }
                else
                    countX++;
            }
            count = 0;
            for(int j = 0; j<rposY.size(); j++){
                if(rposY.get(j) == 1){
                    count++;
                    if(count == y + 1) break;
                    countY++;
                }
                else
                    countY++;
            }
            int px = posX.get(point % posX.size());
            int py;
            if(point == 0)
                py = posY.get(0);
            else
                py = posY.get(point / posX.size());

            layoutParams.setMargins((px *(width-fragLeft-fragRight))/maxWidth + fragLeft, (py *(height-fragTop-fragBottom))/maxHeight + fragTop, 0, 0);
            tv.setLayoutParams(layoutParams);
            tv.setHint(Integer.toString(occupy) + Integer.toString(point));
            tv.setHintTextColor(Color.alpha(0));
            tv.setTextSize(30);
            switch(occupy){
                case 0:
                    tv.setBackgroundResource(R.drawable.edge_empty);
                    break;
                case 1:
                    tv.setBackgroundResource(R.drawable.edge_occupy);
                    break;
                case 2:
                    tv.setBackgroundResource(R.drawable.edge_reversation);
                    break;
            }


            tv.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    String a = tv.getHint().toString();
                    a = a;
                    reserv = reserv;
                    if(tv.getHint().toString().substring(0, 1).equals("0")
                            && !tv.getHint().toString().substring(1).equals(Integer.toString(reserv))){



                        //Dialog
                        reservationDialog = new ReservationDialog(context, tv, plusBillDialog_OkClickListener, plusBillDialog_CancelClickListener);
                        reservationDialog.setCancelable(true);
                        reservationDialog.getWindow().setGravity(Gravity.CENTER);
                        reservationDialog.show();
//                    }
//
//
//
//                    if(Integer.parseInt(tv.getHint().toString().substring(0, 1))  == 0){
//                        tv.setBackgroundResource(R.drawable.edge_myreversation);
//                        tv.setHint(Integer.toString(2) + tv.getHint().toString().substring(1));
//                        //sql수정해야함(occupy)
//                        if(tv_click != null){
//                            tv_click.setBackgroundResource(R.drawable.edge_empty);
//                            tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
//                        }
//                        tv_click = tv;
//
//                    }
//                    else if (tv == tv_click){//본인 예약해제
//                        tv_click.setBackgroundResource(R.drawable.edge_empty);
//                        tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
//                        tv_click = null;
//                        //sql수정해야함(occupy)
                    }
                }
            });



            //topTV1.setId(R.id.layout_Main + i);
            //topTV1.setBackgroundColor(Color.parseColor("#000000"));
            topLL.addView(tv);
            tvList.add(tv);//tv리스트에 추가
        }
    }

    private View.OnClickListener plusBillDialog_OkClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            TextView tv = reservationDialog.getTextView();

            tv.setBackgroundResource(R.drawable.edge_myreversation);
            tv.setHint(Integer.toString(2) + tv.getHint().toString().substring(1));
            if(tv_click != null){
                tv_click.setBackgroundResource(R.drawable.edge_empty);
                tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));

                SetParkingPointReservation a = new SetParkingPointReservation();
                a.setOccupy(0);
                a.setTargetPoint(Integer.parseInt(tv_click.getHint().toString().substring(1)));
                a.execute();
            }
            tv_click = tv;
            reserv = Integer.parseInt(tv.getHint().toString().substring(1));


            SetParkingPointReservation a = new SetParkingPointReservation();
            a.setOccupy(1);
            a.setTargetPoint(reserv);
            a.setTargetID(ID);
            a.execute();

            reservationDialog.dismiss();
        }
    };

    private View.OnClickListener plusBillDialog_CancelClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            reservationDialog.dismiss();
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
                Reservation(0);
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
                    count++;
                }
                dijkstra.INIT(5);
                dijkstra.CONNECT(0, 1, 6);
                dijkstra.CONNECT(0, 2, 2);
                dijkstra.CONNECT(1, 2, 3);
                dijkstra.CONNECT(2, 3, 3);
                dijkstra.CONNECT(1, 3, 6);
                dijkstra.CONNECT(1, 4, 4);
                dijkstra.CONNECT(3, 4, 11);
                PathToPoint(dijkstra.SEARCHPATH());

                int dCarX = ((width-fragLeft-fragRight)*carX)/maxWidth;
                int dCarY = ((height-fragTop-fragBottom)*carY)/maxHeight;

                RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
                layoutParams.setMargins(dCarX+fragLeft, dCarY+fragTop, 0, 0);//차량좌표 제대로해야됨
                im.setLayoutParams(layoutParams);
                im.setImageResource(R.drawable.ic_car);
                topLL.removeView(im);
                topLL.addView(im);

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
        Path.clear();
        Path.add(new Point(0, 0));
        Path.add(new Point(300, 300));
        Path.add(new Point(400, 700));
        Path.add(new Point(500, 800));
        Path.add(new Point(600, 900));
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
                    if(posX.get(j) < parkingPoint.get(i).getStartX()+10 && posX.get(j) > parkingPoint.get(i).getStartX()-10){
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
                    if(posY.get(j) < parkingPoint.get(i).getStartY()+10 && posY.get(j) > parkingPoint.get(i).getStartY()-10){
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
        for(int i = 0; i<rposX.size(); i++)
            if(rposX.get(i) == 0){
                x.add(i-count);
                count++;
            }

        count = 0;
        for(int i = 0; i<rposY.size(); i++)
            if(rposY.get(i) == 0){
                y.add(i-count);
                count++;
            }

        y = y;
        x = x;
        for(int i = 0; i<x.size(); i++)
            for(int j = 0; j<y.size(); j++)
                crossPoint.add(new Point(((width-fragLeft-fragRight)*(posX.get(x.get(i)) + posX.get(x.get(i) - 1) + pWidth) / 2)/maxWidth + fragLeft, ((height-fragTop-fragBottom)*(posY.get(y.get(j)) + posY.get(y.get(j) - 1) + pHeight) / 2)/maxHeight + fragTop));
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
                double dis = NearPoint(new Point(carX, carY), new Point(parkingPoint.get(i).startX, parkingPoint.get(i).startY));
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

    private double NearPoint(Point p1, Point p2){

        return Math.sqrt((Math.pow(p1.x-p2.x, 2) + Math.pow(p1.y-p2.y, 2)));
    }


    public boolean alreadyIn(List<Integer> courseIDList, int item){
        for(int i = 0; i<courseIDList.size(); i++){
            if(courseIDList.get(i) == item){
                return false;
            }
        }
        return true;
    }
}
