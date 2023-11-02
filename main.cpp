#include <sys/ioctl.h>
#include "iostream"
#include <unistd.h>

using namespace std;

class Screen {
public:
    int size_x;
    int size_y;

    Screen() {
        struct winsize size{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        size_y = size.ws_row;
        size_x = size.ws_col;
    }

    void printSize() const {
        cout << "x = " << size_x << ",   y = " << size_y <<endl;
    }
};

int main() {
    Screen screen;
    screen.printSize();
}

