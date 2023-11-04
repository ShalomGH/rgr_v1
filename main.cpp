#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define _USE_MATH_DEFINES

#define RESET   "\033[0m"
//#define BLACK   "\033[30m"      /* Black */
//#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
//#define YELLOW  "\033[33m"      /* Yellow */
//#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
//#define CYAN    "\033[36m"      /* Cyan */
//#define WHITE   "\033[37m"      /* White */
//#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
//#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
//#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
//#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
//#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
//#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
//#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
//#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define CLEAR u8"\033[2J\033[1;1H" /* clear console */


#ifdef _WIN32

#include <windows.h>
#include <conio.h>
#include <valarray>

#else

#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

int getch() {
    struct termios oldt{}, newt{};
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#endif

using namespace std;

//Переменные высоты и ширины экрана
static int SCREEN_HEIGHT;
static int SCREEN_WIDTH;

//Функция создания пустого экрана
vector<vector<char>> generateCanvas() {
    vector<vector<char>> canvas;
    canvas.resize(SCREEN_HEIGHT);
    for (auto &i: canvas) i.resize(SCREEN_WIDTH);
    return canvas;
}


//Класс экрана
class Screen {
public:

    Screen() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        SCREEN_HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top;
        SCREEN_WIDTH = csbi.srWindow.Right - csbi.srWindow.Left;
#else
        struct winsize size{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        SCREEN_HEIGHT = size.ws_row;
        SCREEN_WIDTH = size.ws_col;
#endif
    }

    static void Write(vector<vector<char>> scene) {
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            for (int j = 0; j < SCREEN_WIDTH; ++j)
                if (scene[i][j]) {
                    if (scene[i][j] == ';') cout << GREEN;
                    else if (scene[i][j] == '#') cout << MAGENTA;
                    else if (scene[i][j] == '%') cout << RESET;
                    else cout << scene[i][j];
                } else
                    cout << " ";
            cout << endl;
        }
    }
};


class Menu {
public:
    int point = 1;
    vector<string> menu_items{
            "Menu",
            "1.  Graphic  ",
            "2.  Table    ",
            "3.  Animation",
            "4.  .!.      ",
    };

    Menu() = default;

    vector<vector<char>> render() {
        vector<vector<char>> canvas = generateCanvas();

        size_t y_start = (SCREEN_HEIGHT - menu_items.size()) / 2;

        for (int i = 0; i < menu_items.size(); i++) {
            size_t x_start = (SCREEN_WIDTH - menu_items[i].size()) / 2;
            for (int j = 0; j < menu_items[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menu_items[i][j];
            }
        }

        size_t y_point = y_start + point;
        size_t x_point = (SCREEN_WIDTH - menu_items[1].size()) / 2 - 2;
        canvas[y_point][x_point] = '*';

        return canvas;
    }
};

class Graphic {
public:
    Graphic() = default;

    static vector<vector<char>> render() {
        //Создание экрана
        vector<vector<char>> canvas = generateCanvas();

        //Создание вспомогательных переменных для отрисовки графика
        const double xScale = SCREEN_WIDTH / (2 * M_PI);
        const double yScale = SCREEN_HEIGHT / 2.0;

        //Отрисовка координатной плоскости
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            canvas[SCREEN_HEIGHT / 2][x] = '-';
        }
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            canvas[y][SCREEN_WIDTH / 2] = '|';
        }
        canvas[SCREEN_HEIGHT / 2][SCREEN_WIDTH / 2] = '+';

        //Рисуем график sin(x)
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            double radians = (x - SCREEN_WIDTH / 2.0) / xScale;
            int y = static_cast<int>(round(function(radians) * yScale)) + SCREEN_HEIGHT / 2;
            canvas[y][x] = '*';
        }

        return canvas;
    }

private:
    static double function(double x) {
        return sin(x);
    }
};

