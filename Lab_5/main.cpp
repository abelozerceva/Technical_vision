#include <core/include/opencv2/core.hpp>
#include <highgui/include/opencv2/highgui.hpp>
#include <imgproc/include/opencv2/imgproc.hpp>
#include <imgcodecs/include/opencv2/imgcodecs.hpp>
#include <video/include/opencv2/video/tracking.hpp>
#include <objdetect/include/opencv2/objdetect.hpp>
#include <iostream>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <ctime>

using namespace cv;
using namespace std;

Mat skeletonization(Mat &img);
Mat Lines(Mat &img);
void find_circles();

int main()
{
    Mat img = imread("/home/anastasia/git/Lab_5/1.jpeg", 0);
    Mat bin_img;
    Mat skelet;
    Mat line_img;
    int max = 0;
    int point;

    if( !img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }

 /*   Size size(0.5*img.cols, 0.5*img.rows);
    resize(img, img, size);
   */ cout<<"[info] cols: "<<img.cols<<" pixels"<<endl;
    cout<<"[info] rows: "<<img.rows<<" pixels"<<endl;

    for (int i=0; i<img.cols; i++)
    {
        for (int j=0; j<img.rows; j++)
        {
             if(img.at<uchar>(j, i)>max)
                 max = img.at<uchar>(j, i);
        }

    }
    cout<<"Max Value = "<<max<<endl;
    threshold(img, bin_img, 180, 255, THRESH_BINARY_INV);

    cout<<"Please enter number: "<<endl;
    cout<<"1 - Find Skelet and lines;"<<endl;
    cout<<"2 - Find Circles;"<<endl;
    cout<<"0 - Exit."<<endl;

    point = getchar( );
    while ( point != '0' )
    {
        switch ( point )
        {
            case '1':
            {
                skelet = bin_img.clone();
                skeletonization(skelet);
                line_img = skelet.clone();
                Lines(line_img);
                imshow("Image", img);
                imshow("Binary", bin_img);
                imshow("Skelet",skelet);
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
        }
        point = getchar( );
    }
    return 0;
}

Mat skeletonization(Mat &img)
{
    Mat img1, img2;
    img1 = img.clone();
    img2 = img.clone();
    bool flag = true;
    uint8_t point = 0;
    uint8_t step = 1;
    int16_t P[3][3];
    uint8_t A = 0, B = 0;
    int16_t grad[8];
    uint q1 = 0, q2 = 0;

    while (flag == true)
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

    img = img2.clone();
    return img;
}

Mat Lines(Mat &img)
{
    vector <Vec4i> lines;
    HoughLinesP(img, lines, 1, CV_PI / 180, 50, 50, 10);
    for (size_t i = 0; i < lines.size(); i++)
    {
        Vec4i l = lines[i];
        line(img, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, 8);
    }
    return img;
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
