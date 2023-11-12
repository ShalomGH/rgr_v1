#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>


#define RESET_CODE   "\033[0m"
#define GREEN_CODE   "\033[32m"      /* Green */
#define MAGENTA_CODE "\033[35m"      /* Magenta */


#ifdef _WIN32

#include <windows.h>

#define CLEAR_CODE u8"\033[2J\033[1;1H" /* clear console */


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

static auto previousButtonsTime = std::chrono::system_clock::now();
static const int baseButtonsDelay = 250;

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
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - previousButtonsTime).count() -
            baseButtonsDelay < 0)
            return NOTHING;
#ifdef _WIN32
        if (GetAsyncKeyState(VK_UP) & 0x8000) {  // Верхняя стрелка
            previousButtonsTime = now;
            return ARROW_UP;
        } else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {  // Нижняя стрелка
            previousButtonsTime = now;
            return ARROW_DOWN;
        } else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {  // Клавиша ESC
            previousButtonsTime = now;
            return ESC;
        } else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {  // Клавиша Enter
            previousButtonsTime = now;
            return ENTER;
        }
#else
        char input = getch();
        switch (input) {
            case 65: // up
            previousButtonsTime = baseButtonsDelay;
                return ARROW_UP;
                break;
            case 66: // down
            previousButtonsTime = baseButtonsDelay;
                return ARROW_DOWN;
                break;
            case 13: // enter
            previousButtonsTime = baseButtonsDelay;
                return ENTER;
                break;
            case 27: //esc
            previousButtonsTime = baseButtonsDelay;
                return ESC;
            default:
                break;
#endif
        return NOTHING;
    }
};

enum ScreenIds {
    MENU = 0,
    TABLE,
    GRAPHIC,
    EQUATION,
    INTEGRALS,
    ANIMATION,
    AUTHOR,
    EXIT,
};

ScreenIds screenId = ScreenIds::MENU;


class Screen {
protected:
    vector<vector<char>> canvas;
    vector<string> menuItems;

    size_t yStart;
    size_t xStart;

public:

    virtual void render() {
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ESC):
                screenId = ScreenIds::MENU;
                break;
        }
    };

    virtual void update() {
#ifdef _WIN32
        cout << CLEAR_CODE;
#endif
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
    void configureScreen() {
        canvas = generateCanvas();
        fillMenuItems();
        calculateCords();
        drawMenuItems();
    }

    static vector<vector<char>> generateCanvas() {
        vector<vector<char>> canvas1;
        canvas1.resize(SCREEN_HEIGHT);
        for (auto &i: canvas1) i.resize(SCREEN_WIDTH);
        return canvas1;
    }

    virtual void drawMenuItems() {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menuItems[i][j];
            }
        }
    }

    virtual void fillMenuItems() {};

private:
    virtual void calculateCords() {
        yStart = (SCREEN_HEIGHT - menuItems.size()) / 2;
        xStart = (SCREEN_WIDTH - menuItems[1].size()) / 2;
    }
};


class Menu : public Screen {
protected:
    void fillMenuItems() override {
        menuItems = {
                "Menu         ",
                "1.  Table    ",
                "2.  Graphics ",
                "3.  Equation ",
                "4.  Integrals",
                "5.  Animation",
                "6.  Author   ",
                "7.  Exit     ",
        };
    }

private:
    int point = 1;

    size_t yPoint;
    size_t xPoint;

public:
    Menu() {
        configureScreen();
        yPoint = yStart + point;
        xPoint = xStart - 2;
        canvas[yPoint][xPoint] = '*';
    }

    void render() override {
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ARROW_DOWN):
                movePointDown();
                break;
            case (Buttons::Keys::ARROW_UP):
                movePointUp();
                break;
            case (Buttons::Keys::ENTER):
                screenId = static_cast<ScreenIds>(point);
                break;
        }
    }

    void update() override {
        checkPointPosition();
        Screen::update();
    }

    void movePointDown() {
        if (point < 7) point++;
        else point = 1;
        update();
    }

    void movePointUp() {
        if (point > 1) point--;
        else point = 7;
        update();
    }

