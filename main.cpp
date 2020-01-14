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

class Controller {
   public:
    std::vector<std::pair<std::string, int>> keymap;
    std::queue<std::pair<std::string, int>> buffer;
    int buffer_max = 3;

    Controller() {
        init_keymap(keymap);
        buffer.push(get_key("a"));
        buffer.push(get_key("b"));
        buffer.push(get_key("Esc"));
    }

    void spp() {
        std::string log;
        BtStatus status = Disconnect;
        while (1) {
            if (btnIsPress(btn)) {
                bt.printf("\r\nreset hid profile...\r\n");
                wait_us(100000);
                hid_mode(bt);
                return;
            }
            switch (status) {
                case Connect:
                    if (bt.readable()) {
                        char c = bt.getc();
                        if (c == 10) status = Parse; // newline
                        else log += c;
                    }
                    if (log.find("ESC%DISCONNECT") != std::string::npos) {
                        pc.printf("%s\r\n", log.c_str());
                        status = Disconnect;
                        log.clear();
                    }
                    break;
                case Parse:
                    if (log == "hid") {
                        bt.printf("reset hid profile...\r\n");
                        wait_us(100000);
                        hid_mode(bt);
                        return;
                    }
                    else {
                        this->cmdHandler(log);
                        status = Connect;
                        log.clear();
                    }
                    break;
                case Flush:
                    this->flush_input();
                    bt.printf("Shell ready! Send \"help\" to get supported commands list!");
                    status = Connect;
                    log.clear();
                    break;
                case Disconnect:
                    if (bt.readable()) {
                        log += bt.getc();
                    }
                    if (log.find("ESC%CONNECT") != std::string::npos) {
                        pc.printf("%s\r\n", log.c_str());
                        status = Flush;
                        log.clear();
                    }
                    break;
            }
        }
    }

    void hid() {
        std::string log;
        BtStatus status = Disconnect;
        while (1) {
            if (bt.readable()) {
                log += bt.getc();
            }
            switch (status) {
                case Connect:
                    if (log.find("ESC%DISCONNECT") != std::string::npos) {
                        pc.printf("%s\r\n", log.c_str());
                        status = Disconnect;
                        log.clear();
                    }
                    this->fpHandler();
                    break;
                case Disconnect:
                    if (log.find("ESC%CONNECT") != std::string::npos) {
                        pc.printf("%s\r\n", log.c_str());
                        status = Connect;
                        log.clear();
                    }
                    if (btnIsPress(btn)) {
                        serial_mode(bt);
                        return;
                    }
                    break;
                default:
                    break;
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

    void flush_input() {
        int flushCntr = 100;
        while (flushCntr) {
            if (bt.readable()) {
                bt.getc();
                flushCntr = 100;
            }
            if (!bt.readable()) {
                flushCntr--;
            }
        }
    }

    void cmdHandler(string cmd) {
        // Get Commands
        if (cmd == "gb") {
            gb();
        }
        else if (cmd == "gk") {
            gk();
        }
        // Help
        else if (cmd == "help") {
            help();
        }
        // Other commands
        else {
            bt.printf("Unrecognized command.\r\n");
            bt.printf("\r\n");
            help();
        }
    }

    void fpHandler() {
        bool long_press = false;
        if (fp == 1) { // debounce
            // press
            int counter = 0xffff;
            while (counter) {
                if (fp == 1) counter >>= 1;
                else return;
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
                this->buffer.push(this->buffer.front());
                this->buffer.pop();
                led = 1;
            }
            else {
                auto front = this->buffer.front();
                pc.printf("%s %d\r\n", front.first.c_str(), front.second);
                bt.putc(front.second);
                led = 0;
            }
        }
    }

    // Get Commands
    void gb() {
        for (int i = 0; i < buffer_max; i++) {
            auto k = this->buffer.front();
            bt.printf("key %d: %s %d\r\n", i + 1, k.first.c_str(), k.second);
            this->buffer.push(k);
            this->buffer.pop();
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

int main() {
    pc.baud(9600);
    bt.baud(9600);

    Controller ctlr;
    BtProfile profile = SPP;
    serial_mode(bt);

    while (1) {
        if (profile == HID) {
            ctlr.hid();
            profile = SPP;
        }
        else if (profile == SPP) {
            ctlr.spp();
            profile = HID;
        }
    }
}
