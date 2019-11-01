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


Mat img_frame;																			// ���� img
Mat *rgb;																				// ���콺 �̺�Ʈ ������ ����
Mat dis_img_result;																		// �ְ�� �� img
vector<Point2f> corners(4);																// Warping ���� �̹��� ���� ��ǥ
Mat warpImg;																			// ������ �󺧸�
Mat small_warpImg;																		// ����ĭ�� �󺧸�
Mat dumi_img;																			// �������� ���� �� ������ img
Mat dumi_dis_img;																		// �������� ���� �� �ְ�� �� img
//Mat dumi_small_img;																		// �������� ����ĭ�� �󺧸�
Mat warp_rgb_color;																		// ����ĭ ����
Mat dumi_rgb_color;																		// �������� ���� �� ����ĭ ����
Scalar warp_green;																		// ����ĭ ����(��ǥ���� ���� RGB)
Vec3b lower_blue1, upper_blue1, lower_blue2, upper_blue2, lower_blue3, upper_blue3;     // trackingHSV
Mat img_track;																			// track img


int threshold1 = 100;                   //fortracking
int px, py;                             //���콺 Ŭ�� ��ǥ
int step = 0;                           //0����, 1���콺Ŭ��(���������)

int p_left, p_top, p_width, p_height;   // �ְ��
int minLeft[2];//y, x					// �ְ�� �� ��ǥ
int minRight[2];//y, x					// �ְ�� �� ��ǥ
int minBLeft[2];//y, x					// �ְ�� �� ��ǥ
int minBRight[2];//y, x					// �ְ�� �� ��ǥ
Mat perspect_map, perspect_map1;						// �ְ�� ����

int warp_j, warp_left, warp_top, warp_height, warp_width; // �󺧸� ��ǥ
int dumi_j, dumi_left, dumi_top, dumi_height, dumi_width; // �������� �󺧸� ��ǥ


const static int SENSITIVITY_VALUE = 20;   // Tracking
const static int BLUR_SIZE = 10;
int theObject[2] = { 0,0 };
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);


//php
HINSTANCE hInst;
WSADATA wsaData;


//�Լ�
int captureVideo();
void RecordVideo();
void MouseEventRGB(int event, int x, int y, int flags, void *param);
void Distortion();
void Labeling();
void DumiCapture();
int DumiCaptureFunction();
void DumiDistortion();
void DumiLabeling();
void CheckDummy();
int Frame(int argc, char** argv);
void CreateFolder(const char* path);
void track_mouse_callback(int event, int x, int y, int flags, void *param);
int webcam(int, char**);
int Tracking();
//int RealTimePicture();
//int AddPicture();


//php�Լ�
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
SOCKET connectToServer(char *szServerName, WORD portNum);
int getHeaderLength(char *content);
char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut);
int DB_Insert_ParkingPoint(int index, int occupy, int startX, int startY, int lenX, int lenY);

int DB_Update_ParkingPoint(int point, int occupy);

//InsertDB(12, 0, 2, 3);



class ParkingPoint {
public:
	int point;
	int occupy;
	int startX;
	int startY;
	int lenX;
	int lenY;

	ParkingPoint(int _point, int _occupy, int _startX, int _startY, int _lenX, int _lenY) {
		point = _point;
		occupy = _occupy;
		startX = _startX;
		startY = _startY;
		lenX = _lenX;
		lenY = _lenY;
	}
	void Show() {
		cout << point << "/" << occupy << "/" << startX << "/" << startY << "/" << lenX << "/" << lenY << endl;
	}
	int getlenX() { return lenX; }
};


class PList {
public:
	ParkingPoint *pList[50];
	int index = 0;

	PList() {}
	void AddParkingPoint(ParkingPoint p) {
		pList[index++] = new ParkingPoint(p);
	}
	void Sort() {
		int max = 0;
		int ind = 0;
		for (int i = 0; i < index; i++) {
			if (max < pList[i]->lenX) {
				max = pList[i]->lenX;
				ind = i;
			}
		}
		if (ind != 0) {
			ParkingPoint * p = new ParkingPoint(*pList[ind]);
			pList[ind] = pList[0];
			pList[0] = p;
		}
	}
	void ShowAll() {
		for (int i = 0; i < index; i++) {
			pList[i]->Show();
		}
	}
	void SetIndexZero() {
		index = 0;
	}
	void UploadDB() {
		int sumX = 0;
		int sumY = 0;
		int sX = 0;
		int sY = 0;
		for (int i = 0; i < index; i++) {
			if (i == 0) {//0
				DB_Insert_ParkingPoint(i, 0, pList[i]->lenX, pList[i]->lenY, 0, 0);
				sX = pList[i]->startX;
				sY = pList[i]->startY;
			}
			else {//1~24
				DB_Insert_ParkingPoint(i, 0, pList[i]->startX-sX, pList[i]->startY-sY, pList[i]->lenX, pList[i]->lenY);
			}
		}
	}
};

