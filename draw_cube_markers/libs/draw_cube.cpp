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

double n = 0.01;
double f = 0.1;
int widgh = 640/2;
int height = 480/2;
double view_angle = PI/2;
double aspect = widgh/height;

Mat init_peaks(Mat peaks);
Mat init_perspective(Mat perspective);
Mat init_view(Mat view);

Mat draw_cube(Mat img, vector<cv::Point2f> corners, Vec3d rvec, Vec3d tvec, Mat camMatrix)
{
/*
    double perspective_matrix[][4] = {{1, aspect * tan(view_angle/2), 0, 0}, {0, 1/tan(view_angle/2), 0, 0},
                                     {0, 0, (f + n)/(f - n), 1}, {0, 0, -2*f*n/(f - n), 0}};
    int peaks_cube[][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 1}, {0, 1, 0, 1},
                          {0, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}};
    float view_port[][4] = {{(float)widgh/2, 0, 0, (float)widgh/2}, {0, (float)height/2, 0, (float)height/2},
                            {0, 0, 0, 0}, {0, 0, 0, 1}};
*/
    Mat peaks_cube = Mat::zeros(8, 4, CV_8U);
    Mat perspective_matrix = Mat::zeros(4, 4, CV_64F);
    Mat view_port = Mat::zeros(4, 4, CV_64F);
    peaks_cube = init_peaks(peaks_cube);
    perspective_matrix = init_perspective(perspective_matrix);
    view_port = init_view(view_port);
/*
    for (uint8_t i = 0; i < camMatrix.rows; i++)
    {
        for (uint8_t j = 0; j < camMatrix.cols; j++)
        {
            perspective_matrix.at<double>(i, j) = camMatrix.at<double>(i, j);
        }
    }
    perspective_matrix.at<double>(3, 3) = 1;
//*//*
    rvec[0, 0, 0] *= -1;
    rvec[0, 0, 1] *= -1;
    rvec[0, 0, 2] *= -1;
//*/
    Mat transform_rotate = Mat::zeros(3, 3, CV_64F);
    Rodrigues(rvec, transform_rotate);
    Mat rotate_matrix = Mat::zeros(4, 4, CV_64F);
    for (uint8_t i = 0; i < transform_rotate.rows; i++)
    {
        for (uint8_t j = 0; j < transform_rotate.cols; j++)
        {
            rotate_matrix.at<double>(i, j) = transform_rotate.at<double>(i, j);
        }
    }
    rotate_matrix.at<double>(0, 3) = tvec[0, 0, 0];
    rotate_matrix.at<double>(1, 3) = tvec[0, 0, 1];
    rotate_matrix.at<double>(2, 3) = tvec[0, 0, 2];
    rotate_matrix.at<double>(3, 3) = 1;

    Mat cube_trans = Mat::zeros(8, 4, CV_64F);
    Mat cube_trans1 = Mat::zeros(8, 4, CV_64F);
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            for (uint8_t k = 0; k < cube_trans.cols; k++)
            {
                cube_trans.at<double>(i, j) += rotate_matrix.at<double>(j, k) * peaks_cube.at<int8_t>(i, k);
            }
        }
    }
/*
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            cube_trans.at<double>(i, j) /= cube_trans1.at<double>(i, 2);
        }
    }
//*/
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            for (uint8_t k = 0; k < cube_trans.cols; k++)
            {
                cube_trans.at<double>(i, j) += perspective_matrix.at<double>(j, k) * cube_trans1.at<double>(i, k);
            }
        }
    }
/*
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            cube_trans.at<double>(i, j) /= cube_trans1.at<double>(i, 2);
        }
    }
//*/
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            for (uint8_t k = 0; k < cube_trans.cols; k++)
            {
                cube_trans.at<double>(i, j) += view_port.at<double>(j, k) * cube_trans1.at<double>(i, k);
            }
        }
    }
    //cout<<"cube_trans-1 = "<<cube_trans<<endl;
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            cube_trans.at<double>(i, j) /= cube_trans1.at<double>(i, 3);
        }
    }
