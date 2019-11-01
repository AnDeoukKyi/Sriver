#include <opencv2/opencv.hpp>
#include <iostream>  

using namespace cv;
using namespace std;

int main()
{
	VideoWriter writer;

	for (;;)
	{
		VideoCapture capture(1);
		Mat frame;
		capture >> frame;
		writer << frame;

		writer.write(frame);
		imshow("카메라 영상보기");
	}
}
