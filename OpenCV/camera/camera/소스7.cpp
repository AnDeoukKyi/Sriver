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


Mat img_frame;																			// 원본 img
Mat *rgb;																				// 마우스 이벤트 주차장 색깔
Mat dis_img_result;																		// 왜곡보정 된 img
vector<Point2f> corners(4);																// Warping 전의 이미지 상의 좌표
Mat warpImg;																			// 주차장 라벨링
Mat small_warpImg;																		// 주차칸별 라벨링
Mat dumi_img;																			// 더미차량 있을 때 주차장 img
Mat dumi_dis_img;																		// 더미차량 있을 때 왜곡보정 된 img																																							// 더미차량 주차칸별 라벨링
Mat warp_rgb_color;																		// 주차칸 색상
Mat dumi_rgb_color;																		// 더미차량 있을 때 주차칸 색상
Scalar warp_green;																		// 주차칸 색상(좌표값에 대한 RGB)
Mat video_dis_frame;																	// 영상촬영 시 왜곡보정 된 img
Vec3b lower_blue1, upper_blue1, lower_blue2, upper_blue2, lower_blue3, upper_blue3;     // trackingHSV
Mat img_track;																			// track img


int threshold1 = 100;                   //fortracking
int px, py;                             //마우스 클릭 좌표
int step = 0;                           //0시작, 1마우스클릭(주차장색깔)

int p_left, p_top, p_width, p_height;   // 왜곡보정
int minLeft[2];//y, x					// 왜곡보정 전 좌표
int minRight[2];//y, x					// 왜곡보정 전 좌표
int minBLeft[2];//y, x					// 왜곡보정 전 좌표
int minBRight[2];//y, x					// 왜곡보정 전 좌표
Mat perspect_map, perspect_map1, perspect_map2;						// 왜곡보정 정보

int warp_j, warp_left, warp_top, warp_height, warp_width; // 라벨링 좌표
int dumi_j, dumi_left, dumi_top, dumi_height, dumi_width; // 더미차량 라벨링 좌표


const static int SENSITIVITY_VALUE = 20;   // Tracking
const static int BLUR_SIZE = 10;
int theObject[2] = { 0,0 };
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);


int pos[10][4];
int cntLables = 0;

//php
HINSTANCE hInst;
WSADATA wsaData;


//함수
int captureVideo();
int captureVideo2();
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
int RealTimePicture();
int AddPicture();
Mat VideoDistortion(Mat frame); // 영상왜곡보정
void trac();
void pand(Mat mat1, Mat mat2);
double distance(int x1, int y1, int x2, int y2);
int checkInner(int x, int y);

void VideoCap();

//php함수
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
SOCKET connectToServer(char *szServerName, WORD portNum);
int getHeaderLength(char *content);
char *readUrl2(char *szUrl, long &bytesReturnedOut, char **headerOut);
int DB_Insert_ParkingPoint(int index, int occupy, int startX, int startY, int lenX, int lenY);

int DB_Update_ParkingPoint(int point, int occupy);
int DB_Check_Flag1Member();
int DB_Update_UserPos(char* id, int x, int y);
int DB_Update_Flag(char* id, int flag);
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
				DB_Insert_ParkingPoint(i, 0, pList[i]->startX, pList[i]->startY, pList[i]->lenX, pList[i]->lenY);
				sX = pList[i]->startX;
				sY = pList[i]->startY;
			}
			else//1~24
				DB_Insert_ParkingPoint(i, 0, pList[i]->startX - sX, pList[i]->startY - sY, pList[i]->lenX, pList[i]->lenY);
		}
	}
};

class User {
public:
	char *id;
	int posX = -100;
	int posY = -100;
	int check = 0;
	User(char *_id) {
		id = new char[strlen(_id) + 1];
		strcpy(id, _id);
	}
	User() {
		id = "NULL";
	}
	void Show() {
		cout << id << endl;
		cout << posX << endl;
		cout << posY << endl;
		cout << check << endl;
		cout << "=======" << endl;
	}
	void Send() {

	}
	void setX(int x) {
		posX = x;
	}
	void setY(int y) {
		posY = y;
	}
	void setCheck() {
		check = 0;
	}
	~User() {
		delete[]id;
	}
};

class UList {
public:
	int index = 10;
	User *uList[10];

	UList() {//디폴트값으로 NULL인 유저가 10명 들어감
		for (int i = 0; i < 10; i++) {
			AddUser(new User(), i);
		}
	};

	void AddUser(User *u, int i) {
		uList[i] = u;
	}

	void SendPos() {
		for (int i = 0; i < 10; i++)
			uList[i]->Send();
	}

	void ShowAll() {
		for (int i = 0; i < 10; i++) {
			cout << "asdfasdf" << endl;
			uList[i]->Show();
		}
	}

	void DeleteUser(int i) {
		cout << "삭제 아이디 "<<i<<"번째"<<": ============================================" << uList[i]->id << endl;
		DB_Update_Flag(uList[i]->id, 0);//flag0으로 변경
		uList[i]->setX(-200);
		uList[i]->setY(-200);
		DB_Update_UserPos(uList[i]->id, uList[i]->posX, uList[i]->posY);//DB업데이트
		uList[i]->id = "NULL";//다음사람 사용가능한 칸으로 변경
		uList[i]->check = 0;
	}

	int FindNull() {
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") == 0)
				return i;//NULL이면
		}
		return -1;
	}

	int FindNull(int n) {
		if (strcmp(uList[n]->id, "NULL") == 0)
			return 1;//NULL이면
		else return -1;//User있으면
	}

	int CountNull() {
		int cnt = 0;
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") == 0) {
				cnt++;
			}
		}
		return cnt;
	}

	User *First(int n) {
		int cnt = 0;
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") != 0) {
				cnt++;
				if (cnt == n) return uList[i];
			}
		}
		return NULL;
	}

	int First() {
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") != 0 && uList[i]->posX == -200) {
				return i;
			}
		}
		return -1;
	}

	int CountUser() {
		int cnt = 0;
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") != 0) {
				cnt++;
			}
		}
		return cnt;
	}
};

UList uList;
PList plist;
PList pdlist;




int dLeftX = 100, dLeftY = 100;
int dRightX = 400, dRightY = 400;

int play = 0;