/*
    cube_trans1 = cube_trans;
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            cube_trans.at<double>(i, j) *= cube_trans1.at<double>(i, 2);
        }
    }
//*/
/*
    cout
            <<"before abs"<<endl
            <<"cube_trans.at<uint>(0, 0) = "<<cube_trans.at<uint>(0, 0)<<endl
            <<"cube_trans.at<uint>(0, 1) = "<<cube_trans.at<uint>(0, 1)<<endl
            <<"cube_trans.at<uint>(0, 2) = "<<cube_trans.at<uint>(0, 2)<<endl
            <<"cube_trans.at<uint>(1, 0) = "<<cube_trans.at<uint>(1, 0)<<endl
            <<"cube_trans.at<uint>(1, 1) = "<<cube_trans.at<uint>(1, 1)<<endl
            <<"cube_trans.at<uint>(1, 2) = "<<cube_trans.at<uint>(1, 2)<<endl;
//*/
    cube_trans1 = cube_trans;
    cube_trans = Mat::zeros(cube_trans.rows, cube_trans.cols, CV_16U);
    for (uint8_t i = 0; i < cube_trans.rows; i++)
    {
        for (uint8_t j = 0; j < cube_trans.cols; j++)
        {
            cube_trans.at<uint>(i, j) = 0.5*fabs(cube_trans1.at<double>(i, j));
        }
    }

    for (uint8_t i = 0; i < 4; i += 2)
    {
        uint err_x = corners[i + 1].x - cube_trans.at<uint>(i, 0);
        uint err_y = corners[i + 1].y - cube_trans.at<uint>(i, 1);
        cube_trans.at<uint>(i, 0) += err_x;
        cube_trans.at<uint>(i, 1) += err_y;
        cube_trans.at<uint>(i + 4, 0) += err_x;
        cube_trans.at<uint>(i + 4, 1) += err_y;
        err_x = corners[i].x - cube_trans.at<uint>(i + 1, 0);
        err_y = corners[i].y - cube_trans.at<uint>(i + 1, 1);
        cube_trans.at<uint>(i + 1, 0) += err_x;
        cube_trans.at<uint>(i + 1, 1) += err_y;
        cube_trans.at<uint>(i + 5, 0) += err_x;
        cube_trans.at<uint>(i + 5, 1) += err_y;

    }
/*
    cout
            <<"finish"<<endl
            <<"cube_trans.at<uint>(0, 0) = "<<cube_trans.at<uint>(0, 0)<<endl
            <<"cube_trans.at<uint>(0, 1) = "<<cube_trans.at<uint>(0, 1)<<endl
            <<"cube_trans.at<uint>(0, 2) = "<<cube_trans.at<uint>(0, 2)<<endl
            <<"cube_trans.at<uint>(1, 0) = "<<cube_trans.at<uint>(1, 0)<<endl
            <<"cube_trans.at<uint>(1, 1) = "<<cube_trans.at<uint>(1, 1)<<endl
            <<"cube_trans.at<uint>(1, 2) = "<<cube_trans.at<uint>(1, 2)<<endl;
//*/

    for (uint8_t i = 0; i < 4; i++)
    {
        line(img, Point(cube_trans.at<uint>(i, 0), cube_trans.at<uint>(i, 1)),
             Point(cube_trans.at<uint>(i + 4, 0), cube_trans.at<uint>(i + 4, 1)), Scalar(0, 255/(4*i+1), 255), 4, 5);
    }

    for (uint8_t i = 0; i < cube_trans.rows - 1; i++)
    {
        if (i == 3) //continue;
        {
            line(img, Point(cube_trans.at<uint>(3, 0), cube_trans.at<uint>(3, 1)),
                 Point(cube_trans.at<uint>(0, 0), cube_trans.at<uint>(0, 1)), Scalar(255, 0, 255), 4, 5);
            line(img, Point(cube_trans.at<uint>(7, 0), cube_trans.at<uint>(7, 1)),
                 Point(cube_trans.at<uint>(4, 0), cube_trans.at<uint>(4, 1)), Scalar(255, 255, 0), 4, 5);
            continue;
        }
        line(img, Point(cube_trans.at<uint>(i, 0), cube_trans.at<uint>(i, 1)),
             Point(cube_trans.at<uint>(i + 1, 0), cube_trans.at<uint>(i + 1, 1)), Scalar(255, 255/(i+1), 0), 4, 5);
    }

    return img;
}

Mat init_peaks(Mat peaks)
{
    peaks.at<int8_t>(0, 3) = 1;
    peaks.at<int8_t>(1, 0) = 1;
    peaks.at<int8_t>(1, 3) = 1;
    peaks.at<int8_t>(2, 0) = 1;
    peaks.at<int8_t>(2, 1) = 1;
    peaks.at<int8_t>(2, 3) = 1;
    peaks.at<int8_t>(3, 1) = 1;
    peaks.at<int8_t>(3, 3) = 1;
    peaks.at<int8_t>(4, 2) = 1;
    peaks.at<int8_t>(4, 3) = 1;
    peaks.at<int8_t>(5, 0) = 1;
    peaks.at<int8_t>(5, 2) = 1;
    peaks.at<int8_t>(5, 3) = 1;
    peaks.at<int8_t>(7, 1) = 1;
    peaks.at<int8_t>(7, 2) = 1;
    peaks.at<int8_t>(7, 3) = 1;

    for(uint8_t i = 0; i < 4; i++)
        peaks.at<int8_t>(6, i) = 1;

    return peaks;
}

Mat init_perspective(Mat perspective)
{
    perspective.at<double>(0, 0) = 1;
    perspective.at<double>(0, 1) = 1/(aspect * tan(view_angle/2));
    perspective.at<double>(1, 1) = 1/tan(view_angle/2);
    perspective.at<double>(2, 2) = (f + n)/(f - n);
    perspective.at<double>(2, 3) = 1;
    perspective.at<double>(3, 2) = -2*f*n/(f - n);
    return perspective;
}

Mat init_view(Mat view)
{
    view.at<double>(0, 0) = (double)widgh/2;
    view.at<double>(0, 3) = (double)widgh/2;
    view.at<double>(1, 1) = (double)height/2;
    view.at<double>(1, 3) = (double)height/2;
    view.at<double>(3, 3) = 1;
    return view;
}