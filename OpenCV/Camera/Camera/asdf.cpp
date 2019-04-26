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

Point2f pts[4], smallrect(10, 10);								// 4�� ��ǥ�� ���� �迭 �� �� ��ǥ�� �簢�� ũ��
Size warpSize(500, 720);										// warp ũ��
Mat img_mask1;													// ���� img �������� ��
Mat img_mask2;													// ���� img �������� �� 
Mat corner1, corner2;											// �ڳ� ����� ����
vector<Point2f> corners;										// �츮�� �ڳ� �����

Mat img_labels, stats, centroids;								// ���� img ū �簢�� ��ǥ ���� ���� ���



Mat warp_img_labels, warp_stats, warp_centroids;				// warp img ���� �簢�� ��ǥ ���� ���� ���
int warp_j = 1;
int warp_area;
int warp_left;
int warp_top;
int warp_width;
int warp_height;
Mat warp_img_mask1;												// warp img �������� ��
Mat warp_img_mask2;												// warp img �������� ��
Mat small_warpImg;												// roi warpImg






Mat img_frame;													// ���� img
Mat *rgb;														//���콺 �̺�Ʈ ������ ����



																//

Mat warpImg(warpSize, img_frame.type());						// warp img
																//


int px, py;//���콺 Ŭ�� ��ǥ
int step = 0; //0����, 1���콺Ŭ��(���������)


			  //�Լ�
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

void supp_nonMax(Mat sobel, Mat  direct, Mat& dst)			// ���ִ밪 ����
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
		pos_ck.at<uchar>(pt) = 1;			// ���� �Ϸ� ��ǥ
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


// ���� �簢�� �󺧸� + ��ǥ
//void sort()
//{
//	int warp_range_count = 0;
//	
//		// ���� ���ϴ� ���� ����
//		//Scalar warp_green(153, 174, 124);
//		Scalar warp_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);
//	
//		Mat warp_rgb_color = Mat(1, 1, CV_8UC3, warp_green);									//1,1¥�� ��ķ� ����(dot)
//		Mat warp_hsv_color;
//	
//		cvtColor(warp_rgb_color, warp_hsv_color, COLOR_BGR2HSV);						//BRG->HSV�� ��ȯ
//	
//		int warp_hue = (int)warp_hsv_color.at<Vec3b>(0, 0)[0];							//����
//		int warp_saturation = (int)warp_hsv_color.at<Vec3b>(0, 0)[1];					//ä��
//		int warp_value = (int)warp_hsv_color.at<Vec3b>(0, 0)[2];						//��
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
//		//HSV�� ��ȯ
//				cvtColor(small_warpImg, warp_img_hsv, COLOR_BGR2HSV);					//BRG->HSV�� ��ȯ
//
//				// ������ HSV ������ �̿��Ͽ� ������ ����ȭ
//				//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
//				inRange(warp_img_hsv, Scalar(warp_low_hue1, 20, 50), Scalar(warp_high_hue1, 255, 255), warp_img_mask1);
//				if (warp_range_count == 2) {
//					inRange(warp_img_hsv, Scalar(warp_low_hue2, 20, 50), Scalar(warp_high_hue2, 255, 255), warp_img_mask2);
//					warp_img_mask1 |= warp_img_mask2;
//				}
//		
//				imshow("warpImg �������� ��", warp_img_mask1);
//			
//				// morphological opening ���� ������ ���� (������ ����)
//				erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(21, 21)));		//��������ħ��
//				dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(25, 25)));		//����������â
//				imshow("warpImg �������� ��", warp_img_mask1);
//
//		// �󺧸� 
//		int warp_numOfLables = connectedComponentsWithStats(warp_img_mask1, warp_img_labels,
//			warp_stats, warp_centroids, 8, CV_32S);
//
//		//�����ڽ� �׸���
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
//			cout << warp_j - 1 << "��° ����ĭ ��ǥ " << endl;
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
//		imshow("�󺧸� �� warpImg", small_warpImg);
//		break;
//		if (waitKey(5) >= 0)
//			break;
//	}
//}

