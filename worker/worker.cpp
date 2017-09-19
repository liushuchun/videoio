/*
Universal Worker
*/

#include "decode/decode_frames.h"
#include "decode/extract_flows.h"
#include "universal_worker.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include <iostream>
#include <string>
#include <fstream>
#include "time.h"

#include <json_func.h>

#define d_DecodeWidth 10
#define d_ImgWidth 256
#define d_ImgHeight 256


//global params

worker_params* wp;
global_params* gp;
using namespace std;
using namespace rapidjson;


int pre_execute(){
    wp = (worker_params*)calloc(sizeof(worker_params), 1);
    wp->gp = (global_params*)calloc(sizeof(global_params), 1);
    wp->dp = (decode_params*)calloc(sizeof(decode_params), 1);
    wp->dsp = (serialize_params*)calloc(sizeof(serialize_params), 1);
    wp->esp = (serialize_params*)calloc(sizeof(serialize_params), 1);
    wp->qp = (queue_params*)calloc(sizeof(queue_params), 1);
    wp->sp = (storage_params*)calloc(sizeof(storage_params), 1);
    // get init params

    int iRet = init_globalparams(wp->dp);
    std::cout<<"init"<<std::endl;
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


    if (!pubCli.Publish(trainTopic,msg)){
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





int main(int argc, char*  argv[])
{
    std::cout<<"first"<<std::endl;

    std::string nsqd_tcp_addr,topic_name,redis_addr;
    FILE* fp=fopen("config.json","r");

    char readBuffer[65536];

    rapidjson::FileReadStream is(fp,readBuffer,sizeof(readBuffer));


    rapidjson::Document config;
    config.ParseStream(is);

    fclose(fp);
    std::cout<<"second"<<std::endl;
    int iRet = pre_execute();
    if(iRet!=0){
        std::cout << "pre_execute Error" << std::endl;
        return -1;
    }
    nsqd_tcp_addr = config["nsqd_addr"].GetString();
    topic_name = config["topic_name"].GetString();
    redis_addr = config["redis_addr"].GetString();

    std::cout<<"here"<<std::endl;
    evpp::EventLoop loop;
    evnsq::Consumer client(&loop, topic_name, "default", evnsq::Option());

    //连接nsq的pub
    evpp::EventLoop loop2;
    pubCli.init(&loop2, evnsq::Option());
    //evnsq::Producer pubCli(&loop2,evnsq::Option());
    pubCli.ConnectToNSQDs(nsqd_tcp_addr);


    client.SetMessageCallback(&OnMessage);


    client.ConnectToNSQDs(nsqd_tcp_addr);


    loop.Run();
    return 0;
}