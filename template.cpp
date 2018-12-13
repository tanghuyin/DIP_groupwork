#include <opencv2/opencv.hpp>
#include <iostream>
#define PI 3.1415926
cv::Point2i findLeftWhite(cv::Mat &img);
int comparePic(cv::Mat img, cv::Mat &target, int x, int y);
int main()
{
	cv::Mat out1, out2, out3;
	cv::Point2i Left_Point;
	cv::Mat img = cv::imread("D:\\DIP\\test.png", 0);
	cv::Mat triangle = cv::imread("D:\\DIP\\triangle1.jpg", 0);
	cv::threshold(img, img, 60, 255, CV_THRESH_BINARY);
	cv::threshold(triangle, triangle, 60, 255, CV_THRESH_BINARY);
	int width = img.cols;
	int height = img.rows;
	double matchValue;
	int result_cols = img.cols - triangle.cols + 1;
	int result_rows = img.rows - triangle.rows + 1;
	out1.create(result_cols, result_rows, CV_32FC1);
	cv::matchTemplate(img, triangle, out1, CV_TM_SQDIFF_NORMED);
	cv::normalize(out1, out1, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	double minVal = -1;
	double maxVal;
	cv::Point minLoc;
	cv::Point maxLoc;
	cv::Point matchLoc;
	cv::minMaxLoc(out1, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	std::cout << minVal;
	matchLoc = minLoc;
	for (int i = 0; i < out1.cols; i++)
	{
		for (int j = 0; j < out1.rows; j++)
		{
			matchValue = out1.at<float>(j, i);
			std::cout << matchValue << std::endl;
		}
	}
	rectangle(img, matchLoc, cv::Point(matchLoc.x + triangle.cols, matchLoc.y + triangle.rows), cv::Scalar(255), 2, 8, 0);
	cv::imshow("img", out1);
	//Left_Point = findLeftWhite(img);
	//std::cout << triangle.rows << triangle.cols;
	//std::cout << img.rows << img.cols;
	cv::waitKey(0);
	return 0;
}
int comparePic(cv::Mat img, cv::Mat &target, int x, int y)
{
	img = img(cv::Rect(x, y, target.cols, target.rows));
	int error = 0;
	for (int i = 0; i < img.cols; i++)
	{
		for (int j = 0; j < img.rows; j++)
		{
			error += abs(img.ptr<uchar>(j)[i] - target.ptr<uchar>(j)[i]);
		}
	}
	if(error < 10000)
		std::cout << error << std::endl;
	return 0;
}

cv::Point2i findLeftWhite(cv::Mat &img)
{
	cv::Point2i Left_Point(0, 0);
	int width = img.cols;
	int height = img.rows;
	int gray_scale;
	int i, j;
	bool last_allzero_row, allzero_row = true;
	bool last_allzero_col, allzero_col = true;
	std::vector<int> not_zero_row;
	std::vector<int> not_zero_col;
	//std::cout << width << " " << height;

	for (i = 0; i < height; i++)
	{
		last_allzero_row = allzero_row;
		allzero_row = true;
		for (j = 0; j < width; j++)
		{
			gray_scale = img.ptr<uchar>(i)[j];
			if (gray_scale != 0)
			{
				allzero_row = false;
				break;
			}
		}
		if (last_allzero_row && !allzero_row)
			not_zero_row.push_back(i);
	}

	for (i = 0; i < width; i++)
	{
		last_allzero_col = allzero_col;
		allzero_col = true;
		for (j = 0; j < height; j++)
		{
			gray_scale = img.ptr<uchar>(j)[i];
			if (gray_scale != 0)
			{
				allzero_col = false;
				break;
			}
		}
		if (last_allzero_col && !allzero_col)
			not_zero_col.push_back(i);
	}

	for (i = 0; i < not_zero_col.size(); i++)
	{
		for (j = 0; j < not_zero_row.size(); j++)
		{
			gray_scale = img.ptr<uchar>(not_zero_row[j])[not_zero_col[i]];
			if (gray_scale == 255)
			{
				std::cout << not_zero_col[i] << " " << not_zero_row[j] << std::endl;
				cv::circle(img, cv::Point(not_zero_col[i], not_zero_row[j]), 2, cv::Scalar(255));
			}
				
		}
	}
	Left_Point.x = j;
	Left_Point.y = i;
	return Left_Point;
}
