#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>

using namespace std;
using namespace cv;

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs);
static bool readDetectorParameters1(string filename, Ptr<aruco::DetectorParameters> &params);
int detect_markers(int argc, const char * const argv[]);
static bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params);
static bool saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
                             const Mat &cameraMatrix, const Mat &distCoeffs, double totalAvgErr);
int calibrate_camera(int argc, const char *const argv[]);

Mat draw_cube(Mat img, vector<cv::Point2f> corners, Vec3d rvec, Vec3d tvec, Mat camMatrix);
