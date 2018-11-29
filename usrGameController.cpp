#include "usrGameController.h"
#define PI 3.1415926
#ifdef VIA_OPENCV
//构造与初始化
usrGameController::usrGameController(void* qtCD)
{
	qDebug() << "usrGameController online.";
	device = new deviceCyberDip(qtCD);//设备代理类
	cv::namedWindow(WIN_NAME);
	//cv::setMouseCallback(WIN_NAME, mouseCallback, (void*)&(argM));
}

//析构
usrGameController::~usrGameController()
{
	cv::destroyAllWindows();
	if (device != nullptr)
	{
		delete device;
	}
	qDebug() << "usrGameController offline.";
}

//处理图像 
int usrGameController::usrProcessImage(cv::Mat& img)
{
	cv::Size imgSize(img.cols, img.rows - UP_CUT);
	if (imgSize.height <= 0 || imgSize.width <= 0)
	{
		qDebug() << "Invalid image. Size:" << imgSize.width <<"x"<<imgSize.height;
		return -1;
	}

	//截取图像边缘
	cv::Mat pt = img(cv::Rect(0, UP_CUT, imgSize.width,imgSize.height));
	//our code 
	cv::Mat pt_gray, pt_binary, edge, parts, base, partsinv, partsAndBase;
	int width = pt.cols;
	int height = pt.rows;
	int pixelCount[256] = { 0 };
	int max = 0;
	//parts表示拼图块 base表示拼图的底盘
	cv::cvtColor(pt, pt_gray, CV_BGR2GRAY);// transform RGB into GRAY SCALE 
	//cv::GaussianBlur(pt_gray, pt_gray, cv::Size(5, 5), 0);
	cv::threshold(pt_gray, pt_binary, 60, 255, CV_THRESH_BINARY); // move puzzles only leave the puzzle base(including lines)
	cv::threshold(pt_gray, partsAndBase, 240, 255, CV_THRESH_BINARY); // leave puzzles and puzzle base
	cv::threshold(pt_gray, base, 150, 255, CV_THRESH_BINARY); // leave base without lines

	bitwise_not(partsAndBase, partsAndBase);
	bitwise_not(base, base);
	parts = partsAndBase - base;
	parts = parts(cv::Rect(0, 450, width, 250));
	bitwise_not(parts, partsinv);
	bitwise_not(pt_binary, pt_binary);


	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point>> contours1;
	std::vector<cv::Vec4i> hierarchy1;
	

	// base contours
	cv::findContours(pt_binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	cv::Mat newpt(pt_binary.rows, pt_binary.cols, CV_8U, cv::Scalar(0));
	for (int i = 0; i < contours.size(); i++)
	{
		if (hierarchy[i][3] != -1)
			cv::drawContours(pt_binary, contours, i, cv::Scalar(0, 255, 0), 2, 8);
	}
	
	/*
	cv::Mat element(8, 8, CV_8U, cv::Scalar(1));
	cv::GaussianBlur(partsinv, partsinv, cv::Size(3, 3), 0);
	cv::morphologyEx(partsinv, partsinv, cv::MORPH_CLOSE, element);
	cv::morphologyEx(partsinv, partsinv, cv::MORPH_OPEN, element);
	cv::Canny(partsinv, edge, 5, 10, 3);
	bitwise_not(edge, edge);
	*/

	// find the parts contour
	cv::findContours(partsinv, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_TC89_KCOS, cv::Point(0, 0));
	for (int i = 1; i < contours1.size(); i++)
	{	
		if (hierarchy1[i][3] != -1)
			cv::drawContours(newpt, contours1, i, cv::Scalar(255), 1, 8);
		
	}

	std::vector<cv::Point2f> centers(contours1.size());
	std::vector<int> centerGRAY(contours1.size());
	findCenterAndRGB(pt_gray, contours1, centers, centerGRAY);
	/*test
	for (int i = 0; i < contours1.size(); i++)
	{
		cv::circle(pt_gray, centers[i], 2, cv::Scalar(0, 255, 0));
		qDebug() << centerGRAY[i];

	}
	*/
	cv::bitwise_not(newpt, newpt);

	

	std::vector<cv::Vec4i> Lines;
	//cv::HoughLinesP(newpt, Lines, 1, PI / 4, 2, 0, 5); // hough transform
	cv::cvtColor(newpt, newpt, CV_GRAY2BGR);
	//moveSkewLines(Lines);
	//only for test need, can be deleted when finished
	//addLines2Origin(newpt, Lines);
	//
	cv::imshow(WIN_NAME, pt_gray);
	//cv::Mat histGraph(256, 256, CV_8U, cv::Scalar(0)); only use for one time
	//cv::blur(pt_gray, pt_gray, cv::Size(3, 3));
	//getTheHist(&pt_gray, histGraph);
	//cv::imshow(WIN_NAME, histGraph);
	//cv::Mat pt_threshold;
	//cv::threshold(pt_gray, pt_threshold, 80, 255, CV_THRESH_BINARY);
	//cv::imshow(WIN_NAME, pt_threshold);

	//cv::imshow(WIN_NAME, edge);
	return 0; 
}

void findCenterAndRGB(cv::Mat img, std::vector<std::vector<cv::Point>> &contours, std::vector<cv::Point2f> &centers, std::vector<int> &RGB)
{
	std::vector<cv::Moments> mu(contours.size());
	int grayscale;
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = cv::moments(contours[i], true);
	}
	
	for (int i = 0; i < contours.size(); i++)
	{
		centers[i] = cv::Point2f(round(mu[i].m10 / mu[i].m00), round(mu[i].m01 / mu[i].m00)); // get the center of each parts
	}

	for (int i = 0; i < contours.size(); i++)
	{
		centers[i].y += 450; // get the absolute coordinates before cutting the screen
	}
	for (int i = 0; i < contours.size(); i++)
	{
		RGB[i] = img.ptr<uchar>(int(centers[i].y))[int(centers[i].x)];
	}
	return;
}