private:
    void checkPointPosition() {
        if (yStart + point != yPoint) {
            canvas[yPoint][xPoint] = ' ';
            yPoint = yStart + point;
            canvas[yPoint][xPoint] = '*';
        }
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

protected:
    void fillMenuItems() override {
        menuItems.emplace_back("_____________________________________________");
        menuItems.emplace_back("|    i   |  x[i]  |    F1[i]   |    F2[i]   |");
        menuItems.emplace_back("|___________________________________________|");
        for (int i = 0; i < N; i++) {
            if (i < 9) menuItems.emplace_back("|   " + to_string(i + 1) + "    |        |            |            |");
            else menuItems.emplace_back("|   " + to_string(i + 1) + "   |        |            |            |");
        }
        menuItems.emplace_back("|___________________________________________|");

        Screen::fillMenuItems();
    }

private:
    const double dX = fabs(B - A) / (N - 1.0);

public:
    Table() {
        configureScreen();
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
        canvas = generateCanvas();
        drawCoordinates();
        drawGraphic();
    }

protected:
    void fillMenuItems() override {
        menuItems = {" "};
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

public:
    Equation() {
        configureScreen();
        drawAnswers();
    }


protected:
    void fillMenuItems() override {
        menuItems = {
                "____________________________________________________",
                "| Equation x^3 + 3x + 2 = 0 on the segment [0,4]   |",
                "----------------------------------------------------",
                "----------------------------------------------------",
                "| Bisection method:                                |",
                "----------------------------------------------------",
        };
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

    void drawMenuItems() override {
        for (int i = 0; i < menuItems.size(); i++) {
            for (int j = 0; j < menuItems[i].size(); j++) {
                canvas[i + yStart][j + xStart] = menuItems[i][j];
            }
        }
    }

    void drawAnswers() {
        string answers[1];
        answers[0] = to_string(bisectionMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i += 3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + yStart][j + xStart] = answers[k][j - 35];
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

protected:
    void fillMenuItems() override {
        menuItems = {
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
    }

    const double H = fabs(B - A) / N;

public:
    Integrals() {
        configureScreen();
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

    void drawAnswers() {
        string answers[2];
        answers[0] = to_string(trapezeMethod());
        answers[1] = to_string(rectangleMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i += 3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + yStart][j + xStart] = answers[k][j - 35];
            }
            k += 1;
        }
    }
};


static auto previousAnimationTime = std::chrono::system_clock::now();

class Animation : public Screen {

private:
    const int delay = 50;
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
    };



    vector<vector<vector<char>>> frames;
    int frame = 0;
    bool isGoing = false;

public:
    Animation() {
        frames.resize(framesStr.size());
        for (int i = 0; i < frames.size(); i++) frames[i] = generateCanvas();
        yStart = (SCREEN_HEIGHT - framesStr[0].size()) / 2;
        xStart = (SCREEN_WIDTH - framesStr[0][0].size()) / 2;
        drawFrames();
    }

    void update() override {
        canvas = getFrame();
        Screen::update();
    }

    void render() override {
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - previousAnimationTime).count() -
                    delay > 0) {
            update();
            previousAnimationTime = now;
        }
        Screen::render();
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
                    frames[k][i + yStart][j + xStart] = framesStr[k][i][j];
                }
            }
        }
    }
};

class Author : public Screen {
protected:
    void fillMenuItems() override {
        menuItems = {
                R"(/ \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \)",
                "RGR for programming                                        ",
                "University: OmSTU                                          ",
                "Faculty: FiTIKS                                            ",
                "Group: PI-232                                              ",
                "kizyakin kizyak kizyakovich                                ",
                R"(\ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ /)",
        };
    }

public:
    Author() {
        configureScreen();
    }
};


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


int main() {
    Screen *screens[7];

    configure();

    screens[0] = new Menu;
    screens[1] = new Table;
    screens[2] = new Graphic;
    screens[3] = new Equation;
    screens[4] = new Integrals;
    screens[5] = new Animation;
    screens[6] = new Author;

    ScreenIds preId = ScreenIds::TABLE;

    while (screenId != ScreenIds::EXIT) {
        if (screenId != preId) {
            screens[screenId]->update();
            preId = screenId;
        }
        screens[screenId]->render();
    }

    for (int i = 0; i < 7; i++) delete screens[i];
    return 0;
}