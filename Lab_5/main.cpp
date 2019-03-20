#include <core/include/opencv2/core.hpp>
#include <highgui/include/opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <video/include/opencv2/video/tracking.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <ctime>

using namespace cv;
using namespace std;

int skeletonization(bool Flag, int cadre);
int Lines(Mat &img, bool flag);
void find_circles();
int process(VideoCapture& capture, bool flag);
void correct_img(Mat& img1, Mat& img2);
Mat morphs_frame(Mat& img);


const string videoname = "/home/anastasia/git/Lab_5/LR6/Video/11.avi";
static int cadre;
static Mat bin_img;
static Mat skelet;

int main()
{
    Mat img;
    Mat line_img;
    int point;
/*    if( !img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }*/
    VideoCapture capture(videoname, CAP_ANY);
 /*   if (!capture.isOpened())
    {
        cout << "Failed to open the video device, video file or image sequence!" << endl;
        return -1;
    }
    process(capture, false);*/
    cout
            <<"[info] cols: "<<img.cols<<" pixels"<<endl
            <<"[info] rows: "<<img.rows<<" pixels"<<endl
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
                bin_img = imread("/home/anastasia/git/Lab_5/002.jpeg", 0);//skelet.jpg
                img = imread("/home/anastasia/git/Lab_5/color.jpeg", 1);
                threshold(bin_img, bin_img, 100, 255, THRESH_BINARY);
                morphs_frame(bin_img);
                imshow("Image", img);
                imshow("Binary image", bin_img);
                waitKey(0);
                skelet = bin_img.clone();
                skeletonization(false, cadre);
                line_img = skelet.clone();
                Lines(line_img, false);
          //      morphs_frame(line_img);
                imshow("Skelet", skelet);
                imshow("Lines", line_img);
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
                cadre = process(capture, true);
                Mat img_bin = imread("/home/anastasia/git/Lab_5/Binary_video/002.jpeg", 0);
                img = imread("/home/anastasia/git/Lab_5/002.jpeg", 0);
                correct_img(img, img_bin);
                skeletonization(true, cadre);
                VideoCapture capture_skelet("/home/anastasia/git/Lab_5/line/%03d.jpeg");//"/home/anastasia/git/Lab_5/line/%03d.jpeg");
                if (!capture_skelet.isOpened())
                {
                    cout << "Failed to open the video device, video file or image sequence!" << endl;
                    return -1;
                }
                process(capture_skelet, false);
                destroyAllWindows();
                break;
            }
        }
        point = getchar( );
    }
    return 0;
}

int process(VideoCapture& capture, bool flag = false)
{
    Mat kernel1 = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    Mat kernel2 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    int n = 0, m = 0, cadre = 0;
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
            sprintf(filename,"/home/anastasia/git/Lab_5/original/%03d.jpeg",n);
            imwrite(filename, frame);
            imshow("Original video", frame);
            cvtColor(frame, frame, COLOR_RGB2GRAY);
            threshold(frame, frame, 100, 255, THRESH_BINARY);
            morphs_frame(frame);
            //morphologyEx(frame, frame, MORPH_OPEN, kernel, Point(-1, -1), 1);
            sprintf(filename,"/home/anastasia/git/Lab_5/Binary_video/%03d.jpeg",n++);
            imwrite(filename, frame);
            if (cadre == 2)
                    {
                        sprintf(filename,"/home/anastasia/git/Lab_5/002.jpeg");
                        imwrite(filename, frame);
                    }
            //binary_video<<frame;
        }
        imshow(window_name, frame);
        cadre++;
        char key = (char)waitKey(30); //delay N millis, usually long enough to display and capture input

        switch (key)
        {
            case 'q':
            case 'Q':
            case 27: //escape key
                return cadre;
            case ' ': //Save an image
                sprintf(filename,"filename%.3d.jpg",m++);
                imwrite(filename,frame);
                cout << "Saved " << filename << endl;
                break;
            default:
                break;
        }
    }
    capture.release();
//    binary_video.release();
    cout<<"cadre = "<<cadre<<endl;
    destroyAllWindows();
    return cadre;
}

