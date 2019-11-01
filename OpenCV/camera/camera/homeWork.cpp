

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
////파일열기
//string OpenFileDialog();
////히스토그램 계산
void CalcHistogram(Mat image, Mat& histogram, int bins, int range_max = 256);
////히스토그램 그리기
void GetHistogramImage(Mat histogram, Mat& histogramImage, Size size = Size(256, 200));
////히스토그램,영상 보이기
//Mat ShowHistogram(int select);
////명세화 함수
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
//	cout << "원본 영상 선택" << endl;
//	targetImage = ShowHistogram(1);
//	image1 = ShowHistogram(0);
//	histogram1 = ShowHistogram(1);
//	imshow("1", targetImage);
//
//	cout << "명세화 기준 영상 선택" << endl;
//	image2 = ShowHistogram(0);
//	histogram2 = ShowHistogram(1);
//	imshow("2", targetImage);
//
//	//HistogramMatching(targetImage, matchingHistogram);
//
//	return 0;
//}
//
////파일열기
//string OpenFileDialog() {
//
//	char name[MAX_PATH] = { 0, };
//	OPENFILENAMEA ofn;
//
//	ZeroMemory(&ofn, sizeof(ofn));
//	ofn.lStructSize = sizeof(OPENFILENAMEA);
//	ofn.hwndOwner = NULL;
//	ofn.lpstrFilter = "모든파일(*.*)\0*.*\0";
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

