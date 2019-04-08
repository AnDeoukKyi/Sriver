#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Point2f pts[4], smallrect(10, 10);				// 4개 좌표를 담을 배열 및 각 좌표의 사각형 크기
Mat img_frame;									// 원본 img
Size warpSize(500, 720);						// warp 크기
Mat img_mask1;									// 모폴로지 전 img
Mat corner1, corner2;							// 코너 검출용 변수
Mat warpImg(warpSize, img_frame.type());        // warp img
int px;
int py;
Mat *rgb;

Mat img_labels, stats, centroids;				// 사각형 좌표 관련 변수 목록
int j = 1;
int area = stats.at<int>(j, CC_STAT_AREA);	
int left = stats.at<int>(j, CC_STAT_LEFT);
int top = stats.at<int>(j, CC_STAT_TOP);
int width = stats.at<int>(j, CC_STAT_WIDTH);
int height = stats.at<int>(j, CC_STAT_HEIGHT);

struct point
{
	int xpos;
	int ypos;
};

int cas = 0;

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

void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)			// 비최대값 억제
{
	dst = Mat(sobel.size(), CV_32F, Scalar(0));

	for (int i = 1; i < sobel.rows - 1; i++) {
		for (int j = 1; j < sobel.cols - 1; j++)
		{
			int   dir = direct.at<uchar>(i, j);            // 기울기 값
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
		pos_ck.at<uchar>(pt) = 1;			// 추적 완료 좌표
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
	Scalar green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);
	Scalar blue(255, 0, 0);
	Scalar yellow(0, 255, 255);
	Scalar magenta(255, 0, 255);

	Mat rgb_color = Mat(1, 1, CV_8UC3, green);				//1,1짜리 행렬로 생성(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);			//BRG->HSV로 변환

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];			//색조
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];		//채도
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];			//명도

	cout << endl;
	cout << "hue = " << hue << endl;
	cout << "saturation = " << saturation << endl;
	cout << "value = " << value << endl;
	cout << endl;

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
	cout << endl;

	Mat img_hsv;

	for (;;)
	{
		img_frame = imread("A.jpg", IMREAD_COLOR);

		// HSV로 변환
		cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV로 변환

		// 지정한 HSV 범위를 이용하여 영상을 이진화
		Mat img_mask2;
		//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
		inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
		if (range_count == 2) {
			inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
			img_mask1 |= img_mask2;
		}

		imshow("모폴로지 전", img_mask1);
		// morphological opening 작은 점들을 제거 (노이즈 제거)
		erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(19, 19)));		//모폴로지침식
		dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(29, 29)));		//모폴로지팽창
		//imshow("모폴로지 후", img_mask1);
		
		// 라벨링 
		Mat img_labels, stats, centroids;
		int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
			stats, centroids, 8, CV_32S);

		// 영역박스 그리기
		int max = -1, idx = 0;
		int count = 0;

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

		count = 1;
		int x = centroids.at<double>(j, 0);      //중심좌표
		int y = centroids.at<double>(j, 1);

		circle(img_frame, Point(x, y), 5, Scalar(255, 0, 0), 1);

		rectangle(img_frame, Point(left, top), Point(left + width, top + height),
			Scalar(0, 0, 255), 1);

		putText(img_frame, to_string(j), Point(left + 20, top + 20),
			FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

		cout << "왼쪽 위 점부터 시계방향 순 " << endl;

		if (count == 1)
		{
			cout << Point(left, top) << endl;
			cout << Point(left + width, top) << endl;
			cout << Point(left, top + height) << endl;
			cout << Point(left + width, top + height);
			count++;
		}
		if (count == numOfLables)
			break;
		
		imshow("원본 영상", img_frame);
		if (waitKey(5) >= 0)
			break;
	}
}

// RGB 값 출력
void mouseEventRGB(int event, int x, int y, int flags, void *param)
{
	rgb = (Mat*)param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		cas = 1;
		cout << "(" << x << ", " << y << ") : " << (int)(*rgb).at<Vec3b>(y, x)[0];   // blue
		cout << " " << (int)(*rgb).at<Vec3b>(y, x)[1];		 						 // green
		cout << " " << (int)(*rgb).at<Vec3b>(y, x)[2] << std::endl;					 // red
		px = x;
		py = y;			
	}
}

// 4개 좌표 잇는 사각형 그리기
void draw_rect(Mat img_frame)
{
	Rect img_rect(Point(0, 0), img_frame.size());								// 입력 영상 크기 사각형
	for (int i = 0; i < 4; i++)
	{
		Rect rect(pts[i] - smallrect, pts[i] + smallrect);						// 좌표 사각형
		rect &= img_rect;														// 교차 영역 계산
		img_frame(rect) += Scalar(70, 70, 70);									// 사각형 영역 밝게 하기
		line(img_frame, pts[i], pts[(i + 1) % 4], Scalar(255, 0, 255), 1);      // 4개 좌표를 잇는 선 그리기
		rectangle(img_frame, rect, Scalar(255, 255, 0), 1);						// 좌표 사각형 그리기
	}
	//imshow("select rect", img_frame);
}