int main()
{
	
	VideoCap();
	img_frame = imread("init.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);
	rgb = &img_frame;
	for (;;) {
		switch (step) {
		case 0:


			
			
			px = 363;
			py = 129;

			/*Distortion();
			Labeling();
			plist.UploadDB();
			step++;*/


			plist.AddParkingPoint(ParkingPoint(0, 0, 103, 99, 0, 0));
			Distortion();
			trac();
			step++;



			//imshow("init", img_frame);
			//setMouseCallback("init", MouseEventRGB, (&img_frame));          // BGR값 출력용 마우스 이벤트
			break;
		//case 1://모폴로지 + 원근 왜곡 보정
		//	while (1) {
		//		Distortion();
		//		if (waitKey(100) == 27) {
		//			break;
		//		}
		//	}
		//	step++;
		//	break;
		//case 2://라벨링
		//	Labeling();
		//	break;
		//case 3://더미차량 배치 후 영상 촬영

		//	plist.UploadDB();
		//	step++;
		//	break;



		/*case 0:
			trac();
			break;*/



		//case 2://라벨링

		//	   Labeling();
		//	break;
		//case 3://더미차량 배치 후 영상 촬영

		//	   plist.UploadDB();
		//	   //DumiCapture();
		//	step++;
		//	break;
		//case 4://더미차량 모폴로지 + 원근 왜곡 보정

		//	   //DumiDistortion();
		//	step++;
		//	break;
		//case 5://더미차량 라벨링

		//	   //cout << "더미 라벨링" << endl;
		//	   //Labeling();
		//	step++;
		//case 6://차량영상 프레임저장
		//	//
		//	  /* RealTimePicture();
		//	   AddPicture();
		//	   Tracking();
		//	DB_Check_Flag1Member();
		//	uList.ShowAll();
		//	if (uList.index != 0) {
		//		DB_Update_Flag(uList.uList[0]->id, 2);
		//		DB_Update_UserPos(uList.uList[0]->id, 100, 200);
		//	}*/
		//	break;
		//default:

		//	break;
		}

		if (waitKey(30) >= 0) continue;
	}
	waitKey(0);
	return 0;
}


void VideoCap() {

	VideoCapture capture;
	Mat frame;
	capture.open("car.mp4");
	capture.read(frame);
	imwrite("init.jpg", frame, vector<int>());
}


//함수

int captureVideo()      // 동영상 촬영하기
{
	Mat img_capture;

	//비디오 캡쳐 초기화
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "에러 - 카메라를 열 수 없습니다.\n";
		return -1;
	}


	// 동영상 파일을 저장하기 위한 준비  
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 30.0;
	writer.open("base.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
	if (!writer.isOpened())
	{
		cout << "동영상을 저장하기 위한 초기화 작업 중 에러 발생" << endl;
		return 1;
	}


	while (1)
	{
		cap.read(img_capture);
		if (img_capture.empty()) {
			cerr << "빈 영상이 캡쳐되었습니다.\n";		
			break;
		}

		//동영상 파일에 한 프레임을 저장함.  
		writer.write(img_capture);

		imshow("캡쳐", img_capture);
		img_frame = img_capture;
		imwrite("base.jpg", img_capture, vector<int>());

		if (waitKey(25) >= 0)
			break;

	}

	writer.release();

	return 0;
}


//int captureVideo2()      // 동영상 촬영하기
//{
//	Mat img_capture2;
//
//	//비디오 캡쳐 초기화
//	VideoCapture cap(0);
//	if (!cap.isOpened()) {
//		cerr << "에러 - 카메라를 열 수 없습니다.\n";
//		return -1;
//	}
//
//
//	// 동영상 파일을 저장하기 위한 준비  
//	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
//		(int)cap.get(CAP_PROP_FRAME_HEIGHT));
//
//	VideoWriter writer;
//	double fps = 30.0;
//	writer.open("carmove.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
//	if (!writer.isOpened())
//	{
//		cout << "동영상을 저장하기 위한 초기화 작업 중 에러 발생" << endl;
//		return 1;
//	}
//
//
//	while (1)
//	{
//		cap.read(img_capture2);
//		if (img_capture2.empty()) {
//			cerr << "빈 영상이 캡쳐되었습니다.\n";
//			break;
//		}
//
//		//동영상 파일에 한 프레임을 저장함.  
//		writer.write(img_capture2);
//
//		imshow("캡쳐", img_capture2);
//		imwrite("carmove.jpg", img_capture2, vector<int>());
//
//		if (waitKey() >= 0)
//			break;
//
//	}
//
//	writer.release();
//
//	return 0;
//}

void RecordVideo() { // 사진 불러오기
	img_frame = imread("base.jpg", IMREAD_COLOR);
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
		cout << "(" << px << ", " << py << ")" << endl;
	}
	//cout << "(" << px << ", " << py << ")" << endl;
	//365, 124
	
}


