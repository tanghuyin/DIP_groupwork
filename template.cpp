#include <opencv2/opencv.hpp>
#include <iostream>
#define PI 3.1415926
cv::Point2i findLeftWhite(cv::Mat &img);
int comparePic(cv::Mat img, cv::Mat &target, int x, int y);
void findTemplLeftUp(cv::Mat &img, cv::Mat &templ, std::vector<cv::Point> &Pointlist);
void analog2digital(cv::Mat &img, std::vector<cv::Mat> &templ, std::vector<std::vector<int>> &vectorbase);
int main()
{	
	std::vector<cv::Mat> templ; // 4 is square
	std::vector<std::vector<int>> vectorbase;

	cv::Mat img = cv::imread("D:\\DIP\\test2.jpg", 0);
	templ.push_back(cv::imread("D:\\DIP\\triangle1.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle2.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle3.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle4.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\square.jpg", 0));
	cv::threshold(img, img, 60, 255, CV_THRESH_BINARY);
	for (int i=0;i<templ.size();i++)
		cv::threshold(templ[i], templ[i], 60, 255, CV_THRESH_BINARY);
	analog2digital(img, templ, vectorbase);
	for (int i = 0; i < vectorbase.size(); i++)
	{
		for (int j = 0; j < vectorbase[i].size(); j++)
			std::cout << vectorbase[i][j] << " ";
		std::cout << std::endl;
	}
		
	cv::imshow("img", img);
	//Left_Point = findLeftWhite(img);
	//std::cout << triangle.rows << triangle.cols;
	//std::cout << img.rows << img.cols;
	cv::waitKey(0);
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
	for (int i = 0; i < templ.size(); i++)
	{
		findTemplLeftUp(img, templ[i], LeftUp[i]);
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
		for (int j = 0; j < LeftUp[i].size(); j++)
		{
			LeftUp[i][j].x = round((LeftUp[i][j].x - smallestx) / 50.0);
			LeftUp[i][j].y = round((LeftUp[i][j].y - smallesty) / 50.0);
			cv::circle(result, LeftUp[i][j], 0.5, cv::Scalar(i + 1), -1);
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

void findTemplLeftUp(cv::Mat &img, cv::Mat &templ, std::vector<cv::Point> &Pointlist)
{
	cv::Mat out1;
	double matchValue;
	cv::matchTemplate(img, templ, out1, CV_TM_SQDIFF_NORMED);
	cv::normalize(out1, out1, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	bool flag_new = true;
	for (int i = 0; i < out1.cols; i++)
	{
		for (int j = 0; j < out1.rows; j++)
		{
			matchValue = out1.at<float>(j, i);
			if (matchValue < 0.1)
			{
				flag_new = true;
				for (int k = 0; k < Pointlist.size(); k++)
				{
					if (abs(Pointlist[k].x - i) <= 5 && abs(Pointlist[k].y - j) <= 5)
						flag_new = false;
				}
				if (flag_new)
					Pointlist.push_back(cv::Point(int(i), int(j)));
			}

		}
	}
	/*
	for (int i = 0; i < Pointlist.size(); i++)
	{
	std::cout << "x: " << Pointlist[i].x << " " << "y: " << Pointlist[i].y << std::endl;
	Pointlist[i].x = floor(Pointlist[i].x / 45);
	Pointlist[i].y = floor(Pointlist[i].y / 45);
	std::cout << "x: " << Pointlist[i].x << " " << "y: " << Pointlist[i].y << std::endl;

	}
	*/
	
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
