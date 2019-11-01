#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


int sub_image(IplImage*img1, IplImage*img2, IplImage*dst);



void main()

{


	IplImage * image;

	CvCapture* capture;

	capture = cvCaptureFromCAM(1);


	image = cvQueryFrame(capture);

	int cnt = 0;

	char name[10];

	char buffer[30];


	IplImage * temp = cvCreateImage(cvGetSize(image), 8, 3);

	IplImage * dst = cvCreateImage(cvGetSize(image), 8, 3);



	while (1)

	{

		image = cvQueryFrame(capture);

		//CvCapture 로부터 프레임을 얻어 image 라는 IplImage객체에 넣습니다.


		if (sub_image(image, temp, dst) == 1) //sub_image를 호출하여 리턴값이 1이면 실행

		{


			cnt++; //매 프레임마다 카운트하여 이 카운트를 파일명에 넣습니다.

			itoa(cnt, name, 10);//저장되는사진이름은1,2,3,식으로카운트번호

			sprintf(buffer, "C:\\cctv\\%s.jpg", name);

			cvSaveImage(buffer, image); //지정된 파일명으로 저장합니다.

		}



		cvCopy(image, temp); //if문에 걸리지 않더라도 매 프레임을 temp에 저장합니다.

							 //while문이 다시 돌때 새로들어온 프레임인 image와 이전프레임인 temp와의 차를

							 //구하게 하려는 의도입니다.

		cvShowImage("show", image);

		cvShowImage("cha", dst);



		if (cvWaitKey(10) == 'p')

			break;



	}

	cvReleaseCapture(&capture); //메모리해제

	cvReleaseImage(&image);

	cvReleaseImage(&dst);

	cvReleaseImage(&temp);

	cvDestroyAllWindows();


}



int sub_image(IplImage*img1, IplImage*img2, IplImage*dst) //차영상을 구하는 소스입니다

														  // RGB 기반으로 단순히 이전프레임과 현재프레임의 각 픽셀의 R,G,B 각각이 40이상씩 모두 차이나면

														  //화면에 변화가 생겼다고 가정(기준을 정하여) 조건에 맞으면 return 1을 합니다.

{

	int value1, value2, value3;

	int cnt = 0;


	// 한프레임내의 픽셀을 도는 2중 for문

	for (int x = 0; x<img1->height; x++) //이미지차연산

	{

		for (int y = 0; y <img1->width; y++)

		{

			value1 = abs((img1->imageData[x*img1->widthStep + 3 * y]) - (img2->imageData[x*img2->widthStep + 3 * y]));

			value2 = abs((img1->imageData[x*img1->widthStep + 3 * y + 1]) - (img2->imageData[x*img2->widthStep + 3 * y] + 1));


			value3 = abs((img1->imageData[x*img1->widthStep + 3 * y + 2]) - (img2->imageData[x*img2->widthStep + 3 * y] + 2));




			//한 프레임내에서 R,G,B모두 40이상의 색상차이를 보이면 이 수를 카운트해서(cnt++)

			//한프레임 내에서 총 몇개의 픽셀이 변화를 일으켰는지 셉니다.

			if (value1>40 && value2>40 && value3>40) // RGB 가모두40이상차이날경우

			{

				dst->imageData[x*dst->widthStep + 3 * y + 0] = img1->imageData[x*img1->widthStep + 3 * y + 0];

				dst->imageData[x*dst->widthStep + 3 * y + 1] = img1->imageData[x*img1->widthStep + 3 * y + 1];

				dst->imageData[x*dst->widthStep + 3 * y + 2] = img1->imageData[x*img1->widthStep + 3 * y + 2];

				cnt++; //카운트함

			}

			//그 외에 40이상 차이나지 않는 픽셀의 경우엔 0으로 채워서 검은색으로 만들어

			//화면에 보여줄때 차이를 분명히 하기위해서입니다. 단순히 보는사람입장에서 생각하는거라 파일로 저장화하는데 관련이 없으므로 지워도 무방합니다.

			else

			{

				dst->imageData[x*dst->widthStep + 3 * y + 0] = 0;

				dst->imageData[x*dst->widthStep + 3 * y + 1] = 0;

				dst->imageData[x*dst->widthStep + 3 * y + 2] = 0;

			}

		}

	}



	//한 프레임을 2중for문으로 싹 돌고나서 누적된 카운트수를 세서 300개 이상(임의의 기준)

	// 일 경우 이 장면에 뭔가 새로운게 들어왔다고 가정하고 return 1을 합니다.

	if (cnt>300) //영상1프레임내에서변화가있는픽셀이300개이상일경우리턴1

		return 1;




	//그외에는 0을 리턴합니다.

	return 0;


}
