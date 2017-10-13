OBJS=work.o redis_helper.o json_func.o decode_frames.o extract_flows.o
INCLUDES=-I ./decode/ffmpeg-3.3.2/include -I evpp/apps -I evpp -I ./
CXXFLAGS=-D__STDC_CONSTANT_MACROS `pkg-config --cflags opencv`
LDFLAGS=-L ./decode/ffmpeg-3.3.2/lib  -L ./hiredis-vip -L evpp/build-release/lib -levpp -levnsq -lhiredis_vip -lavformat -lavcodec -lavutil -lrt -lswscale -lswresample -lpthread -lm -lz -ldl
LIBS=`pkg-config --libs opencv`

work:$(OBJS)
        g++ -std=c++11 $(CXXCFLAGS) -o worker $(OBJS) $(LIBS) $(LDFLAGS)
json_func.o:json_func.cpp
        g++ $(CXXFLAGS) -c json_func.cpp -I ./rapidjson/include
work.o:work.cpp
        g++ -std=c++11 $(CXXFLAGS) $(INCLUDES) -c work.cpp -I ./hiredis-vip -I ./rapidjson/include -I ./decode
redis_helper.o:redis_helper.cpp
        g++ $(CXXFLAGS) -c redis_helper.cpp -I ./hiredis-vip
decode_frames.o:./decode/decode_frames.cpp
        g++ $(CXXFLAGS) $(INCLUDES) -c ./decode/decode_frames.cpp -I ./rapidjson/include -I ./hiredis-vip
extract_flows.o:./decode/extract_flows.cpp
        g++ $(CXXFLAGS) -c ./decode/extract_flows.cpp -I ./rapidjson/include -I ./hiredis-vip

clean:
        -rm work ${OBJS} -rf