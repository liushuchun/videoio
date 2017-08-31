#ifndef __UNIVERSAL_WORKER__
#define __UNIVERSAL_WORKER__

/*
Universal Worker
*/

#include "json_func.h"
#include "redis_helper.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

struct global_params{
	std::string videoinfo_file;
    rapidjson::Document video_info;
	std::string video_root;
	int video_class_count;
	int video_count;
    std::vector<int> videos_per_class;
    std::string redis_address;
};
struct decode_params{
	std::string video_file;
	int msgId;
	int cls_idx;
	float f_video_offset;
	int i_video_in_class_idx;
	int i_DecWidth;
	int i_DstWidth;
	int i_DstHeight;
	std::string s_encodeType;
};

struct queue_params{
	std::string cur_key;
	std::string cur_value;
};

struct storage_params{
	redisClusterContext* cc;
};



struct serialize_params{
	unsigned long length;		//结构体总长度 - 序列化共多少字节
	unsigned short int width;	//单个图片的宽
	unsigned short int height;	//单个图片的高
	unsigned short int channels;//单个图片的通道数
	unsigned short int count;	//图片总的张数
	unsigned short int key_idx;	//关键帧的位置
	char* pContent;
};

struct worker_params
{
	global_params* gp;
	queue_params* qp;
	decode_params* dp;
	serialize_params* dsp;	// decode serializer
	serialize_params* esp;	// extract serializer
	storage_params* sp;
};




#endif