#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>

using namespace cv;
using namespace std;

Point2f pts[4], smallrect(10, 10);    // 4�� ��ǥ�� ���� �迭 �� �� ��ǥ�� �簢�� ũ��
Mat img_frame;

void calc_direct(Mat Gy, Mat Gx, Mat& direct)
{
	direct.create(Gy.size(), CV_8U);

	for (int i = 0; i < direct.rows; i++) {
		for (int j = 0; j < direct.cols; j++) {
			float gx = Gx.at<float>(i, j);
			float gy = Gy.at<float>(i, j);
			int theat = int(fastAtan2(gy, gx) / 45);
			direct.at<uchar>(i, j) = theat % 4;
		}
	}
}

void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)      // ���ִ밪 ����
{
	dst = Mat(sobel.size(), CV_32F, Scalar(0));

	for (int i = 1; i < sobel.rows - 1; i++) {
		for (int j = 1; j < sobel.cols - 1; j++)
		{
			int   dir = direct.at<uchar>(i, j);            // ���� ��
			float v1, v2;
			if (dir == 0) {         // ���� ���� 0�� ����
				v1 = sobel.at<float>(i, j - 1);
				v2 = sobel.at<float>(i, j + 1);
			}
			else if (dir == 1) {      // ���� ���� 45��
				v1 = sobel.at<float>(i + 1, j + 1);
				v2 = sobel.at<float>(i - 1, j - 1);
			}
			else if (dir == 2) {      // ���� ���� 90��
				v1 = sobel.at<float>(i - 1, j);
				v2 = sobel.at<float>(i + 1, j);
			}
			else if (dir == 3) {      // ���� ���� 135��
				v1 = sobel.at<float>(i + 1, j - 1);
				v2 = sobel.at<float>(i - 1, j + 1);
			}

			float center = sobel.at<float>(i, j);
			dst.at<float>(i, j) = (center > v1 && center > v2) ? center : 0;
		}
	}
}