// 원근 변환 수행 함수
void warp(Mat img_frame)
{
	Point2f dst_pts[4] = {         //목적 영상 4개 좌표
	Point2f(0,0), Point2f(700, 0),
	Point2f(700, 700), Point2f(0, 700)
	};
	Mat dst;
	Mat perspect_mat = getPerspectiveTransform(pts, dst_pts);            // 원근변환 행렬 계산
	warpPerspective(img_frame, dst, perspect_mat, Size(700, 700), INTER_CUBIC);
	imshow("왜곡보정", dst);
}

// 원근변환에 대한 마우스 이벤트 제어
void onMouse(int event, int x, int y, int flags, void*)
{
	Point curr_pt(x, y);					// 현재 클릭 좌표
	static int check = -1;                  // 마우스 선택 좌표번호

	if (event == EVENT_LBUTTONDOWN)         // 마우스 좌 클릭
	{
		for (int i = 0; i < 4; i++)
		{
			Rect rect(pts[i] - smallrect, pts[i] + smallrect);         //i 좌표 사각형들 선언
			if (rect.contains(curr_pt)) check = i;					   // 선택 좌표 사각형 잡기
		}
	}

	if (event == EVENT_LBUTTONUP)           // 선택 좌표번호 초기화
		check = -1;

	if (check >= 0)
	{
		pts[check] = curr_pt;               // 클릭 좌표를 선택 좌표에 저장
		draw_rect(img_frame.clone());       // 4개 좌표 연결 사각형 그리기
		warp(img_frame.clone());            // 원근 변환 수행
	}
}

int main()
{
	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);

	pts[0] = Point2f(94, 145), pts[1] = Point2f(511, 153);				// 4개 좌표 초기화
	pts[2] = Point2f(626, 376), pts[3] = Point2f(26, 376);

	draw_rect(img_frame.clone());
	imshow("img_frame", img_frame);
	setMouseCallback("select rect", onMouse, 0);						// 수동 원근 왜곡 보정 콜백 함수 등록
	
	vector<Point2f> corners(4);											// Warping 전의 이미지 상의 좌표
	corners[0] = Point2f(157, 137);
	corners[1] = Point2f(600, 132);
	corners[2] = Point2f(136, 810);
	corners[3] = Point2f(616, 820);

	vector<Point2f> warpCorners(4);										// Warping 후의 좌표
	warpCorners[0] = Point2f(0, 0);
	warpCorners[1] = Point2f(warpImg.cols, 0);
	warpCorners[2] = Point2f(0, warpImg.rows);
	warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);

	// Transformation Matrix 구하기
	Mat trans = getPerspectiveTransform(corners, warpCorners);

	// Warping
	/*warpPerspective(img_frame, warpImg, trans, warpSize);
	imshow("warpImg", warpImg);
	imwrite("warpImg.jpg", warpImg);*/

	//for (int i = 0; i < corners.size(); i++)
	//{
	//	circle(img_frame, corners[i], 3, Scalar(0, 255, 0), 3);
	//}
	//imshow("img_frame", img_frame);                                   // 왜곡보정 좌표 찍힌 img 출력

	imshow("img_frame", img_frame);										// BGR값 출력시 사용
	setMouseCallback("img_frame", mouseEventRGB, (&img_frame));			// BGR값 출력용 마우스 이벤트

	for (;;) {
		if (cas == 1)
		{
			CheckBackGround();			// 모폴로지 + 라벨링 함수
			cas = 2;
			break;
		}
		if (waitKey(30) >= 0) continue;
	}

	////비디오 캡쳐 초기화
	//VideoCapture cap(0);
	//if (!cap.isOpened()) {
	//	cerr << "에러 - 카메라를 열 수 없습니다.\n";
	//	return -1;
	//}

	//

	//VideoCapture capture(1);											// 비디오객체 선언 및 0번 카메라 연결
	//CV_Assert(capture.isOpened());

	//Mat frame;
	//double fps = 30.0;												// 초당 프레임 수
	////int delay = cvRound(1000.0 / fps);								// 프레임간 지연시간
	//Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
	//	(int)cap.get(CAP_PROP_FRAME_HEIGHT));
	//// 동영상 파일 해상도
	//int fourcc = VideoWriter::fourcc('X', 'V', 'I', 'D');				//압축 코덱 설정

	//cout << "width x height : " << size << endl;				
	//cout << "VideoWriter::fourcc : " << fourcc << endl;				// 동영상 정보 콘솔창 출력
	////cout << "delay : " << delay << endl;
	//cout << "fps : " << fps << endl;

	//VideoWriter writer;												// 동영상 파일 저장 객체
	//writer.open("video_file.avi", fourcc, fps, size, true);			// 파일 개방 및 설정
	//CV_Assert(writer.isOpened());

	//for (;;)
	//{

	//	capture.read(frame);
	//	writer.write(frame);

	//	imshow("Color", frame);
	//	if (waitKey(30) >= 0)
	//		break;
	//}

	waitKey(0);
	return 0;
}