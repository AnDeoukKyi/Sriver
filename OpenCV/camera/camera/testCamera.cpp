#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <opencv\cv.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <ctime>

using namespace cv;
using namespace std;
using std::string;




int main(void) {

	VideoCapture cap(0);
	VideoWriter writer;
	Mat frame;
	cap.read(frame);
	char buf[2048];


	//imwrite("Dummy.jpg", frame, vector<int>());



	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));
	double fps = 30.0;
	writer.open("Base_Park1.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
	while (1) {
		cap.read(frame);
		writer.write(frame);
	}
	//while (1)
	//{
	//	double fps = 29.97;
	//	writer.open("RealTime.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size);


	//	for (;;) {
	//		sprintf(buf, "c:/ejpicture/img_%03d.jpg", index);
	//		cout << buf << endl;
	//		frameMat = imread(buf, IMREAD_COLOR);
	//		if (frameMat.empty()) {

	//			index = 0;
	//		}
	//		else {

	//			//imshow("이미지 영상", frameMat);
	//			index++;
	//		}
	//		if (index == 6)
	//			return 1;
	//		writer.write(frameMat);
	//	}
	return 0;
}


