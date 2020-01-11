#include "mbed.h"
#include <string>
#define RX_MAX 1000

DigitalIn btn(PC_13);
Serial pc(USBTX, USBRX);
Serial test(PA_0, PA_1);
Serial bt(PC_1, PC_0); // tx, rx
DigitalOut out(PC_5);
DigitalIn in(PC_6);

enum BtProfile {
    SPP,
    HID
};
enum ShellMode {
    Normal,
    Config
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
        bt.scanf("%s", rxBuf);
        if (!strcmp(rxBuf, "hid")) {
            hid_mode();
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
    BtProfile profile = HID;
    hid_mode();

    std::string input;
    
    test.baud(115200);
    pc.baud(115200);
    bt.baud(115200);

    while (1) {
        if (profile == HID) {
            if (btnIsPress()) {
                serial_mode();
                profile = SPP;
            }
        }
        else if (profile == SPP) {
            shell.main();
            profile = HID;
        }
        // pc config
        // if (pc.readable()) {
        //     input = shell.read_input();
        //     if (input == "$$$") {
        //         bt.printf("$$$");
        //     }
        //     else {
        //         bt.printf("%s\r\n", input.c_str());
        //     }
        // }
    }
}