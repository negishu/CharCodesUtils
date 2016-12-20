#pragma once
#include "DataList.h"
#include "DoubleArray.h"
class BaseCompiler : public DataList
{
public:
    BaseCompiler(void){};
    virtual ~BaseCompiler(void){};
    virtual bool Build(const char* pInFileName, const char* pConvName) = 0;
protected:
    virtual const void* GetArray() = 0;
    virtual const unsigned int GetSize() = 0;
    virtual const unsigned int GetCodeLowRange() = 0;
    virtual const unsigned int GetCodeHighRange() = 0;
    virtual bool CreateOutput     (const char* pOutFileName, const std::string& escaped, const char* pConvName = NULL);
    virtual bool CreateDictionary () = 0;
    virtual bool CreateDobuleArray() = 0;
};
class SingleCodeCompiler : public BaseCompiler, public DoubleArray
{
public:
    SingleCodeCompiler(const char* pOutFileName);
    ~SingleCodeCompiler(void);
    bool Build(const char* pInFileName, const char* pConvName);
private:
    virtual const void* GetArray() { return DoubleArray::GetArray(); };
    virtual const unsigned int GetSize() { return DoubleArray::GetSize(); };
    virtual const unsigned int GetCodeLowRange() { return DoubleArray::GetCodeLowRange(); };;
    virtual const unsigned int GetCodeHighRange() { return DoubleArray::GetCodeHighRange(); };;
    void PrepareFiles();
    bool CreateDictionary();
    bool CreateDobuleArray();
    void Escape(const std::string& input, std::string& output);
    std::vector<std::pair<std::string, int>> _dic;
    std::string _output;
    std::string h_filename_;
    std::string cpp_filename_;
};
