#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

using namespace cv;
using namespace std;
using std::string;

Point2f pts[4], smallrect(10, 10);								// 4개 좌표를 담을 배열 및 각 좌표의 사각형 크기
Size warpSize(500, 720);										// warp 크기
Mat img_mask1;													// 원본 img 모폴로지 전
Mat img_mask2;													// 원본 img 모폴로지 후 
Mat corner1, corner2;											// 코너 검출용 변수
vector<Point2f> corners;										// 헤리스 코너 검출용

Mat img_labels, stats, centroids;								// 원본 img 큰 사각형 좌표 관련 변수 목록



Mat warp_img_labels, warp_stats, warp_centroids;				// warp img 작은 사각형 좌표 관련 변수 목록
int warp_j = 1;
int warp_area;
int warp_left;
int warp_top;
int warp_width;
int warp_height;
Mat warp_img_mask1;												// warp img 모폴로지 전
Mat warp_img_mask2;												// warp img 모폴로지 후
Mat small_warpImg;												// roi warpImg






Mat img_frame;													// 원본 img
Mat *rgb;														//마우스 이벤트 주차장 색깔



																//

Mat warpImg(warpSize, img_frame.type());						// warp img
																//


int px, py;//마우스 클릭 좌표
int step = 0; //0시작, 1마우스클릭(주차장색깔)


			  //함수
void RecordVideo();
void MouseEventRGB(int event, int x, int y, int flags, void *param);
void CheckBackGround();
void Labeling();








//php
HINSTANCE hInst;
WSADATA wsaData;
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
SOCKET connectToServer(char *szServerName, WORD portNum);
int getHeaderLength(char *content);
char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut);
int InsertDB(int index, int occupy, int startX, int startY, int lenX, int lenY);

//InsertDB(12, 0, 2, 3, 4, 5);

int InsertDB(int index, int occupy, int startX, int startY, int lenX, int lenY) {
	const int bufLen = 1024;
	char buf[100];
	char url[200];
	char *link = "http://nejoo97.cafe24.com/ParkingPointAdd.php?";
	char *argcat[6] = { "index=", "&occupy=", "&startX=", "&startY=", "&lenX=", "&lenY=" };
	int arg[6];
	arg[0] = index;
	arg[1] = occupy;
	arg[2] = startX;
	arg[3] = startY;
	arg[4] = lenX;
	arg[5] = lenY;
	char tmp[10];
	for (int i = 0; i < 6; i++) {
		itoa(arg[i], tmp, 10);
		sprintf(buf, "%s%s", argcat[i], tmp);
		sprintf(url, "%s%s", link, buf);
		link = url;
	}
	char *szUrl = url;
	printf("%s\m", szUrl);
	long fileSize;
	char *memBuffer, *headerBuffer;
	FILE *fp;

	memBuffer = headerBuffer = NULL;

	if (WSAStartup(0x101, &wsaData) != 0)
		return -1;


	memBuffer = readUrl2(szUrl, fileSize, &headerBuffer);
	printf("returned from readUrl\n");
	printf("data returned:\n%s", memBuffer);
	if (fileSize != 0)
	{
		printf("Got some data\n");
		fp = fopen("downloaded.file", "wb");
		fwrite(memBuffer, 1, fileSize, fp);
		fclose(fp);
		delete(memBuffer);
		delete(headerBuffer);
	}

	WSACleanup();
}

void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename)
{
	string::size_type n;
	string url = mUrl;

	if (url.substr(0, 7) == "http://")
		url.erase(0, 7);

	if (url.substr(0, 8) == "https://")
		url.erase(0, 8);

	n = url.find('/');
	if (n != string::npos)
	{
		serverName = url.substr(0, n);
		filepath = url.substr(n);
		n = filepath.rfind('/');
		filename = filepath.substr(n + 1);
	}

	else
	{
		serverName = url;
		filepath = "/";
		filename = "";
	}
}