// �츮�� �ڳ� ����
void cornerharris(Mat img_frame, Mat& corner, int bSize, int ksize, float k)
{
	Mat dx, dy, dxy, dxx, dyy;
	corner = Mat(img_frame.size(), CV_32F, Scalar(0));

	Sobel(img_frame, dx, CV_32F, 1, 0, ksize);					// �̺� ��� - ���� �Һ� ����ũ
	Sobel(img_frame, dy, CV_32F, 0, 1, ksize);					// �̺� ��� - ���� �Һ� ����ũ
	multiply(dx, dy, dxx);										// �̺� ��� ����
	multiply(dy, dy, dyy);
	multiply(dx, dy, dxy);

	Size msize(5, 5);
	GaussianBlur(dxx, dxx, msize, 0);							// ����þ� ����
	GaussianBlur(dyy, dyy, msize, 0);
	GaussianBlur(dxy, dxy, msize, 0);

	// �ڳ� ���� �Լ� ���
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

Mat draw_coner(Mat corner, Mat img_frame, int thresh)			// �Ӱ谪�̻��� �ڳ� ǥ��
{
	int cnt = 0;
	normalize(corner, corner, 0, 100, NORM_MINMAX, CV_32FC1, Mat());

	for (int i = 0; i < corner.rows; i++)
	{
		for (int j = 0; j < corner.cols; j++)
		{
			float cur = (int)corner.at<float>(i, j);			// �ڳ� ���䰪
			if (cur > thresh)
			{
				if (cur > corner.at<float>(i - 1, j) &&
					cur > corner.at<float>(i + 1, j) &&
					cur > corner.at<float>(i, j - 1) &&
					cur > corner.at<float>(i, j + 1))
				{
					circle(img_frame, Point(j, i), 2, Scalar(255, 0, 0), -1);		// ��ǥ ǥ��
					cnt++;															// ���� ���
				}
			}
		}
	}

	cout << "�ڳ� ���� : " << cnt << endl;
	return img_frame;
}

void cornerHarris_demo(int thresh, void*)
{
	Mat img = draw_coner(corner1, img_frame.clone(), thresh);	 // �ڳ� ǥ��
	imshow("img", img);
}

// RGB �� ���

int main()
{
	RecordVideo();
	//warp(img_frame.clone());            // ���� ��ȯ ����

	//imshow("img_frame", img_frame);										// BGR�� ��½� ���
	setMouseCallback("img_frame", MouseEventRGB, (&img_frame));			// BGR�� ��¿� ���콺 �̺�Ʈ

	for (;;) {
		switch (step) {
		case 1:
			CheckBackGround();			// �������� + �󺧸� �Լ�
			step = 2;
			break;
			//case 2:
			//	vector<Point2f> corners(4);											// Warping ���� �̹��� ���� ��ǥ
			//	corners[0] = Point2f(p_left, p_top);
			//	corners[1] = Point2f(p_left + p_width, p_top);
			//	corners[2] = Point2f(p_left + p_width, p_top + p_height);
			//	corners[3] = Point2f(p_left, p_top + p_height);

			//	vector<Point2f> warpCorners(4);										// Warping ���� ��ǥ
			//	warpCorners[0] = Point2f(0, 0);
			//	warpCorners[1] = Point2f(warpImg.cols, 0);
			//	warpCorners[2] = Point2f(warpImg.cols, warpImg.rows);
			//	warpCorners[3] = Point2f(0, warpImg.rows);

			//	// Transformation Matrix ���ϱ�
			//	Mat trans = getPerspectiveTransform(corners, warpCorners);

			//	//Warping
			//	imshow("���� �󺧸�", img_frame);

			//	img_frame = imread("A.jpg", IMREAD_COLOR);
			//	warpPerspective(img_mask1, warpImg, trans, warpSize);		// �������� ���
			//	imshow("warpImg", warpImg);
			//	warpPerspective(img_frame, warpImg, trans, warpSize);		// �÷�
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

	// �츮�� �ڳ� �����
	//int blockSize = 4;					// �̿�ȭ�� ����
	//int apertureSize = 3;				// �Һ� ����ũ ũ��
	//double k = 0.04;
	//int thresh = 20;
	//Mat gray;							// �ڳ� ���� �Ӱ谪

	//cvtColor(img_frame, gray, CV_BGR2GRAY);
	//cornerharris(gray, corner1, blockSize, apertureSize, k);		// ���� ���� �Լ�

	////���� ĸ�� �ʱ�ȭ
	//VideoCapture cap(0);
	//if (!cap.isOpened()) {
	//	cerr << "���� - ī�޶� �� �� �����ϴ�.\n";
	//	return -1;
	//}

	//

	//VideoCapture capture(1);											// ������ü ���� �� 0�� ī�޶� ����
	//CV_Assert(capture.isOpened());

	//Mat frame;
	//double fps = 30.0;												// �ʴ� ������ ��
	////int delay = cvRound(1000.0 / fps);								// �����Ӱ� �����ð�
	//Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
	//	(int)cap.get(CAP_PROP_FRAME_HEIGHT));
	//// ������ ���� �ػ�
	//int fourcc = VideoWriter::fourcc('X', 'V', 'I', 'D');				//���� �ڵ� ����

	//cout << "width x height : " << size << endl;				
	//cout << "VideoWriter::fourcc : " << fourcc << endl;				// ������ ���� �ܼ�â ���
	////cout << "delay : " << delay << endl;
	//cout << "fps : " << fps << endl;

	//VideoWriter writer;												// ������ ���� ���� ��ü
	//writer.open("video_file.avi", fourcc, fps, size, true);			// ���� ���� �� ����
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











//�Լ�

void RecordVideo() {
	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);
}


void MouseEventRGB(int event, int x, int y, int flags, void *param)//������ ���� ���� ���콺 �̺�Ʈ
{
	rgb = (Mat*)param;
	if (event == CV_EVENT_LBUTTONDOWN)//���ʹ�ư Ŭ����
	{
		step = 1;
		px = x;
		py = y;
	}
}


void CheckBackGround() {
	int range_count = 0;

	// ���� ���ϴ� ���� ����
	Scalar color((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	Mat rgb_color = Mat(1, 1, CV_8UC3, color);				//1,1¥�� ��ķ� ����(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);			//BRG->HSV�� ��ȯ

	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];			//����
															//int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];		//ä��
															//int value = (int)hsv_color.at<Vec3b>(0, 0)[2];			//��

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

	// HSV�� ��ȯ
	cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV�� ��ȯ

												// ������ HSV ������ �̿��Ͽ� ������ ����ȭ
												//inRange(rgb_color, Scalar(50, 50, 50), Scalar(90, 90, 90), img_mask1);
	inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 255), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	//imshow("�������� ��", img_mask1);
	// morphological opening ���� ������ ���� (������ ����)
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(19, 19)));		//��������ħ��
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(37, 37)));		//����������â
																							//imshow("�������� ��", img_mask1);
	Labeling();

}

void Labeling() {
	int p_area, p_left, p_top, p_width, p_height;
	int i = 1;


	// �����ڽ� �׸���
	int max = -1, idx = 0;
	int count = 2;


	// �󺧸� 
	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);


	for (;;)//���� ū �簢���� idx�� index�� ������
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


		//���� ū �簢�� ���

		//count = 1;
		//int x = centroids.at<double>(j, 0);      //�߽���ǥ
		//int y = centroids.at<double>(j, 1);

		//circle(img_frame, Point(x, y), 5, Scalar(255, 0, 0), 1);

		//

		//

		//cout << "���� �� ������ �ð���� �� " << endl;

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