PList plist;
PList pdlist;



int main()
{
	captureVideo();												    // ��ü ������ ĸ��
	RecordVideo();
	imshow("����", img_frame);										//���콺 �̺�Ʈ�� ���
	setMouseCallback("����", MouseEventRGB, (&img_frame));          // BGR�� ��¿� ���콺 �̺�Ʈ

	for (;;) {
		switch (step) {
		case 1://�������� + ���� �ְ� ����
			
			Distortion();
			step++;
			break;
		case 2://�󺧸�
			
			Labeling();
			break;
		case 3://�������� ��ġ �� ���� �Կ�
			
			plist.UploadDB();
			DumiCapture();
			step++;
			break;
		case 4://�������� �������� + ���� �ְ� ����
			
			DumiDistortion();
			step++;
			break;
		case 5://�������� �󺧸�
			
			//cout << "���� �󺧸�" << endl;
			Labeling();
			step++;
		case 6://�������� ����������
			
			/*RealTimePicture();
			AddPicture();
			Tracking();*/
			break;
		default:
			
			/*RealTimePicture();
			AddPicture();
			Tracking();*/
			break;
		}

		if (waitKey(30) >= 0) continue;
	}
	waitKey(0);
	return 0;
}


//�Լ�

int captureVideo()      // ������ �Կ��ϱ�
{
	Mat img_capture;

	//���� ĸ�� �ʱ�ȭ
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "���� - ī�޶� �� �� �����ϴ�.\n";
		return -1;
	}	


	// ������ ������ �����ϱ� ���� �غ�  
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 30.0;
	writer.open("car2.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
	if (!writer.isOpened())
	{
		cout << "�������� �����ϱ� ���� �ʱ�ȭ �۾� �� ���� �߻�" << endl;
		return 1;
	}


	while (1)
	{
		cap.read(img_capture);
		if (img_capture.empty()) {
			cerr << "�� ������ ĸ�ĵǾ����ϴ�.\n";
			break;
		}

		//������ ���Ͽ� �� �������� ������.  
		writer.write(img_capture);

		imshow("ĸ��", img_capture);
		img_frame = img_capture;
		//imwrite("capture.jpg", img_capture, vector<int>());

		if (waitKey(25) >= 0)
			break;

	}

	writer.release();

	return 0;
}