void trace(Mat max_so, Mat& pos_ck, Mat& hy_img, Point pt, int low)
{
	Rect rect(Point(0, 0), pos_ck.size());
	if (!rect.contains(pt)) return;         // ����ȭ���� ���� ���� Ȯ�� 

	if (pos_ck.at<uchar>(pt) == 0 && max_so.at<float>(pt) > low)
	{
		pos_ck.at<uchar>(pt) = 1;         // ���� �Ϸ� ��ǥ
		hy_img.at<uchar>(pt) = 255;         // ���� ����

											// ���� ��� �Լ�
		trace(max_so, pos_ck, hy_img, pt + Point(-1, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(0, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(+1, -1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(-1, 0), low);

		trace(max_so, pos_ck, hy_img, pt + Point(+1, 0), low);
		trace(max_so, pos_ck, hy_img, pt + Point(-1, +1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(0, +1), low);
		trace(max_so, pos_ck, hy_img, pt + Point(+1, +1), low);
	}
}

void  hysteresis_th(Mat max_so, Mat&  hy_img, int low, int high)
{
	Mat pos_ck(max_so.size(), CV_8U, Scalar(0));
	hy_img = Mat(max_so.size(), CV_8U, Scalar(0));

	for (int i = 0; i < max_so.rows; i++) {
		for (int j = 0; j < max_so.cols; j++)
		{
			if (max_so.at<float>(i, j) > high)
				trace(max_so, pos_ck, hy_img, Point(j, i), low);
		}
	}
}

void CheckBackGround() {
	int range_count = 0;

	Scalar red(0, 0, 255);
	//Scalar green(153, 174, 124);
	Scalar green(141, 129, 0);
	Scalar blue(255, 0, 0);
	Scalar yellow(0, 255, 255);

	Scalar magenta(255, 0, 255);
	//���� ���ϴ� ���� ����


	Mat rgb_color = Mat(1, 1, CV_8UC3, green);//1,1¥�� ��ķ� ����(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV); //BRG->HSV�� ��ȯ


	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];//����
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];//ä��
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];//��


	cout << "hue = " << hue << endl;
	cout << "saturation = " << saturation << endl;
	cout << "value = " << value << endl;


	int low_hue = hue - 10;
	int high_hue = hue + 6;

	int low_hue1 = 0, low_hue2 = 0;
	int high_hue1 = 0, high_hue2 = 0;

	if (low_hue < 10) {
		range_count = 2;

		high_hue1 = 180;
		low_hue1 = low_hue + 180;
		high_hue2 = high_hue;
		low_hue2 = 0;
	}
	else if (high_hue > 170) {
		range_count = 2;

		high_hue1 = low_hue;
		low_hue1 = 180;
		high_hue2 = high_hue - 180;
		low_hue2 = 0;
	}
	else {
		range_count = 1;

		low_hue1 = low_hue;
		high_hue1 = high_hue;
	}


	cout << low_hue1 << "  " << high_hue1 << endl;
	cout << low_hue2 << "  " << high_hue2 << endl;


	//VideoCapture cap("A.png");
	Mat img_frame, img_hsv;

	/*

	if (!cap.isOpened()) {
	cerr << "ERROR! Unable to open camera\n";
	return -1;
	}
	*/

	for (;;)
	{
		// wait for a new frame from camera and store it into 'frame'
		//cap.read(img_frame);
		img_frame = imread("A.jpg", IMREAD_COLOR);

		// check if we succeeded
		if (img_frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}


		//HSV�� ��ȯ
		cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV�� ��ȯ

													//������ HSV ������ �̿��Ͽ� ������ ����ȭ
		Mat img_mask1, img_mask2;
		//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
		inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
		if (range_count == 2) {
			inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
			img_mask1 |= img_mask2;
		}

		imshow("img_mask1", img_mask1);
		//morphological opening ���� ������ ���� (������ ����)
		erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(13, 13)));//��������ħ��
																						//dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));//����������â


																						//morphological closing ������ ���� �޿�� 
																						//dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
																						//erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


		imshow("�������� ��", img_mask1);

		//�󺧸� 
		Mat img_labels, stats, centroids;
		int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
			stats, centroids, 8, CV_32S);


		//������� �ּ�����
		////�����ڽ� �׸���
		//int max = -1, idx = 0;
		//for (int j = 1; j < numOfLables; j++) {
		//   int area = stats.at<int>(j, CC_STAT_AREA);

		//   int left = stats.at<int>(j, CC_STAT_LEFT);
		//   int top = stats.at<int>(j, CC_STAT_TOP);
		//   int width = stats.at<int>(j, CC_STAT_WIDTH);
		//   int height = stats.at<int>(j, CC_STAT_HEIGHT);


		//   rectangle(img_frame, Point(left, top), Point(left + width, top + height),
		//      Scalar(0, 0, 255), 1);

		//   putText(img_frame, to_string(j), Point(left + 20, top + 20),
		//      FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
		//}

		////j�� 1�϶� ���ɿ��� �����ؼ� Ȯ���غ��ߵ�
		////1���� ���ɿ������� �����ѵ�
		////������ �ֵ� ������ǥ üũ(sort)�� ��ٸ��� ����� ���ٿְ��
		////->���� �ٽ� ������ DB�� ����




		////imshow("����ȭ ����", img_mask1);
		//imshow("���� ����", img_frame);
		//������� �ּ�Ǯ��
		if (waitKey(5) >= 0)
			break;
	}
}

//// RGB ȭ�Ұ� ���
//void checkPixelOnMouse(int event, int x, int y, int flags, void* param)
//{
//	cv::Mat im = *(cv::Mat*)(param);
//
//	switch (event)
//	{
//	case EVENT_LBUTTONDOWN:
//		//std::cout << "(" << x << ", " << y << "): " << static_cast<int>(im.at<uchar>(cv::Point(x, y))) << std::endl; // (x, y)�� �ִ� ȭ�Ұ� ���
//	
//		break;
//	}
//}

// RGB �� ���
void mouseEventRGB(int event, int x, int y, int flags, void* param)
{
	cv::Mat* rgb = (cv::Mat*) param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		std::cout << "(" << x << ", " << y << " ): " << (int)(*rgb).at<Vec3b>(y, x)[0];  // blue
		std::cout << " " << (int)(*rgb).at<Vec3b>(y, x)[1];  // green
		std::cout << " " << (int)(*rgb).at<Vec3b>(y, x)[2] << std::endl;  // red
	}
}

// 4�� ��ǥ �մ� �簢�� �׸���
void draw_rect(Mat img_frame)
{
	Rect img_rect(Point(0, 0), img_frame.size());      // �Է� ���� ũ�� �簢��
	for (int i = 0; i < 4; i++)
	{
		Rect rect(pts[i] - smallrect, pts[i] + smallrect);      // ��ǥ �簢��
		rect &= img_rect;                              // ���� ���� ���
		img_frame(rect) += Scalar(70, 70, 70);               // �簢�� ���� ��� �ϱ�
		line(img_frame, pts[i], pts[(i + 1) % 4], Scalar(255, 0, 255), 1);      // 4�� ��ǥ�� �մ� �� �׸���
		rectangle(img_frame, rect, Scalar(255, 255, 0), 1);      // ��ǥ �簢�� �׸���
	}
	imshow("select rect", img_frame);
}

