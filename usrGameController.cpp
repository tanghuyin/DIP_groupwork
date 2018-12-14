#include "usrGameController.h"

#define PI 3.1415926
#ifdef VIA_OPENCV
//构造与初始化
usrGameController::usrGameController(void* qtCD)
{
	qDebug() << "usrGameController online.";
	device = new deviceCyberDip(qtCD);//设备代理类
	cv::namedWindow(WIN_NAME);
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

bool flag = true;
int i1 = 0;
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
	//cv::imshow(WIN_NAME, pt);
	 
	//our code 
	cv::Mat pt_gray, pt_binary, edge, parts, base, partsinv, partsAndBase;
	int width = pt.cols;
	int height = pt.rows;
	int pixelCount[256] = { 0 };
	int max = 0;
	std::vector<cv::Mat> templ; // 4 is square
	std::vector<std::vector<int>> vectorbase;
	templ.push_back(cv::imread("D:\\DIP\\triangle1.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle2.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle3.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle4.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\square.jpg", 0));
	for (int i = 0; i<templ.size(); i++)
		cv::threshold(templ[i], templ[i], 60, 255, CV_THRESH_BINARY);
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
	/*
	if (flag)
	{
		for (int i = 1; i < contours1.size(); i++)
		{
			double x = 0.8-centers[i].y / pt_gray.rows;
			double y = centers[i].x / pt_gray.cols;
			qDebug() << centerGRAY[i] << " x:" << x << " y:" << y;
			device->comHitUp();
			device->comMoveToScale(x, y);
			_sleep(1000);
			device->comHitDown();
			device->comMoveToScale(0.3, 0.4);
			_sleep(1000);
			savePicture(pt_gray,i);
			device->comHitUp();
		}
	}
	*/
	flag = false;
	cv::bitwise_not(newpt, newpt);
	// std::vector<cv::Vec4i> Lines;
	cv::cvtColor(newpt, newpt, CV_GRAY2BGR);

	pt_binary = pt_binary(cv::Rect(0, 80, width - 180, 550));

	analog2digital(pt_binary, templ, vectorbase);
	for (int i = 0; i < vectorbase.size(); i++)
	{
		for (int j = 0; j < vectorbase[i].size(); j++)
			std::cout << vectorbase[i][j] << " ";
		std::cout << std::endl;
	}
	cv::imshow(WIN_NAME, pt_binary);

	return 0; 
}

