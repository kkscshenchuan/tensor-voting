// tensor voting.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <opencv.hpp>
#include <cmath>
#include <highgui.h>  
#include <iostream>    
#include <vector>    
#include <algorithm>
#include <functional>

using namespace std;
using namespace cv;

int Angle(double x1, double y1, double x2, double y2);                   //以任意点为中心的投票域
int Angle(double x, double y);                                           //以原点为中心的投票域
int characteristic(double a, double b, double c, double d);              //计算对称矩阵的特征值与特征向量
int rotate(int x1, int y1, int &x2, int &y2, double λ);                 //投票域根据角度旋转之后对应的坐标
double θ = 0;                            //两点之间产生的夹角
double scale = 10;                        //投票域尺度
double π = 3.1415;                       //圆周率
double DF = 0;                            //投票大小
double λ1 = 0;                           //矩阵的两个特征值
double λ2 = 0;
int k = 3;                                //定义搜索尺度（尺度为奇数）
int k1 = k / 2;                           //搜索尺度的一半
const int N = 1;                         //种子点的个数

int main()
{
	Mat srcImage, grayImage, dstImage, resultImg;
	Mat gray_x, gray_y; //存储图像x，y方向的梯度
	srcImage = imread("a.png");
	namedWindow("原图", 2);
	imshow("原图", srcImage);
	resultImg = srcImage.clone();  //拷贝原图，存放最后的结果图

	cvtColor(srcImage, grayImage, COLOR_RGB2GRAY);//灰度图
	//imshow("灰度图", grayImage);

	threshold(grayImage, grayImage, 100, 255, THRESH_BINARY);//二值化图
	//threshold(grayImage, grayImage, 100, 255, THRESH_BINARY_INV);//二值化图
	//imshow("二值图", grayImage);


	int width = grayImage.cols; //图片的长度和宽度
	int height = grayImage.rows;



	//cout << width << " " << height << endl;


	//点的方向与张量矩阵计算
	vector<vector<double> > theta(height, vector<double>(width, 0));//创建动态二维数组，并将数组中所有的值赋值为0,用来存储每个点的梯度信息
	//存储初始张量值
	vector<vector<double> > matrix1(height, vector<double>(width, 0));//存储二阶矩阵第一个值
	vector<vector<double> > matrix23(height, vector<double>(width, 0));//存储二阶矩阵第一行第二个值和第二行第一个值
	vector<vector<double> > matrix4(height, vector<double>(width, 0));//存储二阶矩阵第二行第二个值

	//存储累加之后的张量值
	vector<vector<double> > result1(height, vector<double>(width, 0));//存储二阶矩阵第一个值
	vector<vector<double> > result23(height, vector<double>(width, 0));//存储二阶矩阵第一行第二个值和第二行第一个值
	vector<vector<double> > result4(height, vector<double>(width, 0));//存储二阶矩阵第二行第二个值

	vector<vector<int> > Quadrant(height, vector<int>(width, 0));//每个点的方向属于哪个象限

	double Gx = 0;   //横向梯度值
	double Gy = 0;   //纵向梯度值

	//初始化张量场
	for (int i = 1; i < height - 1; i++)          //考虑边界问题
	{
		uchar *p = grayImage.ptr<uchar>(i - 1);  //获取第i-1行的指针
		uchar *q = grayImage.ptr<uchar>(i);
		uchar *r = grayImage.ptr<uchar>(i + 1);

		for (int j = 1; j < width - 1; j++)
		{
			if (i == 0 || j == 0 || i == height - 1 || j == width - 1)    //为边界点时，赋值为0
			{
				Gx = 0; Gy = 0;
			}
			else
			{
				Gx = (p[j - 1] + 2 * q[j - 1] + r[j - 1]) - (p[j + 1] + 2 * q[j + 1] + r[j + 1]);//计算横向梯度值
				Gy = (r[j - 1] + 2 * r[j] + r[j + 1])-(p[j - 1] + 2 * p[j] + p[j + 1]);     //计算纵向梯度值

				//cout << Gx << " " << Gy ;
				if (Gx > 0 && Gy < 0)
				{
					Quadrant[i][j] = 2;         //点的方向位于第二象限

				}
				else if (Gx < 0 && Gy < 0)
				{
					Quadrant[i][j] = 1;         //点的方向位于第一象限
				}
				else if (Gx < 0 && Gy > 0)
				{
					Quadrant[i][j] = 4;         //点的方向位于第四象限
				}
				else if (Gx >= 0 && Gy > 0)
				{
					Quadrant[i][j] = 3;         //点的方向位于第三象限
				}
				else if (Gx == 0 && Gy > 0)
				{
					Quadrant[i][j] = 10;         //点的方向位于第三和第四象限
				}
				else if (Gx == 0 && Gy < 0)
				{
					Quadrant[i][j] = 20;         //点的方向位于第一和第二象限
				}
				else if (Gx > 0 && Gy == 0)
				{
					Quadrant[i][j] = 30;         //点的方向位于第二和第三象限
				}
				else if (Gx < 0 && Gy == 0)
				{
					Quadrant[i][j] = 40;         //点的方向位于第一和第四象限
				}
				else if (Gx == 0 && Gy == 0)
				{
					Quadrant[i][j] = 50;         //点没有方向，则向领域内搜索
				}
			/*	cout << " ";
				cout << Quadrant[i][j] << endl;*/

				if (Gx == 0)
				{
					theta[i][j] = π/2;
				}
				else
				{
					if (Gy == 0)
					{
						theta[i][j] = 0;
					}
					else
					{
						theta[i][j] = atan(Gy / Gx);
					}
				}
				Gx = 1 * cos(theta[i][j]);       //法向量变成单位向量
				Gy = 1 * sin(theta[i][j]);

				if (q[j] == 0)                    //像素值为0，为球张量
				{
					matrix1[i][j] = 1;
					matrix23[i][j] = 0;               //结构张量矩阵为对称矩阵。第二个和第三个值相等
					matrix4[i][j] = 1;
				}
				else                                 //像素值为255，为棒张量
				{
					matrix1[i][j] = Gx * Gx;
					matrix23[i][j] = Gx * Gy;
					matrix4[i][j] = Gy * Gy;
				}
			}
		}
	}

	//Mat SrcImage(50, 50, CV_8UC1, Scalar(255));   //测试图片
	//int Width = SrcImage.cols;
	//int Height = SrcImage.rows;
	//for (int i = 0; i < Height; i++)   //投票域显示
	//{
	//	uchar *p = SrcImage.ptr<uchar>(i);
	//	for (int j = 0; j < Width; j++)
	//	{
	//		Angle(j, i);      //遍历图片时，按列遍历，所以j对应横坐标
	//		Angle(25, 25, j, i);
	//		p[j] = p[j] * DF;
	//		cout <<"("<<i <<j<< ") "<<endl;
	//		Angle(i, j);
	//	}
	//}
	//namedWindow("线性投票域", 2);
	//imshow("线性投票域", SrcImage);

	for (int i = 1; i < height-1; i++)   //投票域统计
	{
		uchar *p = grayImage.ptr<uchar>(i);
		for (int j = 1; j < width-1; j++)
		{
			if (p[j] == 255)          //棒张量投票
			{
				for (int m = 1; m < height-1; m++)  //对图像中所有点进行张量投票
				{
					uchar *q = grayImage.ptr<uchar>(m);
					for (int n = 1; n < width-1; n++)
					{
						if (q[n] == 255)                    //棒张量处接受投票
						{
							int a = n;                      //将m、n的值赋值给a，b，方便下面使用a，b的应用
							int b = m;
							rotate(j, i, a, b, theta[i][j]);
							Angle(j, i, a, b);    //函数返回DF的值（投票的大小）
							//Angle(j, i, n, m);    //函数返回DF的值（投票的大小）
							result1[m][n] += matrix1[i][j] * DF;
							result23[m][n] += matrix23[i][j] * DF;
							result4[m][n] += matrix4[i][j] * DF;
						}
					}
				}
			}
		}
	}

	vector<vector<int> > state(height, vector<int>(width, 0));//存储每个点的状态。背景标记为0，前景为1
	vector<vector<int> > result(height, vector<int>(width, 0));  //标记每个点是否为最终线段上的点
	vector<vector<double> > array(height, vector<double>(width, 0));   //存储每个点的线性显著性值

	for (int i = 0; i < height; i++)
	{
		uchar *p = grayImage.ptr<uchar>(i);  //指向原图
		for (int j = 0; j < width; j++)
		{
			if (p[j] == 0)
			{
				state[i][j] = 0;
			}
			else
			{
				state[i][j] = 1;
			}
		}
	}

	//计算图像中每个点的线显著性值
	for (int i = 0; i < height; i++)
	{
		uchar *p = grayImage.ptr<uchar>(i);  //指向原图
		for (int j = 0; j < width; j++)
		{
			characteristic(result1[i][j], result23[i][j], result23[i][j], result4[i][j]);
			double a = λ1 - λ2;  //每个点对应的线特征显著性值
			array[i][j] = a;
		}
	}

	int x = 0;//记录下一个出发点的坐标
	int y = 0;

	vector<double> seedpoint;   //存储所有点的显著性
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			seedpoint.push_back(array[i][j]);
		}
	}

	sort(seedpoint.begin(), seedpoint.end(), greater<int>());  //降序排列
	double Seed[N] = { 0 };    //存储种子点
	for (int i = 0; i < N; i++)
	{
		Seed[i] = seedpoint[i];
		//Seed[i] = seedpoint[i * 10];
		//cout << Seed[i]<<" ";
	}
	cout << endl;

	int X[N] = { 0 }; //存储种子点的横坐标
	int Y[N] = { 0 }; //存储种子点的纵坐标

	bool button = false;
	for (int i = 0; i < N;i++)  //找到种子点对应的坐标
	{
		button = false;
		for (int m = 0; m < height; m++)
		{
			if (button==true) break;
			for (int n = 0; n < width; n++)
			{
				if (Seed[i] == array[m][n])
				{
					X[i] = n;
					Y[i] = m;
					button = true;
					break;
				}
			}
		}
	}

	int sx = 0, sy = 0;//用来存储更新点的坐标
	int number;
	for (int i = 0; i < N; i++)
	{
		number = 0;  //统计搜索次数
		if (state[Y[i]][X[i]] == 1)
		{
			x = X[i];
			y = Y[i];
			//result[y][x] = 1;//将种子点标记为线上的点
			while (number<200)
			{
				double stand = 0;
				array[y][x] = 0;          //将初始点赋值为0，避免搜索时又搜索到初始点

				switch (Quadrant[y][x])   //判断点的方向属于哪一个象限
				{
				case 1:                   //点方向在第一象限
				{
					for (int m = y; m > y - k && m >= 0; m--)
					{
						for (int n = x; n < x + k && n < width - 1; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 2:                  //点方向在第二象限
				{
					for (int m = y; m > y - k && m >= 0; m--)
					{
						for (int n = x; n > x - k && n >= 0; n--)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sy] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 3:                 //点方向在第三象限
				{
					for (int m = y; m < y + k && m < height - 1; m++)
					{
						for (int n = x; n > x - k && n >= 0; n--)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 4:                 //点方向在第四象限
				{
					for (int m = y; m < y + k && m < height - 1; m++)
					{
						for (int n = x; n < x + k && n < width - 1; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 10:                 //点方向在第三和四象限
				{
					for (int m = y; m < y + k && m < height - 1; m++)
					{
						for (int n = x - k1; n <= x + k1 && n <= width - 1 && n >= 0; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 20:                 //点方向在第一和二象限
				{
					for (int m = y; m > y - k && m >= 0; m--)
					{
						for (int n = x - k1; n <= x + k1 && n <= width - 1 && n >= 0; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 30:                 //点方向在第二和三象限
				{
					for (int m = y - k1; m <= y + k1 && m <= height - 1 && m >= 0; m++)
					{
						for (int n = x; n >x - k && n >= 0; n--)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 40:                 //点方向在第一和四象限
				{
					for (int m = y - k1; m <= y + k1 && m < height - 1 && m >= 0; m++)
					{
						for (int n = x; n < x + k && n <= width - 1; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				case 50:                 //点没有方向，向邻域内搜索
				{
					for (int m = y - k1; m <= y + k1 && m <= height - 1 && m >= 0; m++)
					{
						for (int n = x - k1; n <= x + k1 && n >= 0 && n <= width - 1; n++)
						{
							if (array[m][n] > stand && state[m][n] == 1)
							{
								stand = array[m][n];
								sy = m;
								sx = n;
							}
							state[m][n] = 0;    //将搜索过得点都标记为背景点
						}
					}
					result[sy][sx] = 1;    //将显著性最大的值记录
					x = sx;
					y = sy;
					break;
				}
				default:
					break;
				}

				number++;
			}
		}
		
	}


	for (int i = 0; i < height; i++)
	{
		uchar *p = grayImage.ptr<uchar>(i);  //指向原图
		for (int j = 0; j < width; j++)
		{
			if (p[j] == 0)
			{
				state[i][j] = 0;
			}
			else
			{
				state[i][j] = 1;
			}
		}
	}

	for (int i = 0; i < height; i++)
	{
		uchar *p = grayImage.ptr<uchar>(i);  //指向原图
		for (int j = 0; j < width; j++)
		{
			if (result[i][j] == 1&&state[i][j]==1)
			{
				p[j] = 255;
			}
			else
			{
				p[j] = 0;
			}
		}
	}
	namedWindow("张量投票结果", 2);
	imshow("张量投票结果", grayImage);

	//characteristic(1,2.5,2.5,6.25);
	cout << "ok" << endl;
	waitKey(0);
	return 0;
}


//计算显著性衰减函数(以原点为中心)
int Angle(double x, double y)    //x代表横坐标，y代表纵坐标
{
	double l = sqrt(x*x + y * y);        //两点之间的距离
	double l1 = 0;                         //y轴上的点与原点的距离
	double a = 0;                          //sinθ的值
	double s = 0;                         //弧长
	double p = 0;     //曲率
	double c;//控制曲率的退化程度
	if (y == 0)
	{
		l1 = 0;
		a = 0;
		θ = 0;
	}
	else
	{
		l1 = y / 2 + x * x / y / 2;
		a = l / 2 / l1;
		θ = asin(a);
	}

	if (a == 0)
	{
		s = l;             //当θ为0时，弧长s与l相等
	}
	else
	{
		s = θ * l / a;    //弧长
	}

	if (l == 0)           //l为0，曲率为0
	{
		p = 0;
	}
	else
	{
		p = 2 * sin(θ) / l;     //曲率
	}
	c = -16 * log(0.1)*(scale - 1) / π / π;
	DF = exp(-(s*s + c * p*p) / scale / scale);
	//cout << θ << " " << l << " " << s << " " << p << " " << c << " " << DF << endl;

	return DF;
}

//计算显著性衰减函数（以任意点为中心）
int Angle(double x1, double y1, double x2, double y2)
{
	double m = abs(x1 - x2);
	double n = abs(y1 - y2);
	double l = sqrt(m*m + n * n);          //两点之间的距离
	double l1 = 0;                         //y轴上的点与原点的距
	double a = 0;                          //存储sinθ的值
	double s = 0;                          //弧长
	double p = 0;     //曲率
	double c;//控制曲率的退化程度

	if (x1 == x2)     //两点x坐标相同 相当与在y轴上的点
	{
		l1 = abs(y2 - y1);
		a = 1;
		θ = π / 2;
	}
	else
	{
		if (y1 == y2)        //两点y坐标相同，相当于在x轴上的点
		{
			a = 0;
			θ = 0;
		}
		//图像坐标系与直角坐标系的差异
		else if (y2 < y1)   //当以投票点为中心，接受点在第一、第二象限时
		{
			l1 = (y1 - y2) / 2 + (x2 - x1) / (y1 - y2)*(x2 - x1) / 2;
			a = l / 2 / l1;
			θ = asin(a);
		}
		else if (y2 > y1)   //当以投票点为中心，接受点在第三、四象限时
		{
			l1 = (y2 - y1) / 2 + (x2 - x1) / (y2 - y1)*(x2 - x1) / 2;
			a = l / 2 / l1;
			θ = asin(a);
		}
	}
	if (a == 0)
	{
		s = l;             //当θ为0时，弧长s与l相等
	}
	else
	{
		s = θ * l / a;    //弧长
	}
	if (a == 0)
	{
		p = 0;
	}

	else
	{
		p = 2 * sin(θ) / l;     //曲率
	}
	c = -16 * log(0.1)*(scale - 1) / π / π;
	if (θ > π / 4)
	{
		DF = 0;
		//DF = exp(-(s*s + c * p*p) / scale / scale);
	}
	else
	{
		DF = exp(-(s*s + c * p*p) / scale / scale);
	}
	return DF;
}


//计算矩阵特征值与特征向量(对称矩阵)
int characteristic(double a, double b, double c, double d)
{
	//输入矩阵
	double Array[2][2] = {
		a,b,
		c,d
	};
	Mat matrix = Mat(2, 2, CV_64F, Array);
	Mat eValues; //特征值
	Mat eVectors;  //特征向量
	eigen(matrix, eValues, eVectors);

	λ1 = eValues.at<double>(0, 0);      //矩阵的两个特征值
	λ2 = eValues.at<double>(1, 0);
	return 0;
}


//计算旋转后的投票域
int rotate(int x1, int y1, int &x2, int &y2, double λ)
{
	int x = 0, y = 0, X = 0, Y = 0;
	double m = x2 - x1;
	double n = y2 - y1;
	double a = atan(abs(n) / abs(m));  //计算点原始的夹角
	double b = sqrt(m*m + n * n);      //两点之间的距离

	if (m < 0 && n<0)                  //第二象限
	{
		a = π - a;
	}
	else if (m < 0 && n > 0)          //第三象限
	{
		a = a + π;
	}
	else if (m > 0 && n > 0)          //第四象限
	{
		a = 2 * π - a;
	}
	else if (m > 0 && n<0)            //第一象限
	{
		a = a;
	}
	else if (m == 0 && n < 0)         //当在坐标轴上时
	{
		a = π / 2;
	}
	else if (m == 0 && n > 0)
	{
		a = π * 3 / 2;
	}
	else if (m > 0 && n == 0)
	{
		a = 0;
	}
	else if (m < 0 && n == 0)
	{
		a = π;
	}
	X = round(b * cos(a));   //四舍五入
	Y = -round(b * sin(a));       //图像坐标与直角坐标的差异

	a = a + π / 2 - λ;
	x = round(b * cos(a));    //存储旋转之后的坐标
	y = -round(b * sin(a));

	x2 = x2 + (x - X);
	y2 = y2 + (y - Y);
	return 0;
}