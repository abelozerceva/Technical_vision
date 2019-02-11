#include <core/include/opencv2/core.hpp>
#include <highgui/include/opencv2/highgui.hpp>
#include <imgproc/include/opencv2/imgproc.hpp>
#include <imgcodecs/include/opencv2/imgcodecs.hpp>
#include <video/include/opencv2/video/tracking.hpp>
#include <objdetect/include/opencv2/objdetect.hpp>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <thread>
#include <mutex>
#include <fstream>

using namespace std;
using namespace cv;

int th = 0;
Mat imgBGR, imgHSV;
double d, minum;
Moments mnts, imnts, lmnts;
Mat img, thimg, input_img, timg, cimg, buff, kernel, result, lamp, red, green, blue, a, b;
vector<vector<Point>> cnts;

int x=0, y=0, k=0, p=0;

void babach();
void teplovisor();
void robots();
void keys();
void result_img(int mini, int maxi, Vec3b color, int n);
void together();


int main()
{
    cout<<"Please enter number: "<<endl;
    cout<<"1 - Find houses; "<<endl;
    cout<<"2 - Find motors "<<endl;
    cout<<"3 - Find robots; "<<endl;
    cout<<"4 - Find keys."<<endl;
    cout<<"0 - Exit"<<endl;

    int control = getchar( );
    while ( control != '0' )
    {
        switch ( control )
        {
            case '1':
            {
                babach();
                destroyAllWindows( );
                break;
            }
            case '2':
            {
                teplovisor();
                destroyAllWindows( );
                break;
            }
            case '3':
            {
                robots();
                break;
            }
            case '4':
            {
                keys();
                break;
            }
        }
        control = getchar( );
    }
}