void Distortion()
{

	Mat img_labels, stats, centroids;                        // 원본 img 큰 사각형 좌표 관련 변수 목록
	Mat img_mask1, img_mask2;
	int range_count = 0;

	// 내가 원하는 색깔 선택
	Scalar color((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]); // Blue Green Red 순서

	

	//cout << "주차장 색깔 (Blue, Green, Red 순)" << color << endl;

	Mat rgb_color = Mat(1, 1, CV_8UC3, color);             //1,1짜리 행렬로 생성(dot)
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);         //BGR->HSV로 변환


	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];           //색조

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

	inRange(img_hsv, Scalar(low_hue1, 20, 55), Scalar(high_hue1, 255, 150), img_mask1);
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 20, 50), Scalar(high_hue2, 255, 255), img_mask2);
		img_mask1 |= img_mask2;
	}

	//imshow("asdff전", img_mask1);
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(13, 13)));       //모폴로지팽창
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(11, 11)));        //모폴로지침식
	//dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(13, 13)));       //모폴로지팽창
	//imshow("asdff", img_mask1);
	Mat im = img_mask1;
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
		//rectangle(img_frame, Point(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP)), Point(stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT)), Scalar(0, 0, 255), 2);
	}
	int top = stats.at<int>(ind, CC_STAT_TOP);
	int left = stats.at<int>(ind, CC_STAT_LEFT);
	int width = stats.at<int>(ind, CC_STAT_WIDTH);
	int height = stats.at<int>(ind, CC_STAT_HEIGHT);
	//rectangle(img_frame, Point(left, top), Point(left + width, top + height),Scalar(0, 0, 255), 2);
	int pix = 0;
	int plus = 10;
	int Larr[9][2];
	int Rarr[9][2];
	int BLarr[9][2];
	int BRarr[9][2];
	int index = 0;
	int minus;

	//왼쪽 위
	while (1) {
		Larr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + plus, left + pix) > 0) break;
			pix++;
			if (pix > 200) break;
		}
		Larr[index++][1] = pix;
		pix = 0;
		plus -= 1;
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
	plus = 10;
	index = 0;

	//오른쪽 위
	while (1) {
		Rarr[index][0] = plus;
		while (1) {
			if (img_mask1.at<uchar>(top + plus, left + width - pix) > 0) break;
			pix++;
			if (pix > 200) break;
		}
		Rarr[index++][1] = pix;
		pix = 0;
		plus -= 1;
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


	//왼쪽 아래
	pix = 0;
	plus = -10;
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
		plus += 1;
		if (index >= 9) break;
	}
	minBLeft[0] = 0;
	minus = BLarr[1][1] - BLarr[0][1];
	for (int i = 1; i < 8; i++) {
		if (BLarr[i][1] >= 400) break;
		if (BLarr[i + 1][1] - BLarr[i][1] < minus + 10 && BLarr[i + 1][1] - BLarr[i][1] > minus - 10) {
			minBLeft[0] = i;
			minus = BLarr[i + 1][1] - BLarr[i][1];
		}
		else break;
	}
	minBLeft[1] = BLarr[minBLeft[0]][1];
	minBLeft[0] = BLarr[minBLeft[0] + 1][0] + top + height;

	//오른쪽 아래
	pix = 0;
	plus = -10;
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
		plus += 1;
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


	//cout << "======================원본==============================" << endl;
	//cout << "왼쪽 위 좌표 : " << Point2f(minLeft[1], minLeft[0]) << endl;
	//cout << "오른쪽 위 좌표 : " << Point2f(minRight[1], minRight[0]) << endl;
	//cout << "오른쪽 아래 좌표 : " << Point2f(minBLeft[1], minBLeft[0]) << endl;
	//cout << "왼쪽 아래 좌표 : " << Point2f(minBRight[1], minBRight[0]) << endl;
	//cout << "========================================================" << endl;
	/*149, 102]
	오른쪽 위 좌표 : [507, 107]
		오른쪽 아래 좌표 : [30, 350]
		왼쪽 아래 좌표 : [634, 345*/
	
	Point2f pts1[4] = {      // 원본 영상 좌표 4개
		Point2f(minLeft[1], minLeft[0]),                             // 왼쪽 위
		Point2f(minRight[1], minRight[0]),                           // 오른쪽 위
		Point2f(minBLeft[1], minBLeft[0]),                           // 왼쪽 아래
		Point2f(minBRight[1], minBRight[0])                          // 오른쪽 아래
	};

	Point2f pts2[4] = {      // 목적 영상 좌표 4개

		Point2f(dLeftX, dLeftY),	// 왼쪽 위 (왼쪽 옆, 위)
		Point2f(dRightX, dLeftY),	// 오른쪽 위 (오른쪽 옆, 위)
		Point2f(dLeftX, dRightY),	// 왼쪽 아래 (전체 높이, 사각형 높이 )
		Point2f(dRightX, dRightY)	//오른쪽 아래

	};
	Mat dis_img_result(img_frame.size(), CV_8UC1);
	perspect_map = getPerspectiveTransform(pts1, pts2);      // 원근 변환 행렬 계산
	warpPerspective(img_frame, dis_img_result, perspect_map, img_frame.size(), INTER_CUBIC);
	//cout << "[perspect_map] = " << endl << perspect_map << endl << endl;

	vector<Point3f> pts3, pts4;      // 3차원 좌표로 동차좌표 표현
	for (int i = 0; i < 4; i++)
	{
		pts3.push_back(Point3f(pts1[i].x, pts1[i].y, 1));      // 원본좌표 -> 동차좌표 저장
	}

	transform(pts3, pts4, perspect_map);      // 변환좌표(pts4) 계산

	for (int i = 0; i < 4; i++)
	{
		pts4[i] /= pts4[i].z;      // 동차좌표에서 w 나누기
		//cout << "pts2[" << i << "]" << pts2[i] << "\t";
		//cout << "pts4[" << i << "]" << pts4[i] << endl;

		//circle(img_frame, pts1[i], 2, Scalar(0, 0, 255), 2);
		//circle(dis_img_result, pts2[i], 2, Scalar(0, 0, 255), 2);
	}
	////imshow("init", img_frame);

	//imshow("왜곡 보정 후", dis_img_result);
	//imshow("왜곡 보정 후", dis_img_result);
	imwrite("dis_img_result.jpg", dis_img_result);
}


