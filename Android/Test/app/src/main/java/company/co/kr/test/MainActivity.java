package company.co.kr.test;


import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.toolbox.Volley;

import org.json.JSONException;
import org.json.JSONObject;


public class MainActivity extends AppCompatActivity {

    boolean run = true;
    MainRequest mainRequest;

    final class Straight extends Thread {
        public void run () {

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        Thread.sleep(1000);

                    }catch (Exception e)
                    {
                        e.printStackTrace();
                    }
                }
            });
        }
    }
    public Response.Listener<String> responseListener = new Response.Listener<String>() {
        @Override
        public void onResponse(String response) {
            JSONObject jsonResponse = null;
            try {
                jsonResponse = new JSONObject(response);
                boolean success = jsonResponse.getBoolean("success");
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
    };

    public void Test(int x, int y){
        Straight straight = new Straight();
        straight.start();

        MainRequest mainRequest = new MainRequest(Integer.toString(x),Integer.toString(y),responseListener );
        RequestQueue queue = Volley.newRequestQueue(MainActivity.this);
        queue.add(mainRequest);

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btn_straight=(Button)findViewById(R.id.btn_straight);

        btn_straight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                int x = 0;
                int y = 0;

                while (run)
                {
                    y+=50;
                    Test(x,y);

                    if (x>200 || y > 200)
                        run=false;
                }

            }
        });
    }
}
