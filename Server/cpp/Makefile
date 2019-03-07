PROTOC=~/Code/SearchEngine/third_part/bin/protoc
FLAG=-g -std=c++11 -I ~/Code/SearchEngine/third_part/include -L ~/Code/SearchEngine/third_part/lib -L ../../Index/cpp -lsofa-pbrpc -lprotobuf -lglog -lgflags -lpthread -lz -lsnappy -lindex

server:server.pb.cc server_main.cc doc_search.cc
	g++ $^ -o $@ $(FLAG)
	cp -f $@ ../bin
server.pb.cc:server.proto
	$(PROTOC) server.proto --cpp_out=.

.PHONY:clean
clean:
	rm -rf server server.pb.*
