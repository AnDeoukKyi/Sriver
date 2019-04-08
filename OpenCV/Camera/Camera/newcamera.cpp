#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat img_frame;                                    // 원본 img
Mat img_mask1;                                    // 모폴로지 전 img
Mat *rgb;                                       // rgb 값 출력용 변수
int px;                                          // 마우스 클릭시 x좌표
int py;                                          // 마우스 클릭시 y좌표
CvMemStorage* storage = cvCreateMemStorage(0);            // 윤곽선 검출
CvSeq* contours = 0;                              // 윤곽선 정보
CvPoint corner[4];                                 // 라벨링 꼭지점 추출용 변수

int cas = 0;

void calc_direct(Mat Gy, Mat Gx, Mat& direct)            // 캐니엣지
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

void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)         // 비최대값 억제
{
	dst = Mat(sobel.size(), CV_32F, Scalar(0));

	for (int i = 1; i < sobel.rows - 1; i++) {
		for (int j = 1; j < sobel.cols - 1; j++)
		{
			int   dir = direct.at<uchar>(i, j);             // 기울기 값
			float v1, v2;
			if (dir == 0) {         // 기울기 방향 0도 방향
				v1 = sobel.at<float>(i, j - 1);
				v2 = sobel.at<float>(i, j + 1);
			}
			else if (dir == 1) {      // 기울기 방향 45도
				v1 = sobel.at<float>(i + 1, j + 1);
				v2 = sobel.at<float>(i - 1, j - 1);
			}
			else if (dir == 2) {      // 기울기 방향 90도
				v1 = sobel.at<float>(i - 1, j);
				v2 = sobel.at<float>(i + 1, j);
			}
			else if (dir == 3) {      // 기울기 방향 135도
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
	if (!rect.contains(pt)) return;         // 추적화소의 영상 범위 확인 

	if (pos_ck.at<uchar>(pt) == 0 && max_so.at<float>(pt) > low)
	{
		pos_ck.at<uchar>(pt) = 1;         // 추적 완료 좌표
		hy_img.at<uchar>(pt) = 255;         // 에지 지정

											// 추적 재귀 함수
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

	// 내가 원하는 색깔 선택
	Scalar red(0, 0, 255);
	cout << "(" << px << ", " << py << " ): " << (int)(*rgb).at<Vec3b>(py, px)[0];   // blue
	cout << " " << (int)(*rgb).at<Vec3b>(py, px)[1];                          // green
	cout << " " << (int)(*rgb).at<Vec3b>(py, px)[2] << std::endl;                // red
	Scalar green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);
	Scalar blue(255, 0, 0);
	Scalar yellow(0, 255, 255);
	Scalar magenta(255, 0, 255);

	Mat rgb_color = Mat(1, 1, CV_8UC3, green);            // 1,1짜리 행렬로 생성(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);         // BRG->HSV로 변환

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];         // 색조
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];      // 채도
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];         // 명도

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

		// 사진이 열리지 않을 시
		if (img_frame.empty()) {
			cerr << "에러! 사진이 열리지 않았습니다.\n";
			break;
		}

		// HSV로 변환
		cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV로 변환

													// 지정한 HSV 범위를 이용하여 영상을 이진화
		inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
		imshow("모폴로지 전", img_mask1);

		// 라벨링 
		Mat img_labels, stats, centroids;
		int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
			stats, centroids, 8, CV_32S);

		//영역박스 그리기
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

		// 초기 위치 설정
		CvPoint *st = (CvPoint *)cvGetSeqElem(contours, 0);

		// 첫번째 꼭지점 추출(최대 거리를 가지는 점 선택)
		double fMaxDist = 0.0;

		for (int x = 1; x < numOfLables; x++)
		{
			CvPoint* pt = (CvPoint *)cvGetSeqElem(contours, x);
		}



		//imshow("이진화 영상", img_mask1);
		imshow("원본 영상", img_frame);
		if (waitKey(5) >= 0)
			break;
		cas = 2;
	}
}

// RGB 값 출력
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

	imshow("img_frame", img_frame);                           // BGR값 출력시 사용
	setMouseCallback("img_frame", mouseEventRGB, (&img_frame));      // BGR값 출력용 마우스 이벤트

	for (;;)
	{
		if (cas == 1)
			CheckBackGround();                              // 모폴로지 + 라벨링 함
		if (waitKey(30) >= 0) continue;
	}

	waitKey(0);
	return 0;
}