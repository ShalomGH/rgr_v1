#include <iostream>
#include <string>
#include <vector>
#include <cmath>


#define RESET_CODE   "\033[0m"
#define GREEN_CODE   "\033[32m"      /* Green */
#define MAGENTA_CODE "\033[35m"      /* Magenta */


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


static int SCREEN_HEIGHT;
static int SCREEN_WIDTH;

class Color {
public:
    static const char GREEN = ';';
    static const char MAGENTA = '?';
    static const char RESET = '%';
};


class Buttons {
public:
    enum Keys {
        NOTHING = 0,
        ARROW_UP,
        ARROW_DOWN,
        ENTER,
        ESC,
    };

    static int getKeyCode() {
#ifdef _WIN32
        if (GetAsyncKeyState(VK_UP) & 0x8000) {  // Верхняя стрелка
            return ARROW_UP;
        } else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {  // Нижняя стрелка
            return ARROW_DOWN;
        } else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {  // Клавиша ESC
            return ESC;
        } else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {  // Клавиша Enter
            return ENTER;
        }
#else
        char input = getch();
        switch (input) {
            case 65: // up
                return ARROW_UP;
                break;
            case 66: // down
                return ARROW_DOWN;
                break;
            case 13: // enter
                return ENTER;
                break;
            case 27: //esc
                return ESC;
            default:
                break;
#endif
        return NOTHING;
    }
};


class Screen {
protected:
    vector<vector<char>> canvas;


public:
    Screen() {
        canvas = generateCanvas(canvas);
    }

    virtual void render() {}

    virtual void update() {
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            for (int j = 0; j < SCREEN_WIDTH; ++j)
                if (canvas[i][j]) {
                    if (canvas[i][j] == Color::GREEN) cout << GREEN_CODE << " ";
                    else if (canvas[i][j] == Color::MAGENTA) cout << MAGENTA_CODE << " ";
                    else if (canvas[i][j] == Color::RESET) cout << RESET_CODE << " ";
                    else cout << canvas[i][j];
                } else
                    cout << " ";
            cout << endl;
        }
    }

protected:

    static vector<vector<char>> generateCanvas(vector<vector<char>> canvas1) {
        canvas1.resize(SCREEN_HEIGHT);
        for (auto &i: canvas1) i.resize(SCREEN_WIDTH);
        return canvas1;
    }
};

class Menu : public Screen {
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

    const size_t yStart = (SCREEN_HEIGHT - menu_items.size()) / 2;
    const size_t xStart = (SCREEN_WIDTH - menu_items[1].size()) / 2;

    size_t yPoint = yStart + point;
    const size_t xPoint = xStart - 2;

public:
    Menu() {
        drawMenuItems();
    }

    void render() override {
        checkPointPosition();
        Screen::update();
        while (true) {
            switch (Buttons::getKeyCode()) {
                case (Buttons::Keys::ARROW_DOWN):
                    movePointDown();
                    break;
                case (Buttons::Keys::ARROW_UP):
                    movePointUp();
                    break;
                case (Buttons::Keys::ENTER):

                    break;
            }
        }
    }

private:
    void movePointDown() {
        if (point < 7) point++;
        else point = 1;
    }

    void movePointUp() {
        if (point > 1) point--;
        else point = 7;
    }

    void checkPointPosition() {
        if (yStart + point != yPoint) {
            canvas[yPoint][xPoint] = ' ';
            yPoint = yStart + point;
            canvas[yPoint][xPoint] = '*';
        }
    }

    void drawMenuItems() {
        for (int i = 0; i < menu_items.size(); i++) {
            for (int j = 0; j < menu_items[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menu_items[i][j];
            }
        }
        canvas[yPoint][xPoint] = '*';
    }
};

class Table : public Screen {
private:
    const int N = 12;
    const int A = 2;
    const int B = 4;

    static double F1(double x) {
        return pow(M_E, 2 * x) * pow(x, 1 / 3) - sin(x);
    }

    static double F2(double x) {
        return 10 / (2 + x * x);
    }


    const double dX = fabs(B - A) / (N - 1.0);
    size_t yStart;
    size_t xStart;
    vector<string> menuItems;

public:
    Table() {
        fillMenuItems();
        yStart = (SCREEN_HEIGHT - menuItems.size()) / 2;
        xStart = (SCREEN_WIDTH - menuItems[0].size()) / 2;
        drawMenuItems();

        drawAnswers();
    }

private:
    [[nodiscard]] vector<vector<double>> calculateArray() const {
        vector<vector<double>> XF1F2;

        XF1F2.resize(3);
        for (auto &i: XF1F2) i.resize(N);

        XF1F2[0][0] = double(A);
        for (int i = 0; i < N; i++) {
            if (i != 0) XF1F2[0][i] = XF1F2[0][i - 1] + dX;
            XF1F2[1][i] = F1(XF1F2[0][i]);
            XF1F2[2][i] = F2(XF1F2[0][i]);
        }

        return XF1F2;
    }

