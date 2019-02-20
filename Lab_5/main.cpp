#include <core/include/opencv2/core.hpp>
#include <highgui/include/opencv2/highgui.hpp>
#include <imgproc/include/opencv2/imgproc.hpp>
#include <imgcodecs/include/opencv2/imgcodecs.hpp>
#include <video/include/opencv2/video/tracking.hpp>
#include <objdetect/include/opencv2/objdetect.hpp>
#include <videoio/include/opencv2/videoio.hpp>
#include <iostream>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <ctime>

using namespace cv;
using namespace std;

int skeletonization(VideoCapture& capture);
Mat Lines(Mat &img);
void find_circles();
int process(VideoCapture& capture, bool flag);

const string videoname = "/home/anastasia/git/Lab_5/LR6/Video/1.avi";

VideoWriter binary_video;

int main()
{
    string img_name = "/home/anastasia/git/Lab_5/3.jpeg";
    Mat img = imread(img_name, 0);
    Mat bin_img;
    Mat skelet;
    Mat line_img;
    int max = 0;
    int point;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    if( !img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }
    VideoCapture capture(videoname, CAP_ANY);
//    VideoCapture capture_skelet(videoname, CAP_ANY);
    if (!capture.isOpened())
    {
        cout << "Failed to open the video device, video file or image sequence!" << endl;
        return -1;
    }
    cout
            <<"[info] cols: "<<img.cols<<" pixels"<<endl
            <<"[info] rows: "<<img.rows<<" pixels"<<endl;

    for (int i=0; i<img.cols; i++)
    {
        for (int j=0; j<img.rows; j++)
        {
             if(img.at<uchar>(j, i)>max)
                 max = img.at<uchar>(j, i);
        }

    }
    threshold(img, bin_img, 80, 255, THRESH_BINARY);
 /*   dilate(bin_img, bin_img, kernel, Point(-1, -1), 1);
    erode(bin_img, bin_img, kernel, Point(-1, -1), 1);
*/    morphologyEx(bin_img, bin_img, MORPH_OPEN, kernel, Point(-1, -1), 1);
    cout
            <<"Please enter number: "<<endl
            <<"1 - Find Skelet and lines;"<<endl
            <<"2 - Find Circles;"<<endl
            <<"3 - Test with video"<<endl
            <<"0 - Exit."<<endl;

    point = getchar( );
    while ( point != '0' )
    {
        switch ( point )
        {
            case '1':
            {
                skelet = bin_img.clone();
              //  skeletonization(skelet);
             /*   line_img = skelet.clone();
                Lines(line_img);
               */ imshow("Image", img);
                imshow("Binary", bin_img);
              //  imshow("Skelet",skelet);
              //  imshow("Lines", line_img);
                waitKey(0);
                destroyAllWindows( );
                break;
            }

            case '2':
            {
                find_circles();
                break;
            }
            case '3':
            {
                process(capture, true);
                VideoCapture capture_skelet("/home/anastasia/git/Lab_5/Binary_video.avi");
                if (!capture_skelet.isOpened())
                {
                    cout << "Failed to open the video device, video file or image sequence!" << endl;
                    return -1;
                }
                //skeletonization(capture_skelet);
                process(capture_skelet, false);
            }
        }
        point = getchar( );
    }
    return 0;
}

int process(VideoCapture& capture, bool flag = false)
{
    if (flag == true)
    {
        int codec = VideoWriter::fourcc('M','J','P','G');//('P','I','M','1');
        binary_video.open("/home/anastasia/git/Lab_5/Binary_video.avi", codec, (int)capture.get(CAP_PROP_FPS),
                          Size((int)capture.get(CAP_PROP_FRAME_WIDTH), (int)capture.get(CAP_PROP_FRAME_HEIGHT)), true);
        if (!binary_video.isOpened())
        {
            cout << "Failed to open the video to write!" << endl;
            return -1;
        }
    }
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    int n = 0;
    char filename[200];
    string window_name = "Video | q or esc to quit";
    cout << "Press space to save a picture. q or esc to quit" << endl;
    namedWindow(window_name, WINDOW_KEEPRATIO); //resizable window;
    Mat frame;

    for (;;)
    {
        capture >> frame;
        if (frame.empty())
        {
            cout<<"Frame is empty!"<<endl;
            break;
        }
        if (flag == true)
        {
            cvtColor(frame, frame, COLOR_RGB2GRAY);
            threshold(frame, frame, 80, 255, THRESH_BINARY);
            morphologyEx(frame, frame, MORPH_OPEN, kernel, Point(-1, -1), 1);
            binary_video<<frame;
        }
        imshow(window_name, frame);
        char key = (char)waitKey(30); //delay N millis, usually long enough to display and capture input

        switch (key)
        {
            case 'q':
            case 'Q':
            case 27: //escape key
                return 0;
            case ' ': //Save an image
                sprintf(filename,"filename%.3d.jpg",n++);
                imwrite(filename,frame);
                cout << "Saved " << filename << endl;
                break;
            default:
                break;
        }
    }
    capture.release();
    destroyAllWindows();
    return 0;
}

