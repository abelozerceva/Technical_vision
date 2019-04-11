#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco/dictionary.hpp>
#include <opencv2/core/utility.hpp>
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "samples.hpp"

#define PI 3.14159265359

using namespace std;
using namespace cv;

double n = 0.1;
double f = 40;
int widgh = 640;
int height = 480;
double sx = 0;
double sy = 0;
double view_angle = 90 * PI / 180;
double aspect = (double)widgh/height;

Mat init_peaks(Mat peaks_);

Mat draw_cube(Mat img, vector<cv::Point2f> corners, Vec3d rvec, Vec3d tvec, Mat camMatrix)
{
/*
    double perspective_matrix[][4] = {{1 / aspect * tan(view_angle/2), 0, 0, 0}, {0, 1/tan(view_angle/2), 0, 0},
                                     {0, 0, (f + n)/(f - n), 1}, {0, 0, -2*f*n/(f - n), 0}};
    int peaks_cube[][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 1}, {0, 1, 0, 1},
                          {0, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}};
    float view_port[][4] = {{(float)widgh/2, 0, 0, (float)widgh/2}, {0, (float)height/2, 0, (float)height/2},
                            {0, 0, 0, 0}, {0, 0, 0, 1}};
*/
    //cout << "corners = " << corners << endl;

    Mat peaks_cube = Mat::zeros(8, 4, CV_64F);
    Mat perspective_matrix = Mat::zeros(4, 4, CV_64F);
    Mat view_port = Mat::zeros(4, 4, CV_64F);
    peaks_cube = init_peaks(peaks_cube);

    //cout << "peaks_cube = " << peaks_cube << endl;

    Mat transform_rotate = Mat::zeros(3, 3, CV_64F);
    Rodrigues(rvec, transform_rotate);
    Mat rotate_matrix = Mat::zeros(4, 4, CV_64F);
    for (uint i = 0; i < transform_rotate.rows; i++)
    {
        for (uint j = 0; j < transform_rotate.cols; j++)
        {
            rotate_matrix.at<double>(i, j) = transform_rotate.at<double>(i, j);
        }
    }
    rotate_matrix.at<double>(0, 3) = tvec[0];
    rotate_matrix.at<double>(1, 3) = tvec[1];
    rotate_matrix.at<double>(2, 3) = tvec[2];
    rotate_matrix.at<double>(3, 3) = 1;

    Mat cube_trans = Mat::zeros(8, 4, CV_64F);
    Mat cube_trans1 = Mat::zeros(8, 4, CV_64F);

    for (uint i = 0; i < cube_trans.rows; i++)
    {
        for (uint j = 0; j < cube_trans.cols; j++)
        {
            for (uint k = 0; k < cube_trans.cols; k++)
            {
                cube_trans.at<double>(i, j) += rotate_matrix.at<double>(j, k) * peaks_cube.at<double>(i, k);
            }
        }
    }

    cube_trans1 = Mat::zeros(cube_trans.rows, 2, CV_32S);//= cube_trans;
    for (uint i = 0; i < cube_trans.rows; i++)
    {

        cube_trans1.at<int>(i, 0) = fabs((cube_trans.at<double>(i, 0) / cube_trans.at<double>(i, 2))
                * camMatrix.at<double>(0, 0) - camMatrix.at<double>(0, 2));

        cube_trans1.at<int>(i, 1) = fabs((cube_trans.at<double>(i, 1)/cube_trans.at<double>(i, 2))
                * camMatrix.at<double>(1, 1) - camMatrix.at<double>(1, 2));

    }

    cube_trans = Mat::zeros(cube_trans1.rows, cube_trans1.cols, CV_32S);
    cube_trans = cube_trans1;

    for (uint i = 0; i < 4; i += 2)
    {
        int temp_x = cube_trans1.at<int>(i, 0);
        int temp_y = cube_trans1.at<int>(i, 1);
        int err_x = corners[i + 1].x - temp_x;
        int err_y = corners[i + 1].y - temp_y;
        cube_trans.at<int>(i, 0) += err_x;
        cube_trans.at<int>(i, 1) += err_y;
        cube_trans.at<int>(i + 4, 0) += err_x;
        cube_trans.at<int>(i + 4, 1) += err_y;
        temp_x = cube_trans1.at<int>(i + 1, 0);
        temp_y = cube_trans1.at<int>(i + 1, 1);
        int err_x1 = corners[i].x - temp_x;
        int err_y1 = corners[i].y - temp_y;
        cube_trans.at<int>(i + 1, 0) += err_x1;
        cube_trans.at<int>(i + 1, 1) += err_y1;
        cube_trans.at<int>(i + 5, 0) += err_x1;
        cube_trans.at<int>(i + 5, 1) += err_y1;

    }//*/
/*
    cout << "end position: " << endl;
    for (uint i = 0; i < cube_trans.rows; i++)
    {
        for (uint j = 0; j < cube_trans.cols; j++)
        {
            int temp = cube_trans.at<int>(i, j);
            cout << "cube_trans.at<int>(" << i << ", " << j << ") = " << temp << endl;
        }
    }
//*/

    for (uint i = 0; i < 4; i++)
    {
        line(img, Point(cube_trans.at<int>(i, 0), cube_trans.at<int>(i, 1)),
             Point(cube_trans.at<int>(i + 4, 0), cube_trans.at<int>(i + 4, 1)), Scalar(0, 255/(4*i+1), 255), 4, 5);
    }
//*/
    for (uint i = 0; i < cube_trans.rows - 1; i++)
    {
        if (i == 3) //continue;
        {
            line(img, Point(cube_trans.at<int>(3, 0), cube_trans.at<int>(3, 1)),
                 Point(cube_trans.at<int>(0, 0), cube_trans.at<int>(0, 1)), Scalar(255, 0, 255), 4, 5);
            line(img, Point(cube_trans.at<int>(7, 0), cube_trans.at<int>(7, 1)),
                 Point(cube_trans.at<int>(4, 0), cube_trans.at<int>(4, 1)), Scalar(255, 255, 0), 4, 5);
            continue;
        }
        line(img, Point(cube_trans.at<int>(i, 0), cube_trans.at<int>(i, 1)),
             Point(cube_trans.at<int>(i + 1, 0), cube_trans.at<int>(i + 1, 1)), Scalar(255, 255/(4*i+1), 0), 4, 5);
    }
    circle(img, Point(cube_trans.at<int>(0, 0), cube_trans.at<int>(0, 1)), 7, Scalar(255, 200, 0), 3, 3);
    circle(img, Point(cube_trans.at<int>(1, 0), cube_trans.at<int>(1, 1)), 7, Scalar(0, 100, 255), 3, 3);//orange

    return img;
}

Mat init_peaks(Mat peaks_)
{
    Mat peaks = Mat::zeros(8, 4, CV_64F);
    peaks.at<double>(1, 0) = 1;
    peaks.at<double>(2, 0) = 1;
    peaks.at<double>(2, 1) = 1;
    peaks.at<double>(3, 1) = 1;
    peaks.at<double>(3, 3) = 1;
    peaks.at<double>(4, 2) = 1;
    peaks.at<double>(5, 0) = 1;
    peaks.at<double>(5, 2) = 1;
    peaks.at<double>(7, 1) = 1;
    peaks.at<double>(7, 2) = 1;

    for(uint8_t i = 0; i < 4; i++)
        peaks.at<double>(6, i) = 1;

    peaks *= 0.03;

    for(uint8_t i = 0; i < peaks.rows; i++)
        peaks.at<double>(i, 3) = 1;

    peaks_ = peaks;
    return peaks_;
}