    void drawAnswers() {
        vector<vector<double>> XF1F2 = calculateArray();

        double maxF1 = findMax(XF1F2[1]);
        double maxF2 = findMax(XF1F2[2]);
        double minF1 = findMin(XF1F2[1]);
        double minF2 = findMin(XF1F2[2]);

        for (int i = 3; i < N + 3; i++) {
            int k = 0;
            for (int j = 0; j < 6; j++) {
                canvas[i + yStart][11 + j + xStart] = to_string(XF1F2[0][i - 3])[k];
                canvas[i + yStart][21 + xStart] =
                        (XF1F2[1][i - 3] == maxF1) ? Color::GREEN : (XF1F2[1][i - 3] == minF1) ? Color::MAGENTA : ' ';
                canvas[i + yStart][22 + j + xStart] = to_string(XF1F2[1][i - 3])[k];
                canvas[i + yStart][22 + 6 + xStart] = Color::RESET;
                canvas[i + yStart][32 + xStart] =
                        (XF1F2[2][i - 3] == maxF2) ? Color::GREEN : (XF1F2[2][i - 3] == minF2) ? Color::MAGENTA : ' ';
                canvas[i + yStart][33 + j + xStart] = to_string(XF1F2[2][i - 3])[k];
                canvas[i + yStart][33 + 6 + xStart] = Color::RESET;
                k++;
            }
        }

        const string maxF1Str = "Max F1: " + to_string(maxF1);
        const string maxF2Str = "Max F2: " + to_string(maxF2);
        const string minF1Str = "Min F1: " + to_string(minF1);
        const string minF2Str = "Min F2: " + to_string(minF2);

        canvas[yStart + 3 + N + 2 + 0][xStart - 1] = Color::GREEN;
        for (int j = 0; j < 15; j++) canvas[yStart + 3 + N + 2 + 0][xStart + j] = maxF1Str[j];
        for (int j = 0; j < 15; j++) canvas[yStart + 3 + N + 2 + 1][xStart + j] = maxF2Str[j];
        canvas[yStart + 3 + N + 2 + 2][xStart - 1] = Color::MAGENTA;
        for (int j = 0; j < 15; j++) canvas[yStart + 3 + N + 2 + 2][xStart + j] = minF1Str[j];
        for (int j = 0; j < 15; j++) canvas[yStart + 3 + N + 2 + 3][xStart + j] = minF2Str[j];
        canvas[yStart + 3 + N + 2 + 4][xStart - 1] = Color::RESET;
    }

    [[nodiscard]] double findMax(vector<double> F) const {
        double max1 = F[0];
        for (int i = 0; i < N; i++)
            if (F[i] > max1)max1 = F[i];
        return max1;
    }

    [[nodiscard]] double findMin(vector<double> F) const {
        double min1 = F[0];
        for (int i = 0; i < N; i++)
            if (F[i] < min1)min1 = F[i];
        return min1;
    }

    void fillMenuItems() {
        menuItems.emplace_back("_____________________________________________");
        menuItems.emplace_back("|    i   |  x[i]  |    F1[i]   |    F2[i]   |");
        menuItems.emplace_back("|___________________________________________|");
        for (int i = 0; i < N; i++) {
            if (i < 9) menuItems.emplace_back("|   " + to_string(i + 1) + "    |        |            |            |");
            else menuItems.emplace_back("|   " + to_string(i + 1) + "   |        |            |            |");
        }
        menuItems.emplace_back("|___________________________________________|");
    }

    void drawMenuItems() {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menuItems[i][j];
            }
        }
    }
};

class Graphic : public Screen {

private:
    static double function(double x) {
        return sin(x);
    }

    const double xScale = SCREEN_WIDTH / (2 * M_PI);
    const double yScale = SCREEN_HEIGHT / 2.0;

public:
    Graphic() {
        drawCoordinates();
        drawGraphic();
    }

private:

    void drawCoordinates() {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            canvas[SCREEN_HEIGHT / 2][x] = '-';
        }
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            canvas[y][SCREEN_WIDTH / 2] = '|';
        }
        canvas[SCREEN_HEIGHT / 2][SCREEN_WIDTH / 2] = '+';
    }

    void drawGraphic() {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            double radians = (x - SCREEN_WIDTH / 2.0) / xScale;
            int y = static_cast<int>(round(function(radians) * yScale)) + SCREEN_HEIGHT / 2;
            canvas[y][x] = '*';
        }
    }
};

class Equation : public Screen {
private:
    const int A = 0;
    const int B = 4;
    const double e = 0.001;

    static double function(double x) {
        return pow(x, 3) + 3 * x + 2;
    }

    vector<string> menuItems{
            "____________________________________________________",
            "| Equation x^3 + 3x + 2 = 0 on the segment [0,4]   |",
            "----------------------------------------------------",
            "----------------------------------------------------",
            "| Bisection method:                                |",
            "----------------------------------------------------",
    };


