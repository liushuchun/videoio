/*
Universal Worker
*/

#include "decode_frames.h"
#include "extract_flows.h"
#include <iostream>
#include <string>
#include <fstream>
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/video/tracking.hpp"

#define g_VideoInfoJson "CFVID.videos.train.json"
#define d_DecodeWidth 10
#define d_ImgWidth 256
#define d_ImgHeight 256

int main(int argc, char*  argv[])
{
	std::string video_name = "../../lsvc011419.webm";
	serialize_params* dsp = 0;
	serialize_params* esp = 0;

	std::cout << sizeof(char)*d_ImgHeight*d_ImgWidth*3*d_DecodeWidth << std::endl;
	std::cout << sizeof(serialize_params) << std::endl;
	dsp = (serialize_params*)calloc(sizeof(serialize_params), 1);
	dsp->pContent = (char*)calloc(sizeof(char), d_ImgHeight*d_ImgWidth*3*d_DecodeWidth);
	std::cout << sizeof(dsp) << std::endl;
	std::cout << sizeof(dsp->pContent) << std::endl;

	esp = (serialize_params*)calloc(sizeof(serialize_params), 1);
	esp->pContent = (char*)calloc(sizeof(char), d_ImgHeight*d_ImgWidth*3*(d_DecodeWidth-1));

	int cnt = 10;
	do{

		const int64 t1 = cv::getTickCount();

		int iRet = DoFrameExport((char*)video_name.c_str(), 0.3, d_ImgWidth, d_ImgHeight, AV_PIX_FMT_BGR24,
	        d_DecodeWidth, dsp, 256, NULL, 0);

		const int64 t2 = cv::getTickCount();

		iRet = CalOptFlow(dsp, esp);

		const int64 t3 = cv::getTickCount();

		const double timeSec1 = (t2 - t1) / cv::getTickFrequency();
		const double timeSec2 = (t3 - t2) / cv::getTickFrequency();

		std::cout << "cpu decode Time : " << timeSec1 << " sec" << std::endl;
		std::cout << "cpu flow Time : " << timeSec2 << " sec" << std::endl;
		cnt --;
	}while(cnt>0);

	std::cout << "Decode Info::" << std::endl;
	std::cout << "Decode Info:: Height:" << dsp->height << "Width: " << dsp->width << "channels: " << dsp->channels << "counts: " << dsp->count << "length: " << dsp->length << std::endl;
	for(int i=0; i<dsp->count; i++){
		cv::Mat matPre(dsp->height, dsp->width, CV_8UC3, dsp->pContent + i * dsp->channels * dsp->width * dsp->height);
		char fname[20];
		sprintf(fname, "img%02d.png", i);
		cv::imwrite(fname, matPre);
		if(i>0){
			cv::Mat flowx(esp->height, esp->width, CV_8UC1, esp->pContent + (i-1) * esp->channels * esp->width * esp->height);
			char fname[20];
			sprintf(fname, "flowx%02d.png", i-1);
			cv::imwrite(fname, flowx);
			cv::Mat flowy(esp->height, esp->width, CV_8UC1, esp->pContent + (i-1) * esp->channels * esp->width * esp->height);
			sprintf(fname, "flowy%02d.png", i-1);
			cv::imwrite(fname, flowy);

		}
	}

	free(dsp->pContent);
	free(esp->pContent);
	free(dsp);
	free(esp);
	return 0;

}
