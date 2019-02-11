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
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <thread>
#include <mutex>
#include <fstream>

using namespace std;
using namespace cv;

Mat img;
Mat magI;
Mat inverse_magI;
Mat img_fourier;
Mat img_myfourier;
Mat img_sobel;

Mat count_W( int signalSize, bool inverse );
Mat count_DFT_first_sum( const Mat img, const Mat W );
Mat count_DFT_second_sum( const Mat img, const Mat W );
void custom_DFT(Mat img );
Mat normalize_fourier( Mat fourier_sums, string name );
void dftConvolution(Mat img_name, int kernel_type );
void cutFreq(Mat img, bool high );
void numb_correlation( );
void correlation( Mat img, Mat ch, string str );
void shift_rect( Mat &fourier_img );

int main()
{
    img = imread("/home/anastasia/git/Lab4/mono.jpeg", 0);

    if(! img.data )
    {
        cout <<  "Could not open or find the image" << endl;
        return -1;
    }

    cout<<"Please enter number: "<<endl;
    cout<<"1 - Fourier transform; "<<endl;
    cout<<"2 - Sobel, Laplasian and Box filters; "<<endl;
    cout<<"3 - Cut frequence; "<<endl;
    cout<<"4 - Correlation images."<<endl;
    cout<<"0 - Exit"<<endl;

    int control = getchar( );
    while ( control != '0' )
    {
        switch ( control )
        {
            case '1':
            {
                custom_DFT(img);
                destroyAllWindows( );
                break;
            }
            case '2':
            {
                dftConvolution(img, 0);
                dftConvolution(img, 1);
                dftConvolution(img, 2);
                dftConvolution(img, 3);
                destroyAllWindows( );
                break;
            }
            case '3':
            {
                cutFreq(img, false);
                cutFreq(img, true);
                destroyAllWindows( );
                break;
            }
            case '4':
            {
                numb_correlation( );
                break;
            }
        }
        control = getchar( );
    }
}

Mat count_W( int signalSize, bool inverse )
{
    int angleSign = inverse ? 1 : -1;

    float angle = 2 * angleSign * CV_PI / signalSize;

    // [0] - Real
    // [1] - Imaginary
    Mat W(signalSize, signalSize, CV_32FC2);

    // если k или n равны нулю, то угол будет равен нулю (умножение на нуль, см.формулу)
    // cos(0) = 1
    // sin(0) = 0
    for ( int i = 0; i < signalSize; i++ )
    {
        // заполняется первый столбец
        W.at < Vec2f >(i, 0)[0] = 1.0;
        W.at < Vec2f >(i, 0)[1] = 0.0;
        // заполняется первая строка
        W.at < Vec2f >(0, i)[0] = 1.0;
        W.at < Vec2f >(0, i)[1] = 0.0;
    }

    // C_[k] = C*C_[k-1] - S*S_[k-1]
    // S_[k] = S*C_[k-1] + C*S_[k-1]
    // итеративный метод для подсчёта синусов и косинусов
    float cos_ = cos(angle);
    float sin_ = sin(angle);

    // косинус и синус от нулевого угла
    float cos_k_1 = 1.0;
    float sin_k_1 = 0.0;
    for ( int k = 1; k < signalSize; k++ )
    {
        // применяем формулу (здесь угол меняется по k)
        float cos_k = cos_ * cos_k_1 - sin_ * sin_k_1;
        float sin_k = sin_ * cos_k_1 + cos_ * sin_k_1;

        // косинус и синус от нулевого угла
        float cos_n_1 = 1.0;
        float sin_n_1 = 0.0;
        for ( int n = 1; n < signalSize; n++ )
        {
            // применяем формулу (здесь угол меняется по n)
            float cos_n = cos_k * cos_n_1 - sin_k * sin_n_1;
            float sin_n = sin_k * cos_n_1 + cos_k * sin_n_1;

            // записываем найденое значение
            W.at < Vec2f >(k, n)[0] = cos_n;
            W.at < Vec2f >(k, n)[1] = sin_n;

            // запоминаем нынешнее значение для следующей итерации
            cos_n_1 = cos_n;
            sin_n_1 = sin_n;
        }
        // запоминаем нынешнее значение для следующей итерации
        cos_k_1 = cos_k;
        sin_k_1 = sin_k;
    }
    return W;
}

