#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>


using namespace cv;
using namespace std;

double d, minum;
Moments mnts, imnts, lmnts;
Mat img, input_img, cimg, buff, kernel, result, lamp, red, green, blue, a, b;
vector<vector<Point>> cnts;

int x=0, y=0, k=0, p=0;

void together()
{
	// проходимся на бинаризованном изображении по всей вертикали изображения
	// закраска происходит по рядам
	// после закраски на бинаризованном изображении будет имитация отсутсвия балки
	for (y = 0; y < 540; y++)
	{
		// 450 и 480 - координаты палки выявленные через обратную бинаризацию
		// (оставил только палку и посчитал циклом сколько пикселов до её границы и сколько пикселов между её границами)
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

int lab3_3( string inp_img )
{
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
	// через два диапазона вычислили всех роботов нужного цвета
	inRange(buff, Vec3b(0, 10, 0), Vec3b(10, 255, 255), a);
	inRange(buff, Vec3b(160, 10, 0), Vec3b(179, 255, 255), b);
	// сложили два бинаризованных изображения и получили одно, со всеми роботами нужного цвета
	cimg = a + b;
	// соединили тех, которых перекрыла палка
	together();
	// находим и рисуем контуры (лампа не обрисуется ибо заданы минимальны и максимальный размеры контуры благодаря первым двум параметрам)
	result_img(40, 120, Vec3b(0, 0, 255), 0);
	/// green
	cimg = buff.clone();
	inRange(cimg, Vec3b(65, 50, 140), Vec3b(80, 255, 255), cimg);
	imshow("binary", cimg);
	together();
	result_img(17, 120, Vec3b(0, 255, 0), 0);
	/// blue
	cimg = buff.clone();
	inRange(cimg, Vec3b(92, 50, 128), Vec3b(102, 255, 255), cimg);
	together();
	result_img(0, 100, Vec3b(255, 0, 0), 0);

	imshow("result", result);
	waitKey(0);
}