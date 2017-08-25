/*
Universal Worker
*/

#include "json_func.hpp"
#include "decode_frames.h"
#include "trans_frames.h"
#include <iostream>
#include <string>
#include <fstream>

#define g_VideoInfoJson "CFVID.videos.train.json"
#define d_DecodeWidth 10
#define d_ImgWidth 256
#define d_ImgHeight 256

struct global_params{
	std::string videoinfojson;
    rapidjson::Document video_info;
	int video_class_count;
	int video_count;
    std::vector<int> videos_per_class;

};
struct decode_params{
	std::string video_file;
	int cls_idx;
	float f_video_in_class_idx;
	float f_video_offset;
	int i_video_in_class_idx;
	int i_video_offset;
	int i_DecWidth;
	int i_DstWidth;
	int i_DstHeight;
	std::string s_encodeType;
};

struct queue_params{

};

struct storage_params{

};



struct serialize_params{
	unsigned long length;		//结构体总长度 - 序列化共多少字节
	unsigned short int width;	//单个图片的宽
	unsigned short int height;	//单个图片的高
	unsigned short int channels;//单个图片的通道数
	unsigned short int count;	//图片总的张数
	unsigned short int key_idx;	//关键帧的位置
	char pContent[];
};

struct worker_params
{
	global_params* gp;
	queue_params* qp;
	storage_params* stp;
	decode_params* dp;
	serialize_params* sep;

};

int init_globalparams(global_params* gp, decode_params* dp)
{

	gp->videoinfojson = g_VideoInfoJson;
    // get video json
    iRet = read_videoinfo(gp->videoinfojson, gp->video_info);
    if (iRet != 0){
            std::cout << "read_randseed_from_txt Error" << std::endl;
            return 1;
    }

    // get video count
    rapidjson::Value folder_json;
    gp->video_count = video_info["videos"].Size();
    if (gp->video_count <= 0){
        std::cout << "Error - class_count empty" << std::endl;
        return -1;
    }

    // get video class count
    rapidjson::Value folder_json;
    gp->video_class_count = video_info["classstats"].Size();
    if (gp->video_class_count <= 0){
        std::cout << "Error - class stats empty" << std::endl;
        return -1;
    }
    
    // get videos per class
    for (auto& v : video_info["classstats"].GetArray())
	    gp->videos_per_class.push_back(v.GetInt());

	dp->i_DecWidth = d_DecodeWidth;
	dp->i_DstWidth = d_ImgWidth;
	dp->i_DstHeight = d_ImgHeight;

	return 0;
}
int init_storage(global_params* gp, storage_params* sp)
{
    std::cout << "init_storage - TODO" << std::endl;
	return -1;
}

int init_queue(global_params* gp, queue_params* qp)
{
    std::cout << "init_queue - TODO" << std::endl;
	return -1;
}

int pre_execute(worker_params* wp)
{
	// get init params
	int iRet = init_globalparams(wp->gp, wp->dp);
	if(iRet!=0){
		std::cout << "init_params Error" << std::endl;
		return iRet;
	}

	// connect redis
	iRet = init_storage(wp->gp, wp->stp);
	if(iRet!=0){
		std::cout << "init_redis Error" << std::endl;
		return iRet;
	}

	// connect queue
	iRet = init_queue(wp->qp);
	if(iRet!=0){
		std::cout << "init_queue Error" << std::endl;
		return iRet;
	}
/*
	// get params from redis and queue
	iRet = init_rest_params(gp);
	if(iRet!=0){
		std::cout << "init_params Error" << std::endl;
		return iRet;
	}
*/
	return iRet;
}


int do_fetch_task(global_params* dp, queue_params* qp, decode_params* dp)
{
    std::cout << "do_fetch_task - TODO" << std::endl;
	return -1;
}


int do_decode_task(decode_params* dp, serialize_params* sep)
{

	iRet = DoFrameExport(dp->video_file.c_str(), dp->i_video_offset,
        dp->i_DstWidth, dp->i_DstHeight, AV_PIX_FMT_BGR24,
        dp->i_DecWidth, sep, 256, NULL, 0,
        &iFrameArrayOutCount);

	return iRet;
}

int do_preserve_task(serialize_params* sep, storage_params* stp)
{
    std::cout << "do_preserve_task - TODO" << std::endl;
	return -1;
}

int do_execute(worker_params* wp)
{
	int iRet = 0;

	//fetch queue job
	iRet = do_fetch_task(params);
	if(iRet!=0){
		std::cout << "do_fetch_task Error" << std::endl;
		return -1;
	}

	//decode / extract and seralize
	iRet = do_decode_task(wp->dp, wp->sep);
	if(iRet!=0){
		std::cout << "do_decode_task Error" << std::endl;
		return -1;
	}

	//preserve to mem or localize
	iRet = do_preserve_task(wp->sp, wp->sep);
	if(iRet!=0){
		std::cout << "do_preserve_task Error" << std::endl;
		return -1;
	}

	return iRet;
}


int post_execute()
{
	// clear job
	// release resource
	return iRet;
}


int main(int argc, char*  argv[])
{
	int iRet = pre_execute();
	if(iRet!=0){
		std::cout << "pre_execute Error" << std::endl;
		return -1;
	}
	while(1){
		do_execute();
		if(iRet!=0){
			std::cout << "do_execute Error" << std::endl;
			return -1;
		}
		sleep(0.1);
	}
	iRet = post_execute();
	if(iRet!=0){
		std::cout << "post_execute Error" << std::endl;
		return -1;
	}
}
