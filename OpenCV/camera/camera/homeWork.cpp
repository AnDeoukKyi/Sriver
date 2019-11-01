

#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <string.h>

using namespace std;
using namespace cv;
//#include <opencv2\highgui.hpp>
//#include <opencv2\opencv.hpp>
//#include <Windows.h>
//#include <string.h>
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//
////���Ͽ���
//string OpenFileDialog();
////������׷� ���
void CalcHistogram(Mat image, Mat& histogram, int bins, int range_max = 256);
////������׷� �׸���
void GetHistogramImage(Mat histogram, Mat& histogramImage, Size size = Size(256, 200));
////������׷�,���� ���̱�
//Mat ShowHistogram(int select);
////��ȭ �Լ�
//void HistogramMatching(Mat targetImage, Mat matchingHistogram);
//
//
//
//Mat image1, image2, image3, histogram1, histogram2, histogram3;
//
//
//int main() {
//	Mat targetImage;
//	Mat matchingHistogram;
//
//	cout << "���� ���� ����" << endl;
//	targetImage = ShowHistogram(1);
//	image1 = ShowHistogram(0);
//	histogram1 = ShowHistogram(1);
//	imshow("1", targetImage);
//
//	cout << "��ȭ ���� ���� ����" << endl;
//	image2 = ShowHistogram(0);
//	histogram2 = ShowHistogram(1);
//	imshow("2", targetImage);
//
//	//HistogramMatching(targetImage, matchingHistogram);
//
//	return 0;
//}
//
////���Ͽ���
//string OpenFileDialog() {
//
//	char name[MAX_PATH] = { 0, };
//	OPENFILENAMEA ofn;
//
//	ZeroMemory(&ofn, sizeof(ofn));
//	ofn.lStructSize = sizeof(OPENFILENAMEA);
//	ofn.hwndOwner = NULL;
//	ofn.lpstrFilter = "�������(*.*)\0*.*\0";
//	ofn.lpstrFile = name;
//	ofn.nMaxFile = MAX_PATH;
//	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//	ofn.lpstrDefExt = "";
//
//	string strName;
//	if (GetOpenFileNameA(&ofn)) {
//		strName = name;
//	}
//
//	return strName;
//}
//

////������׷� ���
void CalcHistogram(Mat image, Mat& histogram, int bins, int range_max) {
	histogram = Mat(bins, 1, CV_32F, Scalar(0));
	float gap = (float)range_max / bins;

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			auto workPixel = image.at<uchar>(i, j);
			int idx = (int)(workPixel / gap);
			histogram.at<float>(idx)++;
		}
	}
}
////������׷� �׸���
void GetHistogramImage(Mat histogram, Mat& histogramImage, Size size) {
	
	histogramImage = Mat(size, CV_8U, Scalar(255));

	float gap = (float)(histogramImage.cols / histogram.rows);
	normalize(histogram, histogram, 0, histogramImage.rows, NORM_MINMAX);

	for (int i = 0; i < histogram.rows; i++) {
		float sx = i * gap;
		float ex = (i * 1) * gap;

		Point2f pt_lb(sx, 0), pt_rt(ex, histogram.at<float>(i));
		
		if (pt_rt.y > 0)
			rectangle(histogramImage, pt_lb, pt_rt, Scalar(0), -1);
	}
	flip(histogramImage, histogramImage, 0);
}
//
////������׷�,���� ���̱�
//Mat ShowHistogram(int select) {
//
//	auto fileName = OpenFileDialog();
//
//	Mat image = imread(fileName, IMREAD_GRAYSCALE);
//
//	//if (image.empty()) {
//	//	cout << "���� �б� ����" << endl;
//	//	return NULL;
//	//}
//
//	Mat histogram, histogramImage;
//
//	CalcHistogram(image, histogram, 256);
//	cout << histogram.t() << endl;
//	GetHistogramImage(histogram, histogramImage);
//	imshow("���� : ��� ����", image);
//	imshow("���� : ������׷�", histogramImage);
//	waitKey();
//
//
//
//
//
//	//-----------------------------------
//	/*int histograma[256] = { 0, };
//
//	for (int y = 0; y < image.rows; y++)
//	{
//		for (int x = 0; x < image.cols; x++)
//		{
//			int value = image.at<uchar>(y, x);
//			histograma[value] += 1;
//		}
//	}
//	for (int i = 0; i < 256; i++)
//		cout << histograma[i] << endl;
//
//
//*/
//
//	//--------------------------------
//
//
//
//	if (select == 1)
//		return image;
//	else
//		return histogramImage;
//}
//
////��ȭ �Լ�
//void HistogramMatching(Mat targetImage, Mat& resultImage, Mat matchingHistogram) {
//	resultImage = Mat(targetImage.size(), CV_8U, Scalar(0));
//
//
//
//}

//



