#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

#define RESET_CODE   "\033[0m"
#define GREEN_CODE   "\033[32m"      /* Green */
#define MAGENTA_CODE "\033[35m"      /* Magenta */
#define CLEAR_CODE u8"\033[2J\033[1;1H" /* clear console */


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
        return NOTHING;
#else
        int input = getch();
        switch (input) {
            case 65: // up
                previousButtonsTime = now;
                return ARROW_UP;
            case 66: // down
                previousButtonsTime = now;
                return ARROW_DOWN;
            case 10: // enter
                previousButtonsTime = now;
                return ENTER;
            case 9: // esc (tab)
                previousButtonsTime = now;
                return ESC;
            default:
                return NOTHING;
        }
#endif
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
                if (
                        i + yStart < canvas.size() &&
                        i + yStart >= 0 &&
                        j + xStart < canvas[0].size() &&
                        j + xStart >= 0
                        )
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
        menuItems.emplace_back("|   i    |  x[i]  |    F1[i]   |    F2[i]   |");
        menuItems.emplace_back("|________|________|____________|____________|");
        for (int i = 0; i < N; i++) {
            if (i < 9)
                menuItems.emplace_back("|   " + to_string(i + 1) + "    |        |            |            |");
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
                        (XF1F2[1][i - 3] == maxF1) ? Color::GREEN : (XF1F2[1][i - 3] == minF1) ? Color::MAGENTA
                                                                                               : ' ';
                canvas[i + yStart][22 + j + xStart] = to_string(XF1F2[1][i - 3])[k];
                canvas[i + yStart][22 + 6 + xStart] = Color::RESET;
                canvas[i + yStart][32 + xStart] =
                        (XF1F2[2][i - 3] == maxF2) ? Color::GREEN : (XF1F2[2][i - 3] == minF2) ? Color::MAGENTA
                                                                                               : ' ';
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
    vector<string> functionsNames{
            "* - E^(2 * x) * x^(1 / 3) - sin(x)  ",
            "# - 10 / (2 + x^2)                  ",
    };

    static double F1(double x) {
        return pow(M_E, 2 * x) * pow(x, 1 / 3) - sin(x);
    }

    static double F2(double x) {
        return 10 / (2 + x * x);
    }


    int scale = 2;

public:
    Graphic() {
        canvas = generateCanvas();
        drawCoordinates();
        drawGraphic();
        drawFunctionsNames();
    }

    void render() override {
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ARROW_DOWN):
                zoomOut();
                break;
            case (Buttons::Keys::ARROW_UP):
                zoomIn();
                break;
            case (Buttons::Keys::ESC):
                screenId = ScreenIds::MENU;
                break;
        }
    }

    void update() override {
        canvas = generateCanvas();
        drawCoordinates();
        drawGraphic();
        drawFunctionsNames();
        drawCordNames();
        Screen::update();
    }

protected:
    void fillMenuItems() override {
        menuItems = {" "};
    }

