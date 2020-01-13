#include <unordered_map>
#include <string>

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