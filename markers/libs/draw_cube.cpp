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

int draw_cube(Mat img, vector<vector<Point2f>> corners, vector<Vec3d> rvec, vector<Vec3d> tvec)
{
    double n = 0.1;
    int f = 100;
    int widgh = 640;
    int height = 480;
    double view_angle = PI/2;
    double aspect = widgh/height;

    double perspective_matrix[][4] = {{1, aspect * tan(view_angle/2), 0, 0}, {0, 1/tan(view_angle/2), 0, 0},
                                     {0, 0, (f + n)/(f - n), 1}, {0, 0, -2*f*n/(f - n), 0}};
    int peaks_cube[][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 1}, {0, 1, 0, 1},
                          {0, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}};
    float view_port[][4] = {{(float)widgh/2, 0, 0, (float)widgh/2}, {0, (float)height/2, 0, (float)height/2},
                            {0, 0, 0, 0}, {0, 0, 0, 1}};
    rvec[0][0, 0, 0] *= -1;
    return 0;
}
