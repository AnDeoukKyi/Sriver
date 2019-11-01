#include <opencv2/opencv.hpp>
#include <iostream>  

using namespace cv;
using namespace std;



int main()
{
	int index = 1;
	char buf[256];
	Mat frameMat;


	VideoCapture videoCapture(1);
	if (!videoCapture.isOpened())
	{
		printf("ù��° ī�޶� ���� �����ϴ�. \n");
	}


	namedWindow("��ķ ����", WINDOW_AUTOSIZE);


	while (1)
	{

		//��ĸ���κ��� �� �������� �о��  
		videoCapture.read(frameMat);

		//ȭ�鿡 ������
		imshow("��ķ ����", frameMat);

		//�̹����� ����  
		sprintf(buf, "c:/ejpicture/img_%03d.jpg", index);
		cout << buf << endl;
		imwrite(buf, frameMat);

		index++;
		if (index == 11)
			return;

		if (waitKey(500) == 27) break; //ESCŰ ������ ����  
	}


	return 0;
}