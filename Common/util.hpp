#pragma once
#include<string>
#include<vector>
#include<fstream>
#include<unordered_set>
#include<boost/algorithm/string.hpp>
#include<sys/time.h>

namespace common
{
    class StringUtil
    {
    public:
        static void Split(const std::string& input,std::vector<std::string>* output,const std::string& split_char)
        {
            //利用boost库中的字符串分割函数帮助分割
            boost::split(*output,input,boost::is_any_of(split_char),boost::token_compress_off);
        }
    };

    class DictUtil
    {
    public:
        bool Load(const std::string& path)
        {
            std::ifstream file(path.c_str());
            if(!file.is_open())
            {
                return false;
            }
            std::string line;
            while(std::getline(file,line))
            {
                _set.insert(line);
            }
            file.close();
            return true;
        }
        
        bool Find(const std::string& key) const
        {
            return _set.find(key) != _set.end();
        }
    private:
        std::unordered_set<std::string> _set;
    };

    class FileUtil
    {
    public:
        static bool Read(const std::string& input_path,std::string* content)
        {
            std::ifstream file(input_path.c_str());
            if(!file.is_open())
            {
                return false;
            }
            //用这种方式读文件，文件最大不能超过2G
            //因为length为int，有符号长整型，最大正好表示为2G，超过2G则会溢出
            file.seekg(0,file.end);
            int length = file.tellg();
            file.seekg(0,file.beg);
            content->resize(length);
            file.read(const_cast<char*>(content->data()),length);
            file.close();
            return true;
        }

        static bool Write(const std::string& output_path,const std::string& content)
        {
            std::ofstream file(output_path.c_str());
            if(!file.is_open())
            {
                return false;
            }
            file.write(content.data(),content.size());
            file.close();
            return true;
        }
    };

    class TimeUtil
    {
    public:
        //获取秒级时间戳
        static int64_t TimeStamp()
        {
            struct ::timeval tv;
            ::gettimeofday(&tv,nullptr);
            return tv.tv_sec;
        }

        static int64_t TimeStampMS()
        {
            struct ::timeval tv;
            ::gettimeofday(&tv,nullptr);
            return tv.tv_sec*1000 + tv.tv_usec/1000;
        }

        static int64_t TimeStampUS()
        {
            struct ::timeval tv;
            ::gettimeofday(&tv,nullptr);
            return tv.tv_sec*1000000 + tv.tv_usec;
        }
    };
}//end common
