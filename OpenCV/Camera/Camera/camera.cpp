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


Mat img_frame;													// 원본 img
Mat *rgb;														//마우스 이벤트 주차장 색깔
vector<Point2f> corners(4);										//Warping 전의 이미지 상의 좌표
Size warpSize;													// warp 크기
Mat warpImg;													// warp img
Mat small_warpImg;												// roi warpImg


int px, py;//마우스 클릭 좌표
int step = 0; //0시작, 1마우스클릭(주차장색깔)


//php
HINSTANCE hInst;
WSADATA wsaData;


//함수
void RecordVideo();
void MouseEventRGB(int event, int x, int y, int flags, void *param);
void CheckBackGround();
void Warp();
void Labeling();

//php함수
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
SOCKET connectToServer(char *szServerName, WORD portNum);
int getHeaderLength(char *content);
char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut);
int InsertDB(int index, int occupy, int startX, int startY, int lenX, int lenY);









//InsertDB(12, 0, 2, 3, 4, 5);





int main()
{
	RecordVideo();
	imshow("원본", img_frame);//마우스 이벤트용 출력
	setMouseCallback("원본", MouseEventRGB, (&img_frame));			// BGR값 출력용 마우스 이벤트

	for (;;) {
		switch (step) {
		case 1://제일 큰 사각형 찾기
			CheckBackGround();			// 모폴로지
			step++;
			break;
		case 2://WrapImg저장
			Warp();
			step++;
			break;
		case 3://라벨링
			Labeling();
			step++;
			break;
		case 4://Trace
			step++;
			break;
		}

		if (waitKey(30) >= 0) continue;
	}
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

	Mat img_labels, stats, centroids;								// 원본 img 큰 사각형 좌표 관련 변수 목록
	Mat img_mask1, img_mask2;
	int range_count = 0;

	// 내가 원하는 색깔 선택
	Scalar color((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	Mat rgb_color = Mat(1, 1, CV_8UC3, color);				//1,1짜리 행렬로 생성(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);			//BRG->HSV로 변환

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];			//색조

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

	Mat img_hsv;
	// HSV로 변환
	cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV로 변환

	inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(19, 19)));		//모폴로지침식
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(37, 37)));		//모폴로지팽창


	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);

	int p_left, p_top, p_width, p_height;


	p_left = stats.at<int>(1, CC_STAT_LEFT);
	p_top = stats.at<int>(1, CC_STAT_TOP);
	p_width = stats.at<int>(1, CC_STAT_WIDTH);
	p_height = stats.at<int>(1, CC_STAT_HEIGHT);

	corners[0] = Point2f(p_left, p_top);
	corners[1] = Point2f(p_left + p_width, p_top);
	corners[2] = Point2f(p_left + p_width, p_top + p_height);
	corners[3] = Point2f(p_left, p_top + p_height);

	//InsertDB(0, 0, p_left, p_top, p_width, p_height);

}


void Warp() {
	vector<Point2f> warpCorners(4);									// Warping 후의 좌표

	warpSize = Size(corners[1].x - corners[0].x, corners[2].y - corners[1].y);	// warp 크기
	warpImg = Mat(warpSize, img_frame.type());							// warp img

	warpCorners[0] = Point2f(0, 0);
	warpCorners[1] = Point2f(warpImg.cols, 0);
	warpCorners[2] = Point2f(warpImg.cols, warpImg.rows);
	warpCorners[3] = Point2f(0, warpImg.rows);

	// Transformation Matrix 구하기
	Mat trans = getPerspectiveTransform(corners, warpCorners);

	//Warping
	img_frame = imread("A.jpg", IMREAD_COLOR);
	warpPerspective(img_frame, warpImg, trans, warpSize);		// 컬러
	imwrite("warpImg.jpg", warpImg);
}

