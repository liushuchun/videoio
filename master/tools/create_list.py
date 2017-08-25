import json
import multiprocessing
import os

from master.tools import rander

ROOT = "/workspace/data/videos/"

trainval_root = os.path.join(ROOT, "trainval")

test_root = os.path.join(ROOT, "test")

temp_dir = "./temp"

task_count = 0

video_info_path = "info.json"

TRAIN_PATH = "../lsvc2017/lsvc_train.txt"
VAL_PATH = "../lsvc2017/lsvc_val.txt"

label_map = {}

idx_map = {}



def init():
    global trainval_count
    with open(TRAIN_PATH, 'r') as f:
        for line in f:
            split_list = line.strip('\n').split(",")
            label_map[split_list[0]] = split_list[1:]

    with open(VAL_PATH, 'r') as f:
        for line in f:
            split_list = line.strip('\n').split(",")
            label_map[split_list[0]] = split_list[1:]

    trainval_files = os.listdir(trainval_root)
    trainval_count = len(trainval_files)


class Video():
    def __init__(self, name, labels, idx, fps, frame_count, width, height):
        """
        
        :param name: 名字
        :param labels: 标签 []
        :param idx: 随机数 []
        :param fps: fps 
        :param frame_count: frame 总数
        :param width:  宽度
        :param height: 高度
        """
        self.name = name
        self.labels = labels
        self.idx = idx
        self.fps = fps
        self.frame_count = frame_count
        self.width = width
        self.height = height

    def get_map(self):
        return {"name": self.name, "labels": self.labels, "idx": self.idx, "fps": self.fps,
                "frame_count": self.frame_count, "width": self.width, "height": self.height}


class Consumer(multiprocessing.Process):
    def __init__(self, task_queue, result_queue, ID):
        multiprocessing.Process.__init__(self)
        self.task_queue = task_queue
        self.result_queue = result_queue
        self.ID = ID
        self.temp_path = temp_dir
        if not os.path.exists(self.temp_path):
            os.mkdir(self.temp_path)

    def run(self):
        proc_name = self.name
        count=0
        while True:
            file = self.task_queue.get()
            if file is None:
                # 可以退出
                self.task_queue.task_done()
                break

            video_name = file.split('/')[-1]
            instance_name = video_name.split('.')[0]

            # if up_files:
            #    cmd = './qshell qupload {} {}'.format(len(up_files), self.qupload_config_file)
            #    os.system(cmd)

            frames, fps, width, height = rander.get_video_info(file)

            video = Video(video_name, labels=label_map[instance_name],
                          idx=self.get_idx_with_label(labels=label_map[instance_name]), fps=fps, frame_count=frames,
                          width=width,
                          height=height)
            count +=1
            print(count)
            self.task_queue.task_done()
            self.result_queue.put(video)
        return

    def get_idx_with_label(self, labels):
        idxs = []
        global idx_map
        for label in labels:
            if label in idx_map:
                idx_map[label] += 1
            else:
                idx_map[label] = 1
            idx = idx_map[label]
            idxs.append(idx)

        return idxs


class Collector():
    def __init__(self, result_queue):
        super(Collector, self).__init__()
        self.result_queue = result_queue

    def run(self):
        json_map = {}
        json_map["root"] = test_root
        json_map["type"] = "trainval"
        videos = []
        index = 0
        with open(video_info_path, 'a+') as f:
            while True:
                video = self.result_queue.get()
                if video is None:
                    json_map["videos"] = videos
                    json.dump(json_map, f)
                    break
                index += 1
                videos.append(video.get_map())
                if index>=78017:
                    json_map["videos"] = videos
                    json.dump(json_map,f)
                    break
        return


def producer():
    trainval = os.listdir(trainval_root)

    global task_count

    task_count = len(trainval)
    test = os.listdir(test_root)
    for f in trainval:
        yield os.path.join(trainval_root, f)


def main():
    init()
    tasks = multiprocessing.JoinableQueue()
    results = multiprocessing.Queue()

    # 开始 消费
    num_consumers = multiprocessing.cpu_count()
    num_consumers = 1
    consumers = [Consumer(tasks, results, i)
                 for i in range(num_consumers)]
    for c in consumers:
        c.start()

    # 入消息队列

    # 消息收集


    for file in producer():
        tasks.put(file)

    # Wait for all of the tasks to finish
    tasks.join()

    collector = Collector(results)
    collector.run()


def unit_test():
    vs = []
    v1 = Video(name="name", labels=[1, 2], idx=[1, 2], fps=10, frame_count=10, width=10, height=10)
    v2 = Video(name="name", labels=[1, 2], idx=[1, 2], fps=10, frame_count=10, width=10, height=10)
    vs.append(v1.get_map())
    vs.append(v2.get_map())
    print(vs)


if __name__ == "__main__":
    main()
