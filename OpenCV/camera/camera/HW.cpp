
#include <iostream>
#include <opencv2\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <string.h>

using namespace std;
using namespace cv;

////히스토그램 계산
void CalcHistogram(Mat image, Mat& histogram, int bins, int range_max = 256);
////히스토그램 그리기
void GetHistogramImage(Mat histogram, Mat& histogramImage, Size size = Size(256, 200));
//히스토그램 출력
Mat ShowHistogram(Mat image);
//그레이스케일로 변경
Mat TransfertoGrayScale(Mat image);

void HistogramMatching(Mat originalImage, Mat matchingImage);

int main() {
	Mat matchingImage, originalImage;
	Mat matchingHistogram, originalHistogram;

	matchingImage = imread("sample128.png", IMREAD_GRAYSCALE);
	originalImage = imread("Lenna512.png", IMREAD_GRAYSCALE);

	//원본영상 사이즈에 맞게 명세서 이미지 바꿈
	resize(matchingImage, matchingImage, Size(originalImage.rows, originalImage.cols), 0, 0, CV_INTER_LINEAR);//크기변환

	//히스토그램 출력
	originalHistogram = ShowHistogram(originalImage);
	matchingHistogram = ShowHistogram(matchingImage);


	imshow("원하는 영상", matchingImage);
	imshow("원본 영상", originalImage);
	
	imshow("원하는 히스토그램", matchingHistogram);
	imshow("원본 히스토그램", originalHistogram);

	HistogramMatching(originalImage, matchingImage);


	while (cvWaitKey(0) == 0);
}


//히스토그램 계산
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

////히스토그램 그리기
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

//그레이스케일로 변경
Mat TransfertoGrayScale(Mat image) {
	Mat imageGray;
	imageGray = Mat(imageGray.rows, imageGray.cols, CV_8UC1);

	for (int y = 0; y < imageGray.rows; y++)
	{
		for (int x = 0; x < imageGray.cols; x++)
		{
			//img_input으로부터 현재 위치 (y,x) 픽셀의
			//blue, green, red 값을 읽어온다. 
			uchar blue = imageGray.at<Vec3b>(y, x)[0];
			uchar green = imageGray.at<Vec3b>(y, x)[1];
			uchar red = imageGray.at<Vec3b>(y, x)[2];

			//blue, green, red를 더한 후, 3으로 나누면 그레이스케일이 된다.
			uchar gray = (blue + green + red) / 3.0;

			//Mat타입 변수 img_gray에 저장한다. 
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

	
	//원본영상 평활화
	//원본영상의 히스토그램 계산
	int original_histogram[256] = { 0, };

	for (int y = 0; y < originalImage.rows; y++)
	{
		for (int x = 0; x < originalImage.cols; x++)
		{
			int value = originalImage.at<uchar>(y, x);
			original_histogram[value] += 1;
		}
	}
	
	//원본영상의 누적 히스토그램 계산
	int ocumulative_histogram[256] = { 0, };
	int sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += original_histogram[i];
		ocumulative_histogram[i] = sum;
	}
	//원본영상의 정규화된 누적 히스토그램 계산
	float onormalized_cumulative_histogram[256] = { 0.0, };
	int oimage_size = originalImage.rows * originalImage.cols;

	for (int i = 0; i < 256; i++)
	{
		onormalized_cumulative_histogram[i] = ocumulative_histogram[i] / (float)oimage_size;
	}

	//원본 히스토그램 평활화 적용 및 결과 영상의 히스토그램 계산
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

	imshow("원본 평활화 영상", originalEqulization);
	Mat originalEqHistogram;
	originalEqHistogram = ShowHistogram(originalEqulization);
	imshow("원본 평활화 히스토그램", originalEqHistogram);




	//원하는 영상의 평활화
	//원하는 영상의 히스토그램 계산
	int matching_histogram[256] = { 0, };

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			int value = matchingImage.at<uchar>(y, x);
			matching_histogram[value] += 1;
		}
	}

	//원하는 영상의 누적 히스토그램 계산
	int mcumulative_histogram[256] = { 0, };
	sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += matching_histogram[i];
		mcumulative_histogram[i] = sum;
	}
	//원하는 영상의 정규화된 누적 히스토그램 계산
	float mnormalized_cumulative_histogram[256] = { 0.0, };
	int mimage_size = matchingImage.rows * matchingImage.cols;

	for (int i = 0; i < 256; i++)
	{
		mnormalized_cumulative_histogram[i] = mcumulative_histogram[i] / (float)mimage_size;
	}

	
	//원하는 히스토그램 평활화 적용 및 결과 영상의 히스토그램 계산
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

	imshow("원하는 평활화 영상", matchingEqulization);
	Mat matchingEqHistogram;
	matchingEqHistogram = ShowHistogram(matchingEqulization);
	imshow("원하는 평활화 히스토그램", matchingEqHistogram);


	//룩업테이블 만들기

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