// 작은 사각형 라벨링 + 좌표
void Labeling()
{
	Mat warp_img_labels, warp_stats, warp_centroids;       // warp img 작은 사각형 좌표 관련 변수 목록
	Mat warp_img_mask1;                                    // warp img 모폴로지 전
	Mat warp_img_mask2;                                    // warp img 모폴로지 후
	small_warpImg = imread("dis_img_result.jpg", IMREAD_COLOR);
	CV_Assert(small_warpImg.data);
	int warp_range_count = 0;

	Mat warp_hsv_color;																		// 주차칸 색상 HSV 값
	Mat warp_img_hsv;																		// BGR에서 HSV로 변환된 색상 저장 변수

	Scalar warp_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	warp_rgb_color = Mat(1, 1, CV_8UC3, warp_green);							   //1,1짜리 행렬로 생성(dot)

	cvtColor(warp_rgb_color, warp_hsv_color, COLOR_BGR2HSV);                       //BRG->HSV로 변환

	int warp_hue = (int)warp_hsv_color.at<Vec3b>(0, 0)[0];                         //색조

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
	//HSV로 변환
	cvtColor(small_warpImg, warp_img_hsv, COLOR_BGR2HSV);               //BRG->HSV로 변환

																		// warp_j HSV 범위를 이용하여 영상을 이진화
	inRange(warp_img_hsv, Scalar(warp_low_hue1, 20, 50), Scalar(warp_high_hue1, 255, 200), warp_img_mask1);
	if (warp_range_count == 2) {
		inRange(warp_img_hsv, Scalar(warp_low_hue2, 20, 50), Scalar(warp_high_hue2, 255, 255), warp_img_mask2);
		warp_img_mask1 |= warp_img_mask2;
	}

	//imshow("전ggg", warp_img_mask1);
	// morphological opening 작은 점들을 제거 (노이즈 제거)
	erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(11, 5)));          //모폴로지침식
	dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(1, 5)));         //모폴로지팽창
	erode(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(11, 11)));          //모폴로지침식
	dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(11, 11)));         //모폴로지팽창
	dilate(warp_img_mask1, warp_img_mask1, getStructuringElement(MORPH_RECT, Size(5, 1)));         //모폴로지팽창

																								   //imshow("ggg", warp_img_mask1);
																								   // 라벨링 
	int warp_numOfLables = connectedComponentsWithStats(warp_img_mask1, warp_img_labels,
		warp_stats, warp_centroids, 8, CV_32S);

	// 영역박스 그리기
	int warp_max = -1, warp_idx = 0;
	plist.SetIndexZero();
	int ind = 1;
	for (warp_j = 1; warp_j < warp_numOfLables; warp_j++) {
		int warp_area = warp_stats.at<int>(warp_j, CC_STAT_AREA);
		int warp_left = warp_stats.at<int>(warp_j, CC_STAT_LEFT);
		int warp_top = warp_stats.at<int>(warp_j, CC_STAT_TOP);
		int warp_width = warp_stats.at<int>(warp_j, CC_STAT_WIDTH);
		int warp_height = warp_stats.at<int>(warp_j, CC_STAT_HEIGHT);
		if (warp_top < 30) continue;
		plist.AddParkingPoint(ParkingPoint(warp_j - 1, 0, warp_left, warp_top, warp_width, warp_height));
		rectangle(small_warpImg, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
			Scalar(0, 0, 255), 2);

		putText(small_warpImg, to_string(warp_j), Point(warp_left + 20, warp_top + 20),
			FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

	}
	plist.Sort();
}

int DumiCaptureFunction()
{
	Mat img_capture;

	cout << "더미 캡쳐 호출" << endl;
	//비디오 캡쳐 초기화
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "에러 - 카메라를 열 수 없습니다.\n";
		return -1;
	}


	// 동영상 파일을 저장하기 위한 준비  
	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 30.0;
	writer.open("dumicar.avi", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);
	if (!writer.isOpened())
	{
		cout << "동영상을 저장하기 위한 초기화 작업 중 에러 발생" << endl;
		return 1;
	}


	while (1)
	{
		cap.read(img_capture);
		if (img_capture.empty()) {
			cerr << "빈 영상이 캡쳐되었습니다.\n";
			break;
		}

		//동영상 파일에 한 프레임을 저장함.  
		writer.write(img_capture);

		imshow("더미차량 캡쳐", img_capture);
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

	Point2f pts1[4] = {      // 원본 영상 좌표 4개
		Point2f(minLeft[1], minLeft[0]),                             // 왼쪽 위
		Point2f(minRight[1], minRight[0]),                           // 오른쪽 위
		Point2f(minBLeft[1], minBLeft[0]),                           // 왼쪽 아래
		Point2f(minBRight[1], minBRight[0])                          // 오른쪽 아래
	};
	cout << Point2f(minLeft[1], minLeft[0]) << endl;
	cout << Point2f(minRight[1], minRight[0]) << endl;
	cout << Point2f(minBLeft[1], minBLeft[0]) << endl;
	cout << Point2f(minBRight[1], minBRight[0]) << endl;
	cout << "---------------------------" << endl;
	Point2f pts2[4] = {      // 목적 영상 좌표 4개

		Point2f(50, 50),	// 왼쪽 위 (왼쪽 옆, 위)
		Point2f(350, 50),	// 오른쪽 위 (오른쪽 옆, 위)
		Point2f(50, 350),	// 왼쪽 아래 (전체 높이, 사각형 높이 )
		Point2f(350, 350)	//오른쪽 아래
	};

	Mat dumi_dis_img(dumi_img.size(), CV_8UC1);
	perspect_map1 = getPerspectiveTransform(pts1, pts2);      // 원근 변환 행렬 계산
	warpPerspective(dumi_img, dumi_dis_img, perspect_map1, dumi_img.size(), INTER_CUBIC);
	//cout << "[perspect_map] = " << endl << perspect_map1 << endl << endl;

	vector<Point3f> pts3, pts4;      // 3차원 좌표로 동차좌표 표현
	for (int i = 0; i < 4; i++)
	{
		pts3.push_back(Point3f(pts1[i].x, pts1[i].y, 1));      // 원본좌표 -> 동차좌표 저장
	}

	transform(pts3, pts4, perspect_map1);      // 변환좌표(pts4) 계산

	for (int i = 0; i < 4; i++)
	{
		pts4[i] /= pts4[i].z;      // 동차좌표에서 w 나누기
		cout << "pts2[" << i << "]" << pts2[i] << "\t";
		cout << "pts4[" << i << "]" << pts4[i] << endl;

		circle(dumi_img, pts1[i], 2, Scalar(0, 0, 255), 2);
		circle(dumi_dis_img, pts2[i], 2, Scalar(0, 0, 255), 2);
	}

	//imshow("더미 왜곡 보정 전", dumi_img);
	imshow("더미 왜곡 보정 후", dumi_dis_img);
	imwrite("dumi_distortion.jpg", dumi_dis_img);

	step++;
}