int skeletonization(VideoCapture &capture)
{
    int codec = VideoWriter::fourcc('M','J','P','G');//('P','I','M','1');
    binary_video.open("/home/anastasia/git/Lab_5/Skelet_video.avi", codec, (int)capture.get(CAP_PROP_FPS),
                      Size((int)capture.get(CAP_PROP_FRAME_WIDTH), (int)capture.get(CAP_PROP_FRAME_HEIGHT)), true);
    if (!binary_video.isOpened())
    {
        cout << "Failed to open the video to write!" << endl;
        return -1;
    }
    Mat img1, img2;
//    img1 = capture.clone();
//    img2 = capture.clone();
    bool flag = true;
    uint8_t point = 0;
    uint8_t step = 1;
    int16_t P[3][3];
    uint8_t A = 0, B = 0;
    int16_t grad[8];
    uint q1 = 0, q2 = 0;

    for (;;)
    {
        capture >> img1;
        if (img1.empty())
            break;
        while (flag == true)//make skelet
        {
            for (int i=0; i<img1.rows; i++)
            {
                for (int j=0; j<img1.cols; j++)
                {
                    A = 0;
                    B = 0;
                    point = 0;
                    if (img1.at<uchar>(i+1, j+1) == 255) //пиксель белый
                        point++;
                    else continue;
                    for (uint8_t k=0; k<3; k++)
                    {
                        for (uint8_t m=0; m<3; m++)
                        {
                            P[k][m] = img1.at<uchar>(i+k, j+m);
                            if (P[k][m] == 255) //img1.at<uchar>(i+k, j+m)
                                B++;
                        }
                    }
                    if (B>=3 && B<=7) //кол-во белых пикселей по соседству
                        point++;
                    else continue;
                    grad[0] = P[0][0]*!P[0][1]; //9->2
                    grad[1] = P[0][1]*!P[0][2]; //2->3
                    grad[2] = P[0][2]*!P[1][2]; //3->4
                    grad[3] = P[1][2]*!P[2][2]; //4->5
                    grad[4] = P[2][2]*!P[2][1]; //5->6
                    grad[5] = P[2][1]*!P[2][0]; //6->7
                    grad[6] = P[2][0]*!P[1][0]; //7->8
                    grad[7] = P[1][0]*!P[0][0]; //8->9
                    for (uint8_t p=0; p<8; p++)
                    {
                        if (grad[p] == 255)
                            A++;
                    }
                    if (A == 1) //переход от черного к белому
                        point++;
                    else continue;
                    switch (step)
                    {
                        case (1):
                        if (P[0][1]*P[1][2]*P[2][1] == 0 && P[1][2]*P[2][1]*P[1][0] == 0)
                        {
                            point++;
                            step = 2;
                        }
                        break;
                        case (2):
                        if (P[0][1]*P[1][2]*P[1][0] == 0 && P[0][1]*P[2][1]*P[1][0] == 0)
                        {
                            point++;
                            step = 1;
                        }
                        break;
                    }
                    if (point == 4)
                        img2.at<uchar>(i+1, j+1) = 0;
                }
            }
            q1 = 0;
            for (int i=0; i<img1.rows; i++)
            {
                for (int j=0; j<img1.cols; j++)
                {
                    if (img1.at<uchar>(i, j) - img2.at<uchar>(i, j) == 0)
                        q1++;
                }
            }
            if (q1 - q2 == 0)
                flag = false;
            else
                img1 = img2.clone();
            q2 = q1;
        }
        binary_video<<img1;
        char key = (char)waitKey(0);
        switch (key)
        {
         case 'q':
         case 'Q':
         case 27: //escape key
             return 0;
        }
    }
//    capture = img2.clone();
//    imshow("Skelet",capture);
    capture.release();
//    waitKey(0);
    destroyAllWindows();
    return 0;
}

