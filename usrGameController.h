#ifdef VIA_OPENCV

#ifndef USRGAMECONTROLLER_H
#define USRGAMECONTROLLER_H

#include "qtcyberdip.h"

#define WIN_NAME "Frame"

//��Ϸ������
class usrGameController
{
private:
	deviceCyberDip* device;
//������Ϊ��ʵ����ʾЧ�������ӵ�����
	//���ص��ṹ��
	struct MouseArgs{
		cv::Rect box;
		bool Drawing, Hit;
		// init
		MouseArgs() :Drawing(false), Hit(false)
		{
			box = cv::Rect(0, 0, -1, -1);
		}
	};
	//���ص�����
	friend void  mouseCallback(int event, int x, int y, int flags, void*param);
	MouseArgs argM;
//������Ϊ��ʵ�ֿ�����ʾЧ�������ӵ�����
public:
	//���캯�������б����ĳ�ʼ����Ӧ�ڴ����
	usrGameController(void* qtCD);
	//�������������ձ���������Դ
	~usrGameController();
	//����ͼ������ÿ���յ�ͼ��ʱ�������
	int usrProcessImage(cv::Mat& img);
};


//������Ϊ��ʵ����ʾЧ�������ӵ�����
//���ص�����
void  mouseCallback(int event, int x, int y, int flags, void*param);
//������Ϊ��ʵ�ֿ�����ʾЧ�������ӵ�����
void getTheHist(cv::Mat *img, cv::Mat &dstImage);
void moveSkewLines(std::vector<cv::Vec4i> &Lines);
void addLines2Origin(cv::Mat &origin, std::vector<cv::Vec4i> &Lines);
int get_max(int array1[], int n);
void findCenterAndRGB(cv::Mat img, std::vector<std::vector<cv::Point>> &contours, std::vector<cv::Point2f> &centers, std::vector<int> &RGB);
void savePicture(cv::Mat img, int i);
void analog2digital(cv::Mat &img, std::vector<cv::Mat> &templ, std::vector<std::vector<int>> &vectorbase, std::vector<std::vector<cv::Point>> &vectorabsolute, bool isBase);
void chubbyVector(std::vector<std::vector<cv::Point>> &vectorabsolute);
void findTemplLeftUp(cv::Mat &img, cv::Mat &templ, std::vector<cv::Point> &Pointlist, int type, bool isBase);
void removeOtherGrayScale(cv::Mat &gray_pt, int target_gray_scale);

#endif
#endif