void DumiLabeling()
{
	Mat dumi_img_labels, dumi_stats, dumi_centroids;       // warp img 작은 사각형 좌표 관련 변수 목록
	Mat dumi_img_mask1;                                    // warp img 모폴로지 전
	Mat dumi_img_mask2;                                    // warp img 모폴로지 후
	dumi_dis_img = imread("dumi_distortion.jpg", IMREAD_COLOR);

	cout << 0 << endl;
	CV_Assert(dumi_dis_img.data);
	int dumi_range_count = 0;
	cout << 1 << endl;
	Mat dumi_hsv_color;																		// 주차칸 색상 HSV 값
	Mat dumi_img_hsv;																		// BGR에서 HSV로 변환된 색상 저장 변수

	Scalar dumi_green((int)(*rgb).at<Vec3b>(py, px)[0], (int)(*rgb).at<Vec3b>(py, px)[1], (int)(*rgb).at<Vec3b>(py, px)[2]);

	dumi_rgb_color = Mat(1, 1, CV_8UC3, dumi_green);							   //1,1짜리 행렬로 생성(dot)

	cvtColor(dumi_rgb_color, dumi_hsv_color, COLOR_BGR2HSV);                       //BRG->HSV로 변환

	int dumi_hue = (int)dumi_hsv_color.at<Vec3b>(0, 0)[0];                         //색조

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
	//HSV로 변환
	cvtColor(dumi_dis_img, dumi_img_hsv, COLOR_BGR2HSV);               //BRG->HSV로 변환

	cout << 2 << endl;

	// warp_j HSV 범위를 이용하여 영상을 이진화
	inRange(dumi_img_hsv, Scalar(dumi_low_hue1, 20, 50), Scalar(dumi_high_hue1, 255, 150), dumi_img_mask1);
	if (dumi_range_count == 2) {
		inRange(dumi_img_hsv, Scalar(dumi_low_hue2, 20, 50), Scalar(dumi_high_hue2, 255, 255), dumi_img_mask2);
		dumi_img_mask1 |= dumi_img_mask2;
	}

	// morphological opening 작은 점들을 제거 (노이즈 제거)
	erode(dumi_img_mask1, dumi_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));          //모폴로지침식
	dilate(dumi_img_mask1, dumi_img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(9, 9)));         //모폴로지팽창
																									  //imshow("모폴로지", warp_img_mask1);


																									  // 라벨링 
	int dumi_numOfLables = connectedComponentsWithStats(dumi_img_mask1, dumi_img_labels,
		dumi_stats, dumi_centroids, 8, CV_32S);

	// 영역박스 그리기
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

	imshow("더미차량 라벨링", dumi_dis_img);

	cout << "더미체크1" << endl;
	CheckDummy();
	cout << "더미체크2" << endl;
}


void CheckDummy()
{
	cout << "더미체크" << endl;
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
			cout << i << "=" << plist.pList[i]->point << endl;
			DB_Update_ParkingPoint(plist.pList[i]->point, 2);
		}
	}
}

// Tracking
string intToString(int number) { //도움말

	stringstream ss;
	ss << number;
	return ss.str();
}

// Tracking
void searchForMovement(Mat thresholdImage, Mat &cameraFeed) { //움직임 감지
	bool objectDetected = false;
	Mat temp;
	thresholdImage.copyTo(temp);
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	if (contours.size() == 0) return;
	cout << contours.size() << endl;
	for (int i = 0; i < contours.size() - 1; i++) {
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(i));
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
		int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

		theObject[0] = xpos, theObject[1] = ypos;

		int x = theObject[0];
		int y = theObject[1];
		circle(cameraFeed, Point(x, y), 30, Scalar(0, 0, 255), 2);
		line(cameraFeed, Point(x, y), Point(x, y - 25), Scalar(0, 0, 255), 2);
		line(cameraFeed, Point(x, y), Point(x, y + 25), Scalar(0, 0, 255), 2);
		line(cameraFeed, Point(x, y), Point(x - 25, y), Scalar(0, 0, 255), 2);
		line(cameraFeed, Point(x, y), Point(x + 25, y), Scalar(0, 0, 255), 2);

		putText(cameraFeed, "Tracking object at (" + intToString(x) + "," + intToString(y) + ")", Point(x, y), 1, 1, Scalar(255, 0, 0), 2);
		cout << "차량 좌표 : " << "(" << intToString(x) << "," << intToString(y) << ")" << endl;
	}
	
}