SOCKET connectToServer(char *szServerName, WORD portNum)
{
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	SOCKET conn;

	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return NULL;

	if (inet_addr(szServerName) == INADDR_NONE)
	{
		hp = gethostbyname(szServerName);
	}
	else
	{
		addr = inet_addr(szServerName);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (hp == NULL)
	{
		closesocket(conn);
		return NULL;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(portNum);
	if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
	{
		closesocket(conn);
		return NULL;
	}
	return conn;
}

int getHeaderLength(char *content)
{
	const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
	char *findPos;
	int ofset = -1;

	findPos = strstr(content, srchStr1);
	if (findPos != NULL)
	{
		ofset = findPos - content;
		ofset += strlen(srchStr1);
	}

	else
	{
		findPos = strstr(content, srchStr2);
		if (findPos != NULL)
		{
			ofset = findPos - content;
			ofset += strlen(srchStr2);
		}
	}
	return ofset;
}

char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut)
{
	const int bufSize = 512;
	char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;

	mParseUrl(szUrl, server, filepath, filename);

	///////////// step 1, connect //////////////////////
	conn = connectToServer((char*)server.c_str(), 80);

	///////////// step 2, send GET request /////////////
	sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
	strcpy(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Host: %s", server.c_str());
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	strcat(sendBuffer, "\r\n");
	send(conn, sendBuffer, strlen(sendBuffer), 0);

	//    SetWindowText(edit3Hwnd, sendBuffer);
	printf("Buffer being sent:\n%s", sendBuffer);

	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);

		if (thisReadSize <= 0)
			break;

		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}

	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	char *myTmp;

	myTmp = new char[headerLen + 1];
	strncpy(myTmp, tmpResult, headerLen);
	myTmp[headerLen] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket(conn);
	return(result);
}
// php






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


