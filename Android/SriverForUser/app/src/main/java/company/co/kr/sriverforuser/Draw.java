package company.co.kr.sriverforuser;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Toast;

import java.util.ArrayList;

public class Draw extends View {

    private Paint paint;

    public Draw(Context context) {
        super(context);
    }

    public Draw(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * 뷰가 화면에 디스플레이 될때 자동으로 호출
     * @param canvas
     */
    @Override
    protected void onDraw(final Canvas canvas) {
        super.onDraw(canvas);

        final Paint paint = new Paint(); // 페인트 객체 생성



        paint.setColor(Color.RED); // 빨간색으로 설정
        paint.setStrokeWidth(10);
        ArrayList<Point> p = MainActivity.Path;
        for(int i = 0; i<p.size()-1; i++)
            canvas.drawLine(p.get(i).x, p.get(i).y, p.get(i+1).x, p.get(i+1).y, paint);
        invalidate();
    }

    /**
     * 터치 이벤트를 처리
     * @param event
     * @return
     */
    @Override
    public boolean onTouchEvent(MotionEvent event) {

        return true;
    }
}