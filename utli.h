#include "mbed.h"
#include <vector>
#include <string>
#include <unordered_map>

enum BtProfile {
    SPP,
    HID
};

enum BtStatus {
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

bool modeSwitch(DigitalIn& btn) {
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

namespace mbed {
    class FootPedal {
       public:
        unsigned char CMD; // FP has been pressed
        unsigned char STAT; // FP is being pressing
        FootPedal(PinName pin): _fp(pin) {
            _fp.mode(PullDown);
            CMD = STAT = 0;
            _tick.attach(callback(this, &FootPedal::sample), SAMPLE_PERIOD);
        };
       private:
        float SAMPLE_PERIOD = 0.001; // s
        int SHORT_PRESS_THOLD = 10;
        int LONG_PRESS_THOLD = 500;
        int debounce_cntr;
        void sample() {
            int press = _fp;
            
            if (press) {
                debounce_cntr++;
                STAT = 1;
            }
            else {
                STAT = 0;
                debounce_cntr = 0;
            }

            if (debounce_cntr >= SHORT_PRESS_THOLD && debounce_cntr < LONG_PRESS_THOLD) {
                CMD = 1;
            }
            else if (debounce_cntr >= LONG_PRESS_THOLD) {
                debounce_cntr = LONG_PRESS_THOLD;
                CMD = 2;
            }
            else {
                CMD = 0;
            }
        }
        DigitalIn _fp;
        Ticker _tick;
    };

    class MAX7219 {
       public:
        MAX7219 (PinName DIN_PIN, PinName CS_PIN, PinName CLK_PIN): DIN(DIN_PIN), CS(CS_PIN), CLK(CLK_PIN) {
            // DECODE_MODE
            set_decode(0x00);
            // INTENSITY
            set_intensity(0xA);
            // SCAN_LIMIT
            set_scan_limit(7);
            // SHUTDOWN
            set_shutdown(0x1);
            // DISPLAY_TEST
            set_display_test(0x0);
            // Clear All 7-Segment
            for (int i = 1; i <= 8; i++) {
                send(i, keymap[' ']);
            }
        }

        void show(string keyname) {
            int len = keyname.length();
            set_scan_limit(len - 1);
            for (int i = len, j = 0; i > 0; i--, j++) {
                send(i, keymap[keyname[j]]);
            }
        }

        void set_decode(int data) {
            send(0x09, data);
        }

        void set_intensity(int data) {
            send(0x0A, data);
        }

        void set_scan_limit(int data) {
            send(0x0B, data);
        }

        void set_shutdown(int data) {
            send(0x0C, data);
        }

        void set_display_test(int data) {
            send(0x0F, data);
        }

        void send(int address, int data) {
            int send = (address << 8) + data;
            int mask;
            for (int i = 16; i > 0; i--) {
                mask = 1 << (i - 1);
                CLK = 0;
                if ((send & mask) == 0) {
                    DIN = 0;
                }
                else {
                    DIN = 1;
                }
                CLK = 1;
            }
            CS = 0;
            CS = 1;
        }

       private:
        DigitalOut DIN;
        DigitalOut CS;
        DigitalOut CLK;
        std::unordered_map<char, int> keymap{
            {'0', 0x7E},
            {'1', 0x30},
            {'2', 0x6D},
            {'3', 0x79},
            {'4', 0x33},
            {'5', 0x5B},
            {'6', 0x5F},
            {'7', 0x70},
            {'8', 0x7F},
            {'9', 0x7B},
            {' ', 0x00},
            {'A', 0x77},
            {'a', 0x7D},
            {'B', 0x7F},
            {'b', 0x1F},
            {'C', 0x4E},
            {'c', 0x0D},
            {'D', 0x7E},
            {'d', 0x3D},
            {'E', 0x4F},
            {'e', 0x6f},
            {'F', 0x47},
            {'f', 0x47},
            {'G', 0x5E},
            {'g', 0x7B},
            {'H', 0x37},
            {'h', 0x17},
            {'I', 0x30},
            {'i', 0x10},
            {'J', 0x3C},
            {'j', 0x38},
            {'K', 0x37},
            {'k', 0x17},
            {'L', 0x0E},
            {'l', 0x06},
            {'M', 0x55},
            {'m', 0x55},
            {'N', 0x15},
            {'n', 0x15},
            {'O', 0x7E},
            {'o', 0x1D},
            {'P', 0x67},
            {'p', 0x67},
            {'Q', 0x73},
            {'q', 0x73},
            {'R', 0x77},
            {'r', 0x05},
            {'S', 0x5B},
            {'s', 0x5B},
            {'T', 0x46},
            {'t', 0x0F},
            {'U', 0x3E},
            {'u', 0x1C},
            {'V', 0x27},
            {'v', 0x23},
            {'W', 0x3F},
            {'w', 0x2B},
            {'X', 0x25},
            {'x', 0x25},
            {'Y', 0x3B},
            {'y', 0x33},
            {'Z', 0x6D},
            {'z', 0x6D},
        };
    };
}