// 작은 사각형 라벨링 + 좌표
//void sort()
//{
//	int warp_range_count = 0;
//	
//		// 내가 원하는 색깔 선택
//		//Scalar warp_green(153, 174, 124);
//		Scalar warp_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);
//	
//		Mat warp_rgb_color = Mat(1, 1, CV_8UC3, warp_green);									//1,1짜리 행렬로 생성(dot)
//		Mat warp_hsv_color;
//	
//		cvtColor(warp_rgb_color, warp_hsv_color, COLOR_BGR2HSV);						//BRG->HSV로 변환
//	
//		int warp_hue = (int)warp_hsv_color.at<Vec3b>(0, 0)[0];							//색조
//		int warp_saturation = (int)warp_hsv_color.at<Vec3b>(0, 0)[1];					//채도
//		int warp_value = (int)warp_hsv_color.at<Vec3b>(0, 0)[2];						//명도
//	
//		cout << endl;
//		cout << "warp_img hue = " << warp_hue << endl;
//		cout << "warp_img saturation = " << warp_saturation << endl;
//		cout << "warp_img value = " << warp_value << endl;
//		cout << endl;
//	
//		int warp_low_hue = warp_hue - 10;
//		int warp_high_hue = warp_hue + 6;
//	
//		int warp_low_hue1 = 0, warp_low_hue2 = 0;
//		int warp_high_hue1 = 0, warp_high_hue2 = 0;
//	
//		if (warp_low_hue < 10) {
//			warp_range_count = 2;
//	
//			warp_high_hue1 = 180;
//			warp_low_hue1 = warp_low_hue + 180;
//			warp_high_hue2 = warp_high_hue;
//			warp_low_hue2 = 0;
//		}
//		else if (warp_high_hue > 170) {
//			warp_range_count = 2;
//	
//			warp_high_hue1 = warp_low_hue;
//			warp_low_hue1 = 180;
//			warp_high_hue2 = warp_high_hue - 180;
//			warp_low_hue2 = 0;
//		}
//		else {
//			warp_range_count = 1;
//			warp_low_hue1 = warp_low_hue;
//			warp_high_hue1 = warp_high_hue;
//		}
//	
//		cout << warp_low_hue1 << "  " << warp_high_hue1 << endl;
//		cout << warp_low_hue2 << "  " << warp_high_hue2 << endl;
//		cout << endl;
//	
//		Mat warp_img_hsv;
//	for(;;){
//
//		//small_warpImg = imread("warpImg.jpg", IMREAD_COLOR);
//		//CV_Assert(small_warpImg.data);
//
//		//HSV로 변환
//				cvtColor(small_warpImg, warp_img_hsv, COLOR_BGR2HSV);					//BRG->HSV로 변환
//
//				// 지정한 HSV 범위를 이용하여 영상을 이진화
//				//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
//				inRange(warp_img_hsv, Scalar(warp_low_hue1, 20, 50), Scalar(warp_high_hue1, 255, 255), warp_img_mask1);
//				if (warp_range_count == 2) {
//					inRange(warp_img_hsv, Scalar(warp_low_hue2, 20, 50), Scalar(warp_high_hue2, 255, 255), warp_img_mask2);
//					warp_img_mask1 |= warp_img_mask2;
//				}
//		
//				imshow("warpImg 모폴로지 전", warp_img_mask1);
//			
//				// morphological opening 작은 점들을 제거 (노이즈 제거)
//				erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(21, 21)));		//모폴로지침식
//				dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(25, 25)));		//모폴로지팽창
//				imshow("warpImg 모폴로지 후", warp_img_mask1);
//
//		// 라벨링 
//		int warp_numOfLables = connectedComponentsWithStats(warp_img_mask1, warp_img_labels,
//			warp_stats, warp_centroids, 8, CV_32S);
//
//		//영역박스 그리기
//		int warp_max = -1, warp_idx = 0;
//		for (int warp_j = 1; warp_j <warp_numOfLables; warp_j++) {
//			int warp_area = warp_stats.at<int>(warp_j, CC_STAT_AREA);
//			int warp_left = warp_stats.at<int>(warp_j, CC_STAT_LEFT);
//			int warp_top = warp_stats.at<int>(warp_j, CC_STAT_TOP);
//			int warp_width = warp_stats.at<int>(warp_j, CC_STAT_WIDTH);
//			int warp_height = warp_stats.at<int>(warp_j, CC_STAT_HEIGHT);
//
//			if (warp_max < warp_area)
//			{
//				warp_max = warp_area;
//				warp_idx = i;
//				rectangle(small_warpImg, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
//					Scalar(0, 0, 255), 3);
//			}
//
//			rectangle(small_warpImg, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
//				Scalar(0, 0, 255), 2);
//
//			putText(small_warpImg, to_string(warp_j), Point(warp_left + 20, warp_top + 20),
//				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
//			
//			cout << warp_j - 1 << "번째 주차칸 좌표 " << endl;
//			cout << Point(warp_left, warp_top) << endl;
//			cout << Point(warp_left + warp_width, warp_top) << endl;
//			cout << Point(warp_left + warp_width, warp_top + warp_height) << endl;
//			cout << Point(warp_left, warp_top + warp_height) << endl;
//			cout << endl;
//
//			InsertDB(warp_j - 1, 0, warp_left, warp_top, warp_width, warp_height);
//		
//		}
//		
//		imshow("라벨링 된 warpImg", small_warpImg);
//		break;
//		if (waitKey(5) >= 0)
//			break;
//	}
//}

// 헤리스 코너 검출
void cornerharris(Mat img_frame, Mat& corner, int bSize, int ksize, float k)
{
	Mat dx, dy, dxy, dxx, dyy;
	corner = Mat(img_frame.size(), CV_32F, Scalar(0));

	Sobel(img_frame, dx, CV_32F, 1, 0, ksize);					// 미분 행렬 - 수평 소벨 마스크
	Sobel(img_frame, dy, CV_32F, 0, 1, ksize);					// 미분 행렬 - 수직 소벨 마스크
	multiply(dx, dy, dxx);										// 미분 행렬 제곱
	multiply(dy, dy, dyy);
	multiply(dx, dy, dxy);

	Size msize(5, 5);
	GaussianBlur(dxx, dxx, msize, 0);							// 가우시안 블러링
	GaussianBlur(dyy, dyy, msize, 0);
	GaussianBlur(dxy, dxy, msize, 0);

	// 코너 응답 함수 계산
	for (int i = 0; i < img_frame.rows; i++)
	{
		for (int j = 0; j < img_frame.cols; j++)
		{
			float a = dxx.at<float>(i, j);
			float b = dyy.at<float>(i, j);
			float c = dxy.at<float>(i, j);
			corner.at<float>(i, j) = (a*b - c*c) - k*(a + b)*(a + b);
		}
	}
}

