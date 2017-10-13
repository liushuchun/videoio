%module work
%include "std_string.i"
%{
#define SWIG_FILE_WITH_INIT
#include "worker_test.h"
#include <string>
%}

std::string do_execute(int id,std::string label,int idx,std::string cmd,std::string video_name,float offset,int frames,std::string file_path,std::string topic);