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
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;

Mat img;
Mat magI;
Mat inverse_magI;
Mat img_fourier;
Mat img_inv_fourier;

Mat DFT(Mat &img);
Mat Inverse_DFT(Mat &img);
Mat myFourier(Mat &img);
Mat beautiful_spectre(Mat &img);
void correct_work(Mat &img1, Mat &img2);
Mat inverse_fourier(complex<double> **sum_dft, Mat &img);

int main()
{
    clock_t begin, end, time_DFT, time_myFourier;

    img = imread("/home/anastasia/git/Lab_4/mono.jpeg", 0);

    if(! img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }

    Size size(0.5*img.cols, 0.5*img.rows);
    resize(img, img, size);

    cout<<"[info] width: "<<img.cols<<" pixels"<<endl;
    cout<<"[info] height: "<<img.rows<<" pixels"<<endl;

    magI = img.clone();
    img_fourier = img.clone();

    begin = clock();
    DFT(magI); //DFT and IDFT
    end = clock();
    time_DFT = (end - begin)/CLOCKS_PER_SEC;
    cout<<"Time DFT = "<<time_DFT<<" s"<<endl;

    begin = clock();
    myFourier(img_fourier); //DFT and IDFT
    end = clock();
    time_myFourier = (end - begin)/CLOCKS_PER_SEC;
    cout<<"Time My Fourier = "<<time_myFourier<<" s"<<endl;
/*
    namedWindow("Input Image", WINDOW_AUTOSIZE);
    namedWindow("Discrete Fourier Trasform", WINDOW_AUTOSIZE);
    namedWindow("My Fourier Transform", WINDOW_AUTOSIZE);
    namedWindow("My Inverse Fourier Transform", WINDOW_AUTOSIZE);
    namedWindow("Inverse Fourier Transform", WINDOW_AUTOSIZE);
*/
    imshow("Input Image", img);
    imshow("Discrete Fourier Trasform", magI);
    imshow("Inverse Fourier Transform", inverse_magI);
    imshow("My Fourier Transform", img_fourier);
    imshow("My Inverse Fourier Transform", img_inv_fourier);

    waitKey();
    destroyAllWindows();
    return 0;
}

