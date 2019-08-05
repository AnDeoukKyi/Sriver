package company.co.kr.sriverforuser;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

public class ParkingDialog extends Dialog {

    private View.OnClickListener ParkingDialog_OkClickListener;
    private Context context;

    public ParkingDialog(Context context, View.OnClickListener ParkingDialog_OkClickListener) {
        super(context, android.R.style.Theme_Translucent_NoTitleBar);
        this.context = context;
        this.ParkingDialog_OkClickListener = ParkingDialog_OkClickListener;
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        WindowManager.LayoutParams lpWindow = new WindowManager.LayoutParams();
        lpWindow.flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
        lpWindow.dimAmount = 0.8f;
        getWindow().setAttributes(lpWindow);
        setContentView(R.layout.parking_dialog);


        TextView tv_ParkingDialog_Ok = (TextView)findViewById(R.id.tv_ParkingDialog_Ok);

        tv_ParkingDialog_Ok.setOnClickListener(ParkingDialog_OkClickListener);
    }


}