void analog2digital(cv::Mat &img, std::vector<cv::Mat> &templ, std::vector<std::vector<int>> &vectorbase)
{
	int gray_scale;
	cv::Mat result(img.rows, img.cols, CV_8U, cv::Scalar(0));
	std::vector<cv::Point> triangle1;
	std::vector<cv::Point> triangle2;
	std::vector<cv::Point> triangle3;
	std::vector<cv::Point> triangle4;
	std::vector<cv::Point> square;
	std::vector<int> vectorbaseline;
	std::vector<std::vector<cv::Point>> LeftUp;
	LeftUp.push_back(triangle1);
	LeftUp.push_back(triangle2);
	LeftUp.push_back(triangle3);
	LeftUp.push_back(triangle4);
	LeftUp.push_back(square);
	std::vector<int> not_zero_row;
	std::vector<int> not_zero_col;
	int smallestx = 9999, smallesty = 9999;
	float maxx = -1, maxy = -1, lastmaxx = 9999, lastmaxy = 9999, needvaluex, needvaluey;
	for (int i = 0; i < templ.size(); i++)
	{
		std::cout << "=====" << std::endl;
		findTemplLeftUp(img, templ[i], LeftUp[i], i);
	}
	for (int i = 0; i < LeftUp.size(); i++)
	{
		for (int j = 0; j < LeftUp[i].size(); j++)
		{
			if (LeftUp[i][j].x < smallestx)
				smallestx = LeftUp[i][j].x;
			if (LeftUp[i][j].y < smallesty)
				smallesty = LeftUp[i][j].y;
		}
	}
	for (int i = 0; i < LeftUp.size(); i++)
	{
		maxx = -1, maxy = -1, lastmaxx = 9999, lastmaxy = 9999;
		needvaluex = 0, needvaluey = 9999;
		for (int j = 0; j < LeftUp[i].size(); j++)
		{
			if (LeftUp[i][j].x > maxx)
				maxx = LeftUp[i][j].x;
			if (LeftUp[i][j].y > maxy)
				maxy = LeftUp[i][j].y;
			if (abs(LeftUp[i][j].x - needvaluex) < 5)
				LeftUp[i][j].x += 20;
			else if ((maxx - lastmaxx) / 50.0 >= 0.8 && ((LeftUp[i][j].x / 10.0 - smallestx / 10.0) / 5.0 - round((LeftUp[i][j].x / 10.0 - smallestx / 10.0) / 5.0)) >= 0.3)
			{
				needvaluex = LeftUp[i][j].x;
				LeftUp[i][j].x += 20;
			}
			if (abs(LeftUp[i][j].y - needvaluey) < 5)
				LeftUp[i][j].y += 20;
			else if ((maxy - lastmaxy) / 50.0 >= 0.8 && ((LeftUp[i][j].y / 10.0 - smallesty / 10.0) / 5.0 - round((LeftUp[i][j].y / 10.0 - smallesty / 10.0) / 5.0)) >= 0.3)
			{
				needvaluey = LeftUp[i][j].y;
				LeftUp[i][j].y += 20;
			}


			LeftUp[i][j].x = round((LeftUp[i][j].x / 10.0 - smallestx / 10.0) / 5.0);
			LeftUp[i][j].y = round((LeftUp[i][j].y / 10.0 - smallesty / 10.0) / 5.0);
			cv::circle(result, LeftUp[i][j], 0.5, cv::Scalar(i + 1), -1);
			lastmaxx = maxx;
			lastmaxy = maxy;
		}
	}
	for (int i = 0; i < result.rows; i++)
	{
		for (int j = 0; j < result.cols; j++)
		{
			gray_scale = result.ptr<uchar>(i)[j];
			if (gray_scale != 0)
			{
				not_zero_row.push_back(i);
				break;
			}
		}
	}
	for (int j = 0; j < result.cols; j++)
	{
		for (int i = 0; i < result.rows; i++)
		{
			gray_scale = result.ptr<uchar>(i)[j];
			if (gray_scale != 0)
			{
				not_zero_col.push_back(j);
				break;
			}
		}
	}
	for (int i = 0; i < not_zero_row.size(); i++)
	{
		for (int j = 0; j < not_zero_col.size(); j++)
		{
			gray_scale = result.ptr<uchar>(not_zero_row[i])[not_zero_col[j]];
			// std::cout << gray_scale << " ";
			vectorbaseline.push_back(gray_scale);
		}
		vectorbase.push_back(vectorbaseline);
		vectorbaseline.clear();
		// std::cout << std::endl;
	}
}

void findTemplLeftUp(cv::Mat &img, cv::Mat &templ, std::vector<cv::Point> &Pointlist, int type)
{
	cv::Mat out1;
	double matchValue;
	cv::matchTemplate(img, templ, out1, CV_TM_SQDIFF_NORMED);
	// cv::normalize(out1, out1, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	bool flag_new = true;
	for (int i = 0; i < out1.cols; i++)
	{
		for (int j = 0; j < out1.rows; j++)
		{
			matchValue = out1.at<float>(j, i);
			if (matchValue < 0.4 && type != 4)
			{
				flag_new = true;
				for (int k = 0; k < Pointlist.size(); k++)
				{
					if (abs(Pointlist[k].x - i) <= 20 && abs(Pointlist[k].y - j) <= 20)
						flag_new = false;
				}
				if (flag_new)
					Pointlist.push_back(cv::Point(int(i), int(j)));
			}
			if (matchValue < 0.3 && type == 4)
			{
				flag_new = true;
				for (int k = 0; k < Pointlist.size(); k++)
				{
					if (abs(Pointlist[k].x - i) <= 10 && abs(Pointlist[k].y - j) <= 10)
						flag_new = false;
				}
				if (flag_new)
					Pointlist.push_back(cv::Point(int(i), int(j)));
			}

		}
	}
	for (int i = 0; i < Pointlist.size(); i++)
	{
		// std::cout << "x: " << Pointlist[i].x << " " << "y: " << Pointlist[i].y << std::endl;
		// Pointlist[i].x = floor(Pointlist[i].x / 45);
		// Pointlist[i].y = floor(Pointlist[i].y / 45);
		// std::cout << "x: " << Pointlist[i].x << " " << "y: " << Pointlist[i].y << std::endl;

	}

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


void savePicture(cv::Mat img, int i)
{
	switch (i)
	{
	case  1:cv::imwrite("contours1.jpg", img); break;
	case  2:cv::imwrite("contours2.jpg", img); break;
	case  3:cv::imwrite("contours3.jpg", img); break;
	case  4:cv::imwrite("contours4.jpg", img); break;
	case  5:cv::imwrite("contours5.jpg", img); break;
	case  6:cv::imwrite("contours6.jpg", img); break;
	case  0:cv::imwrite("base.jpg", img); break;
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