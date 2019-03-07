#include<base/base.h>
#include<sofa/pbrpc/pbrpc.h>
#include"../../Common/util.hpp"
#include"server.pb.h"
#include"doc_search.h"

DEFINE_string(port,"10000","服务器端口号");
DEFINE_string(index_path,"../index/index_file","索引目录路径");

namespace doc_server
{
    typedef doc_server_proto::Request Request;
    typedef doc_server_proto::Response Response;

    class DocServerAPIImpl:public doc_server_proto::DocServerAPI
    {
    public:
        //controllrt；网络通信中的细节控制
        //done：请求处理完毕调用done（闭包——回调函数及其参数）通知RPC框架请求处理结束
        virtual void Search(::google::protobuf::RpcController* controller,const Request* request,Response* response,::google::protobuf::Closure* done)
        {
            (void) controller;
            response->set_sid(request->sid());
            response->set_time_stamp(common::TimeUtil::TimeStamp());
            //根据请求进行响应
            DocSearch searcher;
            searcher.Search(*request,response);
            //调用闭包
            done->Run();
        }
    };
}//end doc_server

//初始化服务器
int main(int argc,char* argv[])
{
    base::InitApp(argc,argv);
    using namespace sofa::pbrpc;
    //0.索引的初始化和加载
    doc_index::Index* index = doc_index::Index::GetInstance();
    CHECK(index->Load(fLS::FLAGS_index_path));
    LOG(INFO)<<"Index Load Done!!";
    std::cout<<"Index Load Done!!"<<std::endl;
    //1.创建RpcServer对象
    RpcServerOptions options;
    options.work_thread_num = 4;//指定处理请求的线程数量
    RpcServer server(options);
    //2.启动RpcServer
    CHECK(server.Start("0.0.0.0:" + fLS::FLAGS_port));
    //3.注册，请求处理方式
    doc_server::DocServerAPIImpl* server_impl = new doc_server::DocServerAPIImpl();
    server.RegisterService(server_impl);
    //4.进入循环等待服务器结束信号
    server.Run();
    return 0;
}
