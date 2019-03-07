#include<iostream>
#include<base/base.h>
#include<sofa/pbrpc/pbrpc.h>
#include"../../Common/util.hpp"
#include"server.pb.h"

DEFINE_string(server_addr,"127.0.0.1:10000","服务器端地址");

namespace doc_client
{
    typedef doc_server_proto::Request Request;
    typedef doc_server_proto::Response Response;

    int GetQueryString(char output[])
    {
        char* method = getenv("REQUEST_METHOD");
        if(method==nullptr)
        {
            fprintf(stderr,"REQUEST_METHOD failed\n");
            return -1;
        }
        //2.获取方法为GET方法，直接从环境变量中获取QUERY—STRING
        if(strcasecmp(method,"GET")==0)
        {
            char* query_string=getenv("QUERY_STRING");
            if(query_string==nullptr)
            {
                fprintf(stderr,"QUERY_STRING failed!!\n");
                return -1;
            }
            strcpy(output,query_string);
        }
        else
        {
            //方法为POST方法，先通过环境变量获取到CONTENT_LENGTH，再从标准输入中读取正文
            char* content_length_str=getenv("CONTENT_LENOGTH");
            if(content_length_str==nullptr)
            {
                fprintf(stderr,"CONTENT_LENGTH failed!!\n");
                return -1;
            }
            int content_length=atoi(content_length_str);
            //记录向output中写了多少个字符
            int i=0;
            for(;i<content_length;++i)
            {
                read(0,&output[i],1);
            }
            output[content_length]='\0';
        }
        return 0;
    }

    void PackageRequest(Request* req)
    {
        req->set_sid(0);
        req->set_time_stamp(common::TimeUtil::TimeStamp());
        char query_string[1024*4]= {0};
        GetQueryString(query_string);
        //quer_string  内容格式：query=filesystem
        char query[1024*4]={0};
        sscanf(query_string,"query=%s",query);
        req->set_query(query);
        std::cerr<<"query="<<query<<std::endl;
    }

    void Search(const Request& req,Response* resp)
    {
        //基于RPC方式调用服务器对应的Add函数
        using namespace sofa::pbrpc;
        //1.RPC概念一：RpcClient
        RpcClient client;
        //2.RPC概念二：RpcChannel   描述一次连接
        RpcChannel channel(&client,fLS::FLAGS_server_addr);
        //3.RPC概念三：DocServerAPI_Stub;   描述具体请求调用哪个RPC函数
        doc_server_proto::DocServerAPI_Stub stub(&channel);
        //4.RPC概念四：RpcContruller    网络通信细节的管理对象
        //管理网络通信中的五元组，以及超时时间的概念
        RpcController ctrl;
        
        //第四个参数为NULL，表示按同步的方式进行请求
        stub.Search(&ctrl,&req,resp,NULL);
        if(ctrl.Failed())
        {
            std::cerr<<"RPC Search failed!!"<<std::endl;
        }
        else
        {
            std::cerr<<"RPC Search success!!"<<std::endl;
        }
    }

    void ParseResponse(const Response& resp)
    {
        //std::cout<<resp.Utf8DebugString()<<std::endl;
        std::cout<<"<html>";
        std::cout<<"<body>";
        for(int i=0;i<resp.item_size();++i)
        {
            const auto& item=resp.item(i);
            std::cout<<"<div>";
            //标题和跳转URL
            std::cout<<"<div><a href=\""<<item.jump_url()<<"\">"<<item.title()<<"</a></div>";
            //描述
            std::cout<<"<div>"<<item.desc()<<"</div>";
            //展示URL
            std::cout<<"<div>"<<item.show_url()<<"</div>";
            std::cout<<"</div>";
        }
        std::cout<<"</body>";
        std::cout<<"</html>";
    }

    void CallServer()
    {
        //1.构造请求
        Request request;
        Response response;
        PackageRequest(&request);
        //2.发送请求并获取响应
        Search(request,&response);
        //3.解析响应并输出结果
        ParseResponse(response);
    }
}//end doc_client

int main(int argc,char* argv[])
{
    base::InitApp(argc,argv);
    doc_client::CallServer();
    return 0;
}