void RecordVideo() { // ���� �ҷ�����
	//img_frame = imread("capture.jpg", IMREAD_COLOR);
	//CV_Assert(img_frame.data);
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


void Distortion()
{
	Mat img_labels, stats, centroids;                        // ���� img ū �簢�� ��ǥ ���� ���� ���
	Mat img_mask1, img_mask2;
	int range_count = 0;

	// ���� ���ϴ� ���� ����
	Scalar color((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]); // Blue Green Red ����

	
	cout << "������ ���� (Blue, Green, Red ��)" << color << endl;

	Mat rgb_color = Mat(1, 1, CV_8UC3, color);             //1,1¥�� ��ķ� ����(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);         //BGR->HSV�� ��ȯ


	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];           //����

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
	// HSV�� ��ȯ
	cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);//BRG->HSV�� ��ȯ

	inRange(img_hsv, Scalar(low_hue1, 20, 50), Scalar(high_hue1, 255, 150), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	imshow("������������", img_mask1);
	//erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(55, 55)));      //��������ħ��
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(9, 9)));       //����������â
																							//imshow("������������1212121", img_mask1);
	Mat im = img_mask1;
	imshow("������������1", img_mask1);
	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);

	int Marea = 0;
	int ind = 0;
	for (int i = 1; i < numOfLables; i++) {
		int area = stats.at<int>(i, CC_STAT_AREA);
		if (area > Marea) {
			ind = i;
			Marea = area;
		}
	}
	int top = stats.at<int>(ind, CC_STAT_TOP);
	int left = stats.at<int>(ind, CC_STAT_LEFT);
	int width = stats.at<int>(ind, CC_STAT_WIDTH);
	int height = stats.at<int>(ind, CC_STAT_HEIGHT);

	int pix = 0;
	int plus = 20;
	int Larr[9][2];
	int Rarr[9][2];
	int BLarr[9][2];
	int BRarr[9][2];
	int index = 0;
	int minus;

	//���� ��
	while (1) {
		Larr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + plus, left + pix) > 0) break;
			pix++;
			if (pix > 200) break;
		}
		Larr[index++][1] = pix;
		pix = 0;
		plus -= 5;
		if (index >= 9) break;
	}
	minus = Larr[1][1] - Larr[0][1];
	for (int i = 1; i < 8; i++) {
		if (Larr[i + 1][1] - Larr[i][1] < minus + 5 && Larr[i + 1][1] - Larr[i][1] > minus - 5) {
			minLeft[0] = i;
			minus = Larr[i + 1][1] - Larr[i][1];
		}
		else break;
	}

	minLeft[1] = Larr[minLeft[0]][1] + left;
	minLeft[0] = Larr[minLeft[0] + 1][0] + top;

	pix = 0;
	plus = 20;
	index = 0;

	//������ ��
	while (1) {
		Rarr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + plus, left + width - pix) > 0) break;
			pix++;
			if (pix > 200) break;
		}
		Rarr[index++][1] = pix;
		pix = 0;
		plus -= 5;
		if (index >= 9) break;
	}
	minus = Rarr[1][1] - Rarr[0][1];
	for (int i = 1; i < 8; i++) {
		if (Rarr[i + 1][1] - Rarr[i][1] < minus + 5 && Rarr[i + 1][1] - Rarr[i][1] > minus - 5) {
			minRight[0] = i;
			minus = Rarr[i + 1][1] - Rarr[i][1];
		}
		else break;
	}

	minRight[1] = left + width - Rarr[minRight[0]][1];
	minRight[0] = Rarr[minRight[0] + 1][0] + top;


	//���� �Ʒ�
	pix = 0;
	plus = -20;
	index = 0;

	while (1) {
		BLarr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + height + plus, pix) > 0) break;
			pix++;
			if (pix > 400) break;
		}
		BLarr[index++][1] = pix;
		pix = 0;
		plus += 5;
		if (index >= 9) break;
	}
	minBLeft[0] = 0;
	minus = BLarr[1][1] - BLarr[0][1];
	for (int i = 1; i < 8; i++) {
		if (BLarr[i][1] >= 400) break;
		if (BLarr[i + 1][1] - BLarr[i][1] < minus + 10 && BLarr[i + 1][1] - BLarr[i][1] > minus - 10) {
			minBLeft[0] = i;
			minus = BLarr[i + 1][1] - BLarr[i][1];
			printf("ȣ��\n");
		}
		else break;
	}
	minBLeft[1] = BLarr[minBLeft[0]][1];
	minBLeft[0] = BLarr[minBLeft[0] + 1][0] + top + height;

	//������ �Ʒ�
	pix = 0;
	plus = -20;
	index = 0;


	while (1) {
		BRarr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + height + plus, left + width - pix) > 0) break;
			pix++;
			if (pix > 400) break;
		}
		BRarr[index++][1] = pix;
		pix = 0;
		plus += 5;
		if (index >= 9) break;
	}
	minus = BRarr[1][1] - BRarr[0][1];
	minBRight[0] = 0;
	for (int i = 1; i < 8; i++) {
		if (BRarr[i][1] == 401) break;
		if (BRarr[i + 1][1] - BRarr[i][1] < minus + 10 && BRarr[i + 1][1] - BRarr[i][1] > minus - 10) {
			minBRight[0] = i;
			minus = BRarr[i + 1][1] - BRarr[i][1];
		}
		else break;
	}

	minBRight[1] = left + width - BRarr[minBRight[0]][1];
	minBRight[0] = BRarr[minBRight[0] + 1][0] + top + height;

	//imshow("ggg", img_frame);

	p_left = left;
	p_top = top;
	p_width = width;
	p_height = height;

	corners[0] = Point2f(p_left, p_top);
	corners[1] = Point2f(p_left + p_width, p_top);
	corners[2] = Point2f(p_left + p_width, p_top + p_height);
	corners[3] = Point2f(p_left, p_top + p_height);


	cout << "======================����==============================" << endl;
	cout << "���� �� ��ǥ : " << Point2f(minLeft[1], minLeft[0]) << endl;
	cout << "������ �� ��ǥ : " << Point2f(minRight[1], minRight[0]) << endl;
	cout << "������ �Ʒ� ��ǥ : " << Point2f(minBLeft[1], minBLeft[0]) << endl;
	cout << "���� �Ʒ� ��ǥ : " << Point2f(minBRight[1], minBRight[0]) << endl;
	cout << "========================================================" << endl;


	Point2f pts1[4] = {      // ���� ���� ��ǥ 4��
		Point2f(minLeft[1], minLeft[0]),                             // ���� ��
		Point2f(minRight[1], minRight[0]),                           // ������ ��
		Point2f(minBLeft[1], minBLeft[0]),                           // ���� �Ʒ�
		Point2f(minBRight[1], minBRight[0])                          // ������ �Ʒ�
	};

	Point2f pts2[4] = {      // ���� ���� ��ǥ 4��

		Point2f(30, 30),
		Point2f(400, 30),
		Point2f(30, 400),
		Point2f(400, 400)
	};
	VideoCapture c(0);
	c.read(img_frame);
	Mat dis_img_result(img_frame.size(), CV_8UC1);
	perspect_map = getPerspectiveTransform(pts1, pts2);      // ���� ��ȯ ��� ���
	warpPerspective(img_frame, dis_img_result, perspect_map, img_frame.size(), INTER_CUBIC);
	cout << "[perspect_map] = " << endl << perspect_map << endl << endl;

	vector<Point3f> pts3, pts4;      // 3���� ��ǥ�� ������ǥ ǥ��
	for (int i = 0; i < 4; i++)
	{
		pts3.push_back(Point3f(pts1[i].x, pts1[i].y, 1));      // ������ǥ -> ������ǥ ����
	}

	transform(pts3, pts4, perspect_map);      // ��ȯ��ǥ(pts4) ���

	for (int i = 0; i < 4; i++)
	{
		pts4[i] /= pts4[i].z;      // ������ǥ���� w ������
		cout << "pts2[" << i << "]" << pts2[i] << "\t";
		cout << "pts4[" << i << "]" << pts4[i] << endl;

		circle(img_frame, pts1[i], 2, Scalar(0, 0, 255), 2);
		circle(dis_img_result, pts2[i], 2, Scalar(0, 0, 255), 2);
	}

	//imshow("�ְ� ���� ��", img_frame);
	//imshow("�ְ� ���� ��", dis_img_result);
	imwrite("dis_img_result.jpg", dis_img_result);
}


