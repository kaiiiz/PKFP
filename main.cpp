#include "mbed.h"
#include "keymap.h"
#include <string>
#include <queue>
#include <unordered_map>

DigitalIn btn(PC_13);
DigitalIn fp(PC_5); // foot pedal
DigitalOut led(LED1);
Serial pc(USBTX, USBRX);
Serial bt(PC_1, PC_0); // tx, rx

enum BtProfile {
    SPP,
    HID
};

void serial_mode() {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,0\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}

void hid_mode() {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,6\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}

class Shell {
   public:
    std::unordered_map<string, int> keymap;
    std::queue<int> buffer;
    int buffer_max = 3;

    Shell() {
        init_keymap(keymap);
        buffer.push(keymap["a"]);
        buffer.push(keymap["b"]);
        buffer.push(keymap["Esc"]);
    }

    void main() {
        char rxBuf[128];
        while (bt.scanf("%s", rxBuf)) {
            pc.printf("%s\r\n", rxBuf);
            if (!strcmp(rxBuf, "hid")) {
                bt.printf("reset hid profile...\r\n");
                wait_us(100000);
                hid_mode();
                break;
            }
            else if (!strcmp(rxBuf, "help")) {
                help();
            }
        }
    }
    
   private:
    void help() {
        bt.printf("usage: \r\n");
        bt.printf("\r\n");
        bt.printf("show buffer: show key buffer\r\n");
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

void fpHandler(std::queue<int>& buffer) {
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
        counter = 0xfffff;
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
            bt.putc(buffer.front());
            led = 0;
        }
    }
}

int main() {
    pc.baud(115200);
    bt.baud(115200);

    Shell shell;
    BtProfile profile = SPP;
    serial_mode();

    while (1) {
        if (profile == HID) {
            if (btnIsPress()) {
                serial_mode();
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