/// Good
Mat count_DFT_first_sum( const Mat img, const Mat W )
{
    if ( img.empty( ))
    {
        cout << "count_DFT_first_sum() : Image is empty !" << endl;
        return img;
    }
    // для удобства отдельно записали размеры преобразуемого изображения
    const int numOfRows = img.rows;
    const int numOfCols = img.cols;

    // создали двухканальное изображение для записи полученых значений
    // в нулевом канале реальная часть
    // в первом канале - мнимая
    Mat transformedImage(numOfRows, numOfCols, CV_32FC2);

    // поиск суммы (построчно)
    for ( int n1 = 0; n1 < numOfRows; n1++ )
    {
        // столбец нового изображения
        for ( int k2 = 0; k2 < numOfCols; k2++ )
        {
            // комплексная переменная
            // (в ней определены методы для арифметических операций над комплексными числами)
            complex < float > sum2(0.0f, 0.0f);
            // столбец исходного изображения
            for ( int n2 = 0; n2 < numOfCols; n2++ )
            {
                // пиксел оригинального изображения умножаем на коэффициент по правилам умножения комплексных чисел
                //** во второй строке коэффициент, найденый раньше преобразуется в комплексную переменную
                //** для правильности вычислений
                sum2 += ( float ) img.at < uchar >(n1, n2) *
                        complex < float >(W.at < Vec2f >(k2, n2)[0], W.at < Vec2f >(k2, n2)[1]);
            }
            // в новую матрицу записываем полученное значение
            transformedImage.at < Vec2f >(n1, k2)[0] = sum2.real( );
            transformedImage.at < Vec2f >(n1, k2)[1] = sum2.imag( );
        }
    }
    // возвращаем первую сумму
    return transformedImage;
}

/// Good
Mat count_DFT_second_sum( const Mat img, const Mat W )
{
    if ( img.empty( ))
    {
        cout << "count_DFT_second_sum() : Image is empty !" << endl;
        return img;
    }
    // для удобства отдельно записали размеры преобразуемого изображения
    const int numOfRows = img.rows;
    const int numOfCols = img.cols;

    // создали двухканальное изображение для записи полученых значений
    // в нулевом канале реальная часть
    // в первом канале - мнимая
    Mat transformedImage(numOfRows, numOfCols, CV_32FC2);

    // поиск суммы (теперь по столбцам)
    for ( int n2 = 0; n2 < numOfCols; n2++ )
    {
        // строка нового изображения
        for ( int k1 = 0; k1 < numOfRows; k1++ )
        {
            complex < float > sum1(0.0f, 0.0f);
            // строка исходного изображения
            for ( int n1 = 0; n1 < numOfRows; n1++ )
            {
                // комплексное значение из предыдущей функции умножается на комплексный коэффициент
                // (a+i*b)*(c+i*d) - так выглядит умножение двух комплексных чисел и раскрытие скобок происходит
                // за счёт типа complex (чисто на всякий случай поясняю, вряд ли Титов про это спросит, но для понимания решил написать)
                sum1 += complex < float >(img.at < Vec2f >(n1, n2)[0], img.at < Vec2f >(n1, n2)[0]) *
                        complex < float >(W.at < Vec2f >(k1, n1)[0], W.at < Vec2f >(k1, n1)[1]);
            }
            // в новую матрицу записываем полученное значение
            transformedImage.at < Vec2f >(k1, n2)[0] = sum1.real( );
            transformedImage.at < Vec2f >(k1, n2)[1] = sum1.imag( );
        }
    }
    // возвращаем образ-Фурье (в комплексном виде)
    return transformedImage;
}

