#include <opencv2\opencv.hpp>

#include <iostream>

void removeOtherGrayScale(cv::Mat &gray_pt, int target_gray_scale);
int main()
{
	cv::Mat pt_gray;
	cv::Mat image = cv::imread("thy.jpg");  //存放自己图像的路径 
	cv::cvtColor(image, pt_gray, CV_BGR2GRAY);
	removeOtherGrayScale(pt_gray, 231);
	imshow("显示图像", pt_gray);
	cv::waitKey(0);
	return 0;
}
void removeOtherGrayScale(cv::Mat &gray_pt, int target_gray_scale)
{
	/*
	遍历所有像素点，只保留指定灰度值的像素点，其余灰度值的像素点统一变黑或白，cv:threshold库函数好像也可以设定上下阈值，可以查查，如果可以用就很方便了
	可以新建一个空的opencv项目，这样测试起来快一点
	Love you!(｡ì _ í｡)
	*/
	int gray_scale;
	for (int i = 0; i<gray_pt.rows; i++)
		for (int j = 0; j < gray_pt.cols; j++)
		{
			gray_scale = gray_pt.ptr<uchar>(i)[j];
			if (gray_scale != target_gray_scale)
				gray_pt.ptr<uchar>(i)[j] = 0;
			else
				gray_pt.ptr<uchar>(i)[j] = 255;
		}
	return;
}
