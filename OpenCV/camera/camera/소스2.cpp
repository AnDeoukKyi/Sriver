#include <opencv2/opencv.hpp>
#include <iostream>  

using namespace cv;
using namespace std;



int main()
{
	int index = 1;
	char buf[256];
	Mat frameMat;


	VideoCapture videoCapture(1);
	if (!videoCapture.isOpened())
	{
		printf("첫번째 카메라를 열수 없습니다. \n");
	}


	namedWindow("웹캠 영상", WINDOW_AUTOSIZE);


	while (1)
	{

		//웹캡으로부터 한 프레임을 읽어옴  
		videoCapture.read(frameMat);

		//화면에 보여줌
		imshow("웹캠 영상", frameMat);

		//이미지로 저장  
		sprintf(buf, "c:/ejpicture/img_%03d.jpg", index);
		cout << buf << endl;
		imwrite(buf, frameMat);

		index++;
		if (index == 11)
			return;

		if (waitKey(500) == 27) break; //ESC키 누르면 종료  
	}


	return 0;
}