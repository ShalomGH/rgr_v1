#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

#define RESET_CODE   "\033[0m"
#define GREEN_CODE   "\033[32m"      /* Green */
#define MAGENTA_CODE "\033[35m"      /* Magenta */

#define Pi 3.14

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
        system("cls");
#endif
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            for (int j = 0; j < SCREEN_WIDTH; ++j)
                if (canvas[i][j]) {
                    switch (canvas[i][j]) {
                        case Color::GREEN:
                            cout << GREEN_CODE << " ";
                            break;
                        case Color::MAGENTA:
                            cout << MAGENTA_CODE << " ";
                            break;
                        case Color::RESET:
                            cout << RESET_CODE << " ";
                            break;
                        default:
                            cout << canvas[i][j];
                            break;
                    }
                } else cout << " ";
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
    const int N = 20, A = 0, B = Pi;

    static double F1(double x) {
        return pow(M_E, 0-x) + cos(2 * x);
    }

    static double F2(double x) {
        return pow(M_E, -2 * x);
    }

protected:
    void fillMenuItems() override {
        vector<vector<double>> XF1F2 = calculateArray();

        double maxF1 = findMax(XF1F2[1]);
        double maxF2 = findMax(XF1F2[2]);
        double minF1 = findMin(XF1F2[1]);
        double minF2 = findMin(XF1F2[2]);

        menuItems = {
                "____________________________________________",
                "|   i    |   x[i]  |    F1[i]  |    F2[i]  |",
                "|________|_________|___________|___________|",
        };

        for (int i = 0; i < N; i++) {
            menuItems.emplace_back("|        |        |            |           |");
            sprintf(menuItems[i + 3].data(), "|   %-2d   | %#2g | %#9g | %#9g |", i, XF1F2[0][i], XF1F2[1][i],
                    XF1F2[2][i]);

            menuItems[i + 3][21] = (XF1F2[1][i] == maxF1) ? Color::GREEN : (XF1F2[1][i] == minF1) ? Color::MAGENTA
                                                                                                  : ' ';
            menuItems[i + 3][30] = Color::RESET;
            menuItems[i + 3][33] = (XF1F2[2][i] == maxF2) ? Color::GREEN : (XF1F2[2][i] == minF2) ? Color::MAGENTA
                                                                                                  : ' ';
            menuItems[i + 3][42] = Color::RESET;
        }
        menuItems.emplace_back("|__________________________________________|");
        menuItems.emplace_back(";Max F1: " + to_string(maxF1));
        menuItems.emplace_back(" Max F2: " + to_string(maxF2) + "%");
        menuItems.emplace_back("?Min F1: " + to_string(minF1));
        menuItems.emplace_back(" Min F2: " + to_string(minF2) + "%");
    }

private:
    const double dX = fabs(B - A) / (N - 1.0);

public:
    Table() {
        configureScreen();
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

    [[nodiscard]] double findMax(vector<double> F) const {
        double max1 = F[0];
        for (int i = 0; i < N; i++) if (F[i] > max1)max1 = F[i];
        return max1;
    }

    [[nodiscard]] double findMin(vector<double> F) const {
        double min1 = F[0];
        for (int i = 0; i < N; i++) if (F[i] < min1)min1 = F[i];
        return min1;
    }
};

class Graphic : public Screen {
private:
    vector<string> functionsNames{
            ";* - E^-x + cos2x % ",
            "?# - E^-2x % ",
    };

    static double F1(double x) {
        return pow(M_E, 0-x) + cos(2 * x);
    }

    static double F2(double x) {
        return pow(M_E, -2 * x);
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
    }

    void drawCoordinates() {
        for (int x = 0; x < SCREEN_WIDTH; ++x) canvas[SCREEN_HEIGHT / 2][x] = '-';
        for (int y = 0; y < SCREEN_HEIGHT; ++y) canvas[y][SCREEN_WIDTH / 2] = '|';
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
    int A = 0, B = 0;
    const double e = 0.001;

    static double function(double x) {
        return pow(x, 3) + 3 * x + 2;
    }

    bool opened = true;

public:
    Equation() {
        configureScreen();
    }

    void render() override {
        if (opened) {
            opened = false;
            configureScreen();
            update();
            cin >> A;
            configureScreen();
            update();
            cin >> B;
            configureScreen();
            update();
        }
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ESC):
                opened = true;
                A = 0;
                B = 0;
                screenId = ScreenIds::MENU;
                break;
        }
    }

protected:
    void fillMenuItems() override {
        menuItems = {
                "____________________________________________________",
                format("| Equation x^3 + 3x + 2 = 0 on the segment[{:3},{:3}]|", A, B),
                "----------------------------------------------------",
                "----------------------------------------------------",
                format("| Bisection method:                 {:14f} |", bisectionMethod()),
                "----------------------------------------------------",
                "----------------------------------------------------",
                format("| Chords method:                    {:14f} |", chordsMethod()),
                "----------------------------------------------------",
        };
    }

private:

    [[nodiscard]] double bisectionMethod() const {
        double a = A, b = B, x = 0;
        while ((b - a) > e) {
            x = (a + b) / 2;
            if (function(a) * function(b) > 0) return NAN;
            if (function(x) * function(a) == 0) return x;
            if (function(x) * function(a) > 0) a = x;
            else b = x;
        }
        const double answer = (a + b) / 2;
        if (answer < A || answer > B) return NAN;
        return answer;
    }

    [[nodiscard]] double chordsMethod() const {
        double a = A, b = B;
        while (fabs(function(b)) > e) {
            a = b - ((b - a) * function(b)) / (function(b) - function(a));
            b = a - ((a - b) * function(a)) / (function(a) - function(b));
        }
        const double answer = b;
        if (answer < A || answer > B) return NAN;
        return answer;
    }
};

