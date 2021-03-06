#ifdef VIA_OPENCV

#ifndef USRGAMECONTROLLER_H
#define USRGAMECONTROLLER_H

#include "qtcyberdip.h"

#define WIN_NAME "Frame"

//游戏控制类
class usrGameController
{
private:
	deviceCyberDip* device;
//以下是为了实现演示效果，增加的内容
	//鼠标回调结构体
	struct MouseArgs{
		cv::Rect box;
		bool Drawing, Hit;
		// init
		MouseArgs() :Drawing(false), Hit(false)
		{
			box = cv::Rect(0, 0, -1, -1);
		}
	};
	//鼠标回调函数
	friend void  mouseCallback(int event, int x, int y, int flags, void*param);
	MouseArgs argM;
//以上是为了实现课堂演示效果，增加的内容
public:
	//构造函数，所有变量的初始化都应在此完成
	usrGameController(void* qtCD);
	//析构函数，回收本类所有资源
	~usrGameController();
	//处理图像函数，每次收到图像时都会调用
	int usrProcessImage(cv::Mat& img);
};


//以下是为了实现演示效果，增加的内容
//鼠标回调函数
void  mouseCallback(int event, int x, int y, int flags, void*param);
//以上是为了实现课堂演示效果，增加的内容
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
void getSolution(int k);
bool judge(int x, int y, int k);
//bool judgeFinish();
void removeFromBase(int x, int y, int k);
void solveProblem(std::vector<std::vector<int>> &vectorbase);

#endif
#endif