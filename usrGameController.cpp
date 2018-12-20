#include "usrGameController.h"
#include <string>
#define PI 3.1415926
#ifdef VIA_OPENCV

bool flag = true;
bool new_game = true;
bool get_parts_vector = false;
bool get_parts = true;
bool find_solution = false;
int partsnum = 0;
double lastx = 0, lasty = 0;
std::string read_file;
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;
std::vector<std::vector<cv::Point>> contours1;
std::vector<cv::Vec4i> hierarchy1;
std::vector<std::vector<int>> vectorbase;
std::vector<std::vector<cv::Point>> vectorabsolute; // absolute x,y according to vectorbase
std::vector<std::vector<std::vector<int>>> vectorparts;
std::vector<std::vector<std::vector<cv::Point>>> vectorabsolute_parts;
std::vector<std::vector<int>> vectorsolution;
cv::Mat pt_gray_start_of_the_game;
cv::Mat parts_for_vec;
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
	
	templ.push_back(cv::imread("D:\\DIP\\triangle1.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle2.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle3.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\triangle4.jpg", 0));
	templ.push_back(cv::imread("D:\\DIP\\square.jpg", 0));
	for (int i = 0; i < templ.size(); i++)
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


	if (new_game)
	{
		pt_gray_start_of_the_game = pt_gray;
		// ===== start finding base contour ======
		cv::findContours(pt_binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		// cv::Mat newpt(pt_binary.rows, pt_binary.cols, CV_8U, cv::Scalar(0));
		for (int i = 0; i < contours.size(); i++)
		{
			if (hierarchy[i][3] != -1)
				cv::drawContours(pt_binary, contours, i, cv::Scalar(0, 255, 0), 2, 8);
		}
		// ===== end finding =====
		// ===== start finding the parts contour =====
		cv::findContours(partsinv, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_TC89_KCOS, cv::Point(0, 0));
		// ===== end finding =====
		//======== vectorize base start=======
		std::cout << "========= Start getting the vector of the base ========" << std::endl;
		pt_binary = pt_binary(cv::Rect(0, 80, width - 180, 550));
		analog2digital(pt_binary, templ, vectorbase, vectorabsolute, true);
		chubbyVector(vectorabsolute);
		for (int i = 0; i < vectorbase.size(); i++)
		{
			for (int j = 0; j < vectorbase[i].size(); j++)
				std::cout << vectorbase[i][j] << " ";
			std::cout << std::endl;
		}
		for (int i = 0; i < vectorbase.size(); i++)
		{
			for (int j = 0; j < vectorbase[i].size(); j++)
				std::cout << "x:" << vectorabsolute[i][j].x << "y:" << vectorabsolute[i][j].y;
			std::cout << std::endl;
		}
		std::cout << "========= Finish getting the vector of the base ========" << std::endl;
		//======== vectorize base end=========
	}
	
	// ===== rely on contours1.size() so ..... =====
	std::vector<cv::Point2f> centers(contours1.size());
	std::vector<int> centerGRAY(contours1.size());
	findCenterAndRGB(pt_gray_start_of_the_game, contours1, centers, centerGRAY);
	// ===== end =====

	// ===== start the control of DIP ======
	if (partsnum < contours1.size() - 1 && get_parts)
	{
		new_game = false; // just for test, need to revise
		double x;
		double y;
		qDebug() << "lastx:" << lastx << "lasty:" << lasty;
		if (partsnum != 0)
		{
			removeOtherGrayScale(pt_gray, centerGRAY[partsnum]);
			bitwise_not(pt_gray, pt_gray);
			savePicture(pt_gray, partsnum);
			device->comMoveToScale(0, 0);
			_sleep(1000);
			device->comHitUp();
		}
		partsnum++;
		x = 0.8 - centers[partsnum].y / pt_gray.rows;
		y = centers[partsnum].x / pt_gray.cols;
		lastx = x;
		lasty = y;
		qDebug() << centerGRAY[partsnum] << " x:" << x << " y:" << y;
		device->comHitUp();
		device->comMoveToScale(x, y);
		_sleep(1000);
		device->comHitDown();
		device->comMoveToScale(0.25, 0.4);
		_sleep(1000);
	}
	else if (partsnum == contours1.size() - 1 && get_parts)
	{
		removeOtherGrayScale(pt_gray, centerGRAY[partsnum]);
		bitwise_not(pt_gray, pt_gray);
		savePicture(pt_gray, partsnum);
		device->comMoveToScale(0, 0);
		_sleep(1000);
		device->comHitUp();
		// device->comMoveToScale(0, 0);
		partsnum++;
		get_parts_vector = true;
		get_parts = false;
	}
	// ===== end =====

	// ===== start get the vector of the parts =====
	while (get_parts_vector == true)
	{
		std::cout << "============Start getting the vector of parts===============" << std::endl;
		std::cout << "There are " << partsnum - 1 << " in this game" << std::endl;
		std::vector<std::vector<int>> vectorparts_one;
		std::vector<std::vector<cv::Point>> vectorabsolute_parts_one;
		for (int i = 1; i < partsnum; i++)
		{
			std::cout << "Start processing " << i << " parts" << std::endl;
			read_file = "D:\\DIP\\qtCyberDIP\\qtCyberDip\\parts" + std::to_string(i) + ".jpg";
			parts_for_vec = cv::imread(read_file, 0);
			cv::blur(parts_for_vec, parts_for_vec, cv::Size(2, 2));
			cv::threshold(parts_for_vec, parts_for_vec, 30, 255, CV_THRESH_BINARY);
			analog2digital(parts_for_vec, templ, vectorparts_one, vectorabsolute_parts_one, false);
			chubbyVector(vectorabsolute_parts_one);
			solveProblem(vectorparts_one);
			for (int j = 0; j < vectorparts_one.size(); j++)
			{
				for (int k = 0; k < vectorparts_one[j].size(); k++)
				{
					std::cout << vectorparts_one[j][k];
				}	
				std::cout << std::endl;
			}
			for (int j = 0; j < vectorabsolute_parts_one.size(); j++)
			{
				for (int k = 0; k < vectorabsolute_parts_one[j].size(); k++)
				{
					std::cout << "x: " << vectorabsolute_parts_one[j][k].x << "y: " << vectorabsolute_parts_one[j][k].y;
				}
				std::cout << std::endl;
			}
			vectorabsolute_parts.push_back(vectorabsolute_parts_one);
			vectorparts.push_back(vectorparts_one);
			vectorparts_one.clear();
			vectorabsolute_parts_one.clear();
			std::cout << "Finish processing " << i << " parts" << std::endl;
		}
		std::cout << "============ Finish getting the vector of parts ===============" << std::endl;
		get_parts_vector = false;
		find_solution = true;
	}
	// ===== end =====

	// start finding the answer.....                %TODO
	// move the parts to correct place
	// new_game = true
	if (find_solution)
	{
		int target_x, target_y;
		int now_x, now_y;
		double center_x, center_y;
		double move_x, move_y;
		int delta_x, delta_y;
		std::cout << "============ Start getting the Solution ===============" << std::endl;
		getSolution(0);	
		// for (int i = 0; i < 3; i++)
		for (int i = 0; i < vectorsolution.size(); i++)
		{
			std::cout << "parts " << i + 1 << " should move to ";
			std::cout << "row: " << vectorsolution[i][0] << ", column: " << vectorsolution[i][1] << std::endl;
			target_x = vectorabsolute[vectorsolution[i][0]][vectorsolution[i][1]].x;
			target_y = vectorabsolute[vectorsolution[i][0]][vectorsolution[i][1]].y;
			now_x = vectorabsolute_parts[i][0][0].x;
			now_y = vectorabsolute_parts[i][0][0].y;
			// control of DIP
			center_x = 0.8 - centers[i+1].y / pt_gray.rows;
			center_y = centers[i+1].x / pt_gray.cols;
			device->comHitUp();
			device->comMoveToScale(center_x, center_y);
			_sleep(1000);
			device->comHitDown();
			// device->comMoveToScale(0.25, 0.4);
			// _sleep(1000);

			delta_x = target_x - now_x;
			delta_y = target_y - now_y;
			// std::cout << "parts " << i + 1 << " should move " << delta_x << " " << delta_y << std::endl;
			move_x = double(delta_y) / pt_gray.rows;
			move_y = double(delta_x) / pt_gray.cols;
			std::cout << "movex: " << move_x << "move_y: " << move_y << std::endl;
			device->comMoveToScale(0.25 - move_x, 0.4 + move_y);
			_sleep(1000);
			device->comHitUp();
		}
		find_solution = false;
	}
	cv::imshow(WIN_NAME, pt_binary);

	return 0; 
}