/// Good
void custom_DFT( Mat img )
{
    clock_t begin, end;
    if ( img.empty( ))
    {
        cout << "custom_DFT() : Failed to load image !" << endl;
//        return img;
    }

    // для удобства отдельно записали размеры преобразуемого изображения
    const int numOfCols = img.cols;
    const int numOfRows = img.rows;
    // получаем матрицу с коэффициентами
    // signalSize это размерность данной матрицы (для простоты матрица делается квадратной
    // и чтоб коэффициентов хватило на все пикселы берём максимум размера исходного изображения)
    int signalSize = (numOfCols < numOfRows) ? numOfRows : numOfCols;
    Mat W(signalSize, signalSize, CV_32FC2);
    W = count_W(signalSize, 1);
    // создали двухканальное изображение для записи образа-Фурье
    // в нулевом канале реальная часть
    // в первом канале - мнимая
    Mat fourier_sums(numOfRows, numOfCols, CV_32FC2);
    // ищем первую сумму и замеряем время поиска
    begin = clock();
    fourier_sums = count_DFT_first_sum(img, W);
    end = clock();
    cout << "First sum time: " << (end - begin)/CLOCKS_PER_SEC << "sec" << endl;
    // ищем вторую сумму и замеряем время поиска
    begin = clock();
    fourier_sums = count_DFT_second_sum(fourier_sums, W);
    end = clock();
    cout << "Second sum time: " << (end - begin)/CLOCKS_PER_SEC << "sec" << endl;
    // здесь нормализуем полученный образ-Фурье
    // (приводим к виду, в котором будет нормально выведен как изображение)
    Mat img_fourier;
    Mat img_fourier_lib;
    // оригинальное изображение конвертировали из целочисленного в дробное (по-сути больше памяти выделили)
    // это необходимо для преобразования Фурье
    img.convertTo(img_fourier_lib, CV_32FC1);
    // библиотечное преобразование Фурье
    dft(img_fourier_lib, fourier_sums, DFT_COMPLEX_OUTPUT);
    // меняем местами квадранты для красоты (никакого прагматизма, чисто эстетика)
    shift_rect(fourier_sums);
    // нормализуем для нормального вывода
    img_fourier = normalize_fourier(fourier_sums, "My");
    ///** библиотечное преобразование Фурье
    // создаём переменную для записи в неё полученного образа-Фурье
    Mat fourier_lib;
    img.convertTo(fourier_lib, CV_32FC2);
    // преобразование Фурье и замер времени
    begin = clock();
    dft(img_fourier_lib, fourier_lib, DFT_COMPLEX_OUTPUT);
    end = clock();
    cout << "Lib sum time: " << (end - begin)/CLOCKS_PER_SEC << "sec" << endl;
    // меняем местами квадранты для красоты (никакого прагматизма, чисто эстетика)
    shift_rect(fourier_lib);
    // нормализуем для нормального вывода
    img_fourier_lib = normalize_fourier(fourier_lib, "lib");

    imshow("Input Image", img);
    waitKey(0);
//    return img_fourier;
}

/// Good
Mat normalize_fourier( Mat fourier_sums, string name )
{
    // приводим образ-Фурье к удобному для вывода виду
    // создаём две матрицы для разделения комплексной матрицы
    Mat fourier_split[2] = {Mat(fourier_sums.rows, fourier_sums.cols, CV_32FC1),
                            Mat(fourier_sums.rows, fourier_sums.cols, CV_32FC1)};
    // разделяем реальную и комплексную части в две отдельные матрицы
    split(fourier_sums, fourier_split);
    // находим магнитуду образа-Фурье
    Mat img_fourier;
    magnitude(fourier_split[0], fourier_split[1], img_fourier);
    img_fourier += Scalar::all(1);
    log(img_fourier, img_fourier);
    normalize(img_fourier, img_fourier, 0, 1, NORM_MINMAX);
    // отображаем полученное изображение с образом-Фурье
    imshow(name + "dft", img_fourier);
    // возвращаем нормализованный образ-Фурье
    return img_fourier;
}

