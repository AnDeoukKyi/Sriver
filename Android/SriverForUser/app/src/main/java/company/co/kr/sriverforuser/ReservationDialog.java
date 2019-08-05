package company.co.kr.sriverforuser;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

public class ReservationDialog extends Dialog {

    private View.OnClickListener ReservationDialog_OkClickListener;
    private View.OnClickListener ReservationDialog_CancelClickListener;
    private Context context;
    private TextView tv;

    public ReservationDialog(Context context, TextView tv ,View.OnClickListener ReservationDialog_OkClickListener, View.OnClickListener ReservationDialog_CancelClickListener) {
        super(context, android.R.style.Theme_Translucent_NoTitleBar);
        this.context = context;
        this.tv = tv;
        this.ReservationDialog_OkClickListener = ReservationDialog_OkClickListener;
        this.ReservationDialog_CancelClickListener = ReservationDialog_CancelClickListener;
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        WindowManager.LayoutParams lpWindow = new WindowManager.LayoutParams();
        lpWindow.flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
        lpWindow.dimAmount = 0.8f;
        getWindow().setAttributes(lpWindow);
        setContentView(R.layout.reservation_dialog);


        TextView tv_ReservationDialog_Ok = (TextView)findViewById(R.id.tv_ReservationDialog_Ok);
        TextView tv_ReservationDialog_Cancel = (TextView)findViewById(R.id.tv_ReservationDialog_Cancel);

        tv_ReservationDialog_Ok.setOnClickListener(ReservationDialog_OkClickListener);
        tv_ReservationDialog_Cancel.setOnClickListener(ReservationDialog_CancelClickListener);
    }


    public TextView getTextView(){
        return tv;
    }
}