void solveProblem(std::vector<std::vector<int>> &vectorbase)
{
	for (int i = 1; i < vectorbase.size() - 1; i++)
	{
		for (int j = 1; j < vectorbase[i].size() - 1; j++)
		{
			if (vectorbase[i][j - 1] != 0 && vectorbase[i][j + 1] != 0 && vectorbase[i + 1][j] != 0 && vectorbase[i - 1][j] != 0)
			{
				vectorbase[i][j] = 5;
			}
		}
	}
}

void chubbyVector(std::vector<std::vector<cv::Point>> &vectorabsolute)
{
	int height = vectorabsolute.size();
	int width = vectorabsolute[0].size();
	std::vector<int> ref_X(width);
	std::vector<int> ref_Y(height);
	for (int i = 0; i < vectorabsolute.size(); i++)
	{
		for (int j = 0; j < vectorabsolute[i].size(); j++)
		{
			if (vectorabsolute[i][j].x != 0)
			{
				ref_X[j] = vectorabsolute[i][j].x;
			}
			if (vectorabsolute[i][j].y != 0)
			{
				ref_Y[i] = vectorabsolute[i][j].y;
			}
		}
	}
	for (int i = 0; i < vectorabsolute.size(); i++)
	{
		for (int j = 0; j < vectorabsolute[i].size(); j++)
		{
			if (vectorabsolute[i][j].x == 0)
			{
				vectorabsolute[i][j].x = ref_X[j];
			}
			if (vectorabsolute[i][j].y == 0)
			{
				vectorabsolute[i][j].y = ref_Y[i];
			}
		}
	}
}

