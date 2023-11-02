#include <iostream>

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

