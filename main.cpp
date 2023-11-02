#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <unistd.h>
#include <termios.h>
#include <ctime>
#include <fcntl.h>
#include <algorithm>
#include <map>
#include <cmath>
#include <tuple>
#include <thread>
#include <chrono>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define CLEAR u8"\033[2J\033[1;1H" /* clear console */


#ifdef _WIN32

#include <windows.h>

#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

class Screen {
public:
    int size_x;
    int size_y;

    Screen() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        size_x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size_y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize size{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        size_y = size.ws_row;
        size_x = size.ws_col;
#endif
    }

    Screen (const Screen &screen)
            = default;

    void printSize() const {
        cout << "x = " << size_x << ",   y = " << size_y << endl;
    }
};

int main() {
    Screen screen;
    screen.printSize();
}

