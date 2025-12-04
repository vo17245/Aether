#pragma once
#include <cstdint>
#include <string>
namespace Aether
{
    class WriterI
    {
    public:
        virtual ~WriterI(){}
        virtual void Write(const char* data,size_t len)=0;
        WriterI& operator<<(const std::string& s)
        {
            Write(s.data(),s.size());
            return *this;
        }
        WriterI& operator<<(const char* s)
        {
            Write(s,strlen(s));
            return *this;
        }
    };  
    class ReaderI
    {
    public:
        virtual ~ReaderI(){}
        virtual void Read(const char* buf,size_t len)=0;
    };
    class MemoryStringWriter:public WriterI
    {
    public:
        MemoryStringWriter(std::string& data):m_Data(data){}
        virtual void Write(const char* data,size_t len) override
        {
            m_Data.append(data,len);
        }

    private:
        std::string& m_Data;
    };
}