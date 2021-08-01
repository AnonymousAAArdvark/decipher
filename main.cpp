#include <iostream>
#include <utility>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/ttycom.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <random>

#include "charset.h"

class Decipher {
public:
    explicit Decipher(std::string r) : ref(std::move(r)) {
        struct winsize w{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        maxlines = w.ws_row - 1;
        maxwidth = w.ws_col;
    }
    void printCipher() {
        set_terminal(0);
        create_decipher();
        getchar();
        solve_decipher();
        set_terminal(1);
    }

private:
    void create_decipher() {
        int currwidth = 0;
        for(int i = 0; i < ref.size(); ++i) {
            std::string d;
            if(currwidth == maxwidth) {
                currwidth = 0;
                int j = i;
                while(ref[j] != '\n') j++;
                ref.erase(i, j - i);
            }
            if(linecount == maxlines) {
                ref = ref.substr(0, i);
                break;
            }
            if(isspace(ref[i])) {
                std::cout << ref[i] << std::flush;
            }
            else {
                std::cout << random_char() << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(7));
            }
            if(ref[i] == '\n') {
                ++linecount;
                currwidth = 0;
            };
            ++currwidth;
        }
    }
    void solve_decipher() {
        std::vector<bool> solved(ref.size());
        std::string decipher;
        int revealed = 0, c = 0;
        std::random_device dev;
        std::mt19937 mt(dev());
        while(revealed != ref.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::cout << "\x1b[" << linecount << "A";
            decipher = "";
            ++c;
            std::uniform_int_distribution<int> dist(0, (int)(ref.size() - revealed) / (6));
            for (int i = 0; i < ref.size(); ++i) {
                if (solved[i]) {
                    decipher += ref[i];
                    std::cout << "\033[1m" << "\033[34m" << ref[i] << "\033[0m";
                }
                else if (isspace(ref[i]) || (c > 30 && dist(mt) == 0)) {
                    solved[i] = true;
                    revealed++;
                    decipher += ref[i];
                    std::cout << "\033[1m" << "\033[34m" << ref[i] << "\033[0m";
                }
                else {
                    std::cout << random_char() << std::flush;
                }
            }
        }
    }
    static inline std::string random_char() {
        std::random_device dev;
        static std::mt19937 mt(dev());
        static std::uniform_int_distribution<std::mt19937::result_type> dist(0,charTable.size());
        return charTable[dist(mt)];
    }
    static void set_terminal(int s) {
        struct termios tp{};
        static struct termios save;
        static int state = 1;

        if (!isatty(STDIN_FILENO))
            freopen("/dev/tty", "r", stdin);

        if (s == 0) {
            if (tcgetattr(STDIN_FILENO, &tp) == -1)
                return;

            save = tp;

            tp.c_lflag &=(~ICANON & ~ECHO);

            if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)
                return;
        } else {
            if (state == 0 && tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1)
                return;
        }

        state = s;
    }

    std::string ref;
    int maxlines, maxwidth, linecount{0};
};

int main() {
    if(isatty(STDIN_FILENO)) {
        std::cout << "This command is meant to work with pipes" << std::endl;
        std::cout << "Usage: output_command | decipher" << std::endl;
        return 0;
    }
    std::string line, output;
    while(std::getline(std::cin, line)) {
        output += line + "\n";
    }
    Decipher decipher(output);
    decipher.printCipher();
    return 0;
}