#include <opencv2/opencv.hpp>

#include <time.h>

#include <string.h>





using namespace std;

using namespace cv;





#define TEN_SEC 242

#define FONTFACE FONT_HERSHEY_COMPLEX_SMALL

#define DELAY 30

#define OUTPUTTIME "%Y.%m.%d.%H.%M.%S"

#define VIDEOFOEMAT ".mp4v"

#define EXTEN 'M', 'P', 'E', 'G'

#define nTh 50



const string currentDateTime() {

	time_t     now = time(NULL);

	struct tm  t;

	char       buf[80];

	localtime_s(&t, &now);

	strftime(buf, sizeof(buf), OUTPUTTIME, &t);



	return buf;

}



int main() {

	int record = false;

	VideoCapture capture(1);

	if (!capture.isOpened()) {

		cout << "Can not open capture!!" << endl;

		return 0;

	}



	Size size = Size((int)capture.get(CV_CAP_PROP_FRAME_WIDTH), (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));

	cout << "Size = " << size << endl;

	int fps = (int)(capture.get(CAP_PROP_FPS));

	cout << "fps = " << fps << endl;



	imshow("frame", NULL);

	waitKey(100);



	float alpha = 0.02;

	int frameNum = -1;

	Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5)); //MORPH_ELLIPSE

	Mat frame, grayImage, grayImage2;

	Mat avgImage, diffImage, mask;

	Mat last_frame, moveImage;



	namedWindow("frame", WINDOW_AUTOSIZE);



	int timecnt = 0;

	int fourcc = VideoWriter::fourcc(EXTEN);

	VideoWriter outputVideo;



	while (1) {

		string text = currentDateTime();

		capture >> frame;

		capture >> last_frame;

		if (frame.empty()) break;



		cvtColor(frame, grayImage, COLOR_BGR2GRAY);

		cvtColor(last_frame, grayImage2, COLOR_BGR2GRAY);

		GaussianBlur(grayImage, grayImage, Size(5, 5), 0.5);

		avgImage = grayImage2;

		absdiff(grayImage, avgImage, diffImage);

		threshold(diffImage, mask, nTh, 255, THRESH_BINARY);



		diffImage.setTo(0);

		int movecnt = countNonZero(mask);

		imshow("mask", mask);

		putText(frame, text, Point(20, capture.get(CV_CAP_PROP_FRAME_HEIGHT) - 20), FONTFACE, 1, Scalar(255, 255, 255));



		if (movecnt >= 5000 && record == false) {

			record = !record;

			//cout << movecnt << endl;

			outputVideo.open(text + VIDEOFOEMAT, fourcc, 24, size, true);

			cout << text + " start record\n";

			//video.write(mask);

		}



		if (record) {

			timecnt++;

			if (timecnt == TEN_SEC) {

				record = !record;

				timecnt = 0;

				outputVideo.release();

				cout << text + " finish record\n";

			}

			outputVideo << frame;

		}



		imshow("frame", frame);

		if (waitKey(DELAY) == 27) break;

	}

	return 0;
}