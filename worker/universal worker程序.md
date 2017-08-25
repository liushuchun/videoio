# universal worker程序
1. pre_execute
2. do_execute
3. post_execute


## pre_execute 函数
* 初始化全局参数
1. 读取Video Info Json
2. 获取Videos数目
3. 获取Videos的分类数
4. 获取Videos的每个类别的个数
5. 设置解码后的长宽，解码的帧数，视频的存放路径

* 初始化redis
* 初始化队列 - TODO
* 创建解码和光流提取的内存

## do_execute 函数
* 从队列获取job参数 - TODO
* 执行解码
* 执行光流提取
* 保存到redis



## post_execute 函数
释放资源