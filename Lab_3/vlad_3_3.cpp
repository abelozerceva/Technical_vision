#include <stdlib.h>
#include <stdio.h>
#include <opencv2\core\core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

void show_target(Mat &img, string str);
int distance(int X1, int Y1, int X2, int  Y2);

int main(int argc, char** argv)
{
	Mat *imgs = new Mat[2];
	imgs[0] = imread("roi_robotov.jpg");
	imgs[1] = imread("roi_robotov_1.jpg");

	cvtColor(imgs[0], imgs[0], CV_BGR2HSV);
	cvtColor(imgs[1], imgs[1], CV_BGR2HSV);

	namedWindow("Threshold window");
	for (int i = 0; i < 2; i++) {
		show_target(imgs[i],
			to_string(i + 1));
	}

	waitKey(0);
	return 0;
}

void show_target(Mat &img, string str)
{
	Mat target = img.clone();

	//Selecting lamp and finding center point
	Mat lamp_pixels;
	inRange(img, Vec3b(0, 0, 248), Vec3b(179, 10, 255), lamp_pixels);
	vector<vector<Point>> lamp_cnts;
	findContours(lamp_pixels.clone(), lamp_cnts, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	int Xlamp, Ylamp;
	for (int i = 0; i < lamp_cnts.size(); i++)
	{
		if (lamp_cnts[i].size() > 70)
		{
			Moments mnts = moments(lamp_cnts[i]);
			double m00 = mnts.m00;
			double m01 = mnts.m01;
			double m10 = mnts.m10;
			Xlamp = int(m10 / m00);
			Ylamp = int(m01 / m00);
			circle(target, { Xlamp, Ylamp }, 5, Scalar(0, 0, 0), -1);
		}
	}


	//Selecting robots and finding the nearest of each colour
	Mat blue_pixels;
	inRange(img, Vec3b(90, 23, 21), Vec3b(139, 231, 255), blue_pixels);
	char a = 'p';
	while (a != 'x') {
		if (a == 'e') erode(blue_pixels, blue_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		if (a == 'd') dilate(blue_pixels, blue_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		imshow("after both " + str, blue_pixels);
		a = waitKey(0);
	}

	vector<vector<Point>> blue_cnts;

	findContours(blue_pixels.clone(), blue_cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int min_blue_dist, Xblue, Yblue;
	for (int i = 0; i < blue_cnts.size(); i++)
	{
		if (blue_cnts[i].size() < 70)
		{
			blue_cnts.erase(blue_cnts.begin() + i);
			i--;
		}
		else
		{
			Moments mnts = moments(blue_cnts[i]);
			double m00 = mnts.m00;
			double m01 = mnts.m01;
			double m10 = mnts.m10;
			int Xc = int(m10 / m00);
			int Yc = int(m01 / m00);
			int dist = distance(Xc, Yc, Xlamp, Ylamp);
			if ((i == 0) || (dist < min_blue_dist))
			{
				min_blue_dist = dist;
				Xblue = Xc;
				Yblue = Yc;
			}
		}
	}
	circle(target, { Xblue, Yblue }, 5, Scalar(120, 255, 255), -1);
	drawContours(target, blue_cnts, -1, Scalar(120, 255, 255), 2);


	//Selecting red robots
	Mat red_pixels;
	//inRange(img, Vec3b(Hue_down, Saturation_down, Value_down), Vec3b(Hue_up, Saturation_up, Value_up), red_pixels); for debug
	inRange(img, Vec3b(10, 0, 0), Vec3b(160, 255, 255), red_pixels);
	bitwise_not(red_pixels, red_pixels);
	a = 'p';
	while (a != 'x') {
		if (a == 'e') erode(red_pixels, red_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		if (a == 'd') dilate(red_pixels, red_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		imshow("after both " + str, red_pixels);
		a = waitKey(0);
	}

	vector<vector<Point>> red_cnts;
	findContours(red_pixels.clone(), red_cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int min_red_dist = 10000, Xred, Yred;

	for (int i = 0; i < red_cnts.size(); i++)
	{
		if ((red_cnts[i].size() < 150) && (red_cnts[i].size() > 40))
		{
			Moments mnts = moments(red_cnts[i]);
			double m00 = mnts.m00;
			double m01 = mnts.m01;
			double m10 = mnts.m10;
			int Xc = int(m10 / m00);
			int Yc = int(m01 / m00);
			int dist = distance(Xc, Yc, Xlamp, Ylamp);
			if ((i == 0) || (dist < min_red_dist))
			{
				min_red_dist = dist;
				Xred = Xc;
				Yred = Yc;
			}
		}
		else
		{
			red_cnts.erase(red_cnts.begin() + i);
			i--;
		}
	}
	circle(target, { Xred, Yred }, 5, Scalar(0, 255, 255), -1);
	drawContours(target, red_cnts, -1, Scalar(0, 255, 255), 2);


	//Selecting green robots
	Mat green_pixels;
	inRange(img, Vec3b(40, 50, 133), Vec3b(90, 255, 255), green_pixels);
	a = 'p';
	while (a != 'x') {
		if (a == 'e') erode(green_pixels, green_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		if (a == 'd') dilate(green_pixels, green_pixels, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)), Point(-1, -1), 1);
		imshow("after both " + str, green_pixels);
		a = waitKey(0);
	}

	vector<vector<Point>> green_cnts;
	findContours(green_pixels.clone(), green_cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int min_green_dist, Xgreen, Ygreen;
	for (int i = 0; i < green_cnts.size(); i++)
	{
		if (green_cnts[i].size() > 49)
		{
			Moments mnts = moments(green_cnts[i]);
			double m00 = mnts.m00;
			double m01 = mnts.m01;
			double m10 = mnts.m10;
			int Xc = int(m10 / m00);
			int Yc = int(m01 / m00);
			int dist = distance(Xc, Yc, Xlamp, Ylamp);
			if ((i == 0) || (dist < min_green_dist))
			{
				min_green_dist = dist;
				Xgreen = Xc;
				Ygreen = Yc;
			}
		}
		else
		{
			green_cnts.erase(green_cnts.begin() + i);
			i--;
		}
	}
	circle(target, { Xgreen, Ygreen }, 5, Scalar(60, 255, 255), -1);
	drawContours(target, green_cnts, -1, Scalar(60, 255, 255), 2);


	cvtColor(target, target, CV_HSV2BGR);
	imshow("Target point " + str, target);
}

int distance(int X1, int Y1, int X2, int  Y2)
{
	return int(sqrt(pow((X1 - X2), 2) + pow((Y1 - Y2), 2)));
}