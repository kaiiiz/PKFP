#include "mbed.h"
#include "utli.h"
#include <unordered_map>
#include <string>
#include <queue>

DigitalIn btn(PC_13);
DigitalIn fp(PC_5); // foot pedal
DigitalOut led(LED1);
Serial pc(USBTX, USBRX);
Serial bt(PC_1, PC_0); // tx, rx

class Shell {
   public:
    std::unordered_map<string, int> keymap;
    std::queue<std::pair<string, int>> buffer;
    int buffer_max = 3;

    Shell() {
        init_keymap(keymap);
        buffer.push(get_key("a"));
        buffer.push(get_key("b"));
        buffer.push(get_key("Esc"));
    }

    void main() {
        char rxBuf[128];
        while (bt.scanf("%s", rxBuf)) {
            pc.printf("%s\r\n", rxBuf);
            if (!strcmp(rxBuf, "hid")) {
                bt.printf("reset hid profile...\r\n");
                wait_us(100000);
                hid_mode(bt);
                break;
            }
            else if (!strcmp(rxBuf, "sb")) {
                for (int i = 0; i < buffer_max; i++) {
                    std::pair<std::string, int> k = buffer.front();
                    bt.printf("key %d: %s %d\r\n", i + 1, k.first.c_str(), k.second);
                    buffer.push(k);
                    buffer.pop();
                }
            }
            else if (!strcmp(rxBuf, "help")) {
                help();
            }
            else {
                bt.printf("Unrecognized command.\r\n");
                bt.printf("\r\n");
                help();
            }
        }
    }
    
   private:
    std::pair<string, int> get_key(std::string name) {
        return make_pair(name, keymap[name]);
    }

    void help() {
        bt.printf("Usage: \r\n");
        bt.printf("\r\n");
        bt.printf("sb: show key buffer\r\n");
        bt.printf("hid: reset profile to hid\r\n");
    }
};

bool btnIsPress() {
    if (btn == 0) { // debounce
        // press
        int counter = 0xffff;
        while (counter) {
            if (btn == 0)  counter >>= 1;
            else return false;
            wait_us(1000);
        }
        // release
        counter = 0xffff;
        while (counter) {
            if (btn == 1) counter >>= 1;
            else counter = 0xffff;
            wait_us(1000);
        }
        return true;
    }
    return false;
}

void fpHandler(std::queue<std::pair<std::string, int>>& buffer) {
    bool long_press = false;
    if (fp == 1) { // debounce
        // press
        int counter = 0xffff;
        while (counter) {
            if (fp == 1) {
                counter >>= 1;
            }
            else {
                return;
            }
            wait_us(1000);
        }
        // long press check
        counter = 0xff;
        while (counter) {
            if (fp == 0) {
                long_press = false;
                break;
            }
            else {
                counter >>= 1;
                long_press = true;
            }
            wait_us(100000);
        }
        // release
        counter = 0xffff;
        while (counter) {
            if (fp == 0) {
                counter >>= 1;
            }
            else {
                counter = 0xffff;
            }
            wait_us(1000);
        }

        if (long_press) {
            buffer.push(buffer.front());
            buffer.pop();
            led = 1;
        }
        else {
            pc.printf("%s %d\r\n", buffer.front().first.c_str(), buffer.front().second);
            bt.putc(buffer.front().second);
            led = 0;
        }
    }
}

int main() {
    pc.baud(115200);
    bt.baud(115200);

    Shell shell;
    BtProfile profile = SPP;
    serial_mode(bt);

    while (1) {
        if (profile == HID) {
            if (btnIsPress()) {
                serial_mode(bt);
                profile = SPP;
            }
            fpHandler(shell.buffer);
        }
        else if (profile == SPP) {
            shell.main();
            profile = HID;
        }
    }
}
