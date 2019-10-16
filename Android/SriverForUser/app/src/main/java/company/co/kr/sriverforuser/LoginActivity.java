package company.co.kr.sriverforuser;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.toolbox.Volley;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class LoginActivity extends AppCompatActivity {

    String ID, PW;
    private AlertDialog dialog;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        SharedPreferences sf = getSharedPreferences("Login",MODE_PRIVATE);
        ID = sf.getString("ID","");
        PW = sf.getString("PW","");



        final CheckBox cb_Login_AutoLogin = (CheckBox)findViewById(R.id.cb_Login_AutoLogin);



        Button loginButton = (Button) findViewById(R.id.loginButton);
//        loginButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Intent loginIntent = new Intent(LoginActivity.this, MainActivity.class);
//                LoginActivity.this.startActivity(loginIntent);
//            }
//        });



        TextView registerButton = (TextView) findViewById(R.id.registerButton);
        registerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent registerIntent = new Intent(LoginActivity.this, RegisterActivity.class);
                LoginActivity.this.startActivity(registerIntent);
            }
        });

        final EditText idText = (EditText) findViewById(R.id.idText);
        final EditText passwordText = (EditText) findViewById(R.id.passwordText);



        if(!ID.equals("")) {
            new Login(ID, PW).execute();


//            Response.Listener<String> responseListener = new Response.Listener<String>() {
//                @Override
//                public void onResponse(String response) {
//                    try
//                    {
//                        JSONObject jsonResponse = new JSONObject(response);
//                        boolean success = jsonResponse.getBoolean("success");
//                        if(success) {
//                            AlertDialog.Builder builder = new AlertDialog.Builder(LoginActivity.this);
//                            dialog = builder.setMessage("Login에 성공했습니다.")
//                                    .setPositiveButton("확인",null)
//                                    .create();
//                            dialog.show();
//                            Intent intent = new Intent(LoginActivity.this, MainActivity.class);
//                            intent.putExtra("ID", jsonResponse.getString("ID"));
//                            LoginActivity.this.startActivity(intent);
//                            finish();
//                        }
//                        else
//                        {
//                            AlertDialog.Builder builder = new AlertDialog.Builder(LoginActivity.this);
//                            dialog = builder.setMessage("계정을 다시 확인하세요.")
//                                    .setNegativeButton("다시 시도",null)
//                                    .create();
//                            dialog.show();
//                        }
//                    }
//                    catch (Exception e)
//                    {
//                        e.printStackTrace();
//                    }
//                }
//            };
//            LoginRequest loginRequest = new LoginRequest(ID, PW, responseListener);
//            RequestQueue queue =  Volley.newRequestQueue(LoginActivity.this);
//            queue.add(loginRequest);
        }


        loginButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                final String userID = idText.getText().toString();
                final String userPassword = passwordText.getText().toString();
                new Login(userID, userPassword).execute();
//                Response.Listener<String> responseListener = new Response.Listener<String>() {
//                    @Override
//                    public void onResponse(String response) {
//                        try
//                        {
//                            JSONObject jsonResponse = new JSONObject(response);
//                            boolean success = jsonResponse.getBoolean("success");
//                            if(success) {
//                                if (cb_Login_AutoLogin.isChecked()){
//                                    SharedPreferences sharedPreferences = getSharedPreferences("Login",MODE_PRIVATE);
//                                    SharedPreferences.Editor editor = sharedPreferences.edit();
//                                    editor.putString("ID", userID); // key, value를 이용하여 저장하는 형태
//                                    editor.putString("PW", userPassword); // key, value를 이용하여 저장하는 형태
//                                    editor.commit();
//                                }
//                                AlertDialog.Builder builder = new AlertDialog.Builder(LoginActivity.this);
//                                dialog = builder.setMessage("Login에 성공했습니다.")
//                                         .setPositiveButton("확인",null)
//                                         .create();
//                                dialog.show();
//                                Intent intent = new Intent(LoginActivity.this, MainActivity.class);
//                                intent.putExtra("ID", jsonResponse.getString("ID"));
//                                LoginActivity.this.startActivity(intent);
//                                finish();
//                            }
//                            else
//                            {
//                                AlertDialog.Builder builder = new AlertDialog.Builder(LoginActivity.this);
//                                dialog = builder.setMessage("계정을 다시 확인하세요.")
//                                        .setNegativeButton("다시 시도",null)
//                                        .create();
//                                dialog.show();
//                            }
//                        }
//                        catch (Exception e)
//                        {
//                            e.printStackTrace();
//                        }
//                    }
//                };
//                LoginRequest loginRequest = new LoginRequest(userID, userPassword, responseListener);
//                RequestQueue queue =  Volley.newRequestQueue(LoginActivity.this);
//                queue.add(loginRequest);
            }
        });


    }


    class Login extends AsyncTask<Void, Void, String> {
        String target;
        String id = "";
        String pw = "";

        public Login(String id, String pw) {
            this.id = id;
            this.pw = pw;
        }

        @Override
        protected void onPreExecute(){//연결
            try{
                target = "http://nejoo97.cafe24.com/UserLogin1.php?ID="+id+"&PW="+pw;
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
                JSONObject jsonResponse = new JSONObject(result);
                boolean success = jsonResponse.getBoolean("success");
                if(success) {

                    SharedPreferences sharedPreferences = getSharedPreferences("Login",MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putString("ID", id); // key, value를 이용하여 저장하는 형태
                    editor.putString("PW", pw); // key, value를 이용하여 저장하는 형태
                    editor.commit();

                    Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                    intent.putExtra("ID", jsonResponse.getString("ID"));
                    LoginActivity.this.startActivity(intent);
                    finish();
                }
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }//주차칸 동기화




    @Override
    protected void onStop() {
        super.onStop();
        if (dialog != null)
        {
            dialog.dismiss();
            dialog = null;
        }
    }
}
