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

#include "libs/samples.hpp"

using namespace std;
using namespace cv;

int main()
{
    const char  *const argv[] = {"", "-d=7", "-c", "-l=0.04", "-r", "-v", "-ci", "-dp", "-refine"};
    int argc = sizeof (&argv);
    detect_markers(argc, argv);
    return 0;
}
