#include "mbed.h"
#include <unordered_map>
#include <string>

enum BtProfile {
    SPP,
    HID
};

void init_keymap(std::unordered_map<std::string, int>& map) {
    map.insert({
        {"Insert", 1},
        {"Home", 2},
        {"PageUp", 3},
        {"Del", 4},
        {"End", 5},
        {"PageDown", 6},
        {"Esc", 27},
        // Printable ASCII
        {"a", 'a'},
        {"b", 'b'},
    });
}

void serial_mode(Serial& bt) {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,0\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}

void hid_mode(Serial& bt) {
    bt.printf("$$$");
    wait_us(100000);
    bt.printf("S~,6\r\n");
    wait_us(100000);
    bt.printf("r,1\r\n");
}