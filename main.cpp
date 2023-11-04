#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define USE_MATH_DEFINES

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

class Screen {
public:
    
    static void coinfigurateConsole() {
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

    static void render(vector<vector<char>> scene) {
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

    //Функция создания пустого экрана
    static vector<vector<char>> generateCanvas() {
        vector<vector<char>> canvas;
        canvas.resize(SCREEN_HEIGHT);
        for (auto &i: canvas) i.resize(SCREEN_WIDTH);
        return canvas;
    }
};


class Menu {
private:
    int point = 1;
    const vector<string> menu_items{
            "Menu",
            "1.  Table    ",
            "2.  Graphics ",
            "3.  Equation ",
            "4.  Integrals",
            "5.  Animation",
            "6.  Author   ",
            "7.  Exit     ",
    };
    vector<vector<char>> canvas;

    const size_t yStart = (SCREEN_HEIGHT - menu_items.size()) / 2;
    const size_t xStart = (SCREEN_WIDTH - menu_items[1].size()) / 2;

    size_t yPoint = yStart + point;
    const size_t xPoint = xStart - 2;

public:
    Menu() {
        canvas = Screen::generateCanvas();
        moveMenuItemsIntoCanvas();
    }

    vector<vector<char>> getCanvas() {
        checkPointPosition();
        return canvas;
    }

    int getPoint() {
        return point;
    }
    void movePointDown() {
        if(point<7) point++;
    }
    void movePointUp() {
        if(point>1) point--;
    }

private:
    void checkPointPosition() {
        if(yStart + point != yPoint) {
            canvas[yPoint][xPoint] = ' ';
            yPoint = yStart + point;
            canvas[yPoint][xPoint] = '*';
        }
    }

    void moveMenuItemsIntoCanvas() {
        for (int i = 0; i < menu_items.size(); i++) {
            for (int j = 0; j < menu_items[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menu_items[i][j];
            }
        }
        canvas[yPoint][xPoint] = '*';
    }
};

class Table {


private:
    const int N = 10;
    const int A = 1;
    const int B = 2;
    const double dX = fabs(B - A) / (N - 1.0);
    size_t yStart;
    size_t xStart;
    vector<vector<char>> canvas;
    vector<string> menuItems;

public:
    Table() {
        canvas = Screen::generateCanvas();

        fillMenuItems();
        yStart = (SCREEN_HEIGHT - menuItems.size()) / 2;
        xStart = (SCREEN_WIDTH - menuItems[0].size()) / 2;
        moveMenuItemsIntoCanvas();

    }
/*
                                _____________________________________________
                                |       i|    x[i]|       F1[i]|       F2[i]|
                                |___________________________________________|
                                |       1|       1|     6.54759|     3.33333|
                                |       2| 1.11111|     8.33162|      3.0916|
                                |       3| 1.22222|     10.5843|     2.86219|
                                |       4| 1.33333|       13.42|     2.64706|
                                |       5| 1.44444|     16.9813|     2.44713|
                                |       6| 1.55556|     21.4461|     2.26257|
                                |       7| 1.66667|     27.0362|     2.09302|
                                |       8| 1.77778|     34.0286|      1.9378|
                                |       9| 1.88889|     42.7689|     1.79601|
                                |      10|       2|     53.6889|     1.66667|
                                |___________________________________________|

                                Минимальный элемент первой функции:-9.25596e+61
                                Максимальный элемент первой функции:8.33162
                                Минимальный элемент второй функции:-9.25596e+61
                                Максимальный элемент второй функции:10.5843
 */
    vector<vector<char>> getCanvas() {
        return canvas;
    }

private:
    double F1(double x) {
        return pow(M_E, 2 * x) * pow(x, 1 / 3) - sin(x);
    }

    double F2(double x) {
        return 10 / (2 + x * x);
    }

    void fillMenuItems() {
        menuItems.push_back("_____________________________________________");
        menuItems.push_back("|    i   |  x[i]  |    F1[i]   |    F2[i]   |");
        menuItems.push_back("|___________________________________________|");
        for (int i = 0; i<N; i++) {
            if (i<9) menuItems.emplace_back("|   " + to_string(i + 1) + "    |        |            |            |");
            else menuItems.emplace_back("|   " + to_string(i + 1) + "   |        |            |            |");
        }
        menuItems.emplace_back("|___________________________________________|");
    }

    void moveMenuItemsIntoCanvas() {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menuItems[i][j];
            }
        }
    }
};

class Graphic {
private:
    //Создание экрана
    vector<vector<char>> canvas;

