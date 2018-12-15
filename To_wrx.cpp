std::vector<std::vector<int>> vectorbase;
std::vector<std::vector<std::vector<int>>> vectorparts;
std::vector<std::vector<int>> vectorsolution;
'''
vectorsolution是一个N*3的二维数组
[0, x0, y0],第0块拼图矩阵最左上的那个数字要拼到矩阵的第x0行，第y0列
[1, 0, 1],第1块拼图矩阵最左上的那个数字要拼到矩阵的第0行，第1列
...

'''
void getSolution(vectorbase, vectorparts, vectorsolution)
{
	// %TODO
}

// Test Data
vectorbase:
[[0, 1, 2, 0],
 [1, 5, 5, 2],
 [5, 5, 5, 5],
 [5, 5, 5, 5],
 [5, 5, 5, 5],
 [4, 5, 5, 3],
 [0, 4, 3, 0]]

vectorparts:
[
[[5, 5],
 [5, 5]],
[[3, 0],
 [5, 3],
 [5, 4],
 [4, 0]],
[[5, 5],
 [5, 3],
 [3, 0]],
[[4, 5],
 [0, 4]],
[[0, 1],
 [1, 5]],
[[0, 1],
 [1, 5],
 [5, 5]],
]