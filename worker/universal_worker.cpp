/*
Universal Worker
*/

#include "decode/decode_frames.h"
#include "decode/extract_flows.h"
#include "universal_worker.h"
#include "rapidjson/include/rapidjson/document.h"
#include <iostream>
#include <string>
#include <fstream>
#include "time.h"
#include "evnsq/consumer.h"
#include "evpp/event_loop.h"
#include <json_func.h>
#include "rapidjson/include/rapidjson/filereadstream.h"

#define g_VideoInfoJson "CFVID.videos.train.json"
#define d_DecodeWidth 10
#define d_ImgWidth 256
#define d_ImgHeight 256


//global params

worker_params* wp;
global_params* gp;

evnsq::Producer client;
evnsq::Producer pubCli;


struct WorkMsg{
    int64 id;
    std::string cmd;
    std::string videoName;
    int offset;
    int frames;
    std::string taskTopic;
};



struct TrainMsg{
    TrainMsg() {

    }

    int64 id;
    std::string cmd;
    std::string isflow;
    std::string redisKey;
    std::string starttime;
    std::string endtime;
    bool  success;
};



std::string msg;


std::string trainTopic;

bool result;

//esp extracted
//dsp decoded


int init_globalparams(decode_params*& dp)
{

	gp->videoinfo_file = g_VideoInfoJson;
    // get video json
    int iRet = read_videoinfo((char*)gp->videoinfo_file.c_str(), gp->video_info);
    if (iRet != 0){
            std::cout << "read_randseed_from_txt Error" << std::endl;
            return 1;
    }
	iRet = verify_videoinfo_cfvid(gp->video_info);
    if (iRet != 0){
            std::cout << "VideoInfo verify Error" << std::endl;
            return 1;
    }

    // get video count
    gp->video_count = gp->video_info["videos"].Size();
    if (gp->video_count <= 0){
        std::cout << "Error - class_count empty" << std::endl;
        return -1;
    }

    // get video class count
    gp->video_class_count = gp->video_info["classstats"].Size();
    if (gp->video_class_count <= 0){
        std::cout << "Error - class stats empty" << std::endl;
        return -1;
    }
    
    // get videos per class
	for(unsigned int j=0; j<gp->video_info["classstats"].Size(); j++){
	    gp->videos_per_class.push_back(gp->video_info["classstats"][j].GetInt());
	}

	gp->video_root = gp->video_info["root"].GetString();

	dp->i_DecWidth = d_DecodeWidth;
	dp->i_DstWidth = d_ImgWidth;
	dp->i_DstHeight = d_ImgHeight;

	return 0;
}
int init_preserve(storage_params*& sp)
{
	int iRet = redis_connect(sp->cc, (char*)gp->redis_address.c_str());
	if(iRet!=0){
		std::cout << "redis_connect error" << std::endl;
		return -1;		
	}
	return 0;
}

int init_queue(queue_params* qp)
{
    std::cout << "init_queue - TODO" << std::endl;
	return -1;
}

int pre_execute()
{
	wp = (worker_params*)calloc(sizeof(worker_params), 1);
	wp->gp = (global_params*)calloc(sizeof(global_params), 1);
	wp->dp = (decode_params*)calloc(sizeof(decode_params), 1);
	wp->dsp = (serialize_params*)calloc(sizeof(serialize_params), 1);
	wp->esp = (serialize_params*)calloc(sizeof(serialize_params), 1);
	wp->qp = (queue_params*)calloc(sizeof(queue_params), 1);
	wp->sp = (storage_params*)calloc(sizeof(storage_params), 1);

	// get init params
	int iRet = init_globalparams(wp->dp);
	if(iRet!=0){
		std::cout << "init_params Error" << std::endl;
		return iRet;
	}

	// connect redis
	iRet = init_preserve(wp->sp);
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

	wp->dsp->width = wp->dp->i_DstWidth;
	wp->dsp->height = wp->dp->i_DstHeight;
	wp->dsp->count = wp->dp->i_DecWidth;
	wp->dsp->channels = 3;
	wp->dsp->pContent = (char*)calloc(sizeof(char), 
		wp->dsp->width * wp->dsp->height * wp->dsp->count * wp->dsp->channels);

	wp->esp->width = wp->dp->i_DstWidth;
	wp->esp->height = wp->dp->i_DstHeight;
	wp->esp->count = wp->dp->i_DecWidth-1;
	wp->esp->channels = 2;
	wp->esp->pContent = (char*)calloc(sizeof(char), 
		wp->esp->width * wp->esp->height * wp->esp->count * wp->esp->channels);

	return iRet;
}