int skeletonization(bool Flag, int cadre)
{
    Mat img1, img2;
    if (Flag == false)
    {
        img1 = skelet.clone();//imread("/home/anastasia/git/Lab_5/000.jpeg", 0);//capture.clone();bin_img.clone();//
        img2 = img1.clone();
        cadre = 1;
    }
    uint8_t point = 0;
    uint8_t step = 1;
    int16_t P[3][3];
    uint8_t A = 0, B = 0;
    int16_t grad[8];
    uint q1 = 0;
    char filename[200];
    char open_file[200];
    Mat deletet;
    for (int i = 0; i<cadre; i++)
    {
        bool flag = true;
        if (Flag == true)
        {
            sprintf(open_file, "/home/anastasia/git/Lab_5/Binary_video/%03d.jpeg", i);
            img1 = imread(open_file, 0);
            threshold(img1, img1, 100, 255, THRESH_BINARY);
            img2 = img1.clone();
        }
        if (img1.empty())
            break;
        while (flag == true)//make skelet
        {
            deletet = Mat::ones(img1.rows, img1.cols, img1.type());
            for (int i=0; i<img1.rows-2; i++)
            {
                for (int j=0; j<img1.cols-2; j++)
                {
                    A = 0;
                    B = 0;
                    point = 0;
                    if (img1.at<uchar>(i+1, j+1) != 0) //пиксель белый
                        point++;
                    else continue;
                    for (uint8_t k=0; k<3; k++)
                    {
                        for (uint8_t m=0; m<3; m++)
                        {
                            P[k][m] = img1.at<uchar>(i+k, j+m);
                            if (P[k][m] != 0) //img1.at<uchar>(i+k, j+m)
                                B++;
                        }
                    }
                    if (B>=3 && B<=7) //кол-во белых пикселей по соседству
                        point++;
                    else continue;
                    grad[0] = P[0][1]*!P[0][2]; //2<-3
                    grad[1] = P[0][2]*!P[1][2]; //3<-4
                    grad[2] = P[1][2]*!P[2][2]; //4<-5
                    grad[3] = P[2][2]*!P[2][1]; //5<-6
                    grad[4] = P[2][1]*!P[2][0]; //6<-7
                    grad[5] = P[2][0]*!P[1][0]; //7<-8
                    grad[6] = P[1][0]*!P[0][0]; //8<-9
                    grad[7] = P[0][0]*!P[0][1]; //9<-2
                    for (uint8_t p=0; p<8; p++)
                    {
                        if (grad[p] != 0)
                            A++;
                    }
                    if (A < 2) //переход от черного к белому
                        point++;
                    else continue;
                    switch (step)
                    {
                        case (1):
                        if (P[0][1]*P[1][2]*P[2][1] == 0 && P[1][2]*P[2][1]*P[1][0] == 0)
                        {
                            point++;
                            //step = 2;
                        }
                        break;
                        case (2):
                        if (P[0][1]*P[1][2]*P[1][0] == 0 && P[0][1]*P[2][1]*P[1][0] == 0)
                        {
                            point++;
                          //  step = 1;
                        }
                        break;
                    }
                    if (point == 4)
                        deletet.at<uchar>(i+1, j+1) = 0;
                  /*  imshow("Deleted", deletet*255);
                    waitKey(20);
                */}
            }
            if(step == 1)
                step = 2;
            else
                step = 1;
            q1 = 0;
            for (int i=0; i<deletet.rows; i++)
            {
                for (int j=0; j<deletet.cols; j++)
                {
                    if (deletet.at<uchar>(i, j) == 0)
                        q1++;
                    img2.at<uchar>(i, j) = img1.at<uchar>(i, j) * deletet.at<uchar>(i, j);
                }
            }
            if (q1 == 0)
                flag = false;
            img1 = img2;
    /*        imshow("Skelet", img1);
            waitKey(30);
      */  }
        if (Flag == true)
        {
            sprintf(filename, "/home/anastasia/git/Lab_5/Binary_video/%03d.jpeg", i);
            imwrite(filename, img1);
            Lines(img1, true);
            sprintf(filename, "/home/anastasia/git/Lab_5/line/%03d.jpeg", i);
            imwrite(filename, img1);
        }
        else
        {
            sprintf(filename, "/home/anastasia/git/Lab_5/skelet.jpeg");
            imwrite(filename, img1);
            skelet = img1;
        }
    }
    return 0;
}

int Lines(Mat &img, bool flag)
{
    static int n = 0;
    char open_file[200];
    size_t i = 0;
    vector <Vec4i> lines;
    HoughLinesP(img, lines, 1, CV_PI/180, 18, 0, 120);
    if (flag == false)
        img = imread("/home/anastasia/git/Lab_5/color.jpeg", 1);//Mat::zeros(img.rows, img.cols, img.type());
    else
    {
        sprintf(open_file, "/home/anastasia/git/Lab_5/original/%03d.jpeg", n++);
        img = imread(open_file, 1);
    }
    if (img.empty())
        return -1;
    for ( i = 0; i < lines.size(); i++)
    {
        Vec4i l = lines[i];
        line(img, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 255), 4, 5);
    }
    cout<<"lines.size() = "<<lines.size()<<endl;
    return 0;
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

Mat morphs_frame(Mat& img)
{
    Mat kernel1 = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    Mat kernel2 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    dilate(img, img, kernel2, Point(-1, -1), 2);
    erode(img, img, kernel2, Point(-1, -1), 2);
    dilate(img, img, kernel1, Point(-1, -1), 1);
    erode(img, img, kernel1, Point(-1, -1), 1);
    dilate(img, img, kernel1, Point(-1, -1), 1);
    erode(img, img, kernel1, Point(-1, -1), 1);
    return img;
}

void correct_img(Mat &img1, Mat &img2)
{
    u_char img_1 = 0, img_2 = 0;
    int Overlap = 0;
    for(int i=0; i<img2.cols; i++)
    {
        for(int j=0; j<img2.rows; j++)
        {
            img_1 = img1.at<uchar>(j,i);
            img_2 = img2.at<uchar>(j,i);
            int value = (img_1 - img_2);
            if(abs(value)<2)
                Overlap++;
        }
    }
    cout<<"Correctness of images = "<<Overlap*100/(img1.cols*img1.rows)<<" %"<<endl;
}
