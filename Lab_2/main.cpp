#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace cv;
using namespace std;

int main()
{
    Mat img = imread("/home/anastasia/git/Lab_2/mono.jpeg", 0);
    Mat img_orig;
    Mat img_blur;
    Mat img_addit;
    Mat img_roi;
    Mat img_gauss;
    Mat img_mygauss;
    Mat kernel_gauss;

    int i=0, j=0, k=0, m=0, n=0;
    int sum = 0;
    u_char sred = 0;
    clock_t begin, end, time_roi, time_blur, time_mygauss, time_gauss;

    Size size(0.7*img.cols, 0.7*img.rows);

    if(! img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }

    cout<<"Please enter size of kernel: ";
    cin>>n;

    cout<<"[info] width: "<<img.cols<<" pixels"<<endl;
    cout<<"[info] height: "<<img.rows<<" pixels"<<endl;

    resize(img, img_orig, size);

    img_addit = img_orig.clone();
    img_roi = img_orig.clone();

    begin = clock();
    for(i=0; i<(img_orig.cols-n); i++)
    {
        for(j=0; j<(img_orig.rows-n); j++)
        {
            for(k=0; k<n; k++)
            {
                for(m=0; m<n; m++)
                {
                    sum += img_orig.at<uchar>(j+k,i+m);
                }

            }
            sred = sum/(n*n);
            img_roi.at<uchar>(j+1,i+1) = sred;
            sum = 0;
        }
    }
    end = clock();

    time_roi = end - begin;
    cout<<"Time ROI = "<<time_roi<<" ms"<<endl;

    namedWindow("ROI",WINDOW_AUTOSIZE);
    imshow("ROI", img_roi);

    namedWindow("Original", WINDOW_AUTOSIZE);
    imshow("Original", img_orig);

    begin = clock();
    blur(img_orig, img_blur, Size(n, n), Point(-1, -1));
    end = clock();
    time_blur = end - begin;
    cout<<"Time Blur = "<<time_blur<<" ms"<<endl;

    namedWindow("Blur", WINDOW_AUTOSIZE);
    imshow("Blur", img_blur);

    int Overlap=0;
    u_int rois, blurs;

    for(i=0; i<img_roi.cols; i++)
    {
        for(j=0; j<img_roi.rows; j++)
        {
            rois = img_roi.at<uchar>(j,i);
            blurs = img_blur.at<uchar>(j,i);
            int value = (rois-blurs);
           // cout<<"rois - blurs = "<<abs(value)<<endl;
            if(abs(value)<2)
                Overlap++;
        }
    }
    cout<<"Correctness of work Blur = "<<Overlap*100/(img_roi.cols*img_roi.rows)<<" %"<<endl;

    img_addit = img_orig.clone();
    img_mygauss = img_orig.clone();

    float sigma = 0.84;
    double temp = 0;
    double sred_gauss = 0;

    kernel_gauss = Mat::ones(n, n, CV_32F);
  //  sigma = 0.3 * ((n - 1) * 0.5 - 1) + 0.8 ;

    int med = n/2;

    begin = clock();
    for(i=0; i<n; i++)
    {
        for(j=0; j<n; j++)
        {
            temp = (exp((-pow(i-med, 2.0) -pow(j-med, 2.0))/(2*pow(sigma, 2.0))))/(2*3.14159*pow(sigma, 2.0));
            kernel_gauss.at<float>(j, i) = temp;
        }
    }
    for(i=0; i<(img_addit.cols-n); i++)
    {
        for(j=0; j<(img_addit.rows-n); j++)
        {
            for(k=0; k<n; k++)
            {
                for(m=0; m<n; m++)
                {
                    sred_gauss += img_addit.at<uchar>(j+m, i+k)*kernel_gauss.at<float>(m, k);
                }

            }
            img_mygauss.at<uchar>(j+med,i+med) = sred_gauss;
            sred_gauss = 0;
        }
    }
    end = clock();

    time_mygauss = end - begin;
    cout<<"Time My Gaussian = "<<time_mygauss<<" ms"<<endl;

    namedWindow("My Gaussian Blur", WINDOW_AUTOSIZE);
    imshow("My Gaussian Blur", img_mygauss);

    begin = clock();
    GaussianBlur(img_orig, img_gauss, Size(n, n), sigma, 0, BORDER_DEFAULT);
    end = clock();

    time_gauss = end - begin;
    cout<<"Time Gaussian Blur = "<<time_gauss<<" ms"<<endl;

    namedWindow("Gaussian Blur", WINDOW_AUTOSIZE);
    imshow("Gaussian Blur", img_gauss);

    Overlap = 0;
    u_int my_gauss, gauss;

    for(i=0; i<img_mygauss.cols; i++)
    {
        for(j=0; j<img_mygauss.rows; j++)
        {
            my_gauss = img_mygauss.at<uchar>(j,i);
            gauss = img_gauss.at<uchar>(j,i);
            int value = (my_gauss - gauss);
           // cout<<"value = "<<value<<endl;
            if(abs(value)<2)
                Overlap++;
        }
    }
    cout<<"Correctness of work Gaussian Blur = "<<Overlap*100/(img_mygauss.cols*img_mygauss.rows)<<" %"<<endl;

    Overlap = 0;

    for(i=0; i<img_mygauss.cols; i++)
    {
        for(j=0; j<img_mygauss.rows; j++)
        {
            rois = img_roi.at<uchar>(j,i);
            my_gauss = img_mygauss.at<uchar>(j,i);
            int value = (rois-my_gauss);
           // cout<<"rois - blurs = "<<abs(value)<<endl;
            if(abs(value)<2)
                Overlap++;
        }
    }
    cout<<"Difference between Blur and Gaussian Blur = "<<Overlap*100/(img_mygauss.cols*img_mygauss.rows)<<" %"<<endl;

    waitKey(0);
    destroyAllWindows();
    return 0;
}
