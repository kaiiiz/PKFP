#include "mbed.h"
#include <string>

DigitalIn btn(PC_13);
DigitalIn fp(PC_5); // foot pedal
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
        }
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

void fpHandler() {
    if (fp == 1) { // debounce
        // press
        int counter = 0xffff;
        while (counter) {
            if (fp == 1)  counter >>= 1;
            else return;
            wait_us(1000);
        }
        // release
        counter = 0xffff;
        while (counter) {
            if (fp == 0) counter >>= 1;
            else counter = 0xffff;
            wait_us(1000);
        }
        bt.putc('a');
    }
}

int main() {
    Shell shell;
    BtProfile profile = HID;
    hid_mode();

    std::string input;
    
    pc.baud(115200);
    bt.baud(115200);

    while (1) {
        if (profile == HID) {
            if (btnIsPress()) {
                serial_mode();
                profile = SPP;
            }
            fpHandler();
        }
        else if (profile == SPP) {
            shell.main();
            profile = HID;
        }
    }
}