#include <iostream>
#include <string>
#include <vector>


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

class Screen {
public:
    vector<vector<char>> canvas;

    Screen() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        SCREEN_WIDTH = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        SCREEN_HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize size{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        SCREEN_HEIGHT = size.ws_row;
        SCREEN_WIDTH = size.ws_col;
#endif
        canvas.resize(SCREEN_HEIGHT);
        for (auto &i: canvas) i.resize(SCREEN_WIDTH);
    }

    Screen(const Screen &screen)
    = default;

//    void printSize() const {
//        cout << "x = " << SCREEN_WIDTH << ",   y = " << SCREEN_HEIGHT << "\n";
//    }

    static void Write(vector<vector<char>> scene) {
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            for (int j = 0; j < SCREEN_WIDTH; ++j)
                if (scene[i][j])
                    cout << scene[i][j];
                else
                    cout << " ";
            cout << "\n";
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
            "4.  Hui      ",
    };

    Menu() = default;

    vector<vector<char>> render() {
        vector<vector<char>> canvas;
        canvas.resize(SCREEN_HEIGHT);
        for (auto &i: canvas) i.resize(SCREEN_WIDTH);

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


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Screen screen;
    Menu menu;
    while (true) {
        Screen::Write(menu.render());
        int input = getch();
        cout << input;
        switch (input) {
            case 49: // 1
                cout << 1 << endl;
                break;
            case 50: // 2
                cout << 2 << endl;
                break;
            case 51: // 3
                cout << 3 << endl;
                break;
            case 52: // 4
                cout << 4 << endl;
                break;
            case 53: // 5
                cout << 5 << endl;
                break;
            case 80: // up
                if (menu.point < (menu.menu_items.size() - 1)) menu.point += 1;
                break;
            case 72: // down
                if (menu.point > 1) menu.point -= 1;
                break;
            case 13: // enter
                break;
            case 27: // esc
                cout << CLEAR;
                exit(1);
            default:
                break;
        }
    }
}