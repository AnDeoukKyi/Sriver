#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat img_frame;			//원본 img
Mat lines;


void videoCapture()
{
	VideoCapture capture(0);
	namedWindow("camera");

	Mat video;

	while (true)
	{
		if (waitKey(1) > 0)
			break;

		capture >> video;
		imshow("camera", video);
	}

	destroyAllWindows();

}

void hough_coord(Mat img_frame, Mat& acc_mat, double rho, double theta)		// 누적 행렬
{
	cout << 1 << endl;
	int acc_h = (img_frame.rows + img_frame.cols) * 2 / rho;			// 누적 행렬 높이
	int	acc_w = CV_PI / theta;											// 누적 행렬 너비
	acc_mat = Mat(acc_h, acc_w, CV_32S, Scalar(0));						// 허프 누적 행렬

	for (int y = 0; y < img_frame.rows; y++)							// 입력 화소 조회
	{
		for (int x = 0; x < img_frame.cols; x++)
		{
			Point pt(x, y);												// 조회 좌표
			if (img_frame.at<uchar>(pt) > 0)							// 직선 여부 검사
			{
				for (int t = 0; t < acc_w; t++)							// 0~180도 반복
				{
					double radian = t * theta;
					float r = pt.x * cos(radian) + pt.y * sin(radian);
					r = cvRound(r / rho + acc_mat.rows / 2);
					acc_mat.at<int>(r, t)++;							// (p, 세타> 좌표에 누적
				}
			}
		}
	}
	cout << 1 << endl;
}

void acc_mask(Mat acc_mat, Mat& acc_dst, Size size, int thresh)			// 지역 최대값
{
	cout << 2 << endl;
	acc_dst = Mat(acc_mat.size(), CV_32S, Scalar(0));
	Point h_m = size / 2;												// 마스크 크기 절반

	for (int r = h_m.y; r < acc_mat.rows - h_m.y; r++)					// 누적 행렬 조회
	{
		for (int t = h_m.x; t < acc_mat.cols - h_m.x; t++)
		{
			Point center = Point(t, r) - h_m;
			int c_value = acc_mat.at<int>(center);						// 중심화소
			if (c_value >= thresh)
			{
				double maxVal = 0;
				for (int u = 0; u < size.height; u++)					// 마스크 범위 조회				
				{
					for (int v = 0; v < size.width; v++)
					{
						Point start = center + Point(v, u);
						if (start != center && acc_mat.at<int>(start) > maxVal)
							maxVal = acc_mat.at<int>(start);
					}
				}

				Rect rect(center, size);
				if (c_value >= maxVal)									// 중심화소가 최대값이면
				{
					acc_dst.at<int>(center) = c_value;					// 반환 행렬에 중심화소값 저장
					acc_mat(rect).setTo(0);
				}
			}
		}
	}
	cout << 2 << endl;
}

void sort_lines(Mat lines, vector<Vec2f>& s_lines)		// 직선 정렬
{
	cout << 3 << endl;
	Mat acc = lines.col(2), idx;										// 누적값
	sortIdx(acc, idx, SORT_EVERY_COLUMN + SORT_DESCENDING);

	for (int i = 0; i < idx.rows; i++)
	{
		int id = idx.at<int>(i);										// 정렬 원소에 대한 원본 인덱스
		float rho = lines.at<float>(id, 0);								// 0번 열 - 수직거리
		float radian = lines.at<float>(id, 1);							// 1번 열 - 각도
		s_lines.push_back(Vec2f(rho, radian));
	}
	cout << 3 << endl;
}

void thres_lines(Mat acc_dst, Mat& lines, double _rho, double theta, int thresh)
{
	cout << 4 << endl;
	for (int r = 0; r < acc_dst.rows; r++)
	{
		for (int t = 0; t < acc_dst.cols; t++)
		{
			float value = (float)acc_dst.at<int>(r, t);
			if (value >= thresh)
			{
				float rho = (float)((r - acc_dst.rows / 2) * _rho);		// 수직거리
				float radian = (float)(t*theta);						// 각도

				Matx13f line(rho, radian, value);						// 단일 직선
				lines.push_back((Mat)line);								// lines 행렬에 직선 저장
			}
		}
	}
	cout << 4 << endl;
}

void houghLines(Mat src, vector<Vec2f>& s_lines, double rho, double theta, int thresh)
{
	cout << 5 << endl;
	Mat acc_mat, acc_dst;
	hough_coord(src, acc_mat, rho, theta);								// 허프 누적 행렬 계산
	acc_mask(acc_mat, acc_dst, Size(3, 7), thresh);						// 마스킹 처리

	thres_lines(acc_dst, lines, rho, theta, thresh);					// 직선 가져옴
	sort_lines(lines, s_lines);											// 누적값에 따른 직선 정렬
	cout << 5 << endl;
}

void draw_houghLines(Mat src, Mat& dst, vector<Vec2f> lines, int nline)
{
	cout << 6 << endl;
	cvtColor(src, dst, CV_GRAY2BGR);									// 컬러 영상 변환
	for (size_t i = 0; i < min((int)lines.size(), nline); i++)			// 검출 직선개수 반복
	{
		float rho = lines[i][0], theta = lines[i][1];					// 수직거리, 각도
		double a = cos(theta), b = sin(theta);
		Point2d pt(a * rho, b * rho);									// 검출 직선상의 한 좌표 계산
		Point2d delta(1000 * -b, 1000 * a);								// 직선상의 이동 위치
		line(dst, pt + delta, pt - delta, Scalar(0, 255, 0), 1, LINE_AA);
	}
	cout << 6 << endl;
}


int main()
{

	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);

	videoCapture();

	double rho = 1, theta = CV_PI / 180;								// 거리간격, 각도간격
	Mat canny, dst1, dst2;
	GaussianBlur(img_frame, canny, Size(5, 5), 2, 2);					// 가우시안 블러링
	Canny(canny, canny, 30, 150, 3);									// 캐니 에지 검출
	cout << 7 << endl;


	vector<Vec2f> lines1, lines2;
	houghLines(canny, lines1, rho, theta, 50);							// 직접 구현 함수
	HoughLines(canny, lines2, rho, theta, 50);							// OpenCV 제공함수
	draw_houghLines(canny, dst1, lines1, 10);							// 검출 직선 그리기
	draw_houghLines(canny, dst2, lines2, 10);

	imshow("source", img_frame);
	imshow("canny", canny);
	imshow("detected lines", dst1);
	imshow("detected OpenCV_lines", dst2);
	waitKey(0);
}