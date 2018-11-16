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
Mat img_fourier;
Mat img_fast_fourier;

Mat DFT(Mat &img);
Mat myFourier(Mat &img);
Mat beautiful_spectre(Mat &img);
void correct_work(Mat &img1, Mat &img2);
Mat fast_fourier(Mat &img);

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

    begin = clock();
    DFT(img);
    end = clock();
    time_DFT = (end - begin)/CLOCKS_PER_SEC;
    cout<<"Time DFT = "<<time_DFT<<" s"<<endl;

    begin = clock();
    myFourier(img);
    end = clock();
    time_myFourier = (end - begin)/CLOCKS_PER_SEC;
    cout<<"Time My Fourier = "<<time_myFourier<<" s"<<endl;

    correct_work(magI, img_fourier);

    fast_fourier(img);
  //  correct_work(img_fast_fourier, img_fourier);

    namedWindow("Input Image", WINDOW_AUTOSIZE);
    namedWindow("Discrete Fourier Trasform", WINDOW_AUTOSIZE);
    namedWindow("My Fourier Transform", WINDOW_AUTOSIZE);
    namedWindow("Buterfly", WINDOW_AUTOSIZE);

    imshow("Input Image", img);
    imshow("Discrete Fourier Trasform", magI);
    imshow("My Fourier Transform", img_fourier);
    imshow("Buterfly", img_fast_fourier);

    waitKey();
    destroyAllWindows();
    return 0;
}

Mat DFT(Mat &img)
{
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize( img.rows );
    int n = getOptimalDFTSize( img.cols ); // on the border add zero values
    copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    dft(complexI, complexI);            // this way the result may fit in the source matrix

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(img))^2 + Im(DFT(img))^2))
    split(complexI, planes);                   // planes[0] = Re(DFT(img), planes[1] = Im(DFT(img))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    magI = planes[0];

    magI += Scalar::all(1);                    // switch to logarithmic scale
    log(magI, magI);

    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    beautiful_spectre(magI);
    return magI;
}

Mat myFourier(Mat &img)
{
    int i=0, j=0, N=0;
    double pi = 3.1416;
    int rows = getOptimalDFTSize( img.rows );
    int cols = getOptimalDFTSize( img.cols );
    double w1 = -2*pi/cols;
    double w2 = -2*pi/rows;

    img_fourier = Mat::zeros(rows, cols, CV_64F);

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
                sum_cols[i][j] += complex<double>(img.at<uchar>(i,N),0)*complex<double>(cos(w1*j*N),sin(w1*j*N));
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
            img_fourier.at<double>(j,i) = log(sum[j][i]+1);
        }
    }
    beautiful_spectre(img_fourier);
    return img_fourier;
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