int Tracking() {
	namedWindow("차량 추적");
	bool objectDetected = false;
	// 디버깅 모드 or tracking
	bool debugMode = false;
	bool trackingEnabled = true;
	// 일시정지
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
	while (1) {



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
			// 디버깅 모드가 아니면 destroyWindow
			destroyWindow("Difference Image");
			destroyWindow("Threshold Image");
		}
		blur(thresholdImage, thresholdImage, Size(BLUR_SIZE, BLUR_SIZE));
		threshold(thresholdImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		if (debugMode == true) {

			imshow("Final Threshold Image", thresholdImage);

		}
		else {
			// 디버깅 모드가 아니면 destroyWindow
			destroyWindow("Final Threshold Image");
		}
		if (trackingEnabled) {

			searchForMovement(thresholdImage, frame1);
		}

		imshow("차량 추적", frame1);

		switch (waitKey(100)) {

		case 27: // 'esc' 누르면 프로그램 종료
			return 0;
		case 116: // 't' 누르면 tracking 시작
			trackingEnabled = !trackingEnabled;
			if (trackingEnabled == false) cout << "Tracking disabled." << endl;
			else cout << "Tracking enabled." << endl;
			break;
		case 100: // 'd' 누르면 디버깅 모드 시작
			debugMode = !debugMode;
			if (debugMode == false) cout << "Debug mode disabled." << endl;
			else cout << "Debug mode enabled." << endl;
			break;
		case 112: //'p' 누르면 프로그램 일시정지
			pause = !pause;
			if (pause == true) {
				cout << "Code paused, press 'p' again to resume" << endl;
				while (pause == true) {
					switch (waitKey()) {
					case 112:
						// 일시정지 해제
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


void pand(Mat mat1, Mat mat2) {

	Mat grayImage1, grayImage2;
	Mat differenceImage;
	Mat thresholdImage;

	bool objectDetected = false;
	// 디버깅 모드 or tracking
	bool debugMode = false;
	bool trackingEnabled = true;
	// 일시정지
	bool pause = false;
	cvtColor(mat1, grayImage1, COLOR_BGR2GRAY);
	cvtColor(mat2, grayImage2, COLOR_BGR2GRAY);
	absdiff(grayImage1, grayImage2, differenceImage);
	threshold(differenceImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
	
	if (debugMode == true) {
		imshow("Difference Image", differenceImage);
		imshow("Threshold Image", thresholdImage);
	}
	else {
		// 디버깅 모드가 아니면 destroyWindow
		destroyWindow("Difference Image");
		destroyWindow("Threshold Image");
	}
	blur(thresholdImage, thresholdImage, Size(BLUR_SIZE, BLUR_SIZE));
	threshold(thresholdImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
	if (debugMode == true) {

		imshow("Final Threshold Image", thresholdImage);

	}
	else {
		// 디버깅 모드가 아니면 destroyWindow
		destroyWindow("Final Threshold Image");
	}
	if (trackingEnabled) {

		//searchForMovement(thresholdImage, mat1);
		//imshow("thresholdImage", thresholdImage);

		dilate(thresholdImage, thresholdImage, getStructuringElement(MORPH_ELLIPSE, Size(20, 20)));         //모폴로지팽창
																										  //imshow("모폴로지", warp_img_mask1);

		erode(thresholdImage, thresholdImage, getStructuringElement(MORPH_ELLIPSE, Size(25, 25)));          //모폴로지침식
		//imshow("thresholdImage모폴로지", thresholdImage);

		Mat warp_img_labels, warp_stats, warp_centroids;

		// 라벨링 
		int warp_numOfLables = connectedComponentsWithStats(thresholdImage, warp_img_labels,
			warp_stats, warp_centroids, 8, CV_32S);
		
		
		if (warp_numOfLables != 2) return;


		cout << "스레시홀드 개수는="<<warp_numOfLables << endl;


		//rectangle(mat1, Point(45, 45), Point(55, 55),
		//Scalar(0, 0, 255), 2);
		//rectangle(mat1, Point(plist.pList[0]->startX-5, plist.pList[0]->startY-5), Point(plist.pList[0]->startX+5, plist.pList[0]->startY+5),
		//Scalar(0, 0, 255), 2);
		//rectangle(mat1, Point(345, 345), Point(355, 355),
		//	Scalar(0, 0, 255), 2);
		
		// 영역박스 그리기
		
		int index = 0;
		int warp_max = -1, warp_idx = 0;
		for (warp_j = 1; warp_j < warp_numOfLables; warp_j++) {
			int warp_area = warp_stats.at<int>(warp_j, CC_STAT_AREA);
			int warp_left = warp_stats.at<int>(warp_j, CC_STAT_LEFT);
			int warp_top = warp_stats.at<int>(warp_j, CC_STAT_TOP);
			int warp_width = warp_stats.at<int>(warp_j, CC_STAT_WIDTH);
			int warp_height = warp_stats.at<int>(warp_j, CC_STAT_HEIGHT);

			int x = warp_centroids.at<double>(warp_j, 0);	// 중심좌표
			int y = warp_top + (warp_height/5) * 4;
			



			circle(mat1, Point(x, y), 5, Scalar(255, 0, 0), 1);
			if (warp_area < 300) continue;
			pos[index][0] = x;
			pos[index++][1] = y;
			if (index == 10) break;

			putText(mat1, to_string(warp_height), Point(warp_left - 20, warp_top - 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
			rectangle(mat1, Point(warp_left, warp_top), Point(warp_left + warp_width, warp_top + warp_height),
				Scalar(0, 0, 255), 2);


			putText(mat1, to_string(warp_j), Point(warp_left + 20, warp_top + 20),
				FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

			
		}

		//rectangle(mat1, Point(plist.pList[0]->startX, plist.pList[0]->startY), Point(plist.pList[0]->startX + plist.pList[0]->lenX, plist.pList[0]->startY + plist.pList[0]->lenY),
		//	Scalar(0, 0, 255), 2);


		if (pos[0][0] != 999 || pos[0][1] != 999) {
			uList.uList[0]->setX(pos[0][0]);
			uList.uList[0]->setY(pos[0][1]);

		}
		

		//check
		//시작 = 0
		//바깥에서 탐색(입장) 0->1
		//내부에 있으면 2
		//바깥에서 탐색(퇴장) 2->3

		//입장, 삭제시
		//double min = 999999;


		//
		//	rectangle(mat1, Point(plist.pList[0]->startX, plist.pList[0]->startY),Point(plist.pList[0]->startX + plist.pList[0]->lenX, plist.pList[0]->startY + plist.pList[0]->lenY),
		//		Scalar(0, 0, 255), 2);
		//	cout << "시작함----------------------------------------------------" << endl;
		//for (int i = 0; i < 10; i++)
		//	cout << pos[i][0] << "," << pos[i][1] << endl;
		//cout << "시작함----------------------------------------------------" << endl;
		//for (int j = 1; j <= uList.CountUser(); j++) {//외부->내부
		//	int cho = -1;
		//	int px = uList.First(j)->posX;
		//	int py = uList.First(j)->posY;
		//	min = 999999;
		//	for (int i = 0; i < 10; i++) {
		//		if (pos[i][0] == 999) continue;
		//		if (min > distance(pos[i][0], pos[i][1], px, py)) {
		//			min = distance(pos[i][0], pos[i][1], px, py);
		//			cho = i;
		//		}
		//	}
		//	if (cho != -1) {
		//		
		//		cout << "==================" << endl;
		//		cout << px<<","<<py << endl;
		//		cout << pos[cho][0] << "," << pos[cho][1] << endl;
		//		cout << "==================" << endl;
		//		//checkInner(px, py);//이전
		//		//checkInner(pos[cho][0]-50, pos[cho][1]-50);//현재

		//		if (!checkInner(px, py)) {//이전(밖), 현재(밖)->진입대기
		//								//이전(밖), 현재(안)->진입
		//			uList.First(j)->setX(pos[cho][0]);
		//			uList.First(j)->setY(pos[cho][1]);
		//			cout << "밖->안,밖" << endl;
		//		}
		//		else {
		//			if (!checkInner(pos[cho][0], pos[cho][1])) {//이전(안), 현재(밖)->탈출(삭제)
		//				uList.DeleteUser(j - 1);//유저 삭제
		//										//삭제한놈 flag0으로 변경
		//										//pos[cho] 999로 설정
		//					pos[cho][0] = 999;
		//					pos[cho][1] = 999;
		//					cout << "안->밖" << endl;
		//			}
		//			else {//이전(안), 현재(안)->위치이동
		//				uList.First(j)->setX(pos[cho][0]);
		//				uList.First(j)->setY(pos[cho][1]);
		//				cout << "안->안" << endl;
		//			}
		//		}
		//	}
		//}



		//for (int j = 1; j <= uList.CountUser(); j++) {//외부->내부
		//	
		//}
		//for (int j = 1; j <= uList.CountUser(); j++) {//외부->내부
		//	if (uList.First(j)->check == 2) {
		//		for (int i = 0; i < 10; i++) {
		//			if (pos[i][0] == 999) continue;
		//			if (pos[i][0]< plist.pList[0]->startX ||
		//				pos[i][1]< plist.pList[0]->startY ||
		//				pos[i][0]> plist.pList[0]->startX + plist.pList[0]->lenX ||
		//				pos[i][1]> plist.pList[0]->startY + plist.pList[0]->lenY) {//바깥 탐색됨
		//			}
		//			else {

		//			}
		//		}
		//		

		//		break;
		//	}
		//}



		//외부먼저 탐색
		//for (int i = 0; i < 10; i++) {
		//	if (pos[i][0] == 999) continue;
		//	if (pos[i][0]< plist.pList[0]->startX ||
		//		pos[i][1]< plist.pList[0]->startY ||
		//		pos[i][0]> plist.pList[0]->startX + plist.pList[0]->lenX ||
		//		pos[i][1]> plist.pList[0]->startY + plist.pList[0]->lenY) {//바깥 탐색됨


		//		for (int j = 1; j <= uList.CountUser(); j++) {//외부->내부
		//			if (uList.First(j)->check == 0) {
		//				uList.First(j)->check = 1;//1로 수정//포커스완료
		//				break;
		//			}
		//			if (uList.First(j)->check == 2) {//내부->외부
		//				int px = uList.First(j)->posX;
		//				int py = uList.First(j)->posY;
		//				if (distance(pos[i][0]-50, pos[i][1]-50, px, py) < 30) {
		//					uList.First(j)->check = 3;//1로 수정//포커스완료

		//					uList.DeleteUser(j - 1);//유저 삭제
		//											//삭제한놈 flag0으로 변경
		//											//pos[cho] 999로 설정
		//					pos[i][0] = 999;
		//					pos[i][1] = 999;
		//					cout << "삭==================제" << endl;
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}

		//////삭제부분
		////for (int j = 1; j <= uList.CountUser(); j++) {
		////	
		////	if (uList.First(j)->check == 3) {//퇴장시 가까운 점 탐색 후 999로 변경 및 uList에서 삭제
		////		int px = uList.First(j)->posX;
		////		int py = uList.First(j)->posY;
		////		int cho = -1;
		////		for (int i = 0; i < 10; i++) {
		////			if (distance(pos[i][0], pos[i][1], px, py) > 50) continue;
		////			if (min > distance(pos[i][0], pos[i][1], px, py)) {
		////				min = distance(pos[i][0], pos[i][1], px, py);
		////				cho = i;
		////			}
		////		}
		////		uList.DeleteUser(j - 1);//유저 삭제
		////								//삭제한놈 flag0으로 변경
		////								//pos[cho] 999로 설정
		////		pos[cho][0] = 999;
		////		pos[cho][1] = 999;
		////	
		////	}

		////}

		//int inuserindex = -1;
		////이동시
		//for (int i = 0; i < 10; i++)
		//	cout << pos[i][0] << "," << pos[i][1] << endl;
		//for (int j = 1; j <= uList.CountUser(); j++) {//유저와 가장 가까운 점 탐색
		//	if (uList.First(j)->check == 1) {//입장하려는애는 패스
		//		inuserindex = j;
		//		continue;//입장
		//	}
		//	if (uList.First(j)->check == 2) {//내부이동중인애만 체크

		//		cout << "내부===="<<j << endl;
		//		min = 9999;
		//		int px = uList.First(j)->posX;
		//		int py = uList.First(j)->posY;
		//		int choice = 0;
		//		int ch = -1;
		//		for (int i = 0; i < 10; i++) {
		//			if (pos[i][0] == 999) continue;
		//			if (pos[i][0]< plist.pList[0]->startX ||
		//				pos[i][1]< plist.pList[0]->startY ||
		//				pos[i][0]> plist.pList[0]->startX + plist.pList[0]->lenX ||
		//				pos[i][1]> plist.pList[0]->startY + plist.pList[0]->lenY) continue;
		//				cout << "내부=============================" << endl;
		//				if (min > distance(pos[i][0] - 50, pos[i][1] - 50, px, py)) {
		//					min = distance(pos[i][0] - 50, pos[i][1] - 50, px, py);
		//					choice = i;
		//					ch = 0;
		//				
		//			}
		//		}
		//		if (ch != -1) {
		//			uList.First(j)->setX(pos[choice][0] - 50);
		//			uList.First(j)->setY(pos[choice][1] - 50);
		//			pos[choice][0] = 999;
		//			pos[choice][1] = 999;
		//			cout << "삭=========삭삭삭=========제" << endl;
		//		}
		//		
		//	}
		//}
		//if (inuserindex != -1) {
		//	for (int i = 0; i < 10; i++) {
		//		if (pos[i][0] == 999) continue;
		//		else {
		//			uList.First(inuserindex)->posX = pos[i][0] - 50;
		//			uList.First(inuserindex)->posY = pos[i][1] - 50;
		//			uList.First(inuserindex)->check = 2;
		//			break;
		//		}
		//	}
		//}
		


		
		uList.ShowAll();
		for (int i = 0; i<uList.index; i++)
			if (strcmp(uList.uList[i]->id, "NULL") != 0) {//User있으면
				DB_Update_UserPos(uList.uList[i]->id, uList.uList[i]->posX - plist.pList[0]->startX , uList.uList[i]->posY - plist.pList[0]->startY);
				cout << "------------------------------------------asdfasdfasdf----------------------------------------" << endl;
			}
		cntLables = warp_numOfLables;
	}
	imshow("차량 추적", mat1);
}


int checkInner(int x, int y) {//0밖, 1안

	if (x< 0 ||
		y< 0 ||
		x>  plist.pList[0]->lenX ||
		y> plist.pList[0]->lenY) {//바깥 탐색됨
		return 0;
	}
	else
		return 1;


	//if (x< plist.pList[0]->startX ||
	//	y< plist.pList[0]->startY ||
	//	x> plist.pList[0]->startX + plist.pList[0]->lenX ||
	//	y> plist.pList[0]->startY + plist.pList[0]->lenY) {//바깥 탐색됨
	//	return 0;
	//}
}





double distance(int x1, int y1, int x2, int y2) {
	return sqrt(abs(x1 - x2) * abs(x1 - x2) - abs(y1 - y2) * abs(y1 - y2));
}

Mat VideoDistortion(Mat frame)
{
	CV_Assert(frame.data);

	Point2f pts1[4] = {      // 원본 영상 좌표 4개
		Point2f(minLeft[1], minLeft[0]),                             // 왼쪽 위
		Point2f(minRight[1], minRight[0]),                           // 오른쪽 위
		Point2f(minBLeft[1], minBLeft[0]),                           // 왼쪽 아래
		Point2f(minBRight[1], minBRight[0])                          // 오른쪽 아래
	};
	Point2f pts2[4] = {      // 목적 영상 좌표 4개

		Point2f(100, 100),	// 왼쪽 위 (왼쪽 옆, 위)
		Point2f(400, 100),	// 오른쪽 위 (오른쪽 옆, 위)
		Point2f(100, 400),	// 왼쪽 아래 (전체 높이, 사각형 높이 )
		Point2f(400, 400)	//오른쪽 아래
		/*Point2f(30, 30),
		Point2f(400, 30),
		Point2f(30, 400),
		Point2f(400, 400)*/
	};

	Mat video_dis_frame(frame.size(), CV_8UC1);
	perspect_map2 = getPerspectiveTransform(pts1, pts2);      // 원근 변환 행렬 계산
	warpPerspective(frame, video_dis_frame, perspect_map2, frame.size(), INTER_CUBIC);
	//cout << "[perspect_map] = " << endl << perspect_map2 << endl << endl;

	vector<Point3f> pts3, pts4;      // 3차원 좌표로 동차좌표 표현
	for (int i = 0; i < 4; i++)
	{
		pts3.push_back(Point3f(pts1[i].x, pts1[i].y, 1));      // 원본좌표 -> 동차좌표 저장
	}

	transform(pts3, pts4, perspect_map2);      // 변환좌표(pts4) 계산

	for (int i = 0; i < 4; i++)
	{
		pts4[i] /= pts4[i].z;      // 동차좌표에서 w 나누기

		//circle(frame, pts1[i], 2, Scalar(0, 0, 255), 2);
		//circle(video_dis_frame, pts2[i], 2, Scalar(0, 0, 255), 2);
	}
	return video_dis_frame;
	//imshow("더미 왜곡 보정 전", frame);
	//imshow("영상 왜곡 보정 후", video_dis_frame);
}

void trac() {


	int cas = 0;
	VideoCapture capture;
	Mat frame, df;
	Mat dum,frameMat1, frameMat2;
	Mat grayImage1, grayImage2;
	capture.open("car.mp4");
	bool pause = false;
	for (int i = 0; i < 10; i++) {
		pos[i][0] = 999;
		pos[i][1] = 999;
	}
	while (capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT) - 1-play) {
		DB_Check_Flag1Member();
		capture.read(frameMat1);
		frameMat1 = VideoDistortion(frameMat1);

		for (int i = 0; i < play; i++) {
			capture.read(frameMat2);
		}
		capture.read(frameMat2);
		frameMat2 = VideoDistortion(frameMat2);


		pand(frameMat1, frameMat2);
		switch (waitKey(1)) {
		case 112://p
			pause = !pause;
			if (pause == true) {
				cout << "Code paused, press 'p' again to resume" << endl;
				while (pause == true) {
					switch (waitKey()) {
					case 112:
						// 일시정지 해제
						pause = false;
						cout << "Code Resumed" << endl;
						break;
					}
				}
			}
			break;
		case 46://>
			play++;
			break;
		case 44://<
			play--;
			if (play < 0) play = 0;
			break;
			
		}

		//if (waitKey(100) == 27) break; //ESC키 누르면 종료
	}
}




// php







int DB_Insert_ParkingPoint(int index, int occupy, int startX, int startY, int lenX, int lenY) {
	const int bufLen = 1024;
	char buf[100];
	char url[300];
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
	printf("%s\n", szUrl);
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
int DB_Update_UserPos(char* id, int x, int y) {
	const int bufLen = 1024;
	char buf[100];
	char url[200];
	char *link = "http://nejoo97.cafe24.com/UserMove.php?id=";
	sprintf(url, "%s%s", link, id);
	char *argcat[2] = { "&POSX=", "&POSY=" };
	int arg[2];
	arg[0] = x;
	arg[1] = y;
	char tmp[10];
	for (int i = 0; i < 2; i++) {
		itoa(arg[i], tmp, 10);
		sprintf(buf, "%s%s", argcat[i], tmp);
		sprintf(url, "%s%s", url, buf);
		link = url;
	}
	char *szUrl = url;
	printf("%s\n", szUrl);
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

int DB_Update_Flag(char* id, int flag) {
	const int bufLen = 1024;
	char url[200];
	char buf[200];
	char *link = "http://nejoo97.cafe24.com/SetMemberFlag.php?id=";
	sprintf(url, "%s%s", link, id);

	char *argcat = "&FLAG=";
	char tmp[10];
	itoa(flag, tmp, 10);
	sprintf(buf, "%s%s", argcat, tmp);
	sprintf(url, "%s%s", url, buf);


	char *szUrl = url;




	printf("%s\n", szUrl);
	cout << endl;
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

int DB_Check_Flag1Member() {
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
	printf("data returned:\n%s\n", memBuffer);
	int count = 0;
	int num = 0;
	int start = 0;
	int end = 0;
	//5, 9
	for (int i = 0; i < fileSize; i++) {
		if (memBuffer[i] == '"')
			count++;
		if (count == 5 + num * 4) {
			start = i;
			for (int j = i + 1; j < fileSize; j++) {
				if (memBuffer[j] == '"') {
					end = j;
					string str = "";
					for (int k = start + 1; k < end; k++) {
						str += memBuffer[k];
					}
					char *ch = (char*)str.c_str();
					uList.AddUser(new User(ch), uList.FindNull());
					DB_Update_Flag(ch, 2);
					printf("\n%sㅎㅎ\n", ch);
					printf("\n");
					break;
				}
			}
			num++;
		}
	}
	if (fileSize != 0)
	{
		printf("Got some data\n");
		fp = fopen("downloaded.file", "wb");
		fwrite(memBuffer, 1, fileSize, fp);
		fclose(fp);
		delete(memBuffer);
		delete(headerBuffer);
	}
	cout << endl;
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

	char *argcat[2] = { "POINT=", "&OCCUPY=" };
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