private:

    void drawCordNames() {
        canvas[SCREEN_HEIGHT / 2 - 1][SCREEN_WIDTH - 1] = 'X';
        canvas[0][SCREEN_WIDTH / 2 + 1] = 'Y';
    }

    void zoomOut() {
        if (scale >= 10) return;
        scale++;
        update();
    }

    void zoomIn() {
        if (scale < 2) return;
        scale--;
        update();
    }

    void drawFunctionsNames() {
        for (int x = 0; x < functionsNames[0].size() - 1; ++x) {
            canvas[0][SCREEN_WIDTH - functionsNames[0].size() + x] = functionsNames[0][x];
            canvas[1][SCREEN_WIDTH - functionsNames[0].size() + x] = functionsNames[1][x];
        }
        canvas[0][SCREEN_WIDTH - functionsNames[0].size() - 2] = Color::GREEN;
        canvas[1][SCREEN_WIDTH - functionsNames[0].size() - 2] = Color::MAGENTA;
        canvas[0][SCREEN_WIDTH - 2] = Color::RESET;
        canvas[1][SCREEN_WIDTH - 2] = Color::RESET;
    }

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
        const double xScale = SCREEN_WIDTH / (2 * M_PI) / scale;
        const double yScale = SCREEN_HEIGHT / 2.0 / scale;
        for (int x = 1; x < SCREEN_WIDTH - 1; ++x) {
            double radians = (x - SCREEN_WIDTH / 2.0) / xScale;
            int y1 = static_cast<int>(round(F1(radians) * yScale)) + SCREEN_HEIGHT / 2.0;
            int y2 = static_cast<int>(round(F2(radians) * yScale)) + SCREEN_HEIGHT / 2.0;

            if (y1 >= 0 && y1 < SCREEN_HEIGHT) {
                canvas[y1][x] = '*';
                if (canvas[y1][x - 1] != '*') canvas[y1][x - 1] = Color::GREEN;
                canvas[y1][x + 1] = Color::RESET;
            }
            if (y2 >= 0 && y2 < SCREEN_HEIGHT) {
                canvas[y2][x] = '#';
                if (canvas[y2][x - 1] != '#') canvas[y2][x - 1] = Color::MAGENTA;
                canvas[y2][x + 1] = Color::RESET;
            }
        }
    }
};

class Equation : public Screen {
private:
    int A = -1;
    int B = 4;
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
                "| Equation x^3 + 3x + 2 = 0 on the segment [  ,  ] |",
                "----------------------------------------------------",
                "----------------------------------------------------",
                "| Bisection method:                                |",
                "----------------------------------------------------",
                "----------------------------------------------------",
                "| Chords method:                                   |",
                "----------------------------------------------------",
        };
    }

public:
    void render() override {
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ESC):
                screenId = ScreenIds::MENU;
                break;
//            case (Buttons::Keys::ENTER):
//                screenId = ScreenIds::MENU;
//                break;
        }
    }

private:

    [[nodiscard]] double bisectionMethod() const {
        double a = A;
        double b = B;
        double x = 0;
        while ((b - a) > e) {
            x = (a + b) / 2;
            if (function(a) * function(b) > 0) return 404;
            if (function(x) * function(a) == 0) return x;
            if (function(x) * function(a) > 0) a = x;
            else b = x;
        }
        return (a + b) / 2;
    }

    [[nodiscard]] double chordMethod(double a, double b) const {
        while (fabs(function(b)) > e) {
            a = b - ((b - a) * function(b)) / (function(b) - function(a));
            b = a - ((a - b) * function(a)) / (function(a) - function(b));
        }
        return b;

    }

    void drawMenuItems() override {
        if (A < 0) menuItems[1][menuItems[1].size() - 8] = '-';
        if (A / 10 != 0) menuItems[1][menuItems[1].size() - 8] = abs(A) / 10 + '0';
        menuItems[1][menuItems[1].size() - 7] = abs(A) % 10 + '0';
        if (B < 0) menuItems[1][menuItems[1].size() - 5] = '-';
        if (B / 10 != 0) menuItems[1][menuItems[1].size() - 5] = abs(B) / 10 + '0';
        menuItems[1][menuItems[1].size() - 4] = abs(B) % 10 + '0';
        Screen::drawMenuItems();
    }

    void drawAnswers() {
        string answers[2];
        answers[0] = to_string(bisectionMethod());
        answers[1] = to_string(chordMethod(1.0, 4.0));
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
                "| cos(x) * pow(e, x) on the segment [  ,  ] |",
                "---------------------------------------------",
                "---------------------------------------------",
                "| Right Rectangle method:                   |",
                "---------------------------------------------",
                "---------------------------------------------",
                "| Trapeze method:                           |",
                "---------------------------------------------",
                "---------------------------------------------",
                "| Gauss method:                             |",
                "---------------------------------------------",
                "---------------------------------------------",
                "| Monte Carlo method:                       |",
                "---------------------------------------------",
                "---------------------------------------------",
                "| Middle Rectangle method:                  |",
                "---------------------------------------------",
        };
        menuItems.push_back("  e = " + to_string(e));
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
        for (float x = float(B); x > A; x -= e) s += function(x) * e;
        return s;
    }

    [[nodiscard]] double gaussMethod() const {
        const double weights[3] = {-0.7745967, 0, 0.7745967};
        const double nodes[3] = {0.5555556, 0.8888889, 0.5555556};

        double ra = (B - A) / 2;
        double su = (A + B) / 2;
        double Q, S = 0.0;
        for (int i = 0; i < 3; i++) {
            Q = su + ra * weights[i];
            S += nodes[i] * function(Q);
        }
        return ra * S;
    }


    [[nodiscard]] double monteCarloMethod() const {
        const int n = N * 100;
        double sum = 0.0;
        for (int i = 0; i < n; i++) {
            double x = A + static_cast<double>(rand()) / RAND_MAX * (B - A);
            sum += function(x);
        }
        return (B - A) * sum / n;
    }

    [[nodiscard]] double midRectangleMethod() const {
        double h = fabs(B - A) / N;
        double sum = 0.0;
        for (int i = 0; i < N; i++) {
            sum += function(A + (i + 0.5) * h);
        }
        return h * sum;
    }

    void drawMenuItems() override {
        if (A / 10 != 0) menuItems[1][menuItems[1].size() - 8] = A / 10 + '0';
        menuItems[1][menuItems[1].size() - 7] = A % 10 + '0';
        if (B / 10 != 0) menuItems[1][menuItems[1].size() - 5] = B / 10 + '0';
        menuItems[1][menuItems[1].size() - 4] = B % 10 + '0';

        Screen::drawMenuItems();
    }


    void drawAnswers() {
        string answers[5];
        answers[0] = to_string(rectangleMethod());
        answers[1] = to_string(trapezeMethod());
        answers[2] = to_string(gaussMethod());
        answers[3] = to_string(monteCarloMethod());
        answers[4] = to_string(midRectangleMethod());
        int k = 0;
        for (int i = 4; i < menuItems.size(); i += 3) {
            for (int j = 35; j < 41; j++) {
                canvas[i + yStart][j + xStart] = answers[k][j - 35];
            }
            k++;
        }
    }
};