Mat draw_coner(Mat corner, Mat img_frame, int thresh)			// 임계값이상인 코너 표시
{
	int cnt = 0;
	normalize(corner, corner, 0, 100, NORM_MINMAX, CV_32FC1, Mat());

	for (int i = 0; i < corner.rows; i++)
	{
		for (int j = 0; j < corner.cols; j++)
		{
			float cur = (int)corner.at<float>(i, j);			// 코너 응답값
			if (cur > thresh)
			{
				if (cur > corner.at<float>(i - 1, j) &&
					cur > corner.at<float>(i + 1, j) &&
					cur > corner.at<float>(i, j - 1) &&
					cur > corner.at<float>(i, j + 1))
				{
					circle(img_frame, Point(j, i), 2, Scalar(255, 0, 0), -1);		// 좌표 표시
					cnt++;															// 개수 계산
				}
			}
		}
	}

	cout << "코너 개수 : " << cnt << endl;
	return img_frame;
}

void cornerHarris_demo(int thresh, void*)
{
	Mat img = draw_coner(corner1, img_frame.clone(), thresh);	 // 코너 표시
	imshow("img", img);
}

// RGB 값 출력

int main()
{
	RecordVideo();
	//warp(img_frame.clone());            // 원근 변환 수행

	//imshow("img_frame", img_frame);										// BGR값 출력시 사용
	setMouseCallback("img_frame", MouseEventRGB, (&img_frame));			// BGR값 출력용 마우스 이벤트

	for (;;) {
		switch (step) {
		case 1:
			CheckBackGround();			// 모폴로지 + 라벨링 함수
			step = 2;
			break;
			//case 2:
			//	vector<Point2f> corners(4);											// Warping 전의 이미지 상의 좌표
			//	corners[0] = Point2f(p_left, p_top);
			//	corners[1] = Point2f(p_left + p_width, p_top);
			//	corners[2] = Point2f(p_left + p_width, p_top + p_height);
			//	corners[3] = Point2f(p_left, p_top + p_height);

			//	vector<Point2f> warpCorners(4);										// Warping 후의 좌표
			//	warpCorners[0] = Point2f(0, 0);
			//	warpCorners[1] = Point2f(warpImg.cols, 0);
			//	warpCorners[2] = Point2f(warpImg.cols, warpImg.rows);
			//	warpCorners[3] = Point2f(0, warpImg.rows);

			//	// Transformation Matrix 구하기
			//	Mat trans = getPerspectiveTransform(corners, warpCorners);

			//	//Warping
			//	imshow("원본 라벨링", img_frame);

			//	img_frame = imread("A.jpg", IMREAD_COLOR);
			//	warpPerspective(img_mask1, warpImg, trans, warpSize);		// 모폴로지 흑백
			//	imshow("warpImg", warpImg);
			//	warpPerspective(img_frame, warpImg, trans, warpSize);		// 컬러
			//	imshow("war", warpImg);
			//	imwrite("warpImg.jpg", warpImg);
			//	step = 3;
			break;
		case 3:
			small_warpImg = imread("warpImg.jpg", IMREAD_COLOR);
			CV_Assert(small_warpImg.data);
			//sort();
			step = 4;
			//break;
			break;
		}

		if (waitKey(30) >= 0) continue;
	}

	// 헤리스 코너 검출용
	//int blockSize = 4;					// 이웃화소 범위
	//int apertureSize = 3;				// 소벨 마스크 크기
	//double k = 0.04;
	//int thresh = 20;
	//Mat gray;							// 코너 응답 임계값

	//cvtColor(img_frame, gray, CV_BGR2GRAY);
	//cornerharris(gray, corner1, blockSize, apertureSize, k);		// 직접 구현 함수

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











//함수

void RecordVideo() {
	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);
}