Mat fast_fourier(Mat &img)
{
    int i=0, j=0, N=0;
    double pi = 3.1416;
    int rows = getOptimalDFTSize( img.rows )-2;
    int cols = getOptimalDFTSize( img.cols )-1;
    double w1 = -2*pi/cols;
    double w2 = -2*pi/rows;

    img_fast_fourier = Mat::zeros(rows, cols, CV_64F);

    complex<double> **sum_cols = new complex<double> *[rows];
    complex<double> **sum_rows = new complex<double> *[rows];
    complex<double> **W = new complex<double> *[rows];
    double **sum = new double *[rows];

/*    for(i=0; i<rows; i++)
    {
        sum[i] = new double [cols];
        W[i] = new complex<double> [cols];
        for(j=0; j<cols; j++)
        {
            sum[i][j] = 0;
            W[i][j] = 0;
        }
    }
*/
    for(i=0; i<rows; i++)
    {
        sum_cols[i] = new complex<double> [cols];
        sum_rows[i] = new complex<double> [cols];
        sum[i] = new double [cols];
        W[i] = new complex<double> [cols];
        for(j=0; j<cols; j++)
        {
            sum_cols[i][j] = 0;
            sum_rows[i][j] = 0;
            sum[i][j] = 0;
            W[i][j] = 0;
        }
    }
    for(i=0; i<rows/2; i++)
    {
        for(j=0; j<cols/2; j++)
        {
            for(N=0; N<cols/2; N++)
            {
                sum_cols[i][j] += complex<double>(img.at<uchar>(2*i,2*N),0)*complex<double>(cos(w1*2*j*N),sin(w1*2*j*N));//00
                sum_cols[i][j+cols/2] += complex<double>(img.at<uchar>(2*i,2*N+1),0)*complex<double>(cos(w1*(2*j)*N),sin(w1*(2*j)*N));//01
                sum_cols[i+rows/2][j] += complex<double>(img.at<uchar>(2*i+1,2*N),0)*complex<double>(cos(w1*(2*j+1)*N),sin(w1*(2*j+1)*N));//10
                sum_cols[i+rows/2][j+cols/2] += complex<double>(img.at<uchar>(2*i+1,2*N+1),0)*complex<double>(cos(w1*(2*j+1)*N),sin(w1*(2*j+1)*N));//11
            }
        }
    }
    for(i=0; i<cols/2; i++)
    {
        for(j=0; j<rows/2; j++)
        {
            for(N=0; N<rows/2; N++)
            {
                sum_rows[j][i] += sum_cols[N][i]*complex<double>(cos(w2*2*j*N),sin(w2*2*j*N));//00
                sum_rows[j][i+cols/2] += sum_cols[N][i+cols/2]*complex<double>(cos(w2*(2*j+1)*N),sin(w2*(2*j+1)*N));//01
                sum_rows[j+rows/2][i] += sum_cols[N+rows/2][i]*complex<double>(cos(w2*(2*j)*N),sin(w2*(2*j)*N));//10
                sum_rows[j+rows/2][i+cols/2] += sum_cols[N+rows/2][i+cols/2]*complex<double>(cos(w2*(2*j+1)*N),sin(w2*(2*j+1)*N));//11
            }
            W[j][i] = sum_rows[j][i]+sum_rows[j][i+cols/2]+sum_rows[j+rows/2][i]+sum_rows[j+rows/2][i+cols/2];
            W[j+rows/2][i] = sum_rows[j][i]+sum_rows[j][i+cols/2]-sum_rows[j+rows/2][i]-sum_rows[j+rows/2][i+cols/2];
            W[j][i+cols/2] = sum_rows[j][i]-sum_rows[j][i+cols/2]+sum_rows[j+rows/2][i]+sum_rows[j+rows/2][i+cols/2];
            W[j+rows/2][i+cols/2] = sum_rows[j][i]-sum_rows[j][i+cols/2]-sum_rows[j+rows/2][i]-sum_rows[j+rows/2][i+cols/2];
        }
    }
    for(i=0; i<cols; i++)
    {
        for(j=0; j<rows; j++)
        {
            sum[j][i] = (sqrt(pow((W[j][i].real()),2) + pow((W[j][i].imag()),2)));
            img_fast_fourier.at<double>(j,i) = log(sum[j][i]+1);
        }
    }
    beautiful_spectre(img_fast_fourier);
  //  normalize(img_fast_fourier, img_fast_fourier, 0, 1, CV_MINMAX);
    return img_fast_fourier;
}


void correct_work(Mat &img1, Mat &img2)
{
    int Overlap=0;
    double img_1, img_2;

    for(int i=0; i<img1.cols; i++)
    {
        for(int j=0; j<img1.rows; j++)
        {
            img_1 = img1.at<double>(j,i);
            img_2 = img2.at<double>(j,i);
            int value = (img_1-img_2);
           // cout<<"img1 - img2 = "<<abs(value)<<endl;
            if(abs(value)<2)
                Overlap++;
        }
    }
    cout<<"Correctness of work = "<<Overlap*100/(img1.cols*img1.rows)<<" %"<<endl;
}
