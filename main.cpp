#include "mbed.h"
#include "utli.h"
#include <algorithm>
#include <vector>
#include <string>
#include <deque>

#ifndef keydef
#define keydef
#include "keydef.h"
#endif

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
    std::unordered_map<std::string, int> keymap;
    std::deque<std::pair<std::string, int>> buffer;
    int buffer_max = 3;

    Controller() {
        keymap.insert(control_keymap.begin(), control_keymap.end());
        keymap.insert(ascii_keymap.begin(), ascii_keymap.end());
        buffer.push_back(get_key("a"));
        buffer.push_back(get_key("b"));
        buffer.push_back(get_key("c"));
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
                    max7219.show("sppidle");
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
                    max7219.show("hididle");
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
        if (keymap.count(name) > 0) {
            return make_pair(name, keymap[name]);
        }
        else {
            return make_pair(name, -1);
        }
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

    void cmdHandler(string cmd_str) {
        pc.printf("%s\r\n", cmd_str.c_str());
        // tokenize
        std::vector<std::string> cmd;
        while (std::find_if(cmd_str.begin(), cmd_str.end(), std::isalnum) != cmd_str.end()) {
            auto beg = std::find_if(cmd_str.begin(), cmd_str.end(), std::isalnum);
            auto end = std::find_if(beg, cmd_str.end(), std::isblank);
            auto beg_pos = beg - cmd_str.begin();
            auto end_pos = end - cmd_str.begin();
            cmd.push_back(cmd_str.substr(beg_pos, end_pos));
            if (end_pos == cmd_str.size()) break;
            cmd_str = cmd_str.substr(end_pos + 1);
        }

        // Set Commands
        if (cmd[0] == "bsize" && cmd.size() == 2) {
            bsize(cmd[1]);
        }
        else if (cmd[0] == "pushf" && cmd.size() == 2) {
            pushf(cmd[1]);
        }
        else if (cmd[0] == "pushb" && cmd.size() == 2) {
            pushb(cmd[1]);
        }
        else if (cmd[0] == "popf" && cmd.size() == 1) {
            popf();
        }
        else if (cmd[0] == "popb" && cmd.size() == 1) {
            popb();
        }
        else if (cmd[0] == "del" && cmd.size() == 2) {
            del(cmd[1]);
        }
        else if (cmd[0] == "insert" && cmd.size() == 3) {
            insert(cmd[1], cmd[2]);
        }
        else if (cmd[0] == "replace" && cmd.size() == 3) {
            replace(cmd[1], cmd[2]);
        }
        // Get Commands
        else if (cmd[0] == "gb" && cmd.size() == 1) {
            gb();
        }
        else if (cmd[0] == "gbsize" && cmd.size() == 1) {
            gbsize();
        }
        else if (cmd[0] == "gk" && cmd.size() == 1) {
            gk();
        }
        // Help
        else if (cmd[0] == "help" && cmd.size() == 1) {
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
                    this->buffer.push_back(front);
                    this->buffer.pop_front();
                    led = 1;
                    break;
            }
        }
        prev_cmd = fp.CMD;
    }

    // Set Commands
    void bsize(std::string arg) {
        for (char c : arg) {
            if (!std::isdigit(c)) {
                bt.printf("Argument must be numeric!\r\n");
                return;
            }
        }
        int bsize = stoi(arg, nullptr, 10);
        if (bsize < this->buffer.size()) {
            int diff = this->buffer.size() - bsize;
            while(diff--) {
                this->buffer.pop_back();
            }
        }
        this->buffer_max = bsize;
        bt.printf("Set buffer size to %d\r\n", bsize);
    }
    void pushf(std::string arg) {
        auto key = get_key(arg);
        if (key.second == -1) {
            bt.printf("Unsupported key name!\r\n");
            return;
        }
        if (this->buffer_max == this->buffer.size()) {
            this->buffer.pop_back();
        }
        this->buffer.push_front(key);
        gb();
    }
    void pushb(std::string arg) {
        auto key = get_key(arg);
        if (key.second == -1) {
            bt.printf("Unsupported key name!\r\n");
            return;
        }
        if (this->buffer_max == this->buffer.size()) {
            this->buffer.pop_front();
        }
        this->buffer.push_back(key);
        gb();
    }
    void popf() {
        if (this->buffer.size() > 0) {
            this->buffer.pop_front();
        }
        gb();
    }
    void popb() {
        if (this->buffer.size() > 0) {
            this->buffer.pop_back();
        }
        gb();
    }
    void del(std::string arg) {
        for (char c : arg) {
            if (!std::isdigit(c)) {
                bt.printf("Argument must be numeric!\r\n");
                return;
            }
        }
        int idx = stoi(arg, nullptr, 10) - 1;
        if (idx > this->buffer.size()) {
            bt.printf("Key-id must smaller than size of buffer!\r\n");
            return;
        }
        this->buffer.erase(this->buffer.begin() + idx);
        gb();
    }
    void insert(std::string keyname, std::string keyid) {
        for (char c : keyid) {
            if (!std::isdigit(c)) {
                bt.printf("Argument must be numeric!\r\n");
                return;
            }
        }
        int idx = stoi(keyid, nullptr, 10) - 1;
        auto key = get_key(keyname);
        if (key.second == -1) {
            bt.printf("Unsupported key name!\r\n");
            return;
        }
        if (this->buffer.size() == this->buffer_max) {
            this->buffer.pop_back();
        }
        this->buffer.insert(this->buffer.begin() + idx, key);
        gb();
    }
    void replace(std::string keyname, std::string keyid) {
        for (char c : keyid) {
            if (!std::isdigit(c)) {
                bt.printf("Argument must be numeric!\r\n");
                return;
            }
        }
        int idx = stoi(keyid, nullptr, 10) - 1;
        auto key = get_key(keyname);
        if (key.second == -1) {
            bt.printf("Unsupported key name!\r\n");
            return;
        }
        this->buffer[idx] = key;
        gb();
    }

    // Get Commands
    void gb() {
        for (int i = 0; i < this->buffer.size(); i++) {
            auto k = this->buffer.front();
            bt.printf("key %d: %s %d\r\n", i + 1, k.first.c_str(), k.second);
            this->buffer.push_back(k);
            this->buffer.pop_front();
        }
    }
    void gbsize() {
        bt.printf("max key buffer size is %d\r\n", this->buffer_max);
    }
    void gk() {
        bt.printf("Control key:\r\n");
        for (auto key : control_keymap) {
            bt.printf("%s %d\r\n", key.first.c_str(), key.second);
        }
        bt.printf("\r\nPrintable ASCII key:\r\n");
        for (auto key : ascii_keymap) {
            bt.printf("%s %d\r\n", key.first.c_str(), key.second);
        }
    }

    // Help
    void help() {
        bt.printf("Usage: \r\n");
        bt.printf("\r\n");
        bt.printf("Set Commands:\r\n");
        bt.printf("bsize <int>: set max key buffer size\r\n");
        bt.printf("pushf <key-name>: push key to front of buffer\r\n");
        bt.printf("pushb <key-name>: push key to back of buffer\r\n");
        bt.printf("popf: pop key from front of buffer\r\n");
        bt.printf("popb: pop key from back of buffer\r\n");
        bt.printf("del <key-id>: delete key in buffer using key-id\r\n");
        bt.printf("insert <key-name> <key-id>: insert key to position <key-id>\r\n");
        bt.printf("replace <key-name> <key-id>: replace key from position <key-id>\r\n");
        bt.printf("\r\n");
        bt.printf("Get Commands:\r\n");
        bt.printf("gb: get current key buffer\r\n");
        bt.printf("gbsize: get max key buffer size\r\n");
        bt.printf("gk: get avaliable key\r\n");
        bt.printf("\r\n");
        bt.printf("Settings:\r\n");
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
