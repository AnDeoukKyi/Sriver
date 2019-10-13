package company.co.kr.sriverforuser;

import android.widget.TextView;

public class ParkingPoint {
    int index;
    int occupy;
    int startX;
    int startY;
    String id;
    TextView tv;

    public ParkingPoint(int index, int occupy, int startX, int startY, String id) {
        this.index = index;
        this.occupy = occupy;
        this.startX = startX;
        this.startY = startY;
        this.id = id;
    }
}