////히스토그램 계산
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
//
////히스토그램,영상 보이기
//Mat ShowHistogram(int select) {
//
//	auto fileName = OpenFileDialog();
//
//	Mat image = imread(fileName, IMREAD_GRAYSCALE);
//
//	//if (image.empty()) {
//	//	cout << "파일 읽기 실패" << endl;
//	//	return NULL;
//	//}
//
//	Mat histogram, histogramImage;
//
//	CalcHistogram(image, histogram, 256);
//	cout << histogram.t() << endl;
//	GetHistogramImage(histogram, histogramImage);
//	imshow("과제 : 대상 영상", image);
//	imshow("과제 : 히스토그램", histogramImage);
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
////명세화 함수
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

	//이미지 파일을 읽어와서 img_input에 저장
	matchingImage = imread("sample128.png", IMREAD_COLOR);
	targetImage = imread("Lenna512.png", IMREAD_COLOR);

	resize(matchingImage, matchingImage, Size(targetImage.rows, targetImage.cols), 0, 0, CV_INTER_LINEAR);//크기변환



	//매칭을 그레이스케일로 변경
	matching_gray = Mat(matchingImage.rows, matchingImage.cols, CV_8UC1);

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			//img_input으로부터 현재 위치 (y,x) 픽셀의
			//blue, green, red 값을 읽어온다. 
			uchar blue = matchingImage.at<Vec3b>(y, x)[0];
			uchar green = matchingImage.at<Vec3b>(y, x)[1];
			uchar red = matchingImage.at<Vec3b>(y, x)[2];

			//blue, green, red를 더한 후, 3으로 나누면 그레이스케일이 된다.
			uchar gray = (blue + green + red) / 3.0;

			//Mat타입 변수 img_gray에 저장한다. 
			matching_gray.at<uchar>(y, x) = gray;
		}
	}

	//타겟을 그레이스케일로 변경
	target_gray = Mat(targetImage.rows, targetImage.cols, CV_8UC1);

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			//img_input으로부터 현재 위치 (y,x) 픽셀의
			//blue, green, red 값을 읽어온다. 
			uchar blue = targetImage.at<Vec3b>(y, x)[0];
			uchar green = targetImage.at<Vec3b>(y, x)[1];
			uchar red = targetImage.at<Vec3b>(y, x)[2];

			//blue, green, red를 더한 후, 3으로 나누면 그레이스케일이 된다.
			uchar gray = (blue + green + red) / 3.0;

			//Mat타입 변수 img_gray에 저장한다. 
			target_gray.at<uchar>(y, x) = gray;
		}
	}

	//매칭 그레이스케일 영상의 히스토그램 계산
	int matching_histogram[256] = { 0, };

	for (int y = 0; y < matchingImage.rows; y++)
	{
		for (int x = 0; x < matchingImage.cols; x++)
		{
			int value = matching_gray.at<uchar>(y, x);
			matching_histogram[value] += 1;
		}
	}

	//타겟 그레이스케일 영상의 히스토그램 계산
	int target_histogram[256] = { 0, };

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			int value = target_gray.at<uchar>(y, x);
			target_histogram[value] += 1;
		}
	}

	//매칭 그레이스케일 영상의 누적 히스토그램 계산
	int mcumulative_histogram[256] = { 0, };
	int sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += matching_histogram[i];
		mcumulative_histogram[i] = sum;
	}

	//타겟 그레이스케일 영상의 누적 히스토그램 계산
	int tcumulative_histogram[256] = { 0, };
	sum = 0;

	for (int i = 1; i < 256; i++)
	{
		sum += target_histogram[i];
		tcumulative_histogram[i] = sum;
	}

	//매칭 그레이스케일 영상의 정규화된 누적 히스토그램 계산
	float mnormalized_cumulative_histogram[256] = { 0.0, };
	int mimage_size = matchingImage.rows * matchingImage.cols;

	for (int i = 0; i < 256; i++)
	{
		mnormalized_cumulative_histogram[i] = mcumulative_histogram[i] / (float)mimage_size;
	}

	//타겟 그레이스케일 영상의 정규화된 누적 히스토그램 계산
	float tnormalized_cumulative_histogram[256] = { 0.0, };
	int timage_size = targetImage.rows * targetImage.cols;

	for (int i = 0; i < 256; i++)
	{
		tnormalized_cumulative_histogram[i] = tcumulative_histogram[i] / (float)timage_size;
	}



	//imshow("매칭 영상", matchingImage);
	//imshow("타겟 영상", targetImage);
	imshow("매칭 그레이스케일 영상", matching_gray);
	imshow("타겟 그레이스케일 영상", target_gray);

	Mat mhistogram, thistogram, mhistogramImage, thistogramImage;
	//매칭
	CalcHistogram(matching_gray, mhistogram, 256);
	GetHistogramImage(mhistogram, mhistogramImage);

	//타겟
	CalcHistogram(target_gray, thistogram, 256);
	GetHistogramImage(thistogram, thistogramImage);

	imshow("매칭 : 히스토그램", mhistogramImage);
	imshow("타겟 : 히스토그램", thistogramImage);



	//매칭 히스토그램 평활화 적용 및 결과 영상의 히스토그램 계산
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

	//매칭 히스토그램 평활화 적용 및 결과 영상의 히스토그램 계산
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


	imshow("매칭 평활화 영상", mimg_result);
	imshow("타겟 평활화 영상", timg_result);



	Mat mhistogramR, thistogramR, mhistogramRImage, thistogramRImage;
	//매칭
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

	imshow("매칭 gggggg 영상", res);

	while (cvWaitKey(0) == 0);

	//





	////결과 영상의 누적 히스토그램 계산
	//int cumulative_histogram2[256] = { 0, };
	//sum = 0;

	//for (int i = 1; i < 256; i++)
	//{
	//	sum += histogram2[i];
	//	cumulative_histogram2[i] = sum;
	//}


	////히스토그램 그리기
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


	////화면에 결과 이미지를 보여준다.
	//imshow("cpu 영상", img_input);
	//imshow("여자 영상", image_output);
	//imshow("입력 그레이스케일 영상", img_gray);
	//imshow("입력 그레이스케일 영상", img_grayg);


	//imshow("입력 그레이스케일 영상", img_gray);
	//imshow("결과 그레이스케일 영상", img_result);
	//imshow("입력 영상의 히스토그램", img_histogram);
	//imshow("평활화 후 히스토그램", img_histogram2);

	////아무키를 누르기 전까지 대기
	//while (cvWaitKey(0) == 0);

	////결과를 파일로 저장
	//imwrite("img_gray.jpg", img_gray);
	//imwrite("img_result.jpg", img_result);
	//imwrite("img_histogram.jpg", img_histogram);
	//imwrite("img_histogram2.jpg", img_histogram2);
}