// 작은 사각형 라벨링 + 좌표
void Labeling()
{
	Mat warp_img_labels, warp_stats, warp_centroids;				// warp img 작은 사각형 좌표 관련 변수 목록
	Mat warp_img_mask1;												// warp img 모폴로지 전
	Mat warp_img_mask2;												// warp img 모폴로지 후
	small_warpImg = imread("warpImg.jpg", IMREAD_COLOR);
	CV_Assert(small_warpImg.data);
	int warp_range_count = 0;

	Scalar warp_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	Mat warp_rgb_color = Mat(1, 1, CV_8UC3, warp_green);									//1,1짜리 행렬로 생성(dot)
	Mat warp_hsv_color;

	cvtColor(warp_rgb_color, warp_hsv_color, COLOR_BGR2HSV);						//BRG->HSV로 변환

	int warp_hue = (int)warp_hsv_color.at<Vec3b>(0, 0)[0];							//색조

	int warp_low_hue = warp_hue - 10;
	int warp_high_hue = warp_hue + 6;

	int warp_low_hue1 = 0, warp_low_hue2 = 0;
	int warp_high_hue1 = 0, warp_high_hue2 = 0;

	if (warp_low_hue < 10) {
		warp_range_count = 2;

		warp_high_hue1 = 180;
		warp_low_hue1 = warp_low_hue + 180;
		warp_high_hue2 = warp_high_hue;
		warp_low_hue2 = 0;
	}
	else if (warp_high_hue > 170) {
		warp_range_count = 2;

		warp_high_hue1 = warp_low_hue;
		warp_low_hue1 = 180;
		warp_high_hue2 = warp_high_hue - 180;
		warp_low_hue2 = 0;
	}
	else {
		warp_range_count = 1;
		warp_low_hue1 = warp_low_hue;
		warp_high_hue1 = warp_high_hue;
	}

	Mat warp_img_hsv;
	for (;;) {

		//HSV로 변환
		cvtColor(small_warpImg, warp_img_hsv, COLOR_BGR2HSV);					//BRG->HSV로 변환

																				// 지정한 HSV 범위를 이용하여 영상을 이진화
		inRange(warp_img_hsv, Scalar(warp_low_hue1, 20, 50), Scalar(warp_high_hue1, 255, 255), warp_img_mask1);
		if (warp_range_count == 2) {
			inRange(warp_img_hsv, Scalar(warp_low_hue2, 20, 50), Scalar(warp_high_hue2, 255, 255), warp_img_mask2);
			warp_img_mask1 |= warp_img_mask2;
		}

		// morphological opening 작은 점들을 제거 (노이즈 제거)
		erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(21, 21)));		//모폴로지침식
		dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(25, 25)));		//모폴로지팽창

																										// 라벨링 
		int warp_numOfLables = connectedComponentsWithStats(warp_img_mask1, warp_img_labels,
			warp_stats, warp_centroids, 8, CV_32S);

		//영역박스 그리기
		int warp_max = -1, warp_idx = 0;
		for (int warp_j = 1; warp_j < warp_numOfLables; warp_j++) {
			int warp_area = warp_stats.at<int>(warp_j, CC_STAT_AREA);
			int warp_left = warp_stats.at<int>(warp_j, CC_STAT_LEFT);
			int warp_top = warp_stats.at<int>(warp_j, CC_STAT_TOP);
			int warp_width = warp_stats.at<int>(warp_j, CC_STAT_WIDTH);
			int warp_height = warp_stats.at<int>(warp_j, CC_STAT_HEIGHT);


			rectangle(small_warpImg, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
				Scalar(0, 0, 255), 2);

			putText(small_warpImg, to_string(warp_j), Point(warp_left + 20, warp_top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

			//InsertDB(warp_j - 1, 0, warp_left, warp_top, warp_width, warp_height);
		}
		imshow("라벨링 된 warpImg", small_warpImg);
		break;
	}
}


// php
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
	//printf("%s\m", szUrl);
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