Mat DFT(Mat &img)
{
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize(img.rows);
    int n = getOptimalDFTSize(img.cols); // on the border add zero values
    copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    dft(complexI, complexI);            // this way the result may fit in the source matrix

    inverse_magI = complexI.clone();
    Inverse_DFT(inverse_magI);

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(img))^2 + Im(DFT(img))^2))
    split(complexI, planes);                   // planes[0] = Re(DFT(img), planes[1] = Im(DFT(img))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    img = planes[0];

    img += Scalar::all(1);                    // switch to logarithmic scale
    log(img, img);

    // crop the spectrum, if it has an odd number of rows or columns
    img = img(Rect(0, 0, img.cols & -2, img.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    beautiful_spectre(img);
 //   normalize(magI, magI, 0, 1, CV_MINMAX);
    return img;
}

Mat Inverse_DFT(Mat &img)
{
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize(img.rows);
    int n = getOptimalDFTSize(img.cols); // on the border add zero values
    copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = { Mat::zeros(padded.size(), CV_32F), Mat::zeros(padded.size(), CV_32F)};

    dft(img, img, DFT_REAL_OUTPUT|DFT_INVERSE);            // this way the result may fit in the source matrix

    Mat temp;
    split(img, planes);                   // planes[0] = Re(DFT(img), planes[1] = Im(DFT(img))
    temp = planes[0];
 //   img = Mat::zeros(m, n, CV_8U);
    normalize(temp, img, 0, 1, CV_MINMAX);
    return img;
}

Mat myFourier(Mat &img)
{
    int i=0, j=0, N=0;
    double pi = 3.14159265359;
    int rows = getOptimalDFTSize(img.rows);
    int cols = getOptimalDFTSize(img.cols);
    double w1 = -2*pi/cols;
    double w2 = -2*pi/rows;

    Mat temp = img.clone();
    img = Mat::zeros(rows, cols, CV_64F);

    complex<double> **sum_cols = new complex<double> *[rows];
    complex<double> **sum_rows = new complex<double> *[rows];
    double **sum = new double *[rows];

    for(i=0; i<rows; i++)
    {
        sum_cols[i] = new complex<double> [cols];
        sum_rows[i] = new complex<double> [cols];
        sum[i] = new double [cols];
        for(j=0; j<cols; j++)
        {
            sum_cols[i][j] = 0;
            sum_rows[i][j] = 0;
            sum[i][j] = 0;
        }
    }
    for(i=0; i<rows; i++)
    {
        for(j=0; j<cols; j++)
        {
            for(N=0; N<cols; N++)
            {
                sum_cols[i][j] += complex<double>(temp.at<uchar>(i,N),0)*complex<double>(cos(w1*j*N),sin(w1*j*N));
            }
        }
    }
    for(i=0; i<cols; i++)
    {
        for(j=0; j<rows; j++)
        {
            for(N=0; N<rows; N++)
            {
                sum_rows[j][i] += sum_cols[N][i]*complex<double>(cos(w2*j*N),sin(w2*j*N));
            }
            sum[j][i] = (sqrt(pow((sum_rows[j][i].real()),2) + pow((sum_rows[j][i].imag()),2)));
            img.at<double>(j,i) = log(sum[j][i]+1);
        }
    }

    img_inv_fourier = temp.clone();
    inverse_fourier(sum_rows, img_inv_fourier);

    beautiful_spectre(img);
    return img;
}

Mat beautiful_spectre(Mat &img)
{
    int cx = img.cols/2;
    int cy = img.rows/2;

    Mat q0(img, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(img, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(img, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(img, Rect(cx, cy, cx, cy)); // Bottom-Right

    Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    normalize(img, img, 0, 1, CV_MINMAX);

    return img;
}

void correct_work(Mat &img1, Mat &img2)
{
    int Overlap = 0;
    double value = 0;
    double img_1, img_2;

    for(int i=0; i<img1.cols; i++)
    {
        for(int j=0; j<img1.rows; j++)
        {
            img_1 = img1.at<double>(j,i);
            img_2 = img2.at<double>(j,i);
            value = (img_1-img_2);
           // cout<<"img1 - img2 = "<<abs(value)<<endl;
            if(abs(value)<0.6)
                Overlap++;
        }
    }
    cout<<"Correctness of work = "<<Overlap*100/(img1.cols*img1.rows)<<" %"<<endl;
}

Mat inverse_fourier(complex<double> **sum_dft, Mat &img)
{
    int i=0, j=0, N=0;
    double pi = 3.14159265359;
    int rows = getOptimalDFTSize(img.rows);
    int cols = getOptimalDFTSize(img.cols);
    double w1 = 2*pi/cols;
    double w2 = 2*pi/rows;

//    Mat temp = img.clone();
    img = Mat::zeros(rows, cols, CV_64F);

    complex<double> **sum_cols = new complex<double> *[rows];
    complex<double> **sum_rows = new complex<double> *[rows];

    for(i=0; i<rows; i++)
    {
        sum_cols[i] = new complex<double> [cols];
        sum_rows[i] = new complex<double> [cols];
        for(j=0; j<cols; j++)
        {
            sum_cols[i][j] = 0;
            sum_rows[i][j] = 0;
        }
    }
    for(i=0; i<rows; i++)
    {
        for(j=0; j<cols; j++)
        {
            for(N=0; N<cols; N++)
            {
                sum_cols[i][j] += sum_dft[i][N]*complex<double>(cos(w1*j*N),sin(w1*j*N));
            }
           // sum_cols[i][j] = sum_cols[i][j]*complex<double>(1/cols, 1/cols);
        }
    }
    for(i=0; i<cols; i++)
    {
        for(j=0; j<rows; j++)
        {
            for(N=0; N<rows; N++)
            {
                sum_rows[j][i] += sum_cols[N][i]*complex<double>(cos(w2*j*N),sin(w2*j*N))*complex<double>(1/(cols*rows), 1/(cols*rows));;
            }
      //      sum_rows[j][i] = sum_rows[j][i]*complex<double>(1/(cols*rows), 1/(cols*rows));
            img.at<double>(j,i) = sum_rows[j][i].real();
        }
    }
//    cout<<"value: "<<img.at<double>(rows/2, cols/2)<<endl;
  //  beautiful_spectre(img);
    normalize(img, img, 0, 1, CV_MINMAX);
    return img;
}
