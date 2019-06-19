package company.co.kr.sriverforuser;

public class ParkingPoint {
    int point;
    int occupy;
    int startX;
    int startY;

    public ParkingPoint(int point, int occupy, int startX, int startY) {
        this.point = point;
        this.occupy = occupy;
        this.startX = startX;
        this.startY = startY;
    }

    public int getPoint() {
        return point;
    }

    public void setPoint(int point) {
        this.point = point;
    }

    public int getOccupy() {
        return occupy;
    }

    public void setOccupy(int occupy) {
        this.occupy = occupy;
    }

    public int getStartX() {
        return startX;
    }

    public void setStartX(int startX) {
        this.startX = startX;
    }

    public int getStartY() {
        return startY;
    }

    public void setStartY(int startY) {
        this.startY = startY;
    }
}