int get_max(int array1[], int n)
{
	int max = array1[0];
	int index = 0;
	for (int i=0;i<n;i++)
		if (max < array1[i])
		{
			max = array1[i];
			index = i;
		}
	return index;
}
void addLines2Origin(cv::Mat &origin, std::vector<cv::Vec4i> &Lines)
{
	qDebug() << Lines.size();
	for (size_t i = 0; i < Lines.size(); i++)
	{
		cv::line(origin, cv::Point(Lines[i][0], Lines[i][1]), cv::Point(Lines[i][2], Lines[i][3]), cv::Scalar(0, 255, 0), 1, 8);
	}
}

void moveSkewLines(std::vector<cv::Vec4i> &Lines)
{
	for (size_t i = 0; i < Lines.size();)
	{
		float angle = atan2(Lines[i][3] - Lines[i][1], Lines[i][2] - Lines[i][0]);

		if ((angle > -1.57 || angle < -1.58) && (angle < -0.01 || angle > 0.01)) // delete skew lines
		{
			Lines.erase(Lines.begin() + i);
			qDebug() << Lines.size();
		}
		else
			i++;
	}
}
void getTheHist(cv::Mat *img, cv::Mat &dstImage)
{
	const int nimages = 1;
	int channels[] = { 0 };
	cv::MatND hist;
	int dims = 1;
	int histSize[] = { 256 };
	float hranges[] = { 0.0, 255.0 };
	const float* ranges[] = { hranges };
	cv::calcHist(img, nimages, channels, cv::Mat(), hist, dims, histSize, ranges);
	double minValue = 0;
	double maxValue = 0;
	minMaxLoc(hist, &minValue, &maxValue);
	int hpt = cv::saturate_cast<int>(0.9 * histSize[0]);
	for (int i = 0; i < 256; i++)
	{
		float binValue = hist.at<float>(i);
		int realValue = cv::saturate_cast<int>(binValue * hpt / maxValue);
		rectangle(dstImage, cv::Point(i, histSize[0] - 1), cv::Point(i + 1, histSize[0] - realValue), cv::Scalar(255));
	}
}

//鼠标回调函数
void mouseCallback(int event, int x, int y, int flags, void*param)
{
	usrGameController::MouseArgs* m_arg = (usrGameController::MouseArgs*)param;
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE: // 鼠标移动时
	{
		if (m_arg->Drawing)
		{
			m_arg->box.width = x - m_arg->box.x;
			m_arg->box.height = y - m_arg->box.y;
		}
	}
	break;
	case CV_EVENT_LBUTTONDOWN:case CV_EVENT_RBUTTONDOWN: // 左/右键按下
	{
		m_arg->Hit = event == CV_EVENT_RBUTTONDOWN;
		m_arg->Drawing = true;
		m_arg->box = cvRect(x, y, 0, 0);
	}
	break;
	case CV_EVENT_LBUTTONUP:case CV_EVENT_RBUTTONUP: // 左/右键弹起
	{
		m_arg->Hit = false;
		m_arg->Drawing = false;
		if (m_arg->box.width < 0)
		{
			m_arg->box.x += m_arg->box.width;
			m_arg->box.width *= -1;
		}
		if (m_arg->box.height < 0)
		{
			m_arg->box.y += m_arg->box.height;
			m_arg->box.height *= -1;
		}
	}
	break;
	}
}
#endif