class Integrals {
private:
    const int A = 1;
    const int B = 5;
    const int N = 10000;
    const double H = fabs(B - A) / N;
    const double e = 0.001;


    double function(double x) {
        return cos(x) * pow(M_E, x);
    }

    double trapezeMethod() {
        double s = function(A) + function(B);
        for (int i = 1; i < N; i++) s += 2 * function(A + i * H);
        return s;
    }
    double rectangleMethod() {
        double s = 0;
        for (double x = B; x > A; x -= e) s += function(x) * e;
        return s;
    }

public:

    Integrals() = default;
/*
    ╔═════════════════════════════════════╗
    ║cos(x) * pow(e, x) на отрезке [1,5]: ║
    ╚═════════════════════════════════════╝
    ╔══════════════════════════════════╗
    ║Методом прямоугольников: -51.9692 ║
    ╚══════════════════════════════════╝
    ╔═══════════════════════════╗
    ║Методом трапеций: -51.9869 ║
    ╚═══════════════════════════╝
 */
    int point = 1;
    vector<string> menu_items{
            "---------------------------------------",
            "| cos(x) * pow(e, x) на отрезке [1,5]:|",
            "---------------------------------------",
            "---------------------------------------",
            "| Методом прямоугольников:            |",
            "---------------------------------------",
            "---------------------------------------",
            "| Методом трапеций:                   |",
            "---------------------------------------",
    };

    vector<vector<char>> render() {
        vector<vector<char>> canvas = generateCanvas();

        size_t y_start = (SCREEN_HEIGHT - menu_items.size()) / 2;

        for (int i = 0; i < menu_items.size(); i++) {
            size_t x_start = (SCREEN_WIDTH - menu_items[i].size()) / 2;
            for (int j = 0; j < menu_items[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menu_items[i][j];
            }
        }

        string answers[2];
        answers[0] = to_string(trapezeMethod());
        answers[1] = to_string(rectangleMethod());

        int k = 0;
        for (int i = 4; i < menu_items.size(); i+=3) {
            size_t x_start = (SCREEN_WIDTH - menu_items[i].size()) / 2;
            for (int j = 29; j < 37; j++) {
                canvas[i + y_start][j + x_start] = answers[k][j-29];
            }
            k+=1;
        }

        return canvas;
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);


    Screen screen;
    Menu menu;
    Graphic gr;
    Integrals in;

    Screen::Write(in.render());
//    Screen::Write(menu.render());
    while (true) {
//        if (GetAsyncKeyState(VK_UP) & 0x8000) {  // Верхняя стрелка
//            if (menu.point > 1) {
//                menu.point--;
//                Screen::Write(menu.render());
//            }
//        } else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {  // Нижняя стрелка
//            if (menu.point < (menu.menu_items.size() - 1)) {
//                menu.point++;
//                Screen::Write(menu.render());
//            }
//
//        } else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {  // Клавиша ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {  // Клавиша ESC
            break;
        } else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {  // Клавиша Enter
            // Ваш код для обработки нажатия Enter
        }
        Sleep(100);

////        switch (input) {
////            case 49: // 1
//////                cout << 1 << endl;
////                menu.point = 1;
////                break;
////            case 50: // 2
//////                cout << 2 << endl;
////                menu.point = 2;
////                break;
////            case 51: // 3
//////                cout << 3 << endl;
////                menu.point = 3;
////                break;
////            case 52: // 4
//////                cout << 4 << endl;
////                menu.point = 4;
////                break;
////            case 53: // 5
//////                cout << 5 << endl;
////                menu.point = 5;
////                break;
////            case 65: // up
////                if (menu.point < (menu.menu_items.size() - 1)) menu.point ++;
////                break;
////            case 66: // down
////                if (menu.point > 1) menu.point --;
////                break;
////            case 13: // enter
////                break;
////            case 27: // esc
////                cout << CLEAR;
////                exit(1);
////            default:
////                break;
////        }
    }
}