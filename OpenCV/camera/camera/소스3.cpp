#include <opencv2/opencv.hpp>
#include <iostream>  

using namespace cv;
using namespace std;



int main()
{
	int index = 1;
	char buf[256];
	Mat frameMat;
	VideoWriter writer;
	VideoCapture cap(1);
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));
	


	namedWindow("이미지 영상", WINDOW_AUTOSIZE);

	while (1)
	{
		double fps = 29.97;
		writer.open("RealTime.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size);


		for (;;) {
			sprintf(buf, "c:/ejpicture/img_%03d.jpg", index);
			cout << buf << endl;
			frameMat = imread(buf, IMREAD_COLOR);
			if (frameMat.empty()) {

				index = 0;
			}
			else {

				imshow("이미지 영상", frameMat);
				index++;
			}
			if (index == 11)
				index = 1;
			writer.write(frameMat);
		}
		//이미지로 저장  
		

		

		
		
		/*if (!writer.isOpened())
		{
			cout << "동영상을 저장하기 위한 초기화 작업 중 에러 발생" << endl;
			return 1;
		}
		for (;;) {
			Mat fr;


		}*/
	
		if (waitKey(250) == 27) break; //ESC키 누르면 종료  
	}


	return 0;
}