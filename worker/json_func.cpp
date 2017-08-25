#include "json_func.h"

int read_videoinfo(char* pVideoInfoFile, rapidjson::Document& video_info)
{
    FILE * pFile = fopen(pVideoInfoFile, "r");
    char buffer[655360];
    if(pFile){//读取文件进行解析成json
        rapidjson::FileReadStream inputStream(pFile, buffer, sizeof(buffer));
        video_info.ParseStream<0>(inputStream);
        fclose(pFile);
        //int iRet = verify_videoinfo(video_info);
        //return iRet;
        return 0;
    }
    else{
      std::cout << "Json Open Error" << std::endl;
      return -1;
    }
    return 0;
}

int verify_videoinfo_cfvid(rapidjson::Document& video_info)
{
  if (video_info.HasParseError()) {
      printf("Json Parse error:%d", video_info.GetParseError()); //打印错误编号
      return -1;
  }
  if (!video_info.HasMember("roots") || !video_info.HasMember("classstats") || !video_info.HasMember("type")){
      printf("Json Member Missing");
      return -1;
  }
  if(!video_info["roots"].IsString() || !video_info["type"].IsString() || !video_info["classstats"].IsArray()){
      printf("Json Member Type Error");
      return -1;
  }

  if (video_info.HasMember("videos")){
    rapidjson::Value videos;
    videos = video_info["videos"];
    if(!videos.IsArray() || videos.Empty()){
      printf("Json-videos Type Error");
      return -1;
    }
    if (!videos.HasMember("height") || !videos.HasMember("width") || !videos.HasMember("idx")
       || !videos.HasMember("frame_count") || !videos.HasMember("labels") || !videos.HasMember("fps") || !videos.HasMember("name")
        ){
      printf("Json-videos Member Missing");
      return -1;
    }
    if(!videos["height"].IsFloat() || !videos["width"].IsFloat() || !videos["idx"].IsArray() || !videos["labels"].IsArray()
      || !videos["frame_count"].IsFloat() || !videos["fps"].IsFloat() || !videos["name"].IsString()){
      printf("Json-videos Member Type Error");
      return -1;
    }
    rapidjson::Value idx;
    idx = videos["idx"][0];
    if(!videos["idx"][0].IsInt() || !videos["labels"][0].IsInt()){
      printf("Json-videos idx/labels Member Type Error");
      return -1;
    }

  }else{
      printf("Json videos Missing");
      return -1;    
  }
  printf("Json verify PASS");
  return 0;

}

int verify_videoinfo_ucf(rapidjson::Document& video_info) 
{
  if (video_info.HasParseError()) {
      printf("Json Parse error:%d", video_info.GetParseError()); //打印错误编号
      return -1;
  }
  if (video_info.HasMember("content")) {
    rapidjson::Value content;
    content = video_info["content"];

    if(content.IsArray() && !content.Empty()){
      printf("content Size: %d\n", content.Size());
      for(unsigned int i=0; i<content.Size(); i++){
        rapidjson::Value folder_json;
        folder_json = content[i];
        if(folder_json.IsObject()){
          if(folder_json.HasMember("clsidx") && folder_json.HasMember("clsname")
           && folder_json.HasMember("root") && folder_json.HasMember("videos")){
            if(folder_json["clsidx"].IsInt() && folder_json["clsname"].IsString()
             && folder_json["root"].IsString() && folder_json["videos"].IsArray() && !folder_json["videos"].Empty()){

              std::cout << folder_json["clsidx"].GetInt() << std::endl;
              std::cout << folder_json["clsname"].GetString() << std::endl;
              std::cout << folder_json["root"].GetString() << std::endl;

              for(unsigned int j=0; j<folder_json["videos"].Size(); j++){
                rapidjson::Value vid;
                vid = folder_json["videos"][j];
                if(vid.IsObject()){
                  if(vid.HasMember("idx") && vid.HasMember("videoname") && vid.HasMember("nFrames")
                   && vid.HasMember("fps") && vid.HasMember("iWidth") && vid.HasMember("iHeight")){
                    if(vid["idx"].IsInt() && vid["videoname"].IsString() && vid["nFrames"].IsFloat()
                       && vid["fps"].IsFloat() && vid["iWidth"].IsFloat() && vid["iHeight"].IsFloat()){
                        std::cout << vid["idx"].GetInt() << std::endl;
                        std::cout << vid["videoname"].GetString() << std::endl;
                        std::cout << vid["nFrames"].GetFloat() << std::endl;
                        std::cout << vid["iWidth"].GetFloat() << std::endl;
                        std::cout << vid["iHeight"].GetFloat() << std::endl;
                        std::cout << vid["fps"].GetFloat() << std::endl;
                    }
                    else{
                      std::cout << "video elemet error" << std::endl;
                      return -1;
                    }
                  }
                  else{
                    std::cout << "video elemet missing" << std::endl;
                    return -1;
                  }
                }
                else{
                  std::cout << "video object error" << std::endl;
                  return -1;
                }

              }
            }
            else{
              std::cout << "folder_json element type error" << std::endl;
              return -1;
            }
          }
          else{
            std::cout << "folder_json element missing" << std::endl;
            return -1;
          }
        }
        else{
          std::cout << "folder_json is Not Object" << std::endl;
          return -1;
        }
      }
    }
    else{
      std::cout << "content is not Array" << std::endl;
      return -1;
    }
  }
  else{
    std::cout << "content missing" << std::endl;
    return -1;
  }

  return 0;
}

