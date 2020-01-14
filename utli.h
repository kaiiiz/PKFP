#include "mbed.h"
#include <vector>
#include <string>
#include <unordered_map>

#ifndef keydef
#define keydef
#include "keydef.h"
#endif

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
        std::unordered_map<char, int> keymap = max7219_alphabet;
    };
}