#ifndef __JSON_FUNC__
#define __JSON_FUNC__

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/filereadstream.h"
#include <stdio.h>
#include <iostream>

int read_videoinfo(char* pVideoInfoFile, rapidjson::Document& video_info);
int verify_videoinfo_ucf(rapidjson::Document& video_info);
int verify_videoinfo_cfvid(rapidjson::Document& video_info);

#endif