void analog2digital(cv::Mat &img, std::vector<cv::Mat> &templ, std::vector<std::vector<int>> &vectorbase, std::vector<std::vector<cv::Point>> &vectorabsolute, bool isBase)
{
	int gray_scale;
	cv::Mat result(img.rows, img.cols, CV_8U, cv::Scalar(0));
	cv::Point test;
	std::vector<cv::Point> triangle1;
	std::vector<cv::Point> triangle2;
	std::vector<cv::Point> triangle3;
	std::vector<cv::Point> triangle4;
	std::vector<cv::Point> square;
	std::vector<int> vectorbaseline;
	std::vector<cv::Point> vectorabsoluteline;
	std::vector<std::vector<cv::Point>> LeftUp;
	std::vector<std::vector<cv::Point>> LeftUpAbs(30);
	LeftUp.push_back(triangle1);
	LeftUp.push_back(triangle2);
	LeftUp.push_back(triangle3);
	LeftUp.push_back(triangle4);
	LeftUp.push_back(square);
	std::vector<int> not_zero_row;
	std::vector<int> not_zero_col;
	int smallestx = 9999, smallesty = 9999;
	int normalized_x, normalized_y;
	float maxx = -1, maxy = -1, lastmaxx = 9999, lastmaxy = 9999, needvaluex, needvaluey;
	for (int i = 0; i < templ.size(); i++)
	{
		std::cout << "=====" << i + 1 << "=====" << std::endl;
		findTemplLeftUp(img, templ[i], LeftUp[i], i, isBase);
	}

	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
			LeftUpAbs[i].push_back(cv::Point(0, 0));
	}
	/*
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
			vectorabsolute[i].push_back(cv::Point(0, 0));
	}
	*/
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
			
			normalized_x = round((LeftUp[i][j].x / 10.0 - smallestx / 10.0) / 5.0);
			normalized_y = round((LeftUp[i][j].y / 10.0 - smallesty / 10.0) / 5.0);
			LeftUpAbs[normalized_x][normalized_y].x = LeftUp[i][j].x;
			if (isBase)
				LeftUpAbs[normalized_x][normalized_y].y = LeftUp[i][j].y + 80; // something was cut
			else
				LeftUpAbs[normalized_x][normalized_y].y = LeftUp[i][j].y;
			cv::circle(result, cv::Point(normalized_x, normalized_y), 0.5, cv::Scalar(i + 1), -1);
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
			vectorabsoluteline.push_back(LeftUpAbs[not_zero_col[j]][not_zero_row[i]]);
			
		}
		vectorbase.push_back(vectorbaseline);
		vectorabsolute.push_back(vectorabsoluteline);
		vectorbaseline.clear();
		vectorabsoluteline.clear();
		// std::cout << std::endl;
	}
}

