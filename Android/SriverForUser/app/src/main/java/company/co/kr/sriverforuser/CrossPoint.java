package company.co.kr.sriverforuser;


public class CrossPoint {
    int dijID;
    int xIndex;
    int yIndex;
    int startX;
    int startY;
    int endX;
    int endY;
    int width;
    int height;
    int centerX;
    int centerY;

    public CrossPoint(int xIndex, int yIndex, int startX, int startY, int endX, int endY) {
        this.xIndex = xIndex;
        this.yIndex = yIndex;
        this.startX = startX;
        this.startY = startY;
        this.endX = endX;
        this.endY = endY;
        width = endX - startX;
        height = endY - startY;
        centerX = startX + width/2;
        centerY = startY + height/2;
    }

}