int do_decode_task(decode_params*& dp, serialize_params*& dsp)
{

	int iRet = DoFrameExport((char*)dp->video_file.c_str(), dp->f_video_offset,
        dp->i_DstWidth, dp->i_DstHeight, AV_PIX_FMT_BGR24,
        dp->i_DecWidth, dsp, 256, NULL, 0);

	return iRet;
}

int do_extract_task(serialize_params*& dsp, serialize_params*& esp)
{
	int iRet = CalOptFlow(dsp, esp);
	return iRet;
}

int do_preserve_task(queue_params* qp, serialize_params* sep, storage_params* sp)
{

	int iRet = redis_set(sp->cc, (char*)qp->cur_key.c_str(), (char*)sep);
	if(iRet!=0){
	    std::cout << "do_preserve_task error" << std::endl;
		return -1;
	}


    //put int the train queue
    msg="{'key':"+qp->cur_key+"}";


    if (!pubCli->Publish(trainTopic,msg)){
        std::cout<<"error!"<<std::endl;
    }

}

int do_execute(worker_params* wp)
{
	int iRet = 0;

	// decode and seralize
	iRet = do_decode_task(wp->dp, wp->dsp);
	if(iRet!=0){
		std::cout << "do_decode_task Error" << std::endl;
		return -1;
	}

	// extract and seralize
	iRet = do_extract_task(wp->dsp, wp->esp);
	if(iRet!=0){
		std::cout << "do_decode_task Error" << std::endl;
		return -1;
	}

	//preserve to mem or localize
	iRet = do_preserve_task(wp->qp, wp->esp, wp->sp);
	if(iRet!=0){
		std::cout << "do_preserve_task Error" << std::endl;
		return -1;
	}

	return iRet;
}


int post_execute(worker_params*& wp)
{
	// clear job
	// release resource
	free(wp->dsp->pContent);
	free(wp->dsp);

	return 0;
}


rapidjson::Value getVideoInfo(std::string name)
{
    rapidjson::Value& videoList=gp->video_info["videos"];
    for(rapidjson::SizeType i=0;i<videoList.Size();i++)
    {
        if (videoList[i]["name"].GetString()==name){
                    return videoList["i"];
            }
    }

}







int OnMessage(const evnsq::Message* msg) {

    rapidjson::Document jsonMsg;
    jsonMsg.Parse(msg->body.ToString());
    WorkMsg workMsg=WorkMsg{};
    workMsg.id=jsonMsg["id"].GetInt64();
    workMsg.cmd=jsonMsg["cmd"].GetString();
    workMsg.frames=jsonMsg["frames"].GetInt();
    workMsg.taskTopic=jsonMsg["task_topic"].GetString();
    workMsg.videoName=jsonMsg["video_name"].GetString();

    std::cout<<msg->body.ToString();


    std::string root=gp->video_info["root"].GetString();
    wp->dp->video_file=root+"/"+workMsg.videoName;
    wp->dp->f_video_offset=float(workMsg.offset);
    wp->dp->i_video_in_class_idx=workMsg.id;
    rapidjson::Value val=getVideoInfo(workMsg.videoName);
    wp->qp->cur_key=""+std::to_string(wp->dp->cls_idx)+std::to_string(workMsg.id);


    wp->dp->i_DstWidth=val["width"].GetInt64();
    wp->dp->i_DecWidth=val["width"].GetFloat();
    wp->dp->i_DstHeight=val["height"].GetFloat();


    do_execute(wp);


    return 0;
}


int main(int argc, char*  argv[])
{


    std::string nsqd_tcp_addr,topic_name,redis_addr;
    FILE* fp=fopen("config.json","r");

    char readBuffer[65536];

    rapidjson::FileReadStream is(fp,readBuffer,sizeof(readBuffer));


    rapidjson::Document config;
    config.ParseStream(is);

    fclose(fp);

	int iRet = pre_execute();
	if(iRet!=0){
		std::cout << "pre_execute Error" << std::endl;
		return -1;
	}

    nsqd_tcp_addr = config["nsqd_addr"].GetString();
    topic_name = config["topic_name"].GetString();
    redis_addr = config["redis_addr"].GetString();


    evpp::EventLoop loop;
    evnsq::Consumer client(&loop, topic_name, "default", evnsq::Option());

    //连接nsq的pub
    evpp::EventLoop loop2;
    evnsq::Producer pubCli(&loop2,evnsq::Option());
    pubCli.ConnectToNSQDs(nsqd_tcp_addr);


    client.SetMessageCallback(&OnMessage);


    client.ConnectToNSQDs(nsqd_tcp_addr);


    loop.Run();
    return 0;





//	while(1){
//		do_execute(wp);
//		if(iRet!=0){
//			std::cout << "do_execute Error" << std::endl;
//			return -1;
//		}
//	}
//	iRet = post_execute(wp);
//	if(iRet!=0){
//		std::cout << "post_execute Error" << std::endl;
//		return -1;
//	}
}
