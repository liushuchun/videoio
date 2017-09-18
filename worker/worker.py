#/encoding:utf-8
import nsq
import tornado.ioloop
import time
import json
import ConfigParser
import sqlite3
import os

import multiprocessing





class Collector(multiprocessing.Process):

    def __init__(self,msg_queue):
        super(Collector,self).__init__()
        self.msg_queue=msg_queue


    def run(self):
        while True:
            msg=self.msg_queue.get()
            if msg is None:
                break


        return










class Worker:
    def __init__(self,msq_queue):
        self.msg_queue=msq_queue
        self.cfg = ConfigParser.SafeConfigParser()
        self.cfg.read("config.ini")
        self.root_path = self.cfg.get("main", "root")
        self.video_info_path = self.cfg.get("main", "video_info")
        self.video_db = self.cfg.get("main", "video_db")
        self.nsqd_addr = self.cfg.get("main", "nsqd_addr")
        self.topic = self.cfg.get("main", "topic")

    def get_cls_num(self, label):
        """
        
        :param label: 某个label的总数量
        :return:  int 
        """
        try:
            conn = sqlite3.connect("")
            cursor = conn.cursor()
            row = cursor.execute("select * from label_info where label=" + label).fetchone()
            sum = row[1]
            cursor.close()
            return sum


        except Exception as  e:
            print(e)
        finally:
            conn.close()

        return -1

    def handler(self,message):
        body = message.body.replace("'", '"')

        print(body)
        data = json.loads(body)

        id = data["id"]
        label = data["label"]
        idx = data["idx"]
        cmd = data["cmd"]
        video_name = data["video_name"]
        offset = data["offset"]
        frames = data["frames"]
        task_topic = data["task_topic"]

        file_path=os.path.join(self.root_path,video_name)

        data["file_path"]=file_path






        return True












    def run(self):
        self.r=nsq.Reader(message_handler=self.handler,nsqd_tcp_addresses=[self.nsqd_addr],topic=self.topic,channel="default",lookupd_poll_interval=15)
        nsq.run()

if __name__ == "__main__":
    results = multiprocessing.Queue()

    worker = Worker(results)
    worker.run()