// ���� �簢�� �󺧸� + ��ǥ
void Labeling()
{
	for (;;) {
	Mat warp_img_labels, warp_stats, warp_centroids;       // warp img ���� �簢�� ��ǥ ���� ���� ���
	Mat warp_img_mask1;                                    // warp img �������� ��
	Mat warp_img_mask2;                                    // warp img �������� ��
	small_warpImg = imread("dis_img_result.jpg", IMREAD_COLOR);
	CV_Assert(small_warpImg.data);
	int warp_range_count = 0;

	Mat warp_hsv_color;																		// ����ĭ ���� HSV ��
	Mat warp_img_hsv;																		// BGR���� HSV�� ��ȯ�� ���� ���� ����

	Scalar warp_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	warp_rgb_color = Mat(1, 1, CV_8UC3, warp_green);							   //1,1¥�� ��ķ� ����(dot)
	
	cvtColor(warp_rgb_color, warp_hsv_color, COLOR_BGR2HSV);                       //BRG->HSV�� ��ȯ

	int warp_hue = (int)warp_hsv_color.at<Vec3b>(0, 0)[0];                         //����

	int warp_low_hue = warp_hue - 7;
	int warp_high_hue = warp_hue + 7;

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
		//HSV�� ��ȯ
		cvtColor(small_warpImg, warp_img_hsv, COLOR_BGR2HSV);               //BRG->HSV�� ��ȯ

																			// warp_j HSV ������ �̿��Ͽ� ������ ����ȭ
		inRange(warp_img_hsv, Scalar(warp_low_hue1, 20, 50), Scalar(warp_high_hue1, 255, 200), warp_img_mask1);
		if (warp_range_count == 2) {
			inRange(warp_img_hsv, Scalar(warp_low_hue2, 20, 50), Scalar(warp_high_hue2, 255, 255), warp_img_mask2);
			warp_img_mask1 |= warp_img_mask2;
		}

		// morphological opening ���� ������ ���� (������ ����)
		erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(11, 11)));          //��������ħ��
		dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(9, 9)));         //����������â
																										  //imshow("��������", warp_img_mask1);
		
		imshow("ggg", warp_img_mask1);
																										  // �󺧸� 
		int warp_numOfLables = connectedComponentsWithStats(warp_img_mask1, warp_img_labels,
			warp_stats, warp_centroids, 8, CV_32S);

		// �����ڽ� �׸���
		int warp_max = -1, warp_idx = 0;
		plist.SetIndexZero();
		for (warp_j = 1; warp_j < warp_numOfLables; warp_j++) {
			int warp_area = warp_stats.at<int>(warp_j, CC_STAT_AREA);
			int warp_left = warp_stats.at<int>(warp_j, CC_STAT_LEFT);
			int warp_top = warp_stats.at<int>(warp_j, CC_STAT_TOP);
			int warp_width = warp_stats.at<int>(warp_j, CC_STAT_WIDTH);
			int warp_height = warp_stats.at<int>(warp_j, CC_STAT_HEIGHT);

			plist.AddParkingPoint(ParkingPoint(warp_j-1, 0, warp_left, warp_top, warp_width, warp_height));
			rectangle(small_warpImg, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
				Scalar(0, 0, 255), 2);

			putText(small_warpImg, to_string(warp_j), Point(warp_left + 20, warp_top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

		}
		plist.Sort();
		imshow("�󺧸� �� warpImg", small_warpImg);
		switch (waitKey(10))
		{
		case 13:
			
			step++;
			cout << "����" << endl;
			return;
		}
		cout << "����ĭ ã����...." << endl;
	}
}