void babach()
{
    string fn;
    fn = "/home/anastasia/git/Lab_3/img_zadan/allababah/ig_0.jpg";
    img = imread(fn, 0);
    /// Show image
    imshow("Input Image", img);
    /// Set to binary mode
    threshold(img, thimg, 230, 255, THRESH_BINARY);
    /// kernel for morphology
    kernel = getStructuringElement(CV_SHAPE_RECT, Size(3, 3));

//	erode(thimg, cimg, kernel, Point(-1, -1), 2);
//	imshow("erode", cimg);
    /// Make object bigger
    dilate(thimg, cimg, kernel, Point(-1, -1), 4);
    imshow("dilate", cimg);
    /// Find contours of object
    vector<vector<Point>> cnts;
    findContours(cimg, cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    imshow("contours", cimg);

    cvtColor(img, img, CV_GRAY2BGR);

    polylines(img, cnts, true, Vec3b(0, 255, 0), 2, 8);
    imshow("polyline", img);

    for (int i = 0; i < cnts.size(); i++)
    {
        if (cnts[i].size() > 10)
        {
            mnts = moments(cnts[i]);
            circle(img, Point(mnts.m10 / mnts.m00, mnts.m01 / mnts.m00), 5, Vec3b(0, 0, 255), -1);
        }
    }
    imshow("result", img);
    waitKey();
}

void teplovisor()
{
    string fn1 = "/home/anastasia/git/Lab_3/img_zadan/teplovizor/5.jpg";
    imgBGR = imread(fn1);
    imshow("Input Image", imgBGR);
    cvtColor(imgBGR, imgHSV, CV_BGR2HSV);
    inRange(imgHSV, Vec3b(0, 50, 50), Vec3b(17, 255, 255), a);
    inRange(imgHSV, Vec3b(155, 50, 50), Vec3b(179, 255, 255), b);
    cimg = a + b;

    kernel = getStructuringElement(CV_SHAPE_RECT, Size(3, 3));
    dilate(cimg, cimg, kernel, Point(-1, -1), 3);
//	imshow("cimg", cimg);

    vector<vector<Point>> cnts;
    findContours(cimg, cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//	imshow("contours", cimg);

    for (int i = 0; i < cnts.size(); i++)
    {
        if (cnts[i].size() > 30)
        {
            mnts = moments(cnts[i]);
            polylines(imgBGR, cnts[i], true, Vec3b(255, 255, 255), 2, 8);
            circle(imgBGR, Point(mnts.m10 / mnts.m00, mnts.m01 / mnts.m00), 5, Vec3b(0, 0, 0), -1);
        }
    }
    imshow("result", imgBGR);
    waitKey(0);
}

void robots()
{
    string inp_img = "/home/anastasia/git/Lab_3/img_zadan/roboti/roi_robotov.jpg";
    input_img = imread(inp_img);
    // уменьшили изображение ,чтоб оно не занимало весь экран
    resize(input_img, img, cv::Size(), 0.5, 0.5);
    resize(input_img, input_img, cv::Size(), 0.75, 0.75);

    // перевели в HSV палитру для более простого поиска
    cvtColor(input_img, cimg, CV_BGR2HSV);
    // делаем две копии
    // buff для всей магии
    // result для рисования на нём
    buff = cimg.clone();
    result = input_img.clone();
    imshow("img", img);
    // ищем лампу - она яркая, выбрать диапазон не трудно
    inRange(cimg, Vec3b(0, 0, 150), Vec3b(35, 12, 255), cimg);
    // создали ядро для морфологии
    kernel = getStructuringElement(CV_SHAPE_RECT, Size(5, 5));
    //cout<<kernel<<endl;
    // эрозия для устранения шумов
    erode(cimg, cimg, kernel, Point(-1, -1), 1);
    // дилатация для увеличения размеров
    dilate(cimg, cimg, kernel, Point(-1, -1), 3);
    // находим и рисуем контур лампы
    result_img(65, 110, Vec3b(0, 255, 255), 1);
    /// red
    cimg = buff.clone();
    // немного эрозии для сглаживания (ибо применяем на HSV палитре, а не бинарке)
    erode(cimg, cimg, kernel, Point(-1, -1), 5);
    //imshow("cimg", cimg);
    // через два диапазона вычислили всех роботов нужного цвета
    inRange(buff, Vec3b(0, 10, 0), Vec3b(10, 255, 255), a);
    inRange(buff, Vec3b(160, 10, 0), Vec3b(179, 255, 255), b);
    // сложили два бинаризованных изображения и получили одно, со всеми роботами нужного цвета
    cimg = a + b;
    imshow("binary red", cimg);
    // соединили тех, которых перекрыла палка
    together();
    // находим и рисуем контуры (лампа не обрисуется ибо заданы минимальны и максимальный размеры контуры благодаря первым двум параметрам)
    result_img(40, 120, Vec3b(0, 0, 255), 0);
    imshow("conturs", result);
    /// green
    cimg = buff.clone();
    inRange(cimg, Vec3b(65, 50, 140), Vec3b(80, 255, 255), cimg);
    imshow("binary green", cimg);
    together();
    result_img(17, 120, Vec3b(0, 255, 0), 0);
    /// blue
    cimg = buff.clone();
    inRange(cimg, Vec3b(92, 50, 128), Vec3b(102, 255, 255), cimg);
    imshow("binary blue", cimg);
    together();
    result_img(0, 100, Vec3b(255, 0, 0), 0);

    imshow("result", result);
    waitKey(0);
}

void together()
{
    // проходимся на бинаризованном изображении по всей вертикали изображения
    // закраска происходит по рядам
    // после закраски на бинаризованном изображении будет имитация отсутсвия балки
    for (y = 0; y < 540; y++)
    {
        // 450 и 480 - координаты палки выявленные через обратную бинаризацию
        // (оставила только палку и посчитала циклом сколько пикселов до её границы и сколько пикселов между её границами)
        const int stick_left = 450;
        const int stick_right = 480;
        for (x = stick_left; x <= stick_right; x++)
        {
            // если слева от балки (или в ней) есть белый пиксел, то он принадлежит объекту (это однозначно ибо все шумы отфильтровали)
            // запоминаем координату этого пиксела и идём в следующий цикл
            if (cimg.at<uchar>(y, x) == 255)
            {
                k = x; break;
            }
        }

        // если справа от балки (см.выше)
        for (x = 480; x >= 450; x--)
        {
            if (cimg.at<uchar>(y, x) == 255)
            {
                p = x; break;
            }
        }

        // если был найден белый пиксел слева, то закрашиваем в этом ряду всё с левого до правого
        if (k!=0)
        {
            for (x = k; x < p; x++)
            {
                cimg.at<uchar>(y, x) = 255;
            }
        }
        k = 0;
    }
    //imshow("3", cimg);
}

void result_img(int mini, int maxi, Vec3b color, int n)
{
    // нашли контуры объектов (роботы уже соединены должны быть)
    findContours(cimg, cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    d = 0;
    // максимальное расстояние от робота до лампы (чтоб рисовать линию)
    minum = 1000;
    // проходимся по всем контурам
    for (int i = 0; i < cnts.size(); i++)
    {
        // если размер контура не меньше минимума и не превышает максимум, то рисуем его и находим его центр масс
        // так, например, можно избежать детектирование плафона
        if (cnts[i].size() > mini && cnts[i].size() < maxi)
        {
            // находим момент контура
            mnts = moments(cnts[i]);
            // если 0, то ищем роботов, иначе лампу
            if (n == 0)
            {
                // по Пифагору нашли расстояние от центра масс робота до центра масс лампы
                d = sqrt((lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) * (lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) +
                        (lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00) * (lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00));
                if (minum > d)
                {
                    // обновили максимальное расстояние (так и ищем минимум)
                    minum = d;
                    imnts = mnts;
                }
            }
            else if (n == 1)
            {
                // отметили моменты лампы (первым просто запомнили, второй для рисования)
                lmnts = mnts; imnts = mnts;
            }

            // нарисовали контур
            polylines(result, cnts[i], true, color, 2, 8);
        }
    }

    // нарисовали центр масс
    circle(result, Point(imnts.m10 / imnts.m00, imnts.m01 / imnts.m00), 5, Vec3b(0, 0, 0), -1);
    // нарисовали линию от центра масс робота, до лампы
    line(result, Point(imnts.m10 / imnts.m00, imnts.m01 / imnts.m00), Point(lmnts.m10 / lmnts.m00, lmnts.m01 / lmnts.m00), Vec3b(0, 0, 0), 2, 8);
}


void keys()
{
    vector<vector<Point>> cnts, tnts;
    input_img = imread("/home/anastasia/git/Lab_3/img_zadan/gk/gk.jpg", 0);
    img = imread("/home/anastasia/git/Lab_3/img_zadan/gk/gk_tmplt.jpg", 0);

    resize(input_img, input_img, cv::Size(), 0.75, 0.75);

    threshold(input_img, cimg, 240, 255, THRESH_BINARY_INV);

    findContours(cimg, cnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    cvtColor(input_img, input_img, CV_GRAY2BGR);

    threshold(img, timg, 240, 255, THRESH_BINARY);

    /// Find contours of mask
    findContours(timg, tnts, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    cvtColor(img, img, CV_GRAY2BGR);
    polylines(img, tnts[0], true, Vec3b(0, 255, 0), 2, 8);
    /// Compare mask with each object and higlight in different color if similar or not
    for (int i = 0; i < cnts.size(); i++)
    {
        float diff = matchShapes(cnts[i], tnts[0], CV_CONTOURS_MATCH_I2, 0);

        if (abs(diff) < 0.5)
        {
            polylines(input_img, cnts[i], true, Vec3b(0, 255, 0), 2, 8);
        }
        else
        {
            polylines(input_img, cnts[i], true, Vec3b(0, 0, 255), 2, 8);
        }
    }
    imshow("input_img", input_img);
    imshow("template", img);

    waitKey(0);
}
