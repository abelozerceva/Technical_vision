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
    Mat img1 = imread("/home/anastasia/git/Lab_5/2.jpeg", 0);
   // adaptiveThreshold(img1, img1, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 5);
    threshold(img1, img1, 160, 255, THRESH_BINARY_INV);
    Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
    erode(img1, img1, kernel, Point(-1, -1), 1);
    dilate(img1, img1, kernel, Point(-1, -1), 2);
    vector <Vec3f> circles;

    if( !img.data )
    {
        cout <<  "Could not open or find the image" << endl;
    }

    HoughCircles(img1, circles, HOUGH_GRADIENT, 3, img1.cols/7);

    for( size_t i = 0; i < circles.size(); i++ )
    {
        cout<<i+1<<circles[i]<<endl;
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( img, center, 1, Scalar(0, 0, 255), 3, 8);
        // circle outline
        int radius = c[2];
        circle( img, center, radius, Scalar(0, 255, 0), 3, 8);
    }
    imshow("Original", img);
    imshow("Circles", img1);
    waitKey(0);
    destroyAllWindows();
}