// ���� ��ȯ ���� �Լ�
void warp(Mat img_frame)
{
	Point2f dst_pts[4] = {         //���� ���� 4�� ��ǥ
		Point2f(0,0), Point2f(500, 0),
		Point2f(500, 500), Point2f(0, 500)
	};
	Mat dst;
	Mat perspect_mat = getPerspectiveTransform(pts, dst_pts);            // ���ٺ�ȯ ��� ���
	warpPerspective(img_frame, dst, perspect_mat, Size(500, 500), INTER_CUBIC);
	imshow("�ְ��", dst);
}

// ���ٺ�ȯ�� ���� ���콺 �̺�Ʈ ����
void onMouse(int event, int x, int y, int flags, void*)
{
	Point curr_pt(x, y);                  // ���� Ŭ�� ��ǥ
	static int check = -1;                  // ���콺 ���� ��ǥ��ȣ

	if (event == EVENT_LBUTTONDOWN)            //���콺 �� Ŭ��
	{
		for (int i = 0; i < 4; i++)
		{
			Rect rect(pts[i] - smallrect, pts[i] + smallrect);         // ��ǥ �簢���� ����
			if (rect.contains(curr_pt)) check = i;                  // ���� ��ǥ �簢�� ���
		}
	}

	if (event == EVENT_LBUTTONUP)            // ���� ��ǥ��ȣ �ʱ�ȭ
		check = -1;

	if (check >= 0)
	{
		pts[check] = curr_pt;               // Ŭ�� ��ǥ�� ���� ��ǥ�� ����
		draw_rect(img_frame.clone());         // 4�� ��ǥ ���� �簢�� �׸���
		warp(img_frame.clone());            // ���� ��ȯ ����
	}
}

int main()
{
	//Mat img_frame;
	img_frame = imread("A.jpg", IMREAD_COLOR);

	CV_Assert(img_frame.data);

	/*
	VideoCapture capture(0);
	if (!capture.isOpened()) {
	cout << "ī�޶� ������� �ʾҽ��ϴ�." << endl;
	exit(1);
	}

	*/

	imshow("img_frame", img_frame);

	/*setMouseCallback("gg", onMouseEvent, (void*)&img_frame);
	imshow("gg", img_frame);*/

	//setMouseCallback("img_frame", mouseEventRGB, (void*)(&img_frame)); // Ư�� â�� ���콺 �ڵ鷯 ����
	//imshow("img_frame", img_frame);

	//CheckBackGround();

	pts[0] = Point2f(94, 145), pts[1] = Point2f(511, 153);            // 4�� ��ǥ �ʱ�ȭ
	pts[2] = Point2f(626, 376), pts[3] = Point2f(26, 376);

	draw_rect(img_frame.clone());
	//imshow("img_frame", img_frame);
	cv::setMouseCallback("select rect", onMouse, 0);                  // �ݹ� �Լ� ���
	


	for (;;) {
	   /*
	   Range r1(100, 300), r2(200, 300);
	   Mat frame, tm, gau_img, Gx, Gy, direct, sobel, max_sobel, hy_img, canny;


	   Mat image;
	   image = imread("A.png", IMREAD_COLOR);
	   if (image.empty())
	   {
	   cout << "Could not open or find the image" << endl;
	   return -1;
	   }

	   namedWindow("Original", WINDOW_AUTOSIZE);
	   imshow("Original", image);
	   */



	   //capture.read(frame);
	   //imwrite("1111.jpg", frame);
	   //�������� jpg���� ����
	   //Sleep(1000);
	   /*
	   GaussianBlur(frame, gau_img, Size(5, 5), 0.3);
	   Sobel(gau_img, Gx, CV_32F, 1, 0, 3);
	   Sobel(gau_img, Gy, CV_32F, 0, 1, 3);
	   sobel = abs(Gx) + abs(Gy);
	   calc_direct(Gy, Gx, direct);
	   supp_nonMax(sobel, direct, max_sobel);
	   hysteresis_th(max_sobel, hy_img, 100, 150);

	   Canny(frame, canny, 100, 150);


	   imshow("ī�޶� ���󺸱�", frame);
	   imshow("OpenCV_canny", canny);
	   */
	   if (waitKey(30) >= 0) continue;

	}

	waitKey(0);
	return 0;
}