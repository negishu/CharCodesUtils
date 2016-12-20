#include <string>
#include "Compiler.h"
int main(int argc, char* argv[])
{
    SingleCodeCompiler _Compiler((const char*)"convcodes");
    _Compiler.SetCodeRange(0x00, 0xFF);
    _Compiler.Build("data/RomajiToHiragana.UTF8.txt", "w_romaji_to_hiragana");
    _Compiler.SetCodeRange(0x3040, 0x30FF);
    _Compiler.Build("data/HiraganaToRomaji.UTF8.txt", "w_hiragana_to_romaji");
    _Compiler.SetCodeRange(0x3040, 0x30FF);
    _Compiler.Build("data/HiraganaToFullKatakana.UTF8.txt", "w_hiragana_to_katakana");
    _Compiler.SetCodeRange(0x3040, 0x30FF);
    _Compiler.Build("data/HiraganaToHalfKatakana.UTF8.txt", "w_hiragana_to_halfwidthkatakana");
    _Compiler.SetCodeRange(0x3040, 0x30FF);
    _Compiler.Build("data/HiraganaToValues.UTF8.txt", "w_hiragana_to_values");
    _Compiler.SetCodeRange(0x30A0, 0x30FF);
    _Compiler.Build("data/FullKatakanaToHiragana.UTF8.txt", "w_katakana_to_hiragana");
    return 0;
}
