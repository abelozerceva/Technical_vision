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
    const char  *const argv1[] = {"", "-@outfile=calibration.xml", "-w=5", "-h=7", "-s=0.01", "-d=7", "-l=0.04", "-rs=false",
                                  "-v", "-ci=0", "-dp", "-zt=false", "-a", "-pc=false"};
    int argc1 = sizeof (&argv1);
    const char  *const argv2[] = {"", "-d=10", "-c", "-l=0.04", "-r", "-v", "-ci", "-dp", "-refine"};
    int argc2 = sizeof (&argv2);
    //calibrate_camera(argc1, argv1);
    detect_markers(argc2, argv2);
    return 0;
}
