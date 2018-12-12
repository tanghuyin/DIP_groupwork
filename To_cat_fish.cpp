// target function
// I think it should be insert in front of Line 137 in usrGameController.cpp
void removeOtherGrayScale(cv::Mat &gray_pt, int target_gray_scale)
{
	/*
	遍历所有像素点，只保留指定灰度值的像素点，其余灰度值的像素点统一变黑或白，cv:threshold库函数好像也可以设定上下阈值，可以查查，如果可以用就很方便了
	可以新建一个空的opencv项目，这样测试起来快一点
	Love you!(｡ì _ í｡)
	*/
}