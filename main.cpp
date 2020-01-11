#include "mbed.h"
#include <string>
#define RX_MAX 1000

Serial pc(USBTX, USBRX);
Serial test(PA_0, PA_1);
Serial bt(PC_1, PC_0); // tx, rx
DigitalOut out(PC_5);
DigitalIn in(PC_6);

enum ShellMode {
    Normal,
    Esc
};
class Shell {
   public:
    string read_input() {
        std::string input;
        bool done = false;
        while (!done) {
            if (pc.readable()) {
                char c = pc.getc();
                switch (mode) {
                    case Normal:
                        done = parse_normal_input(c, input);
                        break;
                    case Esc:
                        parse_esc_input(c, input);
                        break;
                }
            }
        }
        return input;
    }
   private:
    ShellMode mode = Normal;
    bool parse_normal_input(char c, std::string &input) {
        // CR
        if (c == 13) {
            pc.printf("\r\n");
            return true;
        }

        // BS
        if (c == 8) {
            input.pop_back();
            pc.putc(c);
            pc.printf("\033[K");
        }
        // ESC
        else if (c == 27) {
            mode = Esc;
        }
        else {
            input += c;
            pc.putc(c);
        }
        return false;
    }
    void parse_esc_input(char c, std::string &input) {
        // pc.printf("esc: %c", c);
        if (c == 'D') {
            // pc.printf("\033[1D");
        }
        else if (c == 'C') {
            // pc.printf("\033[1C");
        }
        mode = Normal;
    }
};

int main() {
    /* foot pedal control */
    // int flag = 0;
    // out = 0;
    // while (1) {
    //     if (flag == 0 && in == 1) {
    //         out = !out;
    //         flag = 1;
    //     }
    //     if (in == 0) {
    //         flag = 0;
    //     }
    // }
    Shell shell;

    std::string input;
    
    test.baud(115200);
    pc.baud(115200);

    while (1) {
        // reading
        input = shell.read_input();
        // reading done
        test.printf("%s\r\n", input.c_str());
    }

    // int     i = 0;
    // pc.baud(115200);
    // bt.baud(115200);
    // bt.printf("Ready to receive data over Bluetooth.\r\n\r\n");

    // bt.putc('$');
    // bt.putc('$');
    // bt.putc('$');

    // while(1) {
    //     if (bt.readable()) {
    //         pc.putc(bt.getc());
    //     }
    // }
    // while (1) {
    //     while (bt.readable()) {
    //         bt.scanf("%s", rxBuf);
    //         i = strlen(rxBuf);
    //     }
 
    //     if (i > 0) {
    //         bt.printf("Data you just send: %s\r\n", rxBuf);
    //         pc.printf("Data received: %s\r\n", rxBuf);
    //         i = 0;
    //     }
    // }
}