    //Создание вспомогательных переменных для отрисовки графика
    const double xScale = SCREEN_WIDTH / (2 * M_PI);
    const double yScale = SCREEN_HEIGHT / 2.0;

public:
    Graphic() {
        canvas = Screen::generateCanvas();
        drawCoordinates();
        drawGraphic();
    }

    vector<vector<char>> getCanvas() {
        return canvas;
    }

private:
    double function(double x) {
        return sin(x);
    }
    
    void drawCoordinates(){
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            canvas[SCREEN_HEIGHT / 2][x] = '-';
        }
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            canvas[y][SCREEN_WIDTH / 2] = '|';
        }
        canvas[SCREEN_HEIGHT / 2][SCREEN_WIDTH / 2] = '+';
    }
    
    void drawGraphic(){
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            double radians = (x - SCREEN_WIDTH / 2.0) / xScale;
            int y = static_cast<int>(round(function(radians) * yScale)) + SCREEN_HEIGHT / 2;
            canvas[y][x] = '*';
        }
    }
};


class Integrals {
public:

    Integrals() = default;
    vector<string> menuItems{
            "--------------------------------------------",
            "| cos(x) * pow(e, x) on the segment [1,5]: |",
            "--------------------------------------------",
            "--------------------------------------------",
            "| Rectangle method:                        |",
            "--------------------------------------------",
            "--------------------------------------------",
            "| Trapeze method:                          |",
            "--------------------------------------------",
    };

    vector<vector<char>> render() {
        vector<vector<char>> canvas = Screen::generateCanvas();

        size_t y_start = (SCREEN_HEIGHT - menuItems.size()) / 2;
        size_t x_start = (SCREEN_WIDTH - menuItems[0].size()) / 2;

        for (int i = 0; i < menuItems.size(); i++) {

            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menuItems[i][j];
            }
        }

        string answers[2];
        answers[0] = to_string(trapezeMethod());
        answers[1] = to_string(rectangleMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i+=3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + y_start][j + x_start] = answers[k][j-35];
            }
            k+=1;
        }

        return canvas;
    }
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
        return (H / 2) * s;
    }
    double rectangleMethod() {
        double s = 0;
        for (double x = B; x > A; x -= e) s += function(x) * e;
        return s;
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Screen::coinfigurateConsole();

    /*
     "Menu",
            "1.  Table    ",
            "2.  Graphics ",
            "3.  Equation ",
            "4.  Integrals",
            "5.  Animation",
            "6.  Author   ",
            "7.  Exit     ",
     */

    

    Menu menu;

    Table table;
    Graphic graphic;

    Integrals integrals;


    Screen::render(menu.getCanvas());
//    Screen::render(in.render());
    while (true) {
#ifdef _WIN32
        if (GetAsyncKeyState(VK_UP) & 0x8000) {  // Верхняя стрелка
            menu.movePointUp();
            Screen::render(menu.getCanvas());
        } else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {  // Нижняя стрелка
            menu.movePointDown();
            Screen::render(menu.getCanvas());

        } else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {  // Клавиша ESC
            Screen::render(menu.getCanvas());
            break;
        } else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {  // Клавиша Enter
            switch (menu.getPoint()) {
                case 1:
                    Screen::render(table.getCanvas());
                    break;
                case 2:
                    Screen::render(graphic.getCanvas());
                    break;
                case 3:
                    break;
                case 4:
                    Screen::render(integrals.render());
                    break;
                case 5:
                    break;
                case 6:
                    break;
                case 7:
                    exit(1);
                default:
                    break;
            }
        }
        Sleep(100);
#else
        char input = getch();
        switch (input) {
            case 49: // 1
                menu.point = 1;
                break;
            case 50: // 2
                menu.point = 2;
                break;
            case 51: // 3
                menu.point = 3;
                break;
            case 52: // 4
                menu.point = 4;
                break;
            case 53: // 5
                menu.point = 5;
                break;
            case 65: // up
                if (menu.point < (menu.menuItems.size() - 1)) menu.point ++;
                break;
            case 66: // down
                if (menu.point > 1) menu.point --;
                break;
            case 13: // enter
                break;
            case 27: // esc
                cout << CLEAR;
//                exit(1);
            default:
                break;
        }
#endif
    }
}