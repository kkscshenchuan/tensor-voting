// tensor voting.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv.hpp"
#include "cmath"
#include <highgui.h>  
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/highgui/highgui.hpp>    
#include <iostream>    
#include <vector>    

using namespace std;
using namespace cv;

int Angle(double x1, double y1, double x2, double y2);  //以任意点为中心的投票域
int Angle(double x, double y);            //以原点为中心的投票域
int characteristic(double a,double b,double c,double d);                     //计算特征值与特征向量
void Statistics(Mat image);               //统计每个点的张量信息
double θ = 0;                            //两点之间产生的夹角
double scale = 10;                        //投票域尺度
double π = 3.1415;                       //圆周率
double DF = 0;                            //投票大小

const int height1 = 174;//二维数组的大小
const int width1 = 157;
int main()
{
	Mat srcImage, grayImage, dstImage;
	Mat gray_x, gray_y; //存储图像x，y方向的梯度
    srcImage = imread("road.png");
	imshow("原图", srcImage);

	cvtColor(srcImage, grayImage, COLOR_RGB2GRAY);//灰度图
	//imshow("灰度图", grayImage);

	threshold(grayImage, grayImage, 100, 255, THRESH_BINARY);//二值化图
	//threshold(grayImage, dstImage, 100, 255, THRESH_BINARY);//二值化图
	//imshow("二值图", grayImage);

	//计算图像梯度
	int ddepth = CV_16S; //输出图像深度
	int scale = 1;       //计算导数时的缩放因子
	
	//图形x方向梯度
	Sobel(grayImage, gray_x, -1, 1, 0, 3);
	//imshow("sobel算子x方向梯度图", gray_x);

	//图像y方向梯度
	Sobel(grayImage, gray_y, -1, 0, 1, 3);
	//imshow("sobel算子y方向梯度图", gray_y);

	//合并梯度
	addWeighted(gray_x, 0.5, gray_y, 0.5, 0, dstImage);
	namedWindow("sobel算子梯度图", 2);
	imshow("sobel算子梯度图", dstImage);

	int width = dstImage.cols;   //图像的长度与宽度
	int height = dstImage.rows;
	cout << width << " " << height << endl;

	

	//点的方向与张量矩阵计算
	double theta[height1][width1] = { 0 };                         //像素点的方向

	double matrix1[height1][width1] = { 0 };//存储二阶矩阵第一个值
	double matrix2[height1][width1] = { 0 };//存储二阶矩阵第一行第二个值
	double matrix3[height1][width1] = { 0 };//存储二阶矩阵第二行第一个值
	double matrix4[height1][width1] = { 0 };//存储二阶矩阵第二行第二个值

	for (int i = 0; i < height; i++)
	{
		uchar *p_x = gray_x.ptr<uchar>(i);       //x方向梯度图行指针
		uchar *p_y = gray_y.ptr<uchar>(i);       //y方向梯度图行指针
		uchar *data = grayImage.ptr<uchar>(i);   //指向原图
		for (int j = 0; j < width; j++)
		{
			double N_x = 0, N_y = 0;
			if (data[j] == 0)                    //像素值为0，为球张量
			{
				N_x = 1;
				N_y = 1;
				matrix1[i][j] = 1;
				matrix2[i][j] = 0;
				matrix3[i][j] = 0;
				matrix4[i][j] = 1;
			}
			else                                 //像素值为255，为棒张量
			{
				N_x = p_x[j];                    //像素x方向的梯度
			    N_y = p_y[j];                    //像素y方向的梯度
				matrix1[i][j] = N_x* N_x;
				matrix2[i][j] = N_x* N_y;
				matrix3[i][j] = N_x * N_y;
				matrix4[i][j] = N_y* N_y;
			}

			if (N_x == 0)
			{
				theta[i][j] = π / 2;
			}
			else
			theta[i][j] = atan(N_y / N_x);              //计算梯度方向
			//cout << theta[i][j] << " ";
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
	//		//Angle(j, i);      //遍历图片时，按列遍历，所以j对应横坐标
	//		Angle(25, 25, j, i);
	//		p[j] = p[j] * DF;
	//		//cout <<"("<<i <<j<< ") "<<endl;
	//		//Angle(i, j);
	//	}
	//}
	//namedWindow("线性投票域", 2);
	//imshow("线性投票域", grayImage);

	//for (int i = 0; i < height; i++)   //投票域统计
	//{
	//	uchar *p = grayImage.ptr<uchar>(i);
	//	for (int j = 0; j < width; j++)
	//	{
	//		for (int m = 0; m < height; m++)  //对图像中所有点进行张量投票
	//		{
	//			for (int n = 0; n < width; n++)
	//			{
	//				Angle(j, i, n, m);    //函数返回DF的值（投票的大小）
	//				matrix1[m][n] += matrix1[i][j] * DF;
	//				matrix2[m][n] += matrix2[i][j] * DF;
	//				matrix3[m][n] += matrix3[i][j] * DF;
	//				matrix4[m][n] += matrix4[i][j] * DF;
	//			}
	//		}
	//	}
	//}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//characteristic(matrix1[i][j], matrix2[i][j], matrix3[i][j], matrix4[i][j]);
			//characteristic(2, 1, 4, 2);
		}
	}
	

	waitKey(0);
    return 0;
}

//计算显著性衰减函数(以原点为中心)
int Angle(double x, double y)    //x代表横坐标，y代表纵坐标
{
	double l = sqrt(x*x + y * y);        //两点之间的距离
	double l1 = 0;                         //y轴上的点与原点的距
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
	cout << θ << " " << l << " " << s << " " << p << " " << c << " " << DF << endl;

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
	//cout <<l1<<" "<<a<<" "<< θ << " " << l << " " << s << " " << p << " " << DF << endl;
	return DF;
}


//统计每个点的张量信息
void Statistics(Mat image)
{
	int height = image.rows;
	int width = image.cols;

	for (int i = 0; i < image.rows; i++)
	{
		uchar *p = image.ptr<uchar>(i);       //输入图像的指针
		for (int j = 0; j < image.cols; j++)
		{
			if (p[j] == 255)
			{
				Angle(25, 25, j, i);
			}

		}
	}


}

//计算矩阵特征值与特征向量
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
	for (int i = 0; i < eValues.rows; i++)
	{
		for (int j = 0; j < eValues.cols; j++)
		{
			cout << eValues.at<double>(i, j) << " " << endl;
		}
	}

	for (int i = 0; i < eVectors.rows; i++)
	{
		for (int j = 0; j < eVectors.cols; j++)
		{
			cout << eVectors.at<double>(i, j) << " ";
		}
		cout << endl;
	}

	return 0;
}


//计算旋转后的投票域
void rotate(int x,int y ,double λ )
{

	double a = atan(y / x);  //计算点原始的夹角
	double b = sqrt(x*x + y * y); //两点之间的距离
	if (λ < π / 2)
	{
		a = a - λ;
	}
	x = b * cos(a);
	y = b * sin(a);

}
