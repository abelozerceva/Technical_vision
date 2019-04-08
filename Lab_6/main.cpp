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

#define PI 3.14159265359

using namespace std;
using namespace cv;

int process(VideoCapture& capture);
Mat grid(Mat &img);
Point2d map_pixel(int col, int row);

const string videoname = "/home/anastasia/git/Lab_6/22.mp4";//.avi";
int width_map = 600;
int height_map = 600;
float d = -0.25;
double cx, cy, fx, fy;

int main()
{
    Mat img;
    Mat transit_matrix = Mat::zeros(4, 4, CV_64F);
    double angle = 74 * PI / 180;
    VideoCapture capture(videoname, CAP_ANY);
    if (!capture.isOpened())
    {
        cout << "Failed to open the video device, video file or image sequence!" << endl;
        return -1;
    }
    capture >> img;
    cx = img.cols / 2;
    cy = img.rows / 2;
    fx = tan(angle) * cx;
    fy = fx;
    transit_matrix.at<double>(0, 0) = fx;
    transit_matrix.at<double>(1, 1) = fy;
    transit_matrix.at<double>(0, 3) = cx;
    transit_matrix.at<double>(1, 3) = cy;
    transit_matrix.at<double>(3, 3) = 1;

    process(capture);
    return 0;
}

int process(VideoCapture& capture)
{
    Mat kernel = Mat::ones(3, 3, CV_8U);
    Point2d dot;
    Point2i dot1 = Point2i(0, 0);
    Mat frame;
    Mat frame_copy;
    Mat map;
    for (;;)
    {
        capture >> frame;
        if (frame.empty())
            break;
        frame_copy = Mat::zeros(frame.rows, frame.cols, CV_8U);
        imshow("Original video", frame);
        cvtColor(frame, frame_copy, COLOR_BGR2GRAY);
        threshold(frame_copy, frame_copy, 135, 255, THRESH_BINARY);
        dilate(frame_copy, frame_copy, kernel, Point(-1, -1), 2);
        erode(frame_copy, frame_copy, kernel, Point(-1, -1), 2);
        //imshow("Binary video", frame_copy);
        map = Mat::zeros(height_map, width_map, CV_8UC3);
        map = grid(map);
        for (int i = 0; i < frame.rows; i++)
        {
            for (int j = 0; j < frame.cols; j++)
            {
                if (frame_copy.at<uchar>(i, j) != 0)
                {
                    dot = map_pixel(j, i);
                    dot1.x = (int)(width_map - (dot.y) * 100) % width_map;
                    dot1.y = (int)(height_map / 2 - (dot.x) * 100) % height_map;
                    map.at<Vec3b>(dot1.y, dot1.x) = {0, 255, 255};
                }
            }
        }
        rotate(map, map, 2);
        imshow("Map", map);
        char key = (char)waitKey(30); //delay N millis, usually long enough to display and capture input
        if(key >= 0) break;
    }
    capture.release();
    destroyAllWindows();
    return 0;
}

Mat grid(Mat &img)
{
    for (int x = 0; x < img.cols; x = x + 20)
        line(img, Point(0, x), Point(img.rows, x), Scalar(100, 100, 100), 1, 1);
    for (int y = 0; y < img.rows; y += 20)
        line(img, Point(y, 0), Point(y, img.cols), Scalar(100, 100, 100), 1, 1);
    return img;
}

Point2d map_pixel(int col, int row)
{
    Point2d dot = Point2d (0, 0);
    double px = (col - cx) / fx;
    double py = (row - cy) / fy;
    double pz = 1;
    double mod = pow((pow(px, 2.0) + pow(py, 2.0) + pow(pz, 2.0)), 0.5);
    double ex = px / mod;
    double ey = py / mod;
    double ez = pz / mod;
    double k = -d / ey; //т. к. n = [0 1 0]
    dot.x = k * ex;
    dot.y = k * ez;
    return dot;
}