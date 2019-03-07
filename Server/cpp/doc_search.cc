#include"doc_search.h"
#include"../../Common/util.hpp"

DEFINE_int32(desc_max_size,160,"描述的最大长度");

namespace  doc_server
{
    bool DocSearch::Search(const Request& request,Response* response)
    {
        Context context;
        context.request=&request;
        context.response=response;
        CutQuery(&context);
        Retrieve(&context);
        Rank(&context);
        PackageResponse(&context);
        Log(&context);
        return true;
    }

    bool DocSearch::CutQuery(Context* context)
    {
        //基于jieba分词库对查询词分词
        //分词结果保存在context->words中
        //需要去除暂停词
        doc_index::Index* index=doc_index::Index::GetInstance();
        index->CutWordWithoutStopWord(context->request->query(),&context->wrods);
        return true;
    }

    bool DocSearch::Retrieve(Context* context)
    {
        doc_index::Index* index=doc_index::Index::GetInstance();
        //根据分词结果，查找每个关键词对应的倒排索引
        //将查找到的倒排拉链都放入context->all_query_chain中
        //all_query_chain为关键词对应的所有倒排拉链的集合
        for(const auto& word : context->wrods)
        {
            const doc_index::InvertedList* inverted_list=index->GetInvertedList(word);
            if(inverted_list==nullptr)
            {
                //说明当前关键词没有在任何文档中出现
                continue;
            }
            //对当前获取到的倒排拉链进行合并
            for(const doc_index::Weight& weight : *(inverted_list))
            {
                context->all_query_chain.push_back(weight);
            }
        }
        return true;
    }

    bool DocSearch::Rank(Context* context)
    {
        //按照权重降序排序
        //all_query_chain中有若干个关键词的倒排拉链，需要进行重新排序
        std::sort(context->all_query_chain.begin(),context->all_query_chain.end(),doc_index::Index::CmpWeight);
        return true;
    }

    bool DocSearch::PackageResponse(Context* context)
    {
        //构造Response对象，核心是Item元素
        //查询正排索引
        doc_index::Index* index=doc_index::Index::GetInstance();
        const Request* req = context->request;
        context->response->set_sid(context->request->sid());
        context->response->set_time_stamp(common::TimeUtil::TimeStamp());
        for(const auto& weight : context->all_query_chain)
        {
            //根据doc_id查询正排索引
            const doc_index::DocInfo* doc_info=index->GetDocInfo(weight.doc_id());
            auto* item=context->response->add_item();
            item->set_title(doc_info->title());
            //根据正文生成描述信息
            item->set_desc(GetDesc(doc_info->content(),weight.first_pos()));
            item->set_jump_url(doc_info->jump_url());
            item->set_show_url(doc_info->show_url());            
        }
        return true;
    }

    std::string DocSearch::GetDesc(const std::string& content,int first_pos)
    {
       int desc_begin=0;
       //根据first_pos查找关键词出现的第一句的起始位置
       //注意：如果关键词只在标题出现但却没有在正文出现过，则first_pos为-1
       if(first_pos != -1)
       {
           desc_begin=FindSentenceBegin(content,first_pos);
       }
       //保存描述结果
       std::string desc;
       if(desc_begin+FLAGS_desc_max_size >= (int64_t)content.size())
       {
           //剩余正文长度小于最大长度，均作为描述
           desc=content.substr(desc_begin);
       }
       else
       {
           //剩余正文长度超过最大长度
           desc=content.substr(desc_begin,fLI::FLAGS_desc_max_size);
           desc[desc.size()-1]='.';
           desc[desc.size()-2]='.';
           desc[desc.size()-3]='.';
       }
       ReplaceEscape(&desc);
       return desc;
    }

    int DocSearch::FindSentenceBegin(const std::string& content,int first_pos)
    {
        for(int cur=first_pos;cur>=0;--cur)
        {
            if(content[cur]==','||content[cur]==';'||content[cur]=='.'||content[cur]=='!'||content[cur]=='?')
            {
                return cur+1;
            }
        }
        //循环结束还未找到句子分隔符则取正文开始位置
        return 0;
    }

    void DocSearch::ReplaceEscape(std::string* desc)
    {
        //利用boost库中的函数进行替换
        //需要替换的转义字符：
        // "  ->  &quot;
        // &  ->  &amp;
        // <  ->  &lt;
        // >  ->  &gt;
        boost::algorithm::replace_all(*desc,"&","&amp;");
        boost::algorithm::replace_all(*desc,"\"","&quot;");
        boost::algorithm::replace_all(*desc,"<","&lt;");
        boost::algorithm::replace_all(*desc,">","&gt;");
    }

    bool DocSearch::Log(Context* context)
    {
        //记录请求的详细请求信息和响应信息
        LOG(INFO)<<"[Request]"<<context->request->Utf8DebugString();
        LOG(INFO)<<"[Response]"<<context->response->Utf8DebugString();
        return true;
    }
}//end doc_server
