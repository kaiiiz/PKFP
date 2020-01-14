#include <vector>
#include <string>
#include <unordered_map>

std::vector<std::pair<std::string, int>> control_keymap = {
    std::make_pair("Insert",        1),
    std::make_pair("Home",          2),
    std::make_pair("PageUp",        3),
    std::make_pair("Del",           4),
    std::make_pair("End",           5),
    std::make_pair("PageDown",      6),
    std::make_pair("RArrow",        7),
    std::make_pair("Bckspace",      8),
    std::make_pair("Tab",           9),
    std::make_pair("Enter",         10),
    std::make_pair("LArrow",        11),
    std::make_pair("DArrow",        12),
    std::make_pair("UArrow",        14),
    std::make_pair("F1",            15),
    std::make_pair("F2",            16),
    std::make_pair("F3",            17),
    std::make_pair("F4",            18),
    std::make_pair("F5",            19),
    std::make_pair("F6",            20),
    std::make_pair("F7",            21),
    std::make_pair("F8",            22),
    std::make_pair("F9",            23),
    std::make_pair("F10",           24),
    std::make_pair("F11",           25),
    std::make_pair("F12",           26),
    std::make_pair("Esc",           27),
    std::make_pair("CapsLk",        28),
    std::make_pair("ScrollLk",      29),
    std::make_pair("BrkPause",      30),
    std::make_pair("NumLock",       31),
    std::make_pair("LCtrl",         0xE0),
    std::make_pair("LShift",        0xE1),
    std::make_pair("LAlt",          0xE2),
    std::make_pair("LGUI",          0xE3),
    std::make_pair("RCtrl",         0xE4),
    std::make_pair("RShift",        0xE5),
    std::make_pair("RAlt",          0xE6),
    std::make_pair("RGUI",          0xE7),
};

std::vector<std::pair<std::string, int>> ascii_keymap = {
    std::make_pair(" ",             ' '),
    std::make_pair("!",             '!'),
    std::make_pair("\"",            '"'),
    std::make_pair("#",             '#'),
    std::make_pair("$",             '$'),
    std::make_pair("%",             '%'),
    std::make_pair("&",             '&'),
    std::make_pair("'",             '\''),
    std::make_pair("(",             '('),
    std::make_pair(")",             ')'),
    std::make_pair("*",             '*'),
    std::make_pair("+",             '+'),
    std::make_pair(",",             ','),
    std::make_pair("-",             '-'),
    std::make_pair(".",             '.'),
    std::make_pair("/",             '/'),
    std::make_pair("0",             '0'),
    std::make_pair("1",             '1'),
    std::make_pair("2",             '2'),
    std::make_pair("3",             '3'),
    std::make_pair("4",             '4'),
    std::make_pair("5",             '5'),
    std::make_pair("6",             '6'),
    std::make_pair("7",             '7'),
    std::make_pair("8",             '8'),
    std::make_pair("9",             '9'),
    std::make_pair(":",             ':'),
    std::make_pair(";",             ';'),
    std::make_pair("<",             '<'),
    std::make_pair("=",             '='),
    std::make_pair(">",             '>'),
    std::make_pair("?",             '?'),
    std::make_pair("@",             '@'),
    std::make_pair("A",             'A'),
    std::make_pair("B",             'B'),
    std::make_pair("C",             'C'),
    std::make_pair("D",             'D'),
    std::make_pair("E",             'E'),
    std::make_pair("F",             'F'),
    std::make_pair("G",             'G'),
    std::make_pair("H",             'H'),
    std::make_pair("I",             'I'),
    std::make_pair("J",             'J'),
    std::make_pair("K",             'K'),
    std::make_pair("L",             'L'),
    std::make_pair("M",             'M'),
    std::make_pair("N",             'N'),
    std::make_pair("O",             'O'),
    std::make_pair("P",             'P'),
    std::make_pair("Q",             'Q'),
    std::make_pair("R",             'R'),
    std::make_pair("S",             'S'),
    std::make_pair("T",             'T'),
    std::make_pair("U",             'U'),
    std::make_pair("V",             'V'),
    std::make_pair("W",             'W'),
    std::make_pair("X",             'X'),
    std::make_pair("Y",             'Y'),
    std::make_pair("Z",             'Z'),
    std::make_pair("[",             '['),
    std::make_pair("\\",            '\\'),
    std::make_pair("]",             ']'),
    std::make_pair("^",             '^'),
    std::make_pair("_",             '_'),
    std::make_pair("`",             '`'),
    std::make_pair("a",             'a'),
    std::make_pair("b",             'b'),
    std::make_pair("c",             'c'),
    std::make_pair("d",             'd'),
    std::make_pair("e",             'e'),
    std::make_pair("f",             'f'),
    std::make_pair("g",             'g'),
    std::make_pair("h",             'h'),
    std::make_pair("i",             'i'),
    std::make_pair("j",             'j'),
    std::make_pair("k",             'k'),
    std::make_pair("l",             'l'),
    std::make_pair("m",             'm'),
    std::make_pair("n",             'n'),
    std::make_pair("o",             'o'),
    std::make_pair("p",             'p'),
    std::make_pair("q",             'q'),
    std::make_pair("r",             'r'),
    std::make_pair("s",             's'),
    std::make_pair("t",             't'),
    std::make_pair("u",             'u'),
    std::make_pair("v",             'v'),
    std::make_pair("w",             'w'),
    std::make_pair("x",             'x'),
    std::make_pair("y",             'y'),
    std::make_pair("z",             'z'),
    std::make_pair("{",             '{'),
    std::make_pair("|",             '|'),
    std::make_pair("}",             '}'),
    std::make_pair("~",             '~')
};

std::unordered_map<char, int> max7219_alphabet = {
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