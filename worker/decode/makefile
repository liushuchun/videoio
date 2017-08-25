OBJS=decode_test.o decode_frames.o extract_flows.o
INCLUDES=-I ./ffmpeg-3.3.2/include 
CXXFLAGS=-D__STDC_CONSTANT_MACROS `pkg-config --cflags opencv`
LDFLAGS=-L ./ffmpeg-3.3.2/lib -lavformat -lavcodec -lavutil -lrt -lswscale -lswresample -lpthread -lm -lz -ldl
LIBS=`pkg-config --libs opencv`

decode_test:$(OBJS)
	g++ $(CXXCFLAGS) -o decode_test $(OBJS) $(LIBS) $(LDFLAGS) 
decode_test.o:decode_test.cpp
	g++ $(CXXFLAGS) -c decode_test.cpp
decode_frames.o:decode_frames.cpp
	g++ $(CXXFLAGS) $(INCLUDES) -c decode_frames.cpp
extract_flows.o:extract_flows.cpp
	g++ $(CXXFLAGS) -c extract_flows.cpp

clean:
	-rm decode_test ${OBJS} -rf