class Integrals : public Screen {
private:
    int A = 0;
    int B = 0;
    const int N = 10000;
    const double e = 0.001;

    static double function(double x) {
        return cos(x) * pow(M_E, x);
    }

    bool opened1 = true;

protected:
    void fillMenuItems() override {
        menuItems = {
                "---------------------------------------------",
                format("| cos(x) * pow(e, x) on the segment[{:3},{:3}]|",A,B),
                "---------------------------------------------",
                "---------------------------------------------",
                format("| Right Rectangle method:  {:16f} |", rectangleMethod()),
                "---------------------------------------------",
                "---------------------------------------------",
                format("| Trapeze method:          {:16f} |", trapezeMethod()),
                "---------------------------------------------",
                "---------------------------------------------",
                format("| Gauss method:            {:16f} |", gaussMethod()),
                "---------------------------------------------",
                "---------------------------------------------",
                format("| Monte Carlo method:      {:16f} |", monteCarloMethod()),
                "---------------------------------------------",
                "---------------------------------------------",
                format("| Middle Rectangle method: {:16f} |", midRectangleMethod()),
                "---------------------------------------------",
                "  e = " + to_string(e),
        };
    }

    const double H = fabs(B - A) / N;

public:
    Integrals() {
        configureScreen();
    }

    void render() override {
        if (opened1) {
            opened1 = false;
            configureScreen();
            update();
            cin >> A;
            configureScreen();
            update();
            cin >> B;
            configureScreen();
            update();
        }
        switch (Buttons::getKeyCode()) {
            case (Buttons::Keys::ESC):
                opened1 = true;
                A = 0;
                B = 0;
                screenId = ScreenIds::MENU;
                break;
        }
    }

private:
    [[nodiscard]] double trapezeMethod() const {
        double s = function(A) + function(B);
        for (int i = 1; i < N; i++) s += 2.0 * function(A + i * H);
        const double answer = (H / 2.0) * s;
        if (answer < A || answer > B) return NAN;
        return answer;
    }

    [[nodiscard]] double rectangleMethod() const {
        double s = 0;
        for (double x = B; x > A; x -= e) s += function(x) * e;
        const double answer = s;
        if (answer < A || answer > B) return NAN;
        return answer;
    }

    [[nodiscard]] double gaussMethod() const {
        const double weights[3] = {-0.7745967, 0, 0.7745967};
        const double nodes[3] = {0.5555556, 0.8888889, 0.5555556};
        double ra = (B - A) / 2.0, su = (A + B) / 2.0;
        double Q, S = 0.0;
        for (int i = 0; i < 3; i++) {
            Q = su + ra * weights[i];
            S += nodes[i] * function(Q);
        }
        const double answer = ra * S;
        if (answer < A || answer > B) return NAN;
        return answer;
    }


    [[nodiscard]] double monteCarloMethod() const {
        const int n = N * 100;
        double sum = 0.0;
        for (int i = 0; i < n; i++) {
            double x = A + static_cast<double>(rand()) / RAND_MAX * (B - A);
            sum += function(x);
        }
        const double answer = (B - A) * sum / n;
        if (answer < A || answer > B) return NAN;
        return answer;
    }

    [[nodiscard]] double midRectangleMethod() const {
        double h = fabs(B - A) / N, sum = 0.0;
        for (int i = 0; i < N; i++) sum += function(A + (i + 0.5) * h);
        const double answer = h * sum;
        if (answer < A || answer > B) return NAN;
        return answer;
    }
};

static auto previousAnimationTime = std::chrono::system_clock::now();

class Animation : public Screen {

private:
    void fillMenuItems() override {
        menuItems = {
                "       $$$$$$$$$",
                "     $$$  $$$$$$",
                "   $$    $$              $$$$$$",
                "   $   $             $$$$ $$$$$$$$$$",
                "  $$   $$$$$$$$      $$   $$       $$$$$$",
                "  $$ $$$$     $$$$$$$      $        $$ $$",
                "   $$$            $$       $$         $ $",
                "  $$             $$         $         $$ $$",
                " $$              $$         $           $ $$",
                " $                 $$$$    $$ $$    $$$$  $",
                " $                 $$$$$$$$$  $0$  $0$ $ $$",
                " $                $$$$   $$         $$  $$",
                " $$                $ $$            $ $$",
                " $                 $$_$$$$$$ $     $$$$",
                "$        $          $$$$$ $$$$     $$$",
                "$      $$$$$           $$$$$$$$$$$$$$",
                "$     $$ $ $$$            $  $$$     ",
                "$     $  $$  $$$      $$$ $   $$$$     ",
                "$      $  $$   $      $ $$$      $ $    ",
                "$$  $$$$   $$$$$$     $$ $ $      $$$$   ",
                " $$$$$$$      $$$      $$$  $       $$$   ",
                "                 $     $$  $$$       $  $$",
                "                 $$     $$$ $$$         $$",
                "                  $$      $$ $$$$$     $$",
                "                   $$$$ $$$     $$$$$$$",
                "                     $$$$$",
        };
    }

    const int delay = 50;


    const vector<vector<char>> voidCanvas = generateCanvas();

public:
    Animation() {
        configureScreen();
        xStart = 0;
    }

    void render() override {
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - previousAnimationTime).count() - delay > 0) {
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
};

class Author : public Screen {
protected:
    void fillMenuItems() override {
        menuItems = {
                R"(?-------======={ X }=======-------                     %)",
                ";       RGR for programming                                ",
                "         University: OmSTU                                 ",
                "         Faculty: FoITaCS                                  ",
                "          Group: IST-231                                   ",
                "     Ilichev Andrey Sergeevich                            %",
                R"(?-------======={ X }=======-------                     %)",
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
    exit(1);
}