#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "samples.hpp"

using namespace std;
using namespace cv;

namespace {
const char* about = "Basic marker detection";
const char* keys  =
        "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
        "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
        "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
        "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16,"
        "DICT_APRILTAG_16h5=17, DICT_APRILTAG_25h9=18, DICT_APRILTAG_36h10=19, DICT_APRILTAG_36h11=20}"
        "{v        |       | Input from video file, if ommited, input comes from camera }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{c        |       | Camera intrinsic parameters. Needed for camera pose }"
        "{l        | 0.04   | Marker side lenght (in meters). Needed for correct scale in camera pose }"
        "{dp       |       | File of marker detector parameters }"
        "{r        |       | show rejected candidates too }"
        "{refine   |       | Corner refinement: CORNER_REFINE_NONE=0, CORNER_REFINE_SUBPIX=1,"
        "CORNER_REFINE_CONTOUR=2, CORNER_REFINE_APRILTAG=3}";
}

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs)
{
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    return true;
}

static bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params)
{
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["adaptiveThreshWinSizeMin"] >> params->adaptiveThreshWinSizeMin;
    fs["adaptiveThreshWinSizeMax"] >> params->adaptiveThreshWinSizeMax;
    fs["adaptiveThreshWinSizeStep"] >> params->adaptiveThreshWinSizeStep;
    fs["adaptiveThreshConstant"] >> params->adaptiveThreshConstant;
    fs["minMarkerPerimeterRate"] >> params->minMarkerPerimeterRate;
    fs["maxMarkerPerimeterRate"] >> params->maxMarkerPerimeterRate;
    fs["polygonalApproxAccuracyRate"] >> params->polygonalApproxAccuracyRate;
    fs["minCornerDistanceRate"] >> params->minCornerDistanceRate;
    fs["minDistanceToBorder"] >> params->minDistanceToBorder;
    fs["minMarkerDistanceRate"] >> params->minMarkerDistanceRate;
    fs["cornerRefinementMethod"] >> params->cornerRefinementMethod;
    fs["cornerRefinementWinSize"] >> params->cornerRefinementWinSize;
    fs["cornerRefinementMaxIterations"] >> params->cornerRefinementMaxIterations;
    fs["cornerRefinementMinAccuracy"] >> params->cornerRefinementMinAccuracy;
    fs["markerBorderBits"] >> params->markerBorderBits;
    fs["perspectiveRemovePixelPerCell"] >> params->perspectiveRemovePixelPerCell;
    fs["perspectiveRemoveIgnoredMarginPerCell"] >> params->perspectiveRemoveIgnoredMarginPerCell;
    fs["maxErroneousBitsInBorderRate"] >> params->maxErroneousBitsInBorderRate;
    fs["minOtsuStdDev"] >> params->minOtsuStdDev;
    fs["errorCorrectionRate"] >> params->errorCorrectionRate;
    return true;
}

int detect_markers(int argc, const char *const argv[])
{
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if(argc < 2) {
        parser.printMessage();
        return 0;
    }

    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
       return -1;
    Mat edges;

    Mat camMatrix, distCoeffs;
    double totalTime = 0;
    int totalIterations = 0;
    double tick = (double)getTickCount();

    int dictionaryId = parser.get<int>("d");
    bool showRejected = parser.has("r");
    bool estimatePose = parser.has("c");
    float markerLength = parser.get<float>("l");
    Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();

    Ptr<aruco::Dictionary> dictionary =
        aruco::getPredefinedDictionary(aruco::DICT_6X6_250);//aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    if (parser.has("refine"))
    {
        //override cornerRefinementMethod read from config file
        detectorParams->cornerRefinementMethod = parser.get<int>("refine");
    }

    while(cap.isOpened())
    {
       vector< int > ids;
       vector< vector< Point2f > > corners, rejected;
       vector< Vec3d > rvecs, tvecs;
       Mat image, imageCopy;
       Mat frame;
       cap >> frame; // get a new frame from camera
       image = frame.clone();
   //    cvtColor(image, edges, COLOR_BGR2GRAY);

       // detect markers and estimate pose
       aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);
       if(estimatePose && ids.size() > 0)
       {
           aruco::estimatePoseSingleMarkers(corners, markerLength, camMatrix, distCoeffs, rvecs,
                                            tvecs);
           //cout<<"rvecs[0] = "<<rvecs[0]<<endl;
           //cout<<"rvecs[0][1] = "<<rvecs[0][1]<<endl;
           //draw_cube(image, corners, rvecs, tvecs);
       }
       double currentTime = ((double)getTickCount() - tick) / getTickFrequency();
       totalTime += currentTime;
       totalIterations++;
       if(totalIterations % 30 == 0)
       {
           cout << "Detection Time = " << currentTime * 1000 << " ms "
                << "(Mean = " << 1000 * totalTime / double(totalIterations) << " ms)" << endl;
       }

       // draw results
       image.copyTo(imageCopy);
       if(ids.size() > 0)
       {
           aruco::drawDetectedMarkers(imageCopy, corners, ids);

           if(estimatePose) {
               for(unsigned int i = 0; i < ids.size(); i++)
                   aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvecs[i], tvecs[i],
                                   markerLength * 0.5f);
           }
       }

       if(showRejected && rejected.size() > 0)
           aruco::drawDetectedMarkers(imageCopy, rejected, noArray(), Scalar(100, 0, 255));

       imshow("Output", imageCopy);
       if(waitKey(30) >= 0) break;

    }
    return 0;
}
