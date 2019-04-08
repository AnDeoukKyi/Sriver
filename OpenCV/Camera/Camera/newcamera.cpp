#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat img_frame;                                    // ���� img
Mat img_mask1;                                    // �������� �� img
Mat *rgb;                                       // rgb �� ��¿� ����
int px;                                          // ���콺 Ŭ���� x��ǥ
int py;                                          // ���콺 Ŭ���� y��ǥ
CvMemStorage* storage = cvCreateMemStorage(0);            // ������ ����
CvSeq* contours = 0;                              // ������ ����
CvPoint corner[4];                                 // �󺧸� ������ ����� ����

int cas = 0;

void calc_direct(Mat Gy, Mat Gx, Mat& direct)            // ĳ�Ͽ���
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

void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)         // ���ִ밪 ����
{
	dst = Mat(sobel.size(), CV_32F, Scalar(0));

	for (int i = 1; i < sobel.rows - 1; i++) {
		for (int j = 1; j < sobel.cols - 1; j++)
		{
			int   dir = direct.at<uchar>(i, j);             // ���� ��
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

	// ���� ���ϴ� ���� ����
	Scalar red(0, 0, 255);
	cout << "(" << px << ", " << py << " ): " << (int)(*rgb).at<Vec3b>(py, px)[0];   // blue
	cout << " " << (int)(*rgb).at<Vec3b>(py, px)[1];                          // green
	cout << " " << (int)(*rgb).at<Vec3b>(py, px)[2] << std::endl;                // red
	Scalar green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);
	Scalar blue(255, 0, 0);
	Scalar yellow(0, 255, 255);
	Scalar magenta(255, 0, 255);

	Mat rgb_color = Mat(1, 1, CV_8UC3, green);            // 1,1¥�� ��ķ� ����(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);         // BRG->HSV�� ��ȯ

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];         // ����
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];      // ä��
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];         // ��

	cout << endl;
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

	for (;;)
	{
		img_frame = imread("A.jpg", IMREAD_COLOR);

		// ������ ������ ���� ��
		if (img_frame.empty()) {
			cerr << "����! ������ ������ �ʾҽ��ϴ�.\n";
			break;
		}

		// HSV�� ��ȯ
		cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV�� ��ȯ

													// ������ HSV ������ �̿��Ͽ� ������ ����ȭ
		inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
		imshow("�������� ��", img_mask1);

		// �󺧸� 
		Mat img_labels, stats, centroids;
		int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
			stats, centroids, 8, CV_32S);

		//�����ڽ� �׸���
		int max = -1, idx = 0;
		for (int j = 1; j < numOfLables; j++) {
			int area = stats.at<int>(j, CC_STAT_AREA);
			int left = stats.at<int>(j, CC_STAT_LEFT);
			int top = stats.at<int>(j, CC_STAT_TOP);
			int width = stats.at<int>(j, CC_STAT_WIDTH);
			int height = stats.at<int>(j, CC_STAT_HEIGHT);

			if (max < area)
			{
				max = area;
				idx = j;
				rectangle(img_frame, Point(left, top), Point(left + width, top + height),
					Scalar(0, 0, 255), 3);
			}


			rectangle(img_frame, Point(left, top), Point(left + width, top + height),
				Scalar(0, 0, 255), 1);

			putText(img_frame, to_string(j), Point(left + 20, top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
		}

		// �ʱ� ��ġ ����
		CvPoint *st = (CvPoint *)cvGetSeqElem(contours, 0);

		// ù��° ������ ����(�ִ� �Ÿ��� ������ �� ����)
		double fMaxDist = 0.0;

		for (int x = 1; x < numOfLables; x++)
		{
			CvPoint* pt = (CvPoint *)cvGetSeqElem(contours, x);
		}



		//imshow("����ȭ ����", img_mask1);
		imshow("���� ����", img_frame);
		if (waitKey(5) >= 0)
			break;
		cas = 2;
	}
}

// RGB �� ���
void mouseEventRGB(int event, int x, int y, int flags, void *param)
{
	rgb = (Mat*)param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		cas = 1;
		cout << "(" << x << ", " << y << " ): " << (int)(*rgb).at<Vec3b>(y, x)[0];   // blue
		cout << " " << (int)(*rgb).at<Vec3b>(y, x)[1];                          // green
		cout << " " << (int)(*rgb).at<Vec3b>(y, x)[2] << std::endl;                // red
		px = x;
		py = y;
	}
}

int main()
{
	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);

	imshow("img_frame", img_frame);                           // BGR�� ��½� ���
	setMouseCallback("img_frame", mouseEventRGB, (&img_frame));      // BGR�� ��¿� ���콺 �̺�Ʈ

	for (;;)
	{
		if (cas == 1)
			CheckBackGround();                              // �������� + �󺧸� ��
		if (waitKey(30) >= 0) continue;
	}

	waitKey(0);
	return 0;
}