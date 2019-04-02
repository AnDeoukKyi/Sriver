package company.co.kr.sriverforuser;

public class ParkingPoint {
    int point;
    int occupy;
    int startX;
    int startY;
    int lenX;
    int lenY;

    public ParkingPoint(int point, int occupy, int startX, int startY, int lenX, int lenY) {
        this.point = point;
        this.occupy = occupy;
        this.startX = startX;
        this.startY = startY;
        this.lenX = lenX;
        this.lenY = lenY;
    }
}
