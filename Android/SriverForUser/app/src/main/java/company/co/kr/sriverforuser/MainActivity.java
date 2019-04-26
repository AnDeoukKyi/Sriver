package company.co.kr.sriverforuser;

import android.graphics.Color;
import android.graphics.Point;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Display;
import android.view.View;
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
import java.util.List;


public class MainActivity extends AppCompatActivity {


    List<ParkingPoint> parkingPoint;

    int width;
    int height;

    int totalStartX = 0, totalStartY = 0, totalLenX = 0, totalLenY = 0;
    RelativeLayout topLL;
    TextView tv_click = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        parkingPoint = new ArrayList<>();
        new ParkingPointLoad().execute();

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        width = size.x;
        height = size.y;

        topLL = (RelativeLayout)findViewById(R.id.layout_Main);

    }

    void Print(){
        int totalX = 0;
        int totalY = 0;
        for(int i = 0; i<parkingPoint.size(); i++) {
            if (i == 0) {//시작값(전체 사각형크기)
                totalStartX = parkingPoint.get(i).startX;//전체 시작위치x
                totalStartY = parkingPoint.get(i).startY;//전체 시작위치y
                totalLenX = parkingPoint.get(i).lenX;//전체 x길이
                totalLenY = parkingPoint.get(i).lenY;//전체 y길이
            }
            else {
                //occupy(1자리-0,1,2) + point(n자리)
                int point = parkingPoint.get(i).point;//번호
                int occupy = parkingPoint.get(i).occupy;//점유
                int startX = parkingPoint.get(i).startX;//전체 시작위치x
                int startY = parkingPoint.get(i).startY;//전체 시작위치y
                int lenX = parkingPoint.get(i).lenX;//전체 x길이
                int leny = parkingPoint.get(i).lenY;//전체 y길이
                final TextView topTV1 = new TextView(MainActivity.this);
                RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams((width * lenX) / totalLenX, (height * leny) / totalLenY);
                layoutParams.setMargins((width *startX)/totalLenX, (height *startY)/totalLenY, 0, 0);//
                topTV1.setLayoutParams(layoutParams);
                topTV1.setHint(Integer.toString(occupy) + Integer.toString(point));
                topTV1.setHintTextColor(Color.alpha(0));
                topTV1.setTextSize(30);
                switch(occupy){
                    case 0:
                        topTV1.setBackgroundResource(R.drawable.edge_empty);
                        break;
                    case 1:
                        topTV1.setBackgroundResource(R.drawable.edge_occupy);
                        break;
                    case 2:
                        topTV1.setBackgroundResource(R.drawable.edge_reversation);
                        break;
                }


                topTV1.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        if(Integer.parseInt(topTV1.getHint().toString().substring(0, 1))  == 0){
                            topTV1.setBackgroundResource(R.drawable.edge_myreversation);
                            topTV1.setHint(Integer.toString(2) + topTV1.getHint().toString().substring(1));
                            //sql수정해야함(occupy)
                            if(tv_click != null){
                                tv_click.setBackgroundResource(R.drawable.edge_empty);
                                tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
                            }
                            tv_click = topTV1;

                        }
                        else if (topTV1 == tv_click){//본인 예약해제
                            tv_click.setBackgroundResource(R.drawable.edge_empty);
                            tv_click.setHint(Integer.toString(0) + tv_click.getHint().toString().substring(1));
                            tv_click = null;
                            //sql수정해야함(occupy)
                        }
                    }
                });



                //topTV1.setId(R.id.layout_Main + i);
                //topTV1.setBackgroundColor(Color.parseColor("#000000"));
                topLL.addView(topTV1);
            }
        }
    }




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

                int point, occupy, startX, startY, lenX, lenY;
                while(count<jsonArray.length()){
                    JSONObject object = jsonArray.getJSONObject(count);
                    point = object.getInt("POINT");
                    occupy = object.getInt("OCCUPY");
                    startX = object.getInt("STARTX");
                    startY = object.getInt("STARTY");
                    lenX = object.getInt("LENX");
                    lenY = object.getInt("LENY");
                    parkingPoint.add(new ParkingPoint(point, occupy, startX, startY, lenX, lenY));
                    count++;
                }
                Print();//출력함수
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
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
