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
    explicit Decipher(std::string r, bool a, bool e, int c, int s)
    : ref(std::move(r)), auto_decipher(a), encrypt_wspace(e), color(c), speed(s) {
        struct winsize w{};
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        maxlines = w.ws_row - 1;
        maxwidth = w.ws_col;
    }
    void printCipher() {
        set_terminal(0);
        create_decipher();
        if(auto_decipher)
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        else
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
            if((ref[i] == '\n' || !encrypt_wspace) && isspace(ref[i])) {
                std::cout << ref[i] << std::flush;
            }
            else {
                std::cout << random_char() << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(7));
            }
            if(ref[i] == '\n') {
                ++linecount;
                currwidth = 0;
            }
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
            std::uniform_int_distribution<int> dist(0, (int)(ref.size() - revealed) / (speed));
            for (int i = 0; i < ref.size(); ++i) {
                if (solved[i]) {
                    decipher += ref[i];
                    std::cout << "\033[1m" << "\033[3" << color << "m" << ref[i] << "\033[0m";
                }
                else if (((ref[i] == '\n' || !encrypt_wspace)
                        && isspace(ref[i]))
                        || (c > 30 && dist(mt) == 0)) {
                    solved[i] = true;
                    revealed++;
                    decipher += ref[i];
                    std::cout << "\033[1m" << "\033[3" << color << "m" << ref[i] << "\033[0m";
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
    int maxlines, maxwidth, linecount{0}, color, speed;
    bool auto_decipher, encrypt_wspace;
};

int set_solved_color(char *c) {
    if(strcmp("white", c) == 0) return 7;
    else if(strcmp("yellow", c) == 0) return 3;
    else if(strcmp("black", c) == 0) return 0;
    else if(strcmp("magenta", c) == 0) return 5;
    else if(strcmp("green", c) == 0) return 2;
    else if(strcmp("red", c) == 0) return 1;
    else if(strcmp("cyan", c) == 0) return 6;
    else return 4;
}

void print_help() {
    std::cout << "decipher - Hollywood style decryption effect\n\n"
              << "Example:\n  ls -l / | decipher\n\n"
              << "Usage:\n"
              << "  ls -l \\ | decipher -s <speed>  Set decryption speed (higher is faster)\n"
              << "  ls -l \\ | decipher -a          Set auto decipher flag\n"
              << "  ls -l \\ | decipher -w          Encrypt whitespace flag\n"
              << "  ls -l \\ | decipher -c <color>  Set solved color flag\n"
              << "  ls -l \\ | decipher -h          display this help message\n";
}

int main(int argc, char* argv[]) {
    if(isatty(STDIN_FILENO)) {
        print_help();
        return 0;
    }
    int o, color = 4, speed = 6;
    bool auto_decipher = false, encrypt_wspace = false;
    while((o = getopt(argc, argv, "s:c:awh")) != -1) {
        switch(o) {
            case 's':
                try {
                    speed = (int)std::stoi(optarg);
                } catch(const std::invalid_argument&) {
                    break;
                } catch(const std::out_of_range&) {
                    break;
                }
                break;
            case 'c':
                color = set_solved_color(optarg);
                break;
            case 'a':
                auto_decipher = true;
                break;
            case 'w':
                encrypt_wspace = true;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                if(isprint(optopt))
                    std::cerr << "Unknown flag " << (char)optopt << std::endl;
                return 1;
            default:
                break;
        }
    }
    std::string line, output;
    while(std::getline(std::cin, line)) {
        output += line + "\n";
    }
    Decipher decipher(output, auto_decipher, encrypt_wspace, color, speed);
    decipher.printCipher();
    return 0;
}