int DumiCaptureFunction()
{
	Mat img_capture;

	cout << "���� ĸ�� ȣ��" << endl;
	//���� ĸ�� �ʱ�ȭ
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "���� - ī�޶� �� �� �����ϴ�.\n";
		return -1;
	}


	// ������ ������ �����ϱ� ���� �غ�  
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 30.0;
	writer.open("dumicar.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
	if (!writer.isOpened())
	{
		cout << "�������� �����ϱ� ���� �ʱ�ȭ �۾� �� ���� �߻�" << endl;
		return 1;
	}


	while (1)
	{
		cap.read(img_capture);
		if (img_capture.empty()) {
			cerr << "�� ������ ĸ�ĵǾ����ϴ�.\n";
			break;
		}

		//������ ���Ͽ� �� �������� ������.  
		writer.write(img_capture);

		imshow("�������� ĸ��", img_capture);
		imwrite("dumi_capture.jpg", img_capture, vector<int>());

		if (waitKey(25) >= 0)
			break;

	}

	writer.release();

	return 0;
}

void DumiCapture()
{
	for (;;) {
		switch (waitKey(10))
		{
			case 13:
				DumiCaptureFunction();
			return;
		}
	}
}

void DumiDistortion()
{
	dumi_img = imread("dumi_capture.jpg", IMREAD_COLOR);
	CV_Assert(dumi_img.data);

	Point2f pts1[4] = {      // ���� ���� ��ǥ 4��
		Point2f(minLeft[1], minLeft[0]),                             // ���� ��
		Point2f(minRight[1], minRight[0]),                           // ������ ��
		Point2f(minBLeft[1], minBLeft[0]),                           // ���� �Ʒ�
		Point2f(minBRight[1], minBRight[0])                          // ������ �Ʒ�
	};
	cout << "������" << endl;
	cout << Point2f(minLeft[1], minLeft[0]) << endl;
	cout << Point2f(minRight[1], minRight[0]) << endl;
	cout << Point2f(minBLeft[1], minBLeft[0]) << endl;
	cout << Point2f(minBRight[1], minBRight[0]) << endl;
	cout << "---------------------------" << endl;
	Point2f pts2[4] = {      // ���� ���� ��ǥ 4��

		Point2f(30, 30),
		Point2f(400, 30),
		Point2f(30, 400),
		Point2f(400, 400)
	};

	Mat dumi_dis_img(dumi_img.size(), CV_8UC1);
	perspect_map1 = getPerspectiveTransform(pts1, pts2);      // ���� ��ȯ ��� ���
	warpPerspective(dumi_img, dumi_dis_img, perspect_map1, dumi_img.size(), INTER_CUBIC);
	cout << "[perspect_map] = " << endl << perspect_map1 << endl << endl;

	vector<Point3f> pts3, pts4;      // 3���� ��ǥ�� ������ǥ ǥ��
	for (int i = 0; i < 4; i++)
	{
		pts3.push_back(Point3f(pts1[i].x, pts1[i].y, 1));      // ������ǥ -> ������ǥ ����
	}

	transform(pts3, pts4, perspect_map1);      // ��ȯ��ǥ(pts4) ���

	for (int i = 0; i < 4; i++)
	{
		pts4[i] /= pts4[i].z;      // ������ǥ���� w ������
		cout << "pts2[" << i << "]" << pts2[i] << "\t";
		cout << "pts4[" << i << "]" << pts4[i] << endl;

		circle(dumi_img, pts1[i], 2, Scalar(0, 0, 255), 2);
		circle(dumi_dis_img, pts2[i], 2, Scalar(0, 0, 255), 2);
	}

	//imshow("���� �ְ� ���� ��", dumi_img);
	imshow("���� �ְ� ���� ��", dumi_dis_img);
	imwrite("dumi_distortion.jpg", dumi_dis_img);

	step++;
}

