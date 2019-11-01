
#include <iostream>
#include <opencv2\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <string.h>

using namespace std;
using namespace cv;

////������׷� ���
void CalcHistogram(Mat image, Mat& histogram, int bins, int range_max = 256);
////������׷� �׸���
void GetHistogramImage(Mat histogram, Mat& histogramImage, Size size = Size(256, 200));
//������׷� ���
Mat ShowHistogram(Mat image);
//�׷��̽����Ϸ� ����
Mat TransfertoGrayScale(Mat image);

void HistogramMatching(Mat originalImage, Mat matchingImage);

int main() {
	Mat matchingImage, originalImage;
	Mat matchingHistogram, originalHistogram;

	matchingImage = imread("sample128.png", IMREAD_GRAYSCALE);
	originalImage = imread("Lenna512.png", IMREAD_GRAYSCALE);

	//�������� ����� �°� ���� �̹��� �ٲ�
	resize(matchingImage, matchingImage, Size(originalImage.rows, originalImage.cols), 0, 0, CV_INTER_LINEAR);//ũ�⺯ȯ

	//������׷� ���
	originalHistogram = ShowHistogram(originalImage);
	matchingHistogram = ShowHistogram(matchingImage);


	imshow("���ϴ� ����", matchingImage);
	imshow("���� ����", originalImage);
	
	imshow("���ϴ� ������׷�", matchingHistogram);
	imshow("���� ������׷�", originalHistogram);

	HistogramMatching(originalImage, matchingImage);


	while (cvWaitKey(0) == 0);
}


//������׷� ���
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

//�׷��̽����Ϸ� ����
Mat TransfertoGrayScale(Mat image) {
	Mat imageGray;
	imageGray = Mat(imageGray.rows, imageGray.cols, CV_8UC1);

	for (int y = 0; y < imageGray.rows; y++)
	{
		for (int x = 0; x < imageGray.cols; x++)
		{
			//img_input���κ��� ���� ��ġ (y,x) �ȼ���
			//blue, green, red ���� �о�´�. 
			uchar blue = imageGray.at<Vec3b>(y, x)[0];
			uchar green = imageGray.at<Vec3b>(y, x)[1];
			uchar red = imageGray.at<Vec3b>(y, x)[2];

			//blue, green, red�� ���� ��, 3���� ������ �׷��̽������� �ȴ�.
			uchar gray = (blue + green + red) / 3.0;

			//MatŸ�� ���� img_gray�� �����Ѵ�. 
			imageGray.at<uchar>(y, x) = gray;
		}
	}
	return imageGray;
}

Mat ShowHistogram(Mat image) {

	Mat histogram, histogramImage;
	CalcHistogram(image, histogram, 256);
	cout << histogram.t() << endl;
	GetHistogramImage(histogram, histogramImage);
	return histogramImage;
	
}

void HistogramMatching(Mat originalImage, Mat matchingImage) {

	
	//�������� ��Ȱȭ
	//���������� ������׷� ���
	int original_histogram[256] = { 0, };

	for (int y = 0; y < originalImage.rows; y++)
	{
		for (int x = 0; x < originalImage.cols; x++)
		{
			int value = originalImage.at<uchar>(y, x);
			original_histogram[value] += 1;
		}
	}
	
	//���������� ���� ������׷� ���
	int ocumulative_histogram[256] = { 0, };
	int sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += original_histogram[i];
		ocumulative_histogram[i] = sum;
	}
	//���������� ����ȭ�� ���� ������׷� ���
	float onormalized_cumulative_histogram[256] = { 0.0, };
	int oimage_size = originalImage.rows * originalImage.cols;

	for (int i = 0; i < 256; i++)
	{
		onormalized_cumulative_histogram[i] = ocumulative_histogram[i] / (float)oimage_size;
	}

	//���� ������׷� ��Ȱȭ ���� �� ��� ������ ������׷� ���
	Mat originalEqulization;
	originalEqulization = Mat(originalImage.rows, originalImage.cols, CV_8UC1);
	int ohistogram[256] = { 0, };
	for (int y = 0; y<originalEqulization.rows; y++)
	{
		for (int x = 0; x < originalImage.cols; x++)
		{
			originalEqulization.at<uchar>(y, x) = onormalized_cumulative_histogram[originalImage.at<uchar>(y, x)] * 255;
			ohistogram[originalEqulization.at<uchar>(y, x)] += 1;
		}
	}

	imshow("���� ��Ȱȭ ����", originalEqulization);
	Mat originalEqHistogram;
	originalEqHistogram = ShowHistogram(originalEqulization);
	imshow("���� ��Ȱȭ ������׷�", originalEqHistogram);




	//���ϴ� ������ ��Ȱȭ
	//���ϴ� ������ ������׷� ���
	int matching_histogram[256] = { 0, };

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			int value = matchingImage.at<uchar>(y, x);
			matching_histogram[value] += 1;
		}
	}

	//���ϴ� ������ ���� ������׷� ���
	int mcumulative_histogram[256] = { 0, };
	sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += matching_histogram[i];
		mcumulative_histogram[i] = sum;
	}
	//���ϴ� ������ ����ȭ�� ���� ������׷� ���
	float mnormalized_cumulative_histogram[256] = { 0.0, };
	int mimage_size = matchingImage.rows * matchingImage.cols;

	for (int i = 0; i < 256; i++)
	{
		mnormalized_cumulative_histogram[i] = mcumulative_histogram[i] / (float)mimage_size;
	}

	
	//���ϴ� ������׷� ��Ȱȭ ���� �� ��� ������ ������׷� ���
	Mat matchingEqulization;
	matchingEqulization = Mat(matchingImage.rows, matchingImage.cols, CV_8UC1);
	int mhistogram[256] = { 0, };
	for (int y = 0; y<matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			matchingEqulization.at<uchar>(y, x) = mnormalized_cumulative_histogram[matchingImage.at<uchar>(y, x)] * 255;
			mhistogram[matchingEqulization.at<uchar>(y, x)] += 1;
		}
	}

	imshow("���ϴ� ��Ȱȭ ����", matchingEqulization);
	Mat matchingEqHistogram;
	matchingEqHistogram = ShowHistogram(matchingEqulization);
	imshow("���ϴ� ��Ȱȭ ������׷�", matchingEqHistogram);


	//������̺� �����

	for (int i = 0; i < 256; i++)
	{
		mnormalized_cumulative_histogram[i] = mcumulative_histogram[i] / (float)mimage_size;
	}

	float test[256];


	for (int i = 0; i < mimage_size; i++) {
		test[i] = mnormalized_cumulative_histogram[i];
	}

	for (int i = 0; i < mimage_size; i++) {
		//if(mnormalized_cumulative_histogram[i]-(int)mnormalized_cumulative_histogram[i] >= 0.5)

	}


}

