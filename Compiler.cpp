#include "Compiler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstdio>

bool BaseCompiler::CreateOutput(const char* pOutFileName, const std::string& escaped, const char* pConvName)
{
    std::ofstream ofs(pOutFileName, std::ofstream::app);
    if (ofs.is_open()) {
        ofs << "static const DoubleArray " << pConvName << "_da[] = {";
        const BaseDoubleArray::unit_* data = reinterpret_cast<const BaseDoubleArray::unit_*>(GetArray());
        for (size_t k = 0; k < GetSize(); ++k) {
            if (k != 0) ofs << ",";
            ofs << "{" << data[k].base << "," << data[k].check << "}";
        }
        ofs << "};" << std::endl;
        ofs << "static const unsigned char " << pConvName << "_table[] = {" << escaped << "};" << std::endl;
        ofs << "static const unsigned char " << pConvName << "_code_range[] = {";
        const unsigned int nLow = GetCodeLowRange();
        ofs << ((nLow>>8)&0xFF);
        ofs << ", ";
        ofs << ((nLow   )&0xFF);
        ofs << ", ";
        const unsigned int nHigh = GetCodeHighRange();
        ofs << ((nHigh>>8)&0xFF);
        ofs << ", ";
        ofs << ((nHigh   )&0xFF);
        ofs << ", ";
        ofs << "};" << std::endl;
        return true;
    }
    return false;
}
SingleCodeCompiler::SingleCodeCompiler(const char* pOutFileName)
{
    h_filename_ = pOutFileName; h_filename_ += ".h";
    cpp_filename_ = pOutFileName; cpp_filename_ += ".cpp";
    PrepareFiles();
}
SingleCodeCompiler::~SingleCodeCompiler(void)
{
    std::ofstream ofs(cpp_filename_.c_str(), std::ofstream::app);
    ofs << "static int WLookup(const DoubleArray *array, const unsigned char range[], const unsigned short *key, int len, int *result)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    int seekto = 0;" << std::endl;
    ofs << "    int n = 0;" << std::endl;
    ofs << "    int b = array[0].base;" << std::endl;
    ofs << "    const unsigned int lowrange = range[0] << 8 | range[1];" << std::endl;
    ofs << "    const unsigned int highrange = range[2] << 8 | range[3];" << std::endl;
    ofs << "    unsigned int p = 0;" << std::endl;
    ofs << "    unsigned int num = 0;" << std::endl;
    ofs << "    *result = -1;" << std::endl;
    ofs << "    for (int i = 0; i < len; ++i) {" << std::endl;
    ofs << "        p = b;" << std::endl;
    ofs << "        n = array[p].base;" << std::endl;
    ofs << "        if (static_cast<unsigned int>(b) == array[p].check && n < 0) {" << std::endl;
    ofs << "            seekto = i;" << std::endl;
    ofs << "            *result = -n - 1;" << std::endl;
    ofs << "            ++num;" << std::endl;
    ofs << "        }" << std::endl;
    ofs << "        unsigned int nKey = key[i];" << std::endl;
    ofs << "        if (lowrange <= nKey && nKey <= highrange) {" << std::endl;
    ofs << "            p = b + static_cast<unsigned char>((nKey - lowrange)) + 1;" << std::endl;
    ofs << "            if (static_cast<unsigned int>(b) == array[p].check) {" << std::endl;
    ofs << "                b = array[p].base;" << std::endl;
    ofs << "            }" << std::endl;
    ofs << "            else {" << std::endl;
    ofs << "                return seekto;" << std::endl;
    ofs << "            }" << std::endl;
    ofs << "        }" << std::endl;
    ofs << "        else {" << std::endl;
    ofs << "            return seekto;" << std::endl;
    ofs << "        }" << std::endl;
    ofs << "    }" << std::endl;
    ofs << "    p = b;" << std::endl;
    ofs << "    n = array[p].base;" << std::endl;
    ofs << "    if (static_cast<unsigned int>(b) == array[p].check && n < 0) {" << std::endl;
    ofs << "        seekto = len;" << std::endl;
    ofs << "        *result = -n - 1;" << std::endl;
    ofs << "    }" << std::endl;
    ofs << "    return seekto;" << std::endl;
    ofs << "}" << std::endl;

    ofs << "static int ConvertW(const DoubleArray *da, const unsigned char *ctable, const unsigned char range[], const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    unsigned char success = 0;" << std::endl;
    ofs << "    output[0] = 0;" << std::endl;
    ofs << "   	outlen = 0;" << std::endl;
    ofs << "    const unsigned short *begin = input;" << std::endl;
    ofs << "    const unsigned short *end = input + inputlen;" << std::endl;
    ofs << "    while (begin < end) {" << std::endl;
    ofs << "        int result = 0;" << std::endl;
    ofs << "        int mblen = WLookup(da, range, begin, static_cast<int>(end - begin), &result);" << std::endl;
    ofs << "        if (mblen > 0) {" << std::endl;
    ofs << "            const unsigned char *p = (const unsigned char *)&ctable[result * 2];" << std::endl;
    ofs << "            int len = 0;" << std::endl;
    ofs << "            for (int n = 0; n < 0x10; n += 2) {" << std::endl;
    ofs << "                if (p[n] == 0 && p[n + 1] == 0) {" << std::endl;
    ofs << "                    break;" << std::endl;
    ofs << "                }" << std::endl;
    ofs << "                const unsigned short c = ((p[n + 1] << 8) & 0xFF00) | ((p[n]) & 0x00FF);" << std::endl;
    ofs << "                output += c;" << std::endl;
    ofs << "                len += 2;" << std::endl;
    ofs << "            }" << std::endl;
    ofs << "            const unsigned short c = ((p[len + 2] << 8) & 0xFF00) | ((p[len + 3]) & 0x00FF);" << std::endl;
    ofs << "            mblen -= static_cast<unsigned int>(c);" << std::endl;
    ofs << "            begin += mblen;" << std::endl;
    ofs << "            success = 1;" << std::endl;
    ofs << "        }" << std::endl;
    ofs << "        else {" << std::endl;
    ofs << "            const unsigned short c = ((*(begin)& 0xff00) | (*(begin)& 0x00ff));" << std::endl;
    ofs << "            output += c;" << std::endl;
    ofs << "            begin += 1;" << std::endl;
    ofs << "            success = 1;" << std::endl;
    ofs << "        }" << std::endl;
    ofs << "    }" << std::endl;
    ofs << "    return success;" << std::endl;
    ofs << "}" << std::endl;
    ofs << "int Utils::RomajiToHiragana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    return ConvertW(w_romaji_to_hiragana_da, w_romaji_to_hiragana_table, w_romaji_to_hiragana_code_range, input, inputlen, output, outlen);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "int Utils::HiraganaToRomaji(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    return ConvertW(w_hiragana_to_romaji_da, w_hiragana_to_romaji_table, w_hiragana_to_romaji_code_range, input, inputlen, output, outlen);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "int Utils::HiraganaToKatakana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    return ConvertW(w_hiragana_to_katakana_da, w_hiragana_to_katakana_table, w_hiragana_to_katakana_code_range, input, inputlen, output, outlen);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "int Utils::KatakanaToHiragana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    return ConvertW(w_katakana_to_hiragana_da, w_katakana_to_hiragana_table, w_katakana_to_hiragana_code_range, input, inputlen, output, outlen);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "int Utils::HiraganaToHalfwidthKatakana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen)" << std::endl;
    ofs << "{" << std::endl;
    ofs << "    return ConvertW(w_hiragana_to_halfwidthkatakana_da, w_hiragana_to_halfwidthkatakana_table, w_hiragana_to_halfwidthkatakana_code_range, input, inputlen, output, outlen);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "} // namespace CHARCOVS" << std::endl;
}
void SingleCodeCompiler::PrepareFiles()
{
    {
        std::ofstream ofs(h_filename_.c_str(), std::ofstream::app);
        ofs << "namespace CHARCOVS {" << std::endl;
        ofs << "class Utils {" << std::endl;
        ofs << "public:" << std::endl;
        ofs << "  static int RomajiToHiragana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen);" << std::endl;
        ofs << "  static int HiraganaToRomaji(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen);" << std::endl;
        ofs << "  static int HiraganaToKatakana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen);" << std::endl;
        ofs << "  static int KatakanaToHiragana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen);" << std::endl;
        ofs << "  static int HiraganaToHalfwidthKatakana(const unsigned short* input, const int inputlen, unsigned short* output, int& outlen);" << std::endl;
        ofs << "};" << std::endl;
        ofs << "} // namespace CHARCOVS" << std::endl;

    }
    {
        std::ofstream ofs(cpp_filename_.c_str(), std::ofstream::app);
        ofs << "#include\"" << h_filename_ << "\"" << std::endl;
        ofs << "namespace CHARCOVS {" << std::endl;
        ofs << "struct DoubleArray { int base; int check; };" << std::endl;
    }
}
void SingleCodeCompiler::Escape(const std::string &input, std::string &output)
{
    output.clear();
    for (size_t i = 0; i < input.size(); ++i) {
        const int hi = ((static_cast<int>(input[i]) & 0xF0) >> 4);
        const int lo = ((static_cast<int>(input[i]) & 0x0F));
        output += "0x";
        output += static_cast<char>(hi >= 10 ? hi - 10 + 'A' : hi + '0');
        output += static_cast<char>(lo >= 10 ? lo - 10 + 'A' : lo + '0');
        output += ",";
    }
}
bool SingleCodeCompiler::Build(const char* pInFileName, const char* pConvName)
{
    if (LoadFileA(pInFileName)) {
        if (CreateDictionary()) {
            if (CreateDobuleArray()) {
                std::string escaped; Escape(_output, escaped);
                return CreateOutput(cpp_filename_.c_str(), escaped, pConvName);
            }
        }
    }
    return false;
} 
bool SingleCodeCompiler::CreateDictionary()
{
    _dic.clear(); _output.clear();
    if (_LineListA.size() > 0) {
        std::vector<std::string> col;
        for (std::vector<std::string>::iterator it = _LineListA.begin(); it != _LineListA.end(); ++it){
            col.clear();
            std::string line = *it;
            if (line[0] == '#') continue;
            spliterA(line, (const int)'\t', &col);
            const std::string empty = "";
            const std::string &key      = col.size() > 0 ? col[0] : empty;
            const std::string &value    = col.size() > 1 ? col[1] : empty;
            const std::string &newvalue = col.size() > 2 ? col[2] : empty;
            _dic.push_back(std::pair<std::string, int>(key, static_cast<int>(_output.size())));
            _output += value;
            _output += '\0';
            _output += newvalue;
        }
        return true;
    }
    return false;
}
bool SingleCodeCompiler::CreateDobuleArray()
{
    std::sort(_dic.begin(), _dic.end());
    std::vector<char *> ary;
    std::vector<int   > values;
    for (size_t k = 0; k < _dic.size(); ++k) {
        ary.push_back((char*)_dic[k].first.c_str() );
        values.push_back(_dic[k].second);
    }
    return DoubleArray::Build(const_cast<const char **>(&ary[0]), &values[0], _dic.size());
}
