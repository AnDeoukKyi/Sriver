package company.co.kr.sriverforuser;


import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;


public class UserProperty extends Activity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL, WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL);

        // ...but notify us that it happened.
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH, WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH);

        setContentView(R.layout.userproperty);

        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);

        int width = dm.widthPixels;
        int height = dm.heightPixels;

        getWindow().setLayout((int) (width * 0.6), height);
        getWindow().setGravity(Gravity.LEFT);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        overridePendingTransition(R.anim.slide_right ,R.anim.slide_stay);
        TextView tv_UserProperty_LogOut = (TextView)findViewById(R.id.tv_UserProperty_LogOut);

        tv_UserProperty_LogOut.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //Restart();
            }
        });
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(event.getAction() == MotionEvent.ACTION_OUTSIDE) {//외곽눌렀을시
            onBackPressed();
            return true;
        }
        return super.onTouchEvent(event);
    }

    @Override
    public void onBackPressed() {//뒤로가기 버튼눌렀을 시
        super.onBackPressed();
        overridePendingTransition(R.anim.slide_stay ,R.anim.slide_left);



    }
//    public void Restart(){
//        SharedPreferences sharedPreferences = getSharedPreferences("Login",MODE_PRIVATE);
//        SharedPreferences.Editor editor = sharedPreferences.edit();
//        editor.putString("ID", "");
//        editor.putString("PW", "");
//        editor.commit();
//        ActivityCompat.finishAffinity(this);
//        Intent intent = new Intent(getApplicationContext(), LoginActivity.class);
//        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
//        startActivity(intent);
//    }
}