/// Good
void dftConvolution( Mat img, int kernel_type )
{
  //  Mat img = imread(img_name, CV_LOAD_IMAGE_GRAYSCALE);
    imshow("original", img);

    // получаем образ-Фурье и нормализуем его
    Mat dft_img(img.size( ), CV_32FC2);
    img.convertTo(img, CV_32FC1);
    dft(img, dft_img, DFT_COMPLEX_OUTPUT);
    normalize_fourier(dft_img, "CV_DFT");
    // создаём ядро нужного фильтра (тип ядра задаётся подаваемым в функцию парметром)
    int kernel[3][3];
    switch ( kernel_type )
    {
        case 0:
        {
            // горизонтальный Собель
            int sobel_hor[3][3] ={{-1, -2, -1},
                                  {0, 0, 0},
                                  {1, 2, 1}};
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    kernel[i][j] = sobel_hor[i][j];
                }
            }
            break;
        }
        case 1:
        {
            // вертикальный Собель
            int sobel_ver[3][3] = {{-1, 0, 1},
                                  {-2, 0, 2},
                                  {-1, 0, 1}};
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    kernel[i][j] = sobel_ver[i][j];
                }
            }
            break;
        }
        case 2:
        {
            // Лапласиан
            int laplase[3][3] = {{0, 1, 0},
                                {1, -4, 1},
                                {0, 1, 0}};
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    kernel[i][j] = laplase[i][j];
                }
            }
            break;
        }
        case 3:
        {
            // box-фильтр
            int boxfilter[3][3] = {{1, 1, 1},
                                  {1, 1, 1},
                                  {1, 1, 1}};
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    kernel[i][j] = boxfilter[i][j];
                }
            }
            break;
        }
        default:
        {
            cout << "dftConvolution() : Wrong kernel type !" << endl;
            break;
        }
    }

    // создаём матрицу с теми же размерами ,что у оригинала и копируем в неё ядра
    // это необходимо для перемножения спекров (матрицам нужны одинаковые размеры*)
    // * - если точнее, то кол-во строк одной должно быть равно кол-ву столбцов другой, но полагаю, что mulSpectrums()
    //одну из матриц переворачивает при необходимости, поэтому на эту условность можно не обращать внимание
    Mat mat_kernel(img.size( ), CV_32FC1, Scalar(0));
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            mat_kernel.at < float >(i, j) = ( float ) kernel[i][j];
        }
    }

    // получаем образ-Фурье ядра и нормализуем его
    Mat dft_kernel(img.size( ), CV_32FC2);
    dft(mat_kernel, dft_kernel, DFT_COMPLEX_OUTPUT);
    shift_rect(dft_kernel);
    normalize_fourier(dft_kernel, "CV_DFT_kernel");
    // для красивого вывода на экран меняем квадранты
    shift_rect(dft_kernel);

    // создаём матрицу для записи результата
    Mat result(img.size( ), CV_32FC2);
    // перемножаем спектры изображения и ядра
    // перемножение спектров двух функций равнозначно свертке самих функций
    // в контексте изображений перемножение спектра изображения и спектра ядра box-фильтра
    // (например) сделает то же самое, что происходит в лабе 2
    mulSpectrums(dft_kernel, dft_img, result, 0, 0);
    // нормализуем (см.описание функции выше, если забыл)
    normalize_fourier(result, "mulSpectrums");

    // создаём матрицу для записи братного преобразования
    Mat idft_img(img.size( ), CV_32FC1);
    // преобразуем образ-Фурье в простое изображение
    dft(result, idft_img, DFT_INVERSE | DFT_REAL_OUTPUT);
    // нормализуем для нормального отображения
    normalize(idft_img, idft_img, 0.0, 255, CV_MINMAX);
    // конвертируем в тип, который может быть выведен на экран
    idft_img.convertTo(idft_img, CV_8UC1);
    // выводим изображение
    imshow("result", idft_img);
    waitKey(0);
}

/// Good
void cutFreq( Mat img, bool high )
{
  //  Mat img = imread(img_name, CV_LOAD_IMAGE_GRAYSCALE);
    imshow("original", img);

    // если входной параметр =true, то обрезаем высокие частоты, иначе низкие
    // для обрезания высоких частот создаётся маска (матрица, заполненая единицами)
    // и в центре "рисуется" кргу определённого радиуса, заполненый нулями
    // у обрезания низких частот единицы и нули меняются местами
    int zone = high ? 1 : 0;
    // преобразуем оригинальное изображение в Фурье
    Mat img_dft(img.size( ), CV_32FC2);
    img.convertTo(img, CV_32FC1);
    dft(img, img_dft, DFT_COMPLEX_OUTPUT);
    normalize_fourier(img_dft, "CV DFT");

    // создаём маску
    Mat mat_kernel(img.size( ), CV_32FC2, Scalar(zone, 0));
    // зада1м радиус вписываемого круга
    int cut_radius = img_dft.cols > img_dft.rows ? img_dft.rows / 2 : img_dft.cols / 2;
    // чутка обрежем радиус, чтоб не цеплять края изображения
    cut_radius -= 40;
    // рисуем круг с противоположными матрице значениями (см.объяснение выше)
    circle(mat_kernel, Point(img.cols / 2, img.rows / 2), cut_radius, Scalar(1 - zone, 0), -1);

    // перемножаем образ-Фурье изображения и маски
    Mat res;
    mulSpectrums(img_dft, mat_kernel, res, 0);
    normalize_fourier(res, "res");

    // обратное преобразование Фурье, нормализация и вывод полученного изображения
    Mat idft_img(img.size( ), CV_32FC1);
    dft(res, idft_img, DFT_INVERSE | DFT_REAL_OUTPUT);
    normalize(idft_img, idft_img, 0.0, 255, CV_MINMAX);
    idft_img.convertTo(idft_img, CV_8UC1);

    imshow("idft_res", idft_img);
    waitKey(0);
}