void DumiLabeling()
{
		Mat dumi_img_labels, dumi_stats, dumi_centroids;       // warp img ���� �簢�� ��ǥ ���� ���� ���
		Mat dumi_img_mask1;                                    // warp img �������� ��
		Mat dumi_img_mask2;                                    // warp img �������� ��
		dumi_dis_img = imread("dumi_distortion.jpg", IMREAD_COLOR);

		cout << 0 << endl;
		CV_Assert(dumi_dis_img.data);
		int dumi_range_count = 0;
		cout << 1 << endl;
		Mat dumi_hsv_color;																		// ����ĭ ���� HSV ��
		Mat dumi_img_hsv;																		// BGR���� HSV�� ��ȯ�� ���� ���� ����

		Scalar dumi_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

		dumi_rgb_color = Mat(1, 1, CV_8UC3, dumi_green);							   //1,1¥�� ��ķ� ����(dot)

		cvtColor(dumi_rgb_color, dumi_hsv_color, COLOR_BGR2HSV);                       //BRG->HSV�� ��ȯ

		int dumi_hue = (int)dumi_hsv_color.at<Vec3b>(0, 0)[0];                         //����

		int dumi_low_hue = dumi_hue - 10;
		int dumi_high_hue = dumi_hue + 6;

		int dumi_low_hue1 = 0, dumi_low_hue2 = 0;
		int dumi_high_hue1 = 0, dumi_high_hue2 = 0;

		if (dumi_low_hue < 10) {
			dumi_range_count = 2;

			dumi_high_hue1 = 180;
			dumi_low_hue1 = dumi_low_hue + 180;
			dumi_high_hue2 = dumi_high_hue;
			dumi_low_hue2 = 0;
		}
		else if (dumi_high_hue > 170) {
			dumi_range_count = 2;

			dumi_high_hue1 = dumi_low_hue;
			dumi_low_hue1 = 180;
			dumi_high_hue2 = dumi_high_hue - 180;
			dumi_low_hue2 = 0;
		}
		else {
			dumi_range_count = 1;
			dumi_low_hue1 = dumi_low_hue;
			dumi_high_hue1 = dumi_high_hue;
		}
		//HSV�� ��ȯ
		cvtColor(dumi_dis_img, dumi_img_hsv, COLOR_BGR2HSV);               //BRG->HSV�� ��ȯ

		cout <<2 << endl;
			
												// warp_j HSV ������ �̿��Ͽ� ������ ����ȭ
		inRange(dumi_img_hsv, Scalar(dumi_low_hue1, 20, 50), Scalar(dumi_high_hue1, 255, 150), dumi_img_mask1);
		if (dumi_range_count == 2) {
			inRange(dumi_img_hsv, Scalar(dumi_low_hue2, 20, 50), Scalar(dumi_high_hue2, 255, 255), dumi_img_mask2);
			dumi_img_mask1 |= dumi_img_mask2;
		}

		// morphological opening ���� ������ ���� (������ ����)
		erode(dumi_img_mask1, dumi_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));          //��������ħ��
		dilate(dumi_img_mask1, dumi_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(9, 9)));         //����������â
																										  //imshow("��������", warp_img_mask1);


																										  // �󺧸� 
		int dumi_numOfLables = connectedComponentsWithStats(dumi_img_mask1, dumi_img_labels,
			dumi_stats, dumi_centroids, 8, CV_32S);

		// �����ڽ� �׸���
		int dumi_max = -1, dumi_idx = 0;
		pdlist.SetIndexZero();
		for (dumi_j = 1; dumi_j < dumi_numOfLables; dumi_j++) {
			int dumi_area = dumi_stats.at<int>(dumi_j, CC_STAT_AREA);
			int dumi_left = dumi_stats.at<int>(dumi_j, CC_STAT_LEFT);
			int dumi_top = dumi_stats.at<int>(dumi_j, CC_STAT_TOP);
			int dumi_width = dumi_stats.at<int>(dumi_j, CC_STAT_WIDTH);
			int dumi_height = dumi_stats.at<int>(dumi_j, CC_STAT_HEIGHT);

			pdlist.AddParkingPoint(ParkingPoint(dumi_j - 1, 0, dumi_left, dumi_top, dumi_width, dumi_height));
			rectangle(dumi_dis_img, Point(dumi_left, dumi_top), Point(dumi_left + dumi_width, dumi_top + dumi_height),
				Scalar(0, 0, 255), 2);

			putText(dumi_dis_img, to_string(dumi_j), Point(dumi_left + 20, dumi_top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

		}
		pdlist.Sort();

		imshow("�������� �󺧸�", dumi_dis_img);
		
		cout << "����üũ1" << endl;
	CheckDummy();
	cout << "����üũ2" << endl;
}


void CheckDummy() 
{
	cout << "����üũ" << endl;
	int *a = new int(plist.index);
	for (int i = 0; i < plist.index; i++)
		a[i] = 0;

	for (int i = 0; i < plist.index; i++) {
		int sX1 = plist.pList[i]->startX;
		int sY1 = plist.pList[i]->startY;
		for (int j = 0; j < pdlist.index; j++) {
			int sX2 = pdlist.pList[j]->startX;
			int sY2 = pdlist.pList[j]->startY;
			if (sX1 - sX2 < 20 && sX1 - sX2 > -20 && sY1 - sY2 < 20 && sY1 - sY2 > -20) {
				a[i] = 1;
				break;
			}
		}
	}

	for (int i = 0; i < plist.index; i++) {
		if (a[i] == 0) {
			cout << i <<"=" << plist.pList[i]->point<<endl;
			DB_Update_ParkingPoint(plist.pList[i]->point, 2);
		}
	}
}

// Tracking
string intToString(int number) { //����

	stringstream ss;
	ss << number;
	return ss.str();
}

// Tracking
void searchForMovement(Mat thresholdImage, Mat &cameraFeed) { //������ ����
	bool objectDetected = false;
	Mat temp;
	thresholdImage.copyTo(temp);
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	if (contours.size()>0)objectDetected = true;
	else objectDetected = false;

	if (objectDetected) {
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(contours.size() - 1));
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
		int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

		theObject[0] = xpos, theObject[1] = ypos;
	}
	int x = theObject[0];
	int y = theObject[1];

	circle(cameraFeed, Point(x, y), 30, Scalar(0, 0, 255), 2);
	line(cameraFeed, Point(x, y), Point(x, y - 25), Scalar(0, 0, 255), 2);
	line(cameraFeed, Point(x, y), Point(x, y + 25), Scalar(0, 0, 255), 2);
	line(cameraFeed, Point(x, y), Point(x - 25, y), Scalar(0, 0, 255), 2);
	line(cameraFeed, Point(x, y), Point(x + 25, y), Scalar(0, 0, 255), 2);

	putText(cameraFeed, "Tracking object at (" + intToString(x) + "," + intToString(y) + ")", Point(x, y), 1, 1, Scalar(255, 0, 0), 2);
	cout << "���� ��ǥ : " << "(" << intToString(x) << "," << intToString(y) << ")" << endl;
}