    const size_t y_start = (SCREEN_HEIGHT - menuItems.size()) / 2;
    const size_t x_start = (SCREEN_WIDTH - menuItems[0].size()) / 2;

public:
    Equation() {
        drawMenuItems();
        drawAnswers();
    }

private:
    [[nodiscard]] double bisectionMethod() const {
        double a = A;
        double b = B;
        double x = 0;
        while ((b - a) >= e) {
            x = (a + b) / 2;
            if (function(x) * function(a) < 0) b = x;
            else a = x;
        }
        return x;
    }

    void drawMenuItems() {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menuItems[i][j];
            }
        }
    }

    void drawAnswers() {
        string answers[1];
        answers[0] = to_string(bisectionMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i += 3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + y_start][j + x_start] = answers[k][j - 35];
            }
            k += 1;
        }
    }
};

class Integrals : public Screen {
private:
    const int A = 1;
    const int B = 5;
    const int N = 10000;
    const double e = 0.001;

    static double function(double x) {
        return cos(x) * pow(M_E, x);
    }

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


    const size_t y_start = (SCREEN_HEIGHT - menuItems.size()) / 2;
    const size_t x_start = (SCREEN_WIDTH - menuItems[0].size()) / 2;
    const double H = fabs(B - A) / N;

public:
    Integrals() {
        drawMenuItems();
        drawAnswers();
    }

private:
    [[nodiscard]] double trapezeMethod() const {
        double s = function(A) + function(B);
        for (int i = 1; i < N; i++) s += 2 * function(A + i * H);
        return (H / 2) * s;
    }

    [[nodiscard]] double rectangleMethod() const {
        double s = 0;
        for (double x = B; x > A; x -= e) s += function(x) * e;
        return s;
    }

    void drawMenuItems() {
        for (int i = 0; i < menuItems.size(); i++) {

            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menuItems[i][j];
            }
        }
    }

    void drawAnswers() {
        string answers[2];
        answers[0] = to_string(trapezeMethod());
        answers[1] = to_string(rectangleMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i += 3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + y_start][j + x_start] = answers[k][j - 35];
            }
            k += 1;
        }
    }
};

class Animation : public Screen {

private:
    const int delay = 150;


    vector<vector<vector<char>>> frames;
    int frame = 0;
    bool isGoing = false;

    vector<vector<string>> framesStr{
            {
                    "  |    |  ",
                    "  |    |  ",
                    "()|()()|()",
            },
            {
                    "          ",
                    "  |    |  ",
                    "()|()()|()",
            },
            {
                    "          ",
                    "          ",
                    "()|()()|()",
            },
            {
                    "          ",
                    "  |    |  ",
                    "()|()()|()",
            },
            {
                    "          ",
                    "  |    |  ",
                    "()|()()|()",
            },
    };

    const size_t y_start = (SCREEN_HEIGHT - framesStr[0].size()) / 2;
    const size_t x_start = (SCREEN_WIDTH - framesStr[0][0].size()) / 2;

public:
    Animation() {
        frames.resize(framesStr.size());
        for (int i = 0; i < frames.size(); i++) frames[i] = generateCanvas(frames[i]);
        drawFrames();
    }

    void update() override {
        canvas = getFrame();
        Screen::update();
        Sleep(delay);
    }

private:

    vector<vector<char>> getFrame() {
        (frame < frames.size() - 1) ? frame++ : frame = 0;
        return frames[frame];
    }

    void drawFrames() {
        for (int k = 0; k < framesStr.size(); ++k) {
            for (int i = 0; i < framesStr[k].size(); i++) {
                for (int j = 0; j < framesStr[k][i].size(); j++) {
                    frames[k][i + y_start][j + x_start] = framesStr[k][i][j];
                }
            }
        }
    }
};

class Author : public Screen {
private:
    vector<string> menuItems{
            R"(/ \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \)",
            "RGR for programming                                        ",
            "University: OmSTU                                          ",
            "Faculty: FiTIKS                                            ",
            "Group: PI-232                                              ",
            "kizyakin kizyak kizyakovich                                ",
            R"(\ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ /)",
    };



    const size_t y_start = (SCREEN_HEIGHT - menuItems.size()) / 2;
    const size_t x_start = (SCREEN_WIDTH - menuItems[0].size()) / 2;

public:
    Author() {
        drawMenuItems();
    }

private:
    void drawMenuItems() {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + y_start][j + x_start] = menuItems[i][j];
            }
        }
    }
};

class App {
public:
    static const Menu menu;
    static const Table table;
    static const Graphic graphic;
    static const Equation equation;
    static const Integrals integrals;
    static const Animation animation;
    static const Author author;

    App() {
        configure();
    }

    static void setScreen(Screen screen) {
        screen.render();
    }

private:
    static void configure() {
        ios::sync_with_stdio(false);
        cin.tie(nullptr);
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
};

int main() {
    App app;
    App::setScreen(App::menu);
}