void MouseEventRGB(int event, int x, int y, int flags, void *param)//주차장 색깔 검출 마우스 이벤트
{
	rgb = (Mat*)param;
	if (event == CV_EVENT_LBUTTONDOWN)//왼쪽버튼 클릭시
	{
		step = 1;
		px = x;
		py = y;
	}
}


void CheckBackGround() {
	int range_count = 0;

	// 내가 원하는 색깔 선택
	Scalar color((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	Mat rgb_color = Mat(1, 1, CV_8UC3, color);				//1,1짜리 행렬로 생성(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);			//BRG->HSV로 변환

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];			//색조
															//int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];		//채도
															//int value = (int)hsv_color.at<Vec3b>(0, 0)[2];			//명도

															/*cout << endl;
															cout << "img_frame hue = " << hue << endl;
															cout << "img_frame saturation = " << saturation << endl;
															cout << "img_frame value = " << value << endl;
															cout << endl;*/

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

	/*cout << low_hue1 << "  " << high_hue1 << endl;
	cout << low_hue2 << "  " << high_hue2 << endl;
	cout << endl;
	*/
	Mat img_hsv;
	//img_frame = imread("A.jpg", IMREAD_COLOR);

	// HSV로 변환
	cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV로 변환

												// 지정한 HSV 범위를 이용하여 영상을 이진화
												//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
	inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	//imshow("모폴로지 전", img_mask1);
	// morphological opening 작은 점들을 제거 (노이즈 제거)
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(19, 19)));		//모폴로지침식
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(37, 37)));		//모폴로지팽창
																							//imshow("모폴로지 후", img_mask1);
	Labeling();

}

void Labeling() {
	int p_area, p_left, p_top, p_width, p_height;
	int i = 1;


	// 영역박스 그리기
	int max = -1, idx = 0;
	int count = 2;


	// 라벨링 
	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);


	for (;;)//제일 큰 사각형을 idx에 index를 저장함
	{
		p_area = stats.at<int>(i, CC_STAT_AREA);

		if (max < p_area)
		{
			max = p_area;
			idx = i;
		}
		i++;
		if (i == numOfLables)
			break;
	}

	i = 1;

	for (;;) {

		p_left = stats.at<int>(i, CC_STAT_LEFT);
		p_top = stats.at<int>(i, CC_STAT_TOP);
		p_width = stats.at<int>(i, CC_STAT_WIDTH);
		p_height = stats.at<int>(i, CC_STAT_HEIGHT);

		if (i == idx) {

			rectangle(img_frame, Point(p_left, p_top), Point(p_left + p_width, p_top + p_height),
				Scalar(0, 0, 255), 3);

			putText(img_frame, to_string(i), Point(p_left + 20, p_top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
		}
		else {
			rectangle(img_frame, Point(p_left, p_top), Point(p_left + p_width, p_top + p_height),
				Scalar(0, 0, 255), 1);

			putText(img_frame, to_string(i), Point(p_left + 20, p_top + 20),
				FONT_HERSHEY_SIMPLEX, count, Scalar(255, 0, 0), 2);
			count++;
		}
		i++;
		if (i == numOfLables)
			break;


		//제일 큰 사각형 출력

		//count = 1;
		//int x = centroids.at<double>(j, 0);      //중심좌표
		//int y = centroids.at<double>(j, 1);

		//circle(img_frame, Point(x, y), 5, Scalar(255, 0, 0), 1);

		//

		//

		//cout << "왼쪽 위 점부터 시계방향 순 " << endl;

		//if (count == 1)
		//{
		//	cout << Point(p_left, p_top) << endl;
		//	cout << Point(p_left + p_width, p_top) << endl;
		//	cout << Point(p_left + p_width, p_top + p_height) << endl;
		//	cout << Point(p_left, p_top + p_height);
		//	count++;
		//}

	}


}