void findTemplLeftUp(cv::Mat &img, cv::Mat &templ, std::vector<cv::Point> &Pointlist, int type, bool isBase)
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
			if (isBase)
			{
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
			else
			{
				if (matchValue < 0.2 && type != 4)
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
				if (matchValue < 0.2 && type == 4)
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
	}
	for (int i = 0; i < Pointlist.size(); i++)
	{
		std::cout << "x: " << Pointlist[i].x << " " << "y: " << Pointlist[i].y << std::endl;
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
	std::string filename = "parts" + std::to_string(i) + ".jpg";
	cv::imwrite(filename, img);
	return;
}

void removeOtherGrayScale(cv::Mat &gray_pt, int target_gray_scale)
{
	int gray_scale;
	for (int i = 0; i<gray_pt.rows; i++)
		for (int j = 0; j < gray_pt.cols; j++)
		{
			gray_scale = gray_pt.ptr<uchar>(i)[j];
			if (abs(gray_scale - target_gray_scale) <= 1)
				gray_pt.ptr<uchar>(i)[j] = 0;
			else
				gray_pt.ptr<uchar>(i)[j] = 255;
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

void getSolution(int k)
{
	int length = vectorbase.size() - vectorparts[k].size() + 1;
	int width = vectorbase[0].size() - vectorparts[k][0].size() + 1;
	for (int i = 0; i < length; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (judgeFinish()) break;
			if (judge(i, j, k))
			{
				vectorsolution.push_back({ i,j });
				if (judgeFinish())
					return;
				getSolution(k + 1);
			}
		}
	}
	if (!judgeFinish())
	{
		removeFromBase(vectorsolution[k - 1][0], vectorsolution[k - 1][1], k - 1);
		vectorsolution.pop_back();
	}
	return;
}

bool judge(int x, int y, int k)
{
	std::vector<std::vector<int>> ans(vectorbase);
	for (int i = 0; i < vectorparts[k].size(); i++)
	{
		for (int j = 0; j < vectorparts[k][0].size(); j++)
		{
			ans[i + x][j + y] -= vectorparts[k][i][j];
			if (ans[i + x][j + y] < 0)
				return false;
		}
	}
	vectorbase = ans;
	return true;
}

void removeFromBase(int x, int y, int k)
{
	for (int i = 0; i < vectorparts[k].size(); i++)
	{
		for (int j = 0; j < vectorparts[k][0].size(); j++)
		{
			vectorbase[i + x][j + y] += vectorparts[k][i][j];
		}
	}
	return;
}

bool judgeFinish()
{
	for (int i = 0; i < vectorbase.size(); i++)
	{
		for (int j = 0; j < vectorbase[0].size(); j++)
		{
			if (vectorbase[i][j] != 0)
				return false;
		}
	}
	return true;
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