OBJS=universal_worker.o redis_helper.o json_func.o decode_frames.o extract_flows.o
INCLUDES=-I ./decode/ffmpeg-3.3.2/include -I /Users/macbook/pyproject/videoio/worker -I /Users/macbook/pyproject/videoio/worker/nsq
CXXFLAGS=-D__STDC_CONSTANT_MACROS `pkg-config --cflags opencv`
LDFLAGS=-L ./decode/ffmpeg-3.3.2/lib  -L ./hiredis-vip -lhiredis_vip -lavformat -lavcodec -lavutil -lrt -lswscale -lswresample -lpthread -lm -lz -ldl
LIBS=`pkg-config --libs opencv`

universal_worker:$(OBJS)
	g++ $(CXXCFLAGS) -o universal_worker $(OBJS) $(LIBS) $(LDFLAGS) 
json_func.o:json_func.cpp
	g++ $(CXXFLAGS) -c json_func.cpp -I ./rapidjson/include
universal_worker.o:universal_worker.cpp
	g++ $(CXXFLAGS) $(INCLUDES) -c universal_worker.cpp -I ./hiredis-vip -I ./rapidjson/include -I ./decode
redis_helper.o:redis_helper.cpp
	g++ $(CXXFLAGS) -c redis_helper.cpp -I ./hiredis-vip
decode_frames.o:./decode/decode_frames.cpp
	g++ $(CXXFLAGS) $(INCLUDES) -c ./decode/decode_frames.cpp -I ./rapidjson/include -I ./hiredis-vip
extract_flows.o:./decode/extract_flows.cpp
	g++ $(CXXFLAGS) -c ./decode/extract_flows.cpp -I ./rapidjson/include -I ./hiredis-vip

clean:
	-rm universal_worker ${OBJS} -rf