///
void numb_correlation( )
{
    Mat img = imread("/home/anastasia/git/Lab4/images/table.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    imshow("original", img);

    Mat A = imread("/home/anastasia/git/Lab4/images/A.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    imshow("A", A);
    correlation(img.clone( ), A.clone( ), "A");
    destroyAllWindows( );

    imshow("original", img);
    Mat seven = imread("/home/anastasia/git/Lab4/images/seven.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    imshow("seven", seven);
    correlation(img.clone( ), seven.clone( ), "seven");
    destroyAllWindows( );

    imshow("original", img);
    Mat zero = imread("/home/anastasia/git/Lab4/images/zero.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    imshow("zero", zero);
    correlation(img.clone( ), zero.clone( ), "zero");
    destroyAllWindows( );
}

void correlation( Mat img, Mat ch, string str )
{

    Mat dft_img(img.size( ), CV_32FC2);
    img.convertTo(img, CV_32FC1);
    // находим среднее и средне-квадратичное отклонения значений изображения
    Mat mean_img, std_img;
    meanStdDev(img, mean_img, std_img);
    // вычитаем среднее отклонение из всех пикселов
    // в таком случае любая ошибка (несовпадение соотетсвующих пикселов будет обозначаться большим отрицательным числом, что однозначно определит несовпадение)
    img -= mean_img;
    // делим все пикселы на среднеквадратичное
    //  дисперсия - есть разброс значений, если дисперсия большая, значит значения лежат в большом промежутке.
    // Это значит, что большие значения, на обоих картинках, могут давать оч большое число, которое будет очень сильно увеличивать корреляцию
    // если мы поделим на дисперсию мы уменьшим этот разброс
    img /= std_img;

    // преобразуем полученное изображение по Фурье, нормализуем и выводим на экран
    dft(img, dft_img, DFT_COMPLEX_OUTPUT);
    normalize_fourier(dft_img, "_img");

    // готовим кпреобразованию Фурье символ-образец (тот, что ищем на номере)
    ch.convertTo(ch, CV_32FC1);
    Mat mean_ch, std_ch;
    meanStdDev(ch, mean_ch, std_ch);
    ch -= mean_ch;
    ch /= std_ch;

    // создаём черный прямоугольник с размерами оригинального изображения
    Mat Sign(img.size( ), CV_32FC1, Scalar(0));
    // впихиваем в этот прямоугольник знак-образец (для правильного перемножения спектров - уже писал выше)
    Mat roi(Sign, Rect(0, 0, ch.cols, ch.rows));
    ch.copyTo(roi);

    imshow("Sign" + str, Sign);

    // преобразуем подготовленный знак по Фурье, нормализуем и выводим на экран
    Mat dft_sign(Sign.size( ), CV_32FC2);
    Sign.convertTo(Sign, CV_32FC1);
    dft(Sign, dft_sign, DFT_COMPLEX_OUTPUT);
    normalize_fourier(dft_sign, "_" + str);

    // перемножаем спектры изображения и знака и нормализуем произведение
    Mat img_sign;
    // перед перемножением образ-Фурье знака изменяется на комплексно-сопряженный (Влад не знает зачем, так что просто магически нужно)
    mulSpectrums(dft_img, dft_sign, img_sign, 0, true);
    normalize_fourier(img_sign, "_img_" + str);

    // делаем обратное преобразование полученного произведения, нормализуем и выводим на экран
    Mat idft_img(img.size( ), CV_32FC1);
    dft(img_sign, idft_img, DFT_INVERSE | DFT_REAL_OUTPUT);
    normalize(idft_img, idft_img, 0.0, 255, CV_MINMAX);
    idft_img.convertTo(idft_img, CV_8UC1);
    imshow("idft_res" + str, idft_img);

    double minVal;
    double maxVal;
    // находим максимальное значение интенсивности пиксела на полученном изображении
    minMaxLoc(idft_img, &minVal, &maxVal);

    Mat idft_img_bin;
    // бинаризируем изображение опираясь на найденый максиму, чтобы на белом фоне вывести только точки максимальной корреляции
    threshold(idft_img, idft_img_bin, (maxVal - 10), 255, THRESH_BINARY_INV);
    imshow("res_" + str, idft_img_bin);
    waitKey(0);
}

void shift_rect( Mat &fourier_img )
{
    // находим центр изображения
    int cx = fourier_img.cols / 2;
    int cy = fourier_img.rows / 2;

    // создаём 4 ROI для 4х квадрантов
    // левый верхний
    Mat q0(fourier_img, Rect(0,  0,  cx, cy));
    // правый верхний
    Mat q1(fourier_img, Rect(cx, 0,  cx, cy));
    // нижний левый
    Mat q2(fourier_img, Rect(0,  cy, cx, cy));
    // нижний правый
    Mat q3(fourier_img, Rect(cx, cy, cx, cy));

    // меняем местами левый верхний и нижний правый квадранты
    Mat tmp;
    q0.copyTo ( tmp );
    q3.copyTo ( q0 );
    tmp.copyTo( q3 );

    // меняем местами правый верхний и нижний левый квадранты
    q1.copyTo (tmp);
    q2.copyTo (q1);
    tmp.copyTo(q2);
}

