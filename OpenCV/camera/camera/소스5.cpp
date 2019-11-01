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

		//CvCapture �κ��� �������� ��� image ��� IplImage��ü�� �ֽ��ϴ�.


		if (sub_image(image, temp, dst) == 1) //sub_image�� ȣ���Ͽ� ���ϰ��� 1�̸� ����

		{


			cnt++; //�� �����Ӹ��� ī��Ʈ�Ͽ� �� ī��Ʈ�� ���ϸ� �ֽ��ϴ�.

			itoa(cnt, name, 10);//����Ǵ»����̸���1,2,3,������ī��Ʈ��ȣ

			sprintf(buffer, "C:\\cctv\\%s.jpg", name);

			cvSaveImage(buffer, image); //������ ���ϸ����� �����մϴ�.

		}



		cvCopy(image, temp); //if���� �ɸ��� �ʴ��� �� �������� temp�� �����մϴ�.

							 //while���� �ٽ� ���� ���ε��� �������� image�� ������������ temp���� ����

							 //���ϰ� �Ϸ��� �ǵ��Դϴ�.

		cvShowImage("show", image);

		cvShowImage("cha", dst);



		if (cvWaitKey(10) == 'p')

			break;



	}

	cvReleaseCapture(&capture); //�޸�����

	cvReleaseImage(&image);

	cvReleaseImage(&dst);

	cvReleaseImage(&temp);

	cvDestroyAllWindows();


}



int sub_image(IplImage*img1, IplImage*img2, IplImage*dst) //�������� ���ϴ� �ҽ��Դϴ�

														  // RGB ������� �ܼ��� ���������Ӱ� ������������ �� �ȼ��� R,G,B ������ 40�̻� ��� ���̳���

														  //ȭ�鿡 ��ȭ�� ����ٰ� ����(������ ���Ͽ�) ���ǿ� ������ return 1�� �մϴ�.

{

	int value1, value2, value3;

	int cnt = 0;


	// �������ӳ��� �ȼ��� ���� 2�� for��

	for (int x = 0; x<img1->height; x++) //�̹���������

	{

		for (int y = 0; y <img1->width; y++)

		{

			value1 = abs((img1->imageData[x*img1->widthStep + 3 * y]) - (img2->imageData[x*img2->widthStep + 3 * y]));

			value2 = abs((img1->imageData[x*img1->widthStep + 3 * y + 1]) - (img2->imageData[x*img2->widthStep + 3 * y] + 1));


			value3 = abs((img1->imageData[x*img1->widthStep + 3 * y + 2]) - (img2->imageData[x*img2->widthStep + 3 * y] + 2));




			//�� �����ӳ����� R,G,B��� 40�̻��� �������̸� ���̸� �� ���� ī��Ʈ�ؼ�(cnt++)

			//�������� ������ �� ��� �ȼ��� ��ȭ�� �����״��� ���ϴ�.

			if (value1>40 && value2>40 && value3>40) // RGB �����40�̻����̳����

			{

				dst->imageData[x*dst->widthStep + 3 * y + 0] = img1->imageData[x*img1->widthStep + 3 * y + 0];

				dst->imageData[x*dst->widthStep + 3 * y + 1] = img1->imageData[x*img1->widthStep + 3 * y + 1];

				dst->imageData[x*dst->widthStep + 3 * y + 2] = img1->imageData[x*img1->widthStep + 3 * y + 2];

				cnt++; //ī��Ʈ��

			}

			//�� �ܿ� 40�̻� ���̳��� �ʴ� �ȼ��� ��쿣 0���� ä���� ���������� �����

			//ȭ�鿡 �����ٶ� ���̸� �и��� �ϱ����ؼ��Դϴ�. �ܼ��� ���»�����忡�� �����ϴ°Ŷ� ���Ϸ� ����ȭ�ϴµ� ������ �����Ƿ� ������ �����մϴ�.

			else

			{

				dst->imageData[x*dst->widthStep + 3 * y + 0] = 0;

				dst->imageData[x*dst->widthStep + 3 * y + 1] = 0;

				dst->imageData[x*dst->widthStep + 3 * y + 2] = 0;

			}

		}

	}



	//�� �������� 2��for������ �� ������ ������ ī��Ʈ���� ���� 300�� �̻�(������ ����)

	// �� ��� �� ��鿡 ���� ���ο�� ���Դٰ� �����ϰ� return 1�� �մϴ�.

	if (cnt>300) //����1�����ӳ�������ȭ���ִ��ȼ���300���̻��ϰ�츮��1

		return 1;




	//�׿ܿ��� 0�� �����մϴ�.

	return 0;


}