int main()
{
	//Mat img_input, image_output, img_gray, img_grayg, img_g, img_result, img_histogram, img_histogram2;

	Mat matchingImage, targetImage, matching_gray, target_gray, mimg_result, timg_result;

	//�̹��� ������ �о�ͼ� img_input�� ����
	matchingImage = imread("sample128.png", IMREAD_COLOR);
	targetImage = imread("Lenna512.png", IMREAD_COLOR);

	resize(matchingImage, matchingImage, Size(targetImage.rows, targetImage.cols), 0, 0, CV_INTER_LINEAR);//ũ�⺯ȯ



	//��Ī�� �׷��̽����Ϸ� ����
	matching_gray = Mat(matchingImage.rows, matchingImage.cols, CV_8UC1);

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			//img_input���κ��� ���� ��ġ (y,x) �ȼ���
			//blue, green, red ���� �о�´�. 
			uchar blue = matchingImage.at<Vec3b>(y, x)[0];
			uchar green = matchingImage.at<Vec3b>(y, x)[1];
			uchar red = matchingImage.at<Vec3b>(y, x)[2];

			//blue, green, red�� ���� ��, 3���� ������ �׷��̽������� �ȴ�.
			uchar gray = (blue + green + red) / 3.0;

			//MatŸ�� ���� img_gray�� �����Ѵ�. 
			matching_gray.at<uchar>(y, x) = gray;
		}
	}

	//Ÿ���� �׷��̽����Ϸ� ����
	target_gray = Mat(targetImage.rows, targetImage.cols, CV_8UC1);

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			//img_input���κ��� ���� ��ġ (y,x) �ȼ���
			//blue, green, red ���� �о�´�. 
			uchar blue = targetImage.at<Vec3b>(y, x)[0];
			uchar green = targetImage.at<Vec3b>(y, x)[1];
			uchar red = targetImage.at<Vec3b>(y, x)[2];

			//blue, green, red�� ���� ��, 3���� ������ �׷��̽������� �ȴ�.
			uchar gray = (blue + green + red) / 3.0;

			//MatŸ�� ���� img_gray�� �����Ѵ�. 
			target_gray.at<uchar>(y, x) = gray;
		}
	}

	//��Ī �׷��̽����� ������ ������׷� ���
	int matching_histogram[256] = { 0, };

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			int value = matching_gray.at<uchar>(y, x);
			matching_histogram[value] += 1;
		}
	}

	//Ÿ�� �׷��̽����� ������ ������׷� ���
	int target_histogram[256] = { 0, };

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			int value = target_gray.at<uchar>(y, x);
			target_histogram[value] += 1;
		}
	}

	//��Ī �׷��̽����� ������ ���� ������׷� ���
	int mcumulative_histogram[256] = { 0, };
	int sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += matching_histogram[i];
		mcumulative_histogram[i] = sum;
	}

	//Ÿ�� �׷��̽����� ������ ���� ������׷� ���
	int tcumulative_histogram[256] = { 0, };
	sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += target_histogram[i];
		tcumulative_histogram[i] = sum;
	}

	//��Ī �׷��̽����� ������ ����ȭ�� ���� ������׷� ���
	float mnormalized_cumulative_histogram[256] = { 0.0, };
	int mimage_size = matchingImage.rows * matchingImage.cols;

	for (int i = 0; i < 256; i++)
	{
		mnormalized_cumulative_histogram[i] = mcumulative_histogram[i] / (float)mimage_size;
	}

	//Ÿ�� �׷��̽����� ������ ����ȭ�� ���� ������׷� ���
	float tnormalized_cumulative_histogram[256] = { 0.0, };
	int timage_size = targetImage.rows * targetImage.cols;

	for (int i = 0; i < 256; i++)
	{
		tnormalized_cumulative_histogram[i] = tcumulative_histogram[i] / (float)timage_size;
	}



	//imshow("��Ī ����", matchingImage);
	//imshow("Ÿ�� ����", targetImage);
	imshow("��Ī �׷��̽����� ����", matching_gray);
	imshow("Ÿ�� �׷��̽����� ����", target_gray);

	Mat mhistogram, thistogram, mhistogramImage, thistogramImage;
	//��Ī
	CalcHistogram(matching_gray, mhistogram, 256);
	GetHistogramImage(mhistogram, mhistogramImage);

	//Ÿ��
	CalcHistogram(target_gray, thistogram, 256);
	GetHistogramImage(thistogram, thistogramImage);

	imshow("��Ī : ������׷�", mhistogramImage);
	imshow("Ÿ�� : ������׷�", thistogramImage);



	//��Ī ������׷� ��Ȱȭ ���� �� ��� ������ ������׷� ���
	mimg_result = Mat(matchingImage.rows, matchingImage.cols, CV_8UC1);
	int mhistogram2[256] = { 0, };
	for (int y = 0; y<matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			mimg_result.at<uchar>(y, x) = mnormalized_cumulative_histogram[matching_gray.at<uchar>(y, x)] * 255;
			mhistogram2[mimg_result.at<uchar>(y, x)] += 1;
		}
	}

	//��Ī ������׷� ��Ȱȭ ���� �� ��� ������ ������׷� ���
	timg_result = Mat(targetImage.rows, targetImage.cols, CV_8UC1);
	int thistogram2[256] = { 0, };
	for (int y = 0; y<targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			timg_result.at<uchar>(y, x) = tnormalized_cumulative_histogram[target_gray.at<uchar>(y, x)] * 255;
			thistogram2[timg_result.at<uchar>(y, x)] += 1;
		}
	}


	imshow("��Ī ��Ȱȭ ����", mimg_result);
	imshow("Ÿ�� ��Ȱȭ ����", timg_result);



	Mat mhistogramR, thistogramR, mhistogramRImage, thistogramRImage;
	//��Ī
	CalcHistogram(mimg_result, mhistogramR, 256);
	GetHistogramImage(mhistogramR, mhistogramRImage);
	Mat k = mhistogramR.t();
	cout << k << endl;
	cout << "gggg" << endl;

	int look[256] = { 0 };
	int mat[256] = { 0 };
	for (int i = 0; i < 256; i++)
		mat[i] = (int)k.at<float>(0, i);

	for (int i = 0; i < 256; i++) {
		cout << mat[i];
		cout << ",";
	}
	for (int i = 0; i < 256; i++) {
		int j = mat[i];
		look[j] = i;
		look[j + 1] = i;
	}

	cout << "gggg" << endl;
	for (int i = 0; i < 256; i++) {
		cout << look[i];
		cout << ",";
	}


	Mat res = Mat(targetImage.rows, targetImage.cols, CV_8UC1);

	for (int y = 0; y<timg_result.rows; y++)
	{
		for (int x = 0; x < timg_result.cols; x++)
		{
			res.at<uchar>(y, x) = look[(int)timg_result.at<uchar>(y, x)];
		}
	}

	imshow("��Ī gggggg ����", res);

	while (cvWaitKey(0) == 0);

	//





	////��� ������ ���� ������׷� ���
	//int cumulative_histogram2[256] = { 0, };
	//sum = 0;

	//for (int i = 1; i < 256; i++)
	//{
	//	sum += histogram2[i];
	//	cumulative_histogram2[i] = sum;
	//}


	////������׷� �׸���
	//img_histogram = Mat(300, 600, CV_8UC1, Scalar(0));
	//img_histogram2 = Mat(300, 600, CV_8UC1, Scalar(0));

	//int max = -1;
	//for (int i = 0; i < 256; i++)
	//	if (max < histogram[i]) max = histogram[i];

	//int max2 = -1;
	//for (int i = 0; i < 256; i++)
	//	if (max2 < histogram2[i]) max2 = histogram2[i];

	//for (int i = 0; i<256; i++)
	//{
	//	int histo = 300 * histogram[i] / (float)max;
	//	int cumulative_histo = 300 * cumulative_histogram[i] / (float)cumulative_histogram[255];

	//	line(img_histogram, cvPoint(i + 10, 300), cvPoint(i + 10, 300 - histo), Scalar(255, 255, 255));
	//	line(img_histogram, cvPoint(i + 300, 300), cvPoint(i + 300, 300 - cumulative_histo), Scalar(255, 255, 255));


	//	int histo2 = 300 * histogram2[i] / (float)max2;
	//	int cumulative_histo2 = 300 * cumulative_histogram2[i] / (float)cumulative_histogram2[255];

	//	line(img_histogram2, cvPoint(i + 10, 300), cvPoint(i + 10, 300 - histo2), Scalar(255, 255, 255));
	//	line(img_histogram2, cvPoint(i + 300, 300), cvPoint(i + 300, 300 - cumulative_histo2), Scalar(255, 255, 255));
	//}


	////ȭ�鿡 ��� �̹����� �����ش�.
	//imshow("cpu ����", img_input);
	//imshow("���� ����", image_output);
	//imshow("�Է� �׷��̽����� ����", img_gray);
	//imshow("�Է� �׷��̽����� ����", img_grayg);


	//imshow("�Է� �׷��̽����� ����", img_gray);
	//imshow("��� �׷��̽����� ����", img_result);
	//imshow("�Է� ������ ������׷�", img_histogram);
	//imshow("��Ȱȭ �� ������׷�", img_histogram2);

	////�ƹ�Ű�� ������ ������ ���
	//while (cvWaitKey(0) == 0);

	////����� ���Ϸ� ����
	//imwrite("img_gray.jpg", img_gray);
	//imwrite("img_result.jpg", img_result);
	//imwrite("img_histogram.jpg", img_histogram);
	//imwrite("img_histogram2.jpg", img_histogram2);
}
