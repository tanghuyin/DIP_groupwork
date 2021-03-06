// test1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdio>
#include <iostream>
#include <vector>

std::vector<std::vector<int>> vectorbase;
std::vector<std::vector<std::vector<int>>> vectorparts;
std::vector<std::vector<int>> vectorsolution;

void getSolution(int k);
bool judge(int x, int y, int k);
bool judgeFinish();
void removeFromBase(int x, int y, int k);

int main()
{
	vectorbase = { {0,4,3,0},{4,5,5,3},{5,5,5,5},{5,5,5,5},{5,5,5,5},{2,5,5,1},{0,2,1,0} };
	vectorparts = {
	{ {5,5},{5,5} },
	{ {3,0},{5,3},{5,1},{1,0} },
	{ {5,5},{5,1},{1,0} },
	{ {2,5},{0,2} },
	{ {0,4},{4,5} },
	{ {0,4},{4,5},{5,5} },
	};
//	vectorbase = { {1,0},{2,1} };
//	vectorparts = { {{1,1}},{{1,0}},{ { 1 } } };
//	vectorsolution = { {0,0} };
	getSolution(0);
	for (int i = 0; i < vectorsolution.size(); i++)
	{
		std::cout << vectorsolution[i][0] << "," << vectorsolution[i][1] << std::endl;
	}
	//std::cout << vectorsolution.size() ;
    return 0;
}


void getSolution(int k)//？只得一解，用judgeFinish判断？
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

bool judge(int x,int y,int k)
{
	std::vector<std::vector<int>> ans(vectorbase);
	for (int i = 0; i < vectorparts[k].size(); i++)
	{
		for (int j = 0; j < vectorparts[k][0].size(); j++)
		{
			ans[i + x][j + y] -= vectorparts[k][i][j];
			if (ans[i+x][j+y] < 0)
				return false;
		}
	}
	vectorbase=ans;
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