#include "mbed.h"
#include <vector>
#include <string>

enum BtProfile {
    SPP,
    HID
};

enum BtSerialStatus {
    Connect,
    Flush,
    Parse,
    Disconnect,
};

void init_keymap(std::vector<std::pair<std::string, int>>& map) {
    map = {
        make_pair("Insert",     1),
        make_pair("Home",       2),
        make_pair("PageUp",     3),
        make_pair("Del",        4),
        make_pair("End",        5),
        make_pair("PageDown",   6),
        make_pair("Esc",        27),
        // Printable ASCII
        make_pair("a", 'a'),
        make_pair("b", 'b'),
    };
}

void serial_mode(Serial& bt) {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,0\r\n");
    wait_us(100000);
    bt.printf("SO,ESC%%\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}

void hid_mode(Serial& bt) {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,6\r\n");
    wait_us(100000);
    bt.printf("SO,ESC%%\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}

bool btnIsPress(DigitalIn& btn) {
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