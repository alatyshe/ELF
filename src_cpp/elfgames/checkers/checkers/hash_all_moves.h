#pragma once
# include <map>

// all possible moves for checkers

// Since the board is represented by six numbers(forward, backward, pieces),
// the pattern of transition from one state to another is changing.
// For the step we use the number in which all the logic is encoded 2 active bits.
// If the bit on the board and on the move is active - we deactivate it,
// otherwise - activate. This way we move the chips.
// 
// в m_to_i 
//    {"68, 1", 2}:
//        68  - our move, 
//        1   - direction(backward, forward)
//        2   - index of move
namespace moves {
const std::map<std::string, int>  m_to_i = {
  {"17, 1", 0},
  {"34, 1", 1},
  {"68, 1", 2},
  {"136, 1", 3},
  {"544, 1", 4},
  {"1088, 1", 5},
  {"2176, 1", 6},
  {"8704, 1", 7},
  {"17408, 1", 8},
  {"34816, 1", 9},
  {"69632, 1", 10},
  {"278528, 1", 11},
  {"557056, 1", 12},
  {"1114112, 1", 13},
  {"4456448, 1", 14},
  {"8912896, 1", 15},
  {"17825792, 1", 16},
  {"35651584, 1", 17},
  {"142606336, 1", 18},
  {"285212672, 1", 19},
  {"570425344, 1", 20},
  {"2281701376, 1", 21},
  {"4563402752, 1", 22},
  {"9126805504, 1", 23},
  {"18253611008, 1", 24},
  {"33, 1", 25},
  {"66, 1", 26},
  {"132, 1", 27},
  {"528, 1", 28},
  {"1056, 1", 29},
  {"2112, 1", 30},
  {"4224, 1", 31},
  {"16896, 1", 32},
  {"33792, 1", 33},
  {"67584, 1", 34},
  {"270336, 1", 35},
  {"540672, 1", 36},
  {"1081344, 1", 37},
  {"2162688, 1", 38},
  {"8650752, 1", 39},
  {"17301504, 1", 40},
  {"34603008, 1", 41},
  {"138412032, 1", 42},
  {"276824064, 1", 43},
  {"553648128, 1", 44},
  {"1107296256, 1", 45},
  {"4429185024, 1", 46},
  {"8858370048, 1", 47},
  {"17716740096, 1", 48},
  {"17, 0", 49},
  {"34, 0", 50},
  {"68, 0", 51},
  {"136, 0", 52},
  {"544, 0", 53},
  {"1088, 0", 54},
  {"2176, 0", 55},
  {"8704, 0", 56},
  {"17408, 0", 57},
  {"34816, 0", 58},
  {"69632, 0", 59},
  {"278528, 0", 60},
  {"557056, 0", 61},
  {"1114112, 0", 62},
  {"4456448, 0", 63},
  {"8912896, 0", 64},
  {"17825792, 0", 65},
  {"35651584, 0", 66},
  {"142606336, 0", 67},
  {"285212672, 0", 68},
  {"570425344, 0", 69},
  {"2281701376, 0", 70},
  {"4563402752, 0", 71},
  {"9126805504, 0", 72},
  {"18253611008, 0", 73},
  {"33, 0", 74},
  {"66, 0", 75},
  {"132, 0", 76},
  {"528, 0", 77},
  {"1056, 0", 78},
  {"2112, 0", 79},
  {"4224, 0", 80},
  {"16896, 0", 81},
  {"33792, 0", 82},
  {"67584, 0", 83},
  {"270336, 0", 84},
  {"540672, 0", 85},
  {"1081344, 0", 86},
  {"2162688, 0", 87},
  {"8650752, 0", 88},
  {"17301504, 0", 89},
  {"34603008, 0", 90},
  {"138412032, 0", 91},
  {"276824064, 0", 92},
  {"553648128, 0", 93},
  {"1107296256, 0", 94},
  {"4429185024, 0", 95},
  {"8858370048, 0", 96},
  {"17716740096, 0", 97},
  {"-514, 1", 98},
  {"-1028, 1", 99},
  {"-2056, 1", 100},
  {"-8224, 1", 101},
  {"-16448, 1", 102},
  {"-32896, 1", 103},
  {"-263168, 1", 104},
  {"-526336, 1", 105},
  {"-1052672, 1", 106},
  {"-4210688, 1", 107},
  {"-8421376, 1", 108},
  {"-16842752, 1", 109},
  {"-134742016, 1", 110},
  {"-269484032, 1", 111},
  {"-538968064, 1", 112},
  {"-2155872256, 1", 113},
  {"-4311744512, 1", 114},
  {"-8623489024, 1", 115},
  {"-1025, 1", 116},
  {"-2050, 1", 117},
  {"-4100, 1", 118},
  {"-16400, 1", 119},
  {"-32800, 1", 120},
  {"-65600, 1", 121},
  {"-524800, 1", 122},
  {"-1049600, 1", 123},
  {"-2099200, 1", 124},
  {"-8396800, 1", 125},
  {"-16793600, 1", 126},
  {"-33587200, 1", 127},
  {"-268697600, 1", 128},
  {"-537395200, 1", 129},
  {"-1074790400, 1", 130},
  {"-4299161600, 1", 131},
  {"-8598323200, 1", 132},
  {"-17196646400, 1", 133},
  {"-514, 0", 134},
  {"-1028, 0", 135},
  {"-2056, 0", 136},
  {"-8224, 0", 137},
  {"-16448, 0", 138},
  {"-32896, 0", 139},
  {"-263168, 0", 140},
  {"-526336, 0", 141},
  {"-1052672, 0", 142},
  {"-4210688, 0", 143},
  {"-8421376, 0", 144},
  {"-16842752, 0", 145},
  {"-134742016, 0", 146},
  {"-269484032, 0", 147},
  {"-538968064, 0", 148},
  {"-2155872256, 0", 149},
  {"-4311744512, 0", 150},
  {"-8623489024, 0", 151},
  {"-1025, 0", 152},
  {"-2050, 0", 153},
  {"-4100, 0", 154},
  {"-16400, 0", 155},
  {"-32800, 0", 156},
  {"-65600, 0", 157},
  {"-524800, 0", 158},
  {"-1049600, 0", 159},
  {"-2099200, 0", 160},
  {"-8396800, 0", 161},
  {"-16793600, 0", 162},
  {"-33587200, 0", 163},
  {"-268697600, 0", 164},
  {"-537395200, 0", 165},
  {"-1074790400, 0", 166},
  {"-4299161600, 0", 167},
  {"-8598323200, 0", 168},
  {"-17196646400, 0", 169}
};


const std::map<int, std::array<int64_t, 2>> i_to_m = {
  {0, {17, true}},
  {1, {34, true}},
  {2, {68, true}},
  {3, {136, true}},
  {4, {544, true}},
  {5, {1088, true}},
  {6, {2176, true}},
  {7, {8704, true}},
  {8, {17408, true}},
  {9, {34816, true}},
  {10, {69632, true}},
  {11, {278528, true}},
  {12, {557056, true}},
  {13, {1114112, true}},
  {14, {4456448, true}},
  {15, {8912896, true}},
  {16, {17825792, true}},
  {17, {35651584, true}},
  {18, {142606336, true}},
  {19, {285212672, true}},
  {20, {570425344, true}},
  {21, {2281701376, true}},
  {22, {4563402752, true}},
  {23, {9126805504, true}},
  {24, {18253611008, true}},
  {25, {33, true}},
  {26, {66, true}},
  {27, {132, true}},
  {28, {528, true}},
  {29, {1056, true}},
  {30, {2112, true}},
  {31, {4224, true}},
  {32, {16896, true}},
  {33, {33792, true}},
  {34, {67584, true}},
  {35, {270336, true}},
  {36, {540672, true}},
  {37, {1081344, true}},
  {38, {2162688, true}},
  {39, {8650752, true}},
  {40, {17301504, true}},
  {41, {34603008, true}},
  {42, {138412032, true}},
  {43, {276824064, true}},
  {44, {553648128, true}},
  {45, {1107296256, true}},
  {46, {4429185024, true}},
  {47, {8858370048, true}},
  {48, {17716740096, true}},
  {49, {17, false}},
  {50, {34, false}},
  {51, {68, false}},
  {52, {136, false}},
  {53, {544, false}},
  {54, {1088, false}},
  {55, {2176, false}},
  {56, {8704, false}},
  {57, {17408, false}},
  {58, {34816, false}},
  {59, {69632, false}},
  {60, {278528, false}},
  {61, {557056, false}},
  {62, {1114112, false}},
  {63, {4456448, false}},
  {64, {8912896, false}},
  {65, {17825792, false}},
  {66, {35651584, false}},
  {67, {142606336, false}},
  {68, {285212672, false}},
  {69, {570425344, false}},
  {70, {2281701376, false}},
  {71, {4563402752, false}},
  {72, {9126805504, false}},
  {73, {18253611008, false}},
  {74, {33, false}},
  {75, {66, false}},
  {76, {132, false}},
  {77, {528, false}},
  {78, {1056, false}},
  {79, {2112, false}},
  {80, {4224, false}},
  {81, {16896, false}},
  {82, {33792, false}},
  {83, {67584, false}},
  {84, {270336, false}},
  {85, {540672, false}},
  {86, {1081344, false}},
  {87, {2162688, false}},
  {88, {8650752, false}},
  {89, {17301504, false}},
  {90, {34603008, false}},
  {91, {138412032, false}},
  {92, {276824064, false}},
  {93, {553648128, false}},
  {94, {1107296256, false}},
  {95, {4429185024, false}},
  {96, {8858370048, false}},
  {97, {17716740096, false}},
  {98, {-514, true}},
  {99, {-1028, true}},
  {100, {-2056, true}},
  {101, {-8224, true}},
  {102, {-16448, true}},
  {103, {-32896, true}},
  {104, {-263168, true}},
  {105, {-526336, true}},
  {106, {-1052672, true}},
  {107, {-4210688, true}},
  {108, {-8421376, true}},
  {109, {-16842752, true}},
  {110, {-134742016, true}},
  {111, {-269484032, true}},
  {112, {-538968064, true}},
  {113, {-2155872256, true}},
  {114, {-4311744512, true}},
  {115, {-8623489024, true}},
  {116, {-1025, true}},
  {117, {-2050, true}},
  {118, {-4100, true}},
  {119, {-16400, true}},
  {120, {-32800, true}},
  {121, {-65600, true}},
  {122, {-524800, true}},
  {123, {-1049600, true}},
  {124, {-2099200, true}},
  {125, {-8396800, true}},
  {126, {-16793600, true}},
  {127, {-33587200, true}},
  {128, {-268697600, true}},
  {129, {-537395200, true}},
  {130, {-1074790400, true}},
  {131, {-4299161600, true}},
  {132, {-8598323200, true}},
  {133, {-17196646400, true}},
  {134, {-514, false}},
  {135, {-1028, false}},
  {136, {-2056, false}},
  {137, {-8224, false}},
  {138, {-16448, false}},
  {139, {-32896, false}},
  {140, {-263168, false}},
  {141, {-526336, false}},
  {142, {-1052672, false}},
  {143, {-4210688, false}},
  {144, {-8421376, false}},
  {145, {-16842752, false}},
  {146, {-134742016, false}},
  {147, {-269484032, false}},
  {148, {-538968064, false}},
  {149, {-2155872256, false}},
  {150, {-4311744512, false}},
  {151, {-8623489024, false}},
  {152, {-1025, false}},
  {153, {-2050, false}},
  {154, {-4100, false}},
  {155, {-16400, false}},
  {156, {-32800, false}},
  {157, {-65600, false}},
  {158, {-524800, false}},
  {159, {-1049600, false}},
  {160, {-2099200, false}},
  {161, {-8396800, false}},
  {162, {-16793600, false}},
  {163, {-33587200, false}},
  {164, {-268697600, false}},
  {165, {-537395200, false}},
  {166, {-1074790400, false}},
  {167, {-4299161600, false}},
  {168, {-8598323200, false}},
  {169, {-17196646400, false}}
  };

const std::map<int, std::string>  m_to_h = {
  {0   ,"62 => 55"},
  {1   ,"60 => 53"},
  {2   ,"58 => 51"},
  {3   ,"56 => 49"},
  {4   ,"53 => 46"},
  {5   ,"51 => 44"},
  {6   ,"49 => 42"},
  {7   ,"46 => 39"},
  {8   ,"44 => 37"},
  {9   ,"42 => 35"},
  {10  ,"40 => 33"},
  {11  ,"37 => 30"},
  {12  ,"35 => 28"},
  {13  ,"33 => 26"},
  {14  ,"30 => 23"},
  {15  ,"28 => 21"},
  {16  ,"26 => 19"},
  {17  ,"24 => 17"},
  {18  ,"21 => 14"},
  {19  ,"19 => 12"},
  {20  ,"17 => 10"},
  {21  ,"14 => 7"},
  {22  ,"12 => 5"},
  {23  ,"10 => 3"},
  {24  ,"8 => 1"},
  {25  ,"62 => 53"},
  {26  ,"60 => 51"},
  {27  ,"58 => 49"},
  {28  ,"55 => 46"},
  {29  ,"53 => 44"},
  {30  ,"51 => 42"},
  {31  ,"49 => 40"},
  {32  ,"46 => 37"},
  {33  ,"44 => 35"},
  {34  ,"42 => 33"},
  {35  ,"39 => 30"},
  {36  ,"37 => 28"},
  {37  ,"35 => 26"},
  {38  ,"33 => 24"},
  {39  ,"30 => 21"},
  {40  ,"28 => 19"},
  {41  ,"26 => 17"},
  {42  ,"23 => 14"},
  {43  ,"21 => 12"},
  {44  ,"19 => 10"},
  {45  ,"17 => 8"},
  {46  ,"14 => 5"},
  {47  ,"12 => 3"},
  {48  ,"10 => 1"},
  {49  ,"55 => 62"},
  {50  ,"53 => 60"},
  {51  ,"51 => 58"},
  {52  ,"49 => 56"},
  {53  ,"46 => 53"},
  {54  ,"44 => 51"},
  {55  ,"42 => 49"},
  {56  ,"39 => 46"},
  {57  ,"37 => 44"},
  {58  ,"35 => 42"},
  {59  ,"33 => 40"},
  {60  ,"30 => 37"},
  {61  ,"28 => 35"},
  {62  ,"26 => 33"},
  {63  ,"23 => 30"},
  {64  ,"21 => 28"},
  {65  ,"19 => 26"},
  {66  ,"17 => 24"},
  {67  ,"14 => 21"},
  {68  ,"12 => 19"},
  {69  ,"10 => 17"},
  {70  ,"7 => 14"},
  {71  ,"5 => 12"},
  {72  ,"3 => 10"},
  {73  ,"1 => 8"},
  {74  ,"53 => 62"},
  {75  ,"51 => 60"},
  {76  ,"49 => 58"},
  {77  ,"46 => 55"},
  {78  ,"44 => 53"},
  {79  ,"42 => 51"},
  {80  ,"40 => 49"},
  {81  ,"37 => 46"},
  {82  ,"35 => 44"},
  {83  ,"33 => 42"},
  {84  ,"30 => 39"},
  {85  ,"28 => 37"},
  {86  ,"26 => 35"},
  {87  ,"24 => 33"},
  {88  ,"21 => 30"},
  {89  ,"19 => 28"},
  {90  ,"17 => 26"},
  {91  ,"14 => 23"},
  {92  ,"12 => 21"},
  {93  ,"10 => 19"},
  {94  ,"8 => 17"},
  {95  ,"5 => 14"},
  {96  ,"3 => 12"},
  {97  ,"1 => 10"},
  {98  ,"60 => 46"},
  {99  ,"58 => 44"},
  {100 ,"56 => 42"},
  {101 ,"53 => 39"},
  {102 ,"51 => 37"},
  {103 ,"49 => 35"},
  {104 ,"44 => 30"},
  {105 ,"42 => 28"},
  {106 ,"40 => 26"},
  {107 ,"37 => 23"},
  {108 ,"35 => 21"},
  {109 ,"33 => 19"},
  {110 ,"28 => 14"},
  {111 ,"26 => 12"},
  {112 ,"24 => 10"},
  {113 ,"21 => 7"},
  {114 ,"19 => 5"},
  {115 ,"17 => 3"},
  {116 ,"62 => 44"},
  {117 ,"60 => 42"},
  {118 ,"58 => 40"},
  {119 ,"55 => 37"},
  {120 ,"53 => 35"},
  {121 ,"51 => 33"},
  {122 ,"46 => 28"},
  {123 ,"44 => 26"},
  {124 ,"42 => 24"},
  {125 ,"39 => 21"},
  {126 ,"37 => 19"},
  {127 ,"35 => 17"},
  {128 ,"30 => 12"},
  {129 ,"28 => 10"},
  {130 ,"26 => 8"},
  {131 ,"23 => 5"},
  {132 ,"21 => 3"},
  {133 ,"19 => 1"},
  {134 ,"46 => 60"},
  {135 ,"44 => 58"},
  {136 ,"42 => 56"},
  {137 ,"39 => 53"},
  {138 ,"37 => 51"},
  {139 ,"35 => 49"},
  {140 ,"30 => 44"},
  {141 ,"28 => 42"},
  {142 ,"26 => 40"},
  {143 ,"23 => 37"},
  {144 ,"21 => 35"},
  {145 ,"19 => 33"},
  {146 ,"14 => 28"},
  {147 ,"12 => 26"},
  {148 ,"10 => 24"},
  {149 ,"7 => 21"},
  {150 ,"5 => 19"},
  {151 ,"3 => 17"},
  {152 ,"44 => 62"},
  {153 ,"42 => 60"},
  {154 ,"40 => 58"},
  {155 ,"37 => 55"},
  {156 ,"35 => 53"},
  {157 ,"33 => 51"},
  {158 ,"28 => 46"},
  {159 ,"26 => 44"},
  {160 ,"24 => 42"},
  {161 ,"21 => 39"},
  {162 ,"19 => 37"},
  {163 ,"17 => 35"},
  {164 ,"12 => 30"},
  {165 ,"10 => 28"},
  {166 ,"8 => 26"},
  {167 ,"5 => 23"},
  {168 ,"3 => 21"},
  {169 ,"1 => 19"}
  };
} 

// all_moves