static auto previousAnimationTime = std::chrono::system_clock::now();

class Animation : public Screen {

private:
    const int delay = 10;
    const vector<vector<char>> voidCanvas = generateCanvas();

public:
    Animation() {
        configureScreen();
        xStart = 0;
    }

    void render() override {
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - previousAnimationTime).count() -
            delay > 0) {
            canvas = voidCanvas;
            moveDrawing();
            update();
            previousAnimationTime = now;
        }
        Screen::render();
    }

private:
    void moveDrawing() {
        if (xStart < SCREEN_WIDTH - 1 || xStart > 0 - menuItems[0].size() - 1) xStart++;
        else xStart = 0 - menuItems[0].size();
        drawMenuItems();
    }

    void fillMenuItems() override {
        menuItems = {
                " _________________________    ",
                "|   |     |     |    | |  \\  ",
                "|___|_____|_____|____|_|___\\ ",
                "|                    | |    \\",
                "`--(o)(o)--------------(o)--' ",
        };
    }
};

class Author : public Screen {
protected:
    void fillMenuItems() override {
        menuItems = {
                R"(/ \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \    )",
                "RGR for programming                                       ",
                " University: OmSTU                                         ",
                " Faculty: FiTIKS                                           ",
                " Group: PI-232                                             ",
                " pistrunov pistrun pistrunovich                            ",
                R"(\ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ /    )",
        };


        menuItems[0].insert(0, 1, Color::GREEN);
        menuItems[1].insert(0, 1, Color::MAGENTA);
        menuItems[menuItems.size() - 1].insert(0, 1, Color::GREEN);
        menuItems[menuItems.size() - 1].insert(menuItems[0].size() - 2, menuItems[0].size() - 1, Color::RESET);
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
    SCREEN_HEIGHT = size.ws_row - 1;
    SCREEN_WIDTH = size.ws_col;
#endif
}


int main() {
    configure();
    Screen *screens[7];


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

    for (auto &screen: screens) delete screen;
    cout << CLEAR_CODE;
    exit(1);
}