int Tracking() {
	namedWindow("���� ����");
	bool objectDetected = false;
	// ����� ��� or tracking
	bool debugMode = false;
	bool trackingEnabled = true;
	// �Ͻ�����
	bool pause = false;

	Mat frame1, frame2;
	Mat grayImage1, grayImage2;
	Mat differenceImage;
	Mat thresholdImage;
	VideoCapture capture;

	cout << "Tracking enabled." << endl;
	
	

	//while (1) {

		//we can loop the video by re-opening the capture every time the video reaches its last frame

		
		capture.open("RealTime.avi");

		if (!capture.isOpened()) {
			cout << "ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}

		//while (capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT) - 1) {
		while(1){



			capture.read(frame1);
			cvtColor(frame1, grayImage1, COLOR_BGR2GRAY);
			capture.read(frame2);
			cvtColor(frame2, grayImage2, COLOR_BGR2GRAY);
			absdiff(grayImage1, grayImage2, differenceImage);
			threshold(differenceImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			if (debugMode == true) {
				imshow("Difference Image", differenceImage);
				imshow("Threshold Image", thresholdImage);
			}
			else {
				// ����� ��尡 �ƴϸ� destroyWindow
				destroyWindow("Difference Image");
				destroyWindow("Threshold Image");
			}
			blur(thresholdImage, thresholdImage, Size(BLUR_SIZE, BLUR_SIZE));
			threshold(thresholdImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			if (debugMode == true) {

				imshow("Final Threshold Image", thresholdImage);

			}
			else {
				// ����� ��尡 �ƴϸ� destroyWindow
				destroyWindow("Final Threshold Image");
			}
			if (trackingEnabled) {
				
				searchForMovement(thresholdImage, frame1);
			}

			imshow("���� ����", frame1);

			switch (waitKey(100)) {

			case 27: // 'esc' ������ ���α׷� ����
				return 0;
			case 116: // 't' ������ tracking ����
				trackingEnabled = !trackingEnabled;
				if (trackingEnabled == false) cout << "Tracking disabled." << endl;
				else cout << "Tracking enabled." << endl;
				break;
			case 100: // 'd' ������ ����� ��� ����
				debugMode = !debugMode;
				if (debugMode == false) cout << "Debug mode disabled." << endl;
				else cout << "Debug mode enabled." << endl;
				break;
			case 112: //'p' ������ ���α׷� �Ͻ�����
				pause = !pause;
				if (pause == true) {
					cout << "Code paused, press 'p' again to resume" << endl;
					while (pause == true) {
						switch (waitKey()) {
						case 112:
							// �Ͻ����� ����
							pause = false;
							cout << "Code Resumed" << endl;
							break;
						}
					}
				}
			}
		}
		capture.release();
	//}
	return 0;
}


int RealTimePicture()
{
	int index = 1;
	char buf[2048];
	Mat frameMat;
	VideoCapture videoCapture(0);
	if (!videoCapture.isOpened())
	{
		printf("ù��° ī�޶� ���� �����ϴ�. \n");
	}


	namedWindow("��ķ ����", WINDOW_AUTOSIZE);


	while (1)
	{

		//��ĸ���κ��� �� �������� �о��  
		videoCapture.read(frameMat);

		//ȭ�鿡 ������
		imshow("��ķ ����", frameMat);

		//�̹����� ����  
		sprintf(buf, "c:/ejpicture/img_%03d.jpg", index);
		cout << buf << endl;
		imwrite(buf, frameMat);

		index++;
		if (index == 6)
			return 1;

		if (waitKey(10) == 27) break; //ESCŰ ������ ����  
	}
	return 0;
}


int AddPicture()
{
	int index = 1;
	char buf[2048];
	Mat frameMat;
	VideoWriter writer;
	VideoCapture cap(0);
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));



	namedWindow("�̹��� ����", WINDOW_AUTOSIZE);

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

				//imshow("�̹��� ����", frameMat);
				index++;
			}
			if (index == 6)
				return 1;
			writer.write(frameMat);
		}
		//�̹����� ����  






		/*if (!writer.isOpened())
		{
		cout << "�������� �����ϱ� ���� �ʱ�ȭ �۾� �� ���� �߻�" << endl;
		return 1;
		}
		for (;;) {
		Mat fr;


		}*/

		if (waitKey(250) == 27) break; //ESCŰ ������ ����  
	}


	return 0;
}


