#include "mbed.h"
#include "utli.h"
#include <algorithm>
#include <vector>
#include <string>
#include <queue>

DigitalOut led(LED1);
Serial pc(USBTX, USBRX);

// MAX7219
#define DIN_PIN PA_5
#define CS_PIN PA_6
#define CLK_PIN PA_7
MAX7219 max7219(DIN_PIN, CS_PIN, CLK_PIN);

// Switch Button
DigitalIn btn(PC_13);

// Foot Pedal
FootPedal fp(PC_5);

// Bluetooth Module
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
            if (modeSwitch(btn)) {
                bt.printf("\r\nreset hid profile...\r\n");
                wait_us(100000);
                hid_mode(bt);
                return;
            }
            switch (status) {
                case Connect:
                    max7219.show("config");
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
                        this->flush_input();
                        status = Connect;
                        log.clear();
                    }
                    break;
                case Flush:
                    this->flush_input();
                    bt.printf("Shell ready! Send \"help\" to get supported commands list!\r\n");
                    status = Connect;
                    log.clear();
                    break;
                case Disconnect:
                    max7219.show("noConn");
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
                    max7219.show("noConn");
                    if (log.find("ESC%CONNECT") != std::string::npos) {
                        pc.printf("%s\r\n", log.c_str());
                        status = Connect;
                        log.clear();
                    }
                    if (modeSwitch(btn)) {
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
        int flushCntr = 50;
        while (flushCntr) {
            if (bt.readable()) {
                bt.getc();
                flushCntr = 50;
            }
            if (!bt.readable()) {
                flushCntr--;
            }
        }
    }

    void cmdHandler(string cmd) {
        pc.printf("%s\r\n", cmd.c_str());
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

    unsigned char prev_cmd = 0;
    void fpHandler() {
        auto front = this->buffer.front();
        max7219.show(front.first);
        // only trigger when release foot pedal
        if (prev_cmd && fp.CMD == 0) {
            switch (prev_cmd) {
                case 1: // short press
                    pc.printf("%s %d\r\n", front.first.c_str(), front.second);
                    bt.putc(front.second);
                    led = 0;
                    break;
                case 2: // long press
                    this->buffer.push(front);
                    this->buffer.pop();
                    led = 1;
                    break;
            }
        }
        prev_cmd = fp.CMD;
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
