#include "mbed.h"
#include "utli.h"
#include <algorithm>
#include <vector>
#include <string>
#include <queue>

DigitalIn btn(PC_13);
DigitalIn fp(PC_5); // foot pedal
DigitalOut led(LED1);
Serial pc(USBTX, USBRX);
Serial bt(PC_1, PC_0); // tx, rx

class Shell {
   public:
    std::vector<std::pair<std::string, int>> keymap;
    std::queue<std::pair<std::string, int>> buffer;
    int buffer_max = 3;

    Shell() {
        init_keymap(keymap);
        buffer.push(get_key("a"));
        buffer.push(get_key("b"));
        buffer.push(get_key("Esc"));
    }

    void main() {
        char cmd[128];
        while (bt.scanf("%s", cmd)) {
            pc.printf("%s\r\n", cmd);

            // exit serial mode
            if (!strcmp(cmd, "hid")) {
                bt.printf("reset hid profile...\r\n");
                wait_us(100000);
                hid_mode(bt);
                break;
            }

            // Get Commands
            if (!strcmp(cmd, "gb")) {
                gb();
            }
            else if (!strcmp(cmd, "gk")) {
                gk();
            }
            // Help
            else if (!strcmp(cmd, "help")) {
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
        auto it = std::find_if(keymap.begin(), keymap.end(),
            [&name](const std::pair<std::string, int>& key){ return key.first == name; });
        if (it == keymap.end()) return make_pair("", 0);
        else return *it;
    }

    // Get Commands
    void gb() {
        for (int i = 0; i < buffer_max; i++) {
            std::pair<std::string, int> k = buffer.front();
            bt.printf("key %d: %s %d\r\n", i + 1, k.first.c_str(), k.second);
            buffer.push(k);
            buffer.pop();
        }
    }
    void gk() {
        bt.printf("Key Name\t\t\tHID Code\r\n");
        for (auto key : keymap) {
            bt.printf("%s\t\t\t%d\r\n", key.first.c_str(), key.second);
        }
    }

    // Help
    void help() {
        bt.printf("Usage: \r\n");
        bt.printf("\r\n");
        bt.printf("Get Commands:\r\n");
        bt.printf("gb: get current key buffer\r\n");
        bt.printf("gk: get avaliable key\r\n");
        bt.printf("\r\n");
        bt.printf("hid: reset profile to hid\r\n");
    }
};

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
    pc.baud(9600);
    bt.baud(9600);

    Shell shell;
    BtProfile profile = SPP;
    serial_mode(bt);

    while (1) {
        if (profile == HID) {
            bool connect = false;
            std::string log;
            while (1) {
                if (bt.readable()) {
                    log += bt.getc();
                }

                if (!connect && log.find("ESC%CONNECT") != std::string::npos) {
                    connect = true;
                    log.erase(log.begin(), log.begin() + log.find("ESC%CONNECT"));
                    pc.printf("connection %s\r\n", log.c_str());
                }

                if (connect && log.find("ESC%DISCONNECT") != std::string::npos) {
                    connect = false;
                    log.clear();
                    pc.printf("disconn\r\n");
                }

                if (!connect && btnIsPress(btn)) {
                    serial_mode(bt);
                    profile = SPP;
                    break;
                }

                if (connect) {
                    fpHandler(shell.buffer);
                }
            }
        }
        else if (profile == SPP) {
            shell.main();
            profile = HID;
        }
    }
}