// php
int DB_Insert_ParkingPoint(int index, int occupy, int startX, int startY, int lenX, int lenY) {
	const int bufLen = 1024;
	char buf[100];
	char url[200];
	char *link = "http://nejoo97.cafe24.com/ParkingPointAdd.php?";
	char *argcat[6] = { "POINT=", "&OCCUPY=", "&STARTX=", "&STARTY=", "&LENX=", "&LENY=" };
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

int DB_Update_FlagMember(char *id, int flag) {
	const int bufLen = 1024;
	char buf[100];
	char url[200];
	char *link = "http://nejoo97.cafe24.com/FlagMember.php?";
	char *argcat[2] = { "ID=", "&FLAG=" };
	sprintf(buf, "%s%s", argcat[0], id);
	sprintf(url, "%s%s", link, buf);
	link = url;


	char tmp[10];
	itoa(flag, tmp, 10);
	sprintf(buf, "%s%s", argcat[1], tmp);
	sprintf(url, "%s%s", link, buf);
	link = url;
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

int DB_Check_FlagMember() {
	const int bufLen = 1024;
	char buf[100];

	char *szUrl = "http://nejoo97.cafe24.com/CheckMember.php";
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
int DB_Update_ParkingPoint(int point, int occupy) {
	const int bufLen = 1024;
	char buf[100];
	char url[200];
	char *link = "http://nejoo97.cafe24.com/ParkingPointOccupy.php?";

	char *argcat[2] = { "POINT=", "&OCCUPY="};
	int arg[2];
	arg[0] = point;
	arg[1] = occupy;
	char tmp[10];
	for (int i = 0; i < 2; i++) {
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