package company.co.kr.sriverforuser;

import android.graphics.Point;

public class CrossPoint {
    Point p;
    int startX;
    int startY;
    int width;
    int height;

    public CrossPoint(Point p, int startX, int startY, int width, int height) {
        this.p = p;
        this.startX = startX;
        this.startY = startY;
        this.width = width;
        this.height = height;
    }

    public Point getPoint() {
        return p;
    }

    public int getStartX() {
        return startX;
    }

    public int getStartY() {
        return startY;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }
}
