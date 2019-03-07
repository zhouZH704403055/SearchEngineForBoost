#pragma once
#include"../../Index/cpp/index.h"
#include"server.pb.h"
namespace doc_server
{
    typedef doc_server_proto::Request Request;
    typedef doc_server_proto::Response Response;
    typedef doc_index_proto::Weight Weight;
    //上下文，请求过程中所依赖的数据以及中间数据
    //Context 中放置的一定是请求相关的数据
    struct Context
    {
        const Request* request;
        Response* response;
        //保存分词结果
        std::vector<std::string> wrods;
        //保存触发结果的倒排拉链
        std::vector<Weight> all_query_chain;
    };

    class DocSearch
    {
    public:
        //对外的API函数，完成搜索
        bool Search(const Request& req,Response* resp);
    private:
        //1.根据关键词进行分词
        bool CutQuery(Context* context);
        //2.对分词结果触发，得到相关倒排拉链
        bool Retrieve(Context* context);
        //3.对触发出的文档排序
        bool Rank(Context* context);
        //4.包装得到的结果，构造成Response结果返回
        bool PackageResponse(Context* context);
        //记录日志文件
        bool Log(Context* context);
        //生成描述信息,依赖正文信息和关键词第一次在正文中出现在正文中的位置信息
        std::string GetDesc(const std::string& content,int first_pos);
        int FindSentenceBegin(const std::string& content,int first_pos);
        void ReplaceEscape(std::string* desc);
    };
}//end doc_server