Mat Lines(Mat &img)
{
    size_t i = 0;
    Mat line_image = Mat::ones(img.rows, img.cols, 0);
    vector <Vec4i> lines;
    HoughLinesP(img, lines, 1, CV_PI / 180, 4, 0, 30);
    vector <Vec2i> point_lines;
    for ( i = 0; i < lines.size(); i++)
    {

        Vec4i l = lines[i];
        line(line_image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 3, 5);
       /* cout<<"Point "<<i<<endl;
        int x = lines[i][0];
        int y = lines[i][1];
        Point p_l = Point(x, y);
        //point_lines[i] = p_l;
        point_lines[i].x = p_l.x;
        point_lines[i].y = p_l.y;
    */}
    const Point *pts = (const Point*) Mat(lines).data;
    int npts = Mat(lines).rows;
 //   cout<<"size point lines = "<<npts<<endl;
//   polylines(line_image, &pts, &npts, 1, false, Scalar(255), 3, 5);
    imshow("Lines", line_image);
    waitKey(0);
    destroyAllWindows();
    return line_image;
}

void find_circles()
{
    Mat img = imread("/home/anastasia/git/Lab_5/2.jpeg", 1);
    Mat binary = imread("/home/anastasia/git/Lab_5/2.jpeg", 0);
    Mat binary1 = imread("/home/anastasia/git/Lab_5/2.jpeg", 0);
    Mat img1 = img.clone(), temp = img.clone();
    Mat lat, nik, channel[3];

    threshold(binary, binary, 160, 255, THRESH_BINARY_INV);
    threshold(binary1, binary1, 160, 255, THRESH_BINARY);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

    erode(binary1, binary1, kernel, Point(-1, -1), 1);
    dilate(binary1, binary1, kernel, Point(-1, -1), 4);

    dilate(binary, binary, kernel, Point(-1, -1), 1);
    erode(binary, binary, kernel, Point(-1, -1), 5);

    vector <Vec3f> circle_nick, circle_brass;

    if( !img.data )
    {
        cout <<  "Could not open or find the image" << endl;
    }

    split(img1, channel);
    for (uint8_t i = 0; i<3; i++)
    {
        channel[i] -= binary1;
    }
    merge(channel, 3, img1);

    cvtColor(img1, img1, COLOR_BGR2HSV);
    inRange(img1, Vec3b(20, 175, 80), Vec3b(70, 255, 255), lat);
  //  inRange(img1, Vec3b(7, 50, 55), Vec3b(19, 200, 255), nik);

    dilate(lat, lat, kernel, Point(-1, -1), 7);
    erode(lat, lat, kernel, Point(-1, -1), 1);

    nik = binary - lat;
    erode(nik, nik, kernel, Point(-1, -1), 2);
    dilate(nik, nik, kernel, Point(-1, -1), 6);

    HoughCircles(lat, circle_brass, HOUGH_GRADIENT, 3, lat.cols/6);
    for( size_t i = 0; i < circle_brass.size(); i++ )
    {
        //cout<<"binary brass"<<i+1<<circle_brass[i]<<endl;
        Vec3i c = circle_brass[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( temp, center, 1, Scalar(0, 0, 255), 3, 8);
        // circle outline
        int radius = c[2];
        circle( temp, center, radius, Scalar(255, 255, 0), 3, 8);
        putText(temp, "Brass", center, FONT_HERSHEY_SIMPLEX, 0.6, Scalar(201, 27, 228), 2);
    }

    HoughCircles(nik, circle_nick, HOUGH_GRADIENT, 3, lat.cols/6);
    for( size_t i = 0; i < circle_nick.size(); i++ )
    {
        //cout<<"binary nickel"<<i+1<<circle_nick[i]<<endl;
        Vec3i c = circle_nick[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( temp, center, 1, Scalar(0, 0, 255), 3, 8);
        // circle outline
        int radius = c[2];
        circle( temp, center, radius, Scalar(0, 255, 255), 3, 8);
        putText(temp, "Nickel", center, FONT_HERSHEY_SIMPLEX, 0.6, Scalar(60, 179, 113), 2);
    }

    imshow("Original", img);
/*    imshow("Circles", binary1);
    imshow("Money", img1);
    imshow("binary brass", lat);
    imshow("binary nickel", nik);
  */  imshow("Materials", temp);

    waitKey(0);
    destroyAllWindows();
}
