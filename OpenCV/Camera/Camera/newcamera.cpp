#pragma warning(disable:4819) 
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat img_frame;			//���� img
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

void hough_coord(Mat img_frame, Mat& acc_mat, double rho, double theta)		// ���� ���
{
	cout << 1 << endl;
	int acc_h = (img_frame.rows + img_frame.cols) * 2 / rho;			// ���� ��� ����
	int	acc_w = CV_PI / theta;											// ���� ��� �ʺ�
	acc_mat = Mat(acc_h, acc_w, CV_32S, Scalar(0));						// ���� ���� ���

	for (int y = 0; y < img_frame.rows; y++)							// �Է� ȭ�� ��ȸ
	{
		for (int x = 0; x < img_frame.cols; x++)
		{
			Point pt(x, y);												// ��ȸ ��ǥ
			if (img_frame.at<uchar>(pt) > 0)							// ���� ���� �˻�
			{
				for (int t = 0; t < acc_w; t++)							// 0~180�� �ݺ�
				{
					double radian = t * theta;
					float r = pt.x * cos(radian) + pt.y * sin(radian);
					r = cvRound(r / rho + acc_mat.rows / 2);
					acc_mat.at<int>(r, t)++;							// (p, ��Ÿ> ��ǥ�� ����
				}
			}
		}
	}
	cout << 1 << endl;
}

void acc_mask(Mat acc_mat, Mat& acc_dst, Size size, int thresh)			// ���� �ִ밪
{
	cout << 2 << endl;
	acc_dst = Mat(acc_mat.size(), CV_32S, Scalar(0));
	Point h_m = size / 2;												// ����ũ ũ�� ����

	for (int r = h_m.y; r < acc_mat.rows - h_m.y; r++)					// ���� ��� ��ȸ
	{
		for (int t = h_m.x; t < acc_mat.cols - h_m.x; t++)
		{
			Point center = Point(t, r) - h_m;
			int c_value = acc_mat.at<int>(center);						// �߽�ȭ��
			if (c_value >= thresh)
			{
				double maxVal = 0;
				for (int u = 0; u < size.height; u++)					// ����ũ ���� ��ȸ				
				{
					for (int v = 0; v < size.width; v++)
					{
						Point start = center + Point(v, u);
						if (start != center && acc_mat.at<int>(start) > maxVal)
							maxVal = acc_mat.at<int>(start);
					}
				}

				Rect rect(center, size);
				if (c_value >= maxVal)									// �߽�ȭ�Ұ� �ִ밪�̸�
				{
					acc_dst.at<int>(center) = c_value;					// ��ȯ ��Ŀ� �߽�ȭ�Ұ� ����
					acc_mat(rect).setTo(0);
				}
			}
		}
	}
	cout << 2 << endl;
}

void sort_lines(Mat lines, vector<Vec2f>& s_lines)		// ���� ����
{
	cout << 3 << endl;
	Mat acc = lines.col(2), idx;										// ������
	sortIdx(acc, idx, SORT_EVERY_COLUMN + SORT_DESCENDING);

	for (int i = 0; i < idx.rows; i++)
	{
		int id = idx.at<int>(i);										// ���� ���ҿ� ���� ���� �ε���
		float rho = lines.at<float>(id, 0);								// 0�� �� - �����Ÿ�
		float radian = lines.at<float>(id, 1);							// 1�� �� - ����
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
				float rho = (float)((r - acc_dst.rows / 2) * _rho);		// �����Ÿ�
				float radian = (float)(t*theta);						// ����

				Matx13f line(rho, radian, value);						// ���� ����
				lines.push_back((Mat)line);								// lines ��Ŀ� ���� ����
			}
		}
	}
	cout << 4 << endl;
}

void houghLines(Mat src, vector<Vec2f>& s_lines, double rho, double theta, int thresh)
{
	cout << 5 << endl;
	Mat acc_mat, acc_dst;
	hough_coord(src, acc_mat, rho, theta);								// ���� ���� ��� ���
	acc_mask(acc_mat, acc_dst, Size(3, 7), thresh);						// ����ŷ ó��

	thres_lines(acc_dst, lines, rho, theta, thresh);					// ���� ������
	sort_lines(lines, s_lines);											// �������� ���� ���� ����
	cout << 5 << endl;
}

void draw_houghLines(Mat src, Mat& dst, vector<Vec2f> lines, int nline)
{
	cout << 6 << endl;
	cvtColor(src, dst, CV_GRAY2BGR);									// �÷� ���� ��ȯ
	for (size_t i = 0; i < min((int)lines.size(), nline); i++)			// ���� �������� �ݺ�
	{
		float rho = lines[i][0], theta = lines[i][1];					// �����Ÿ�, ����
		double a = cos(theta), b = sin(theta);
		Point2d pt(a * rho, b * rho);									// ���� �������� �� ��ǥ ���
		Point2d delta(1000 * -b, 1000 * a);								// �������� �̵� ��ġ
		line(dst, pt + delta, pt - delta, Scalar(0, 255, 0), 1, LINE_AA);
	}
	cout << 6 << endl;
}


int main()
{

	img_frame = imread("A.jpg", IMREAD_COLOR);
	CV_Assert(img_frame.data);

	videoCapture();

	double rho = 1, theta = CV_PI / 180;								// �Ÿ�����, ��������
	Mat canny, dst1, dst2;
	GaussianBlur(img_frame, canny, Size(5, 5), 2, 2);					// ����þ� ����
	Canny(canny, canny, 30, 150, 3);									// ĳ�� ���� ����
	cout << 7 << endl;


	vector<Vec2f> lines1, lines2;
	houghLines(canny, lines1, rho, theta, 50);							// ���� ���� �Լ�
	HoughLines(canny, lines2, rho, theta, 50);							// OpenCV �����Լ�
	draw_houghLines(canny, dst1, lines1, 10);							// ���� ���� �׸���
	draw_houghLines(canny, dst2, lines2, 10);

	imshow("source", img_frame);
	imshow("canny", canny);
	imshow("detected lines", dst1);
	imshow("detected OpenCV_lines", dst2);
	waitKey(0);
}