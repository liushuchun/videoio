import json
import logging
import os
import sqlite3
from configparser import ConfigParser

import pub as pb

from master import util

logging.basicConfig(level=logging.INFO)

TRAIN_PATH = "lsvc2017/lsvc_train.txt"
VAL_PATH = "lsvc2017/lsvc_val.txt"
INFO_PATH = "tools/info.json"


class Gen():
    """
    Gen used to produce the data into the queue
    """
    Account = 0

    def __init__(self, train_path=TRAIN_PATH, val_path=VAL_PATH, info_path=INFO_PATH):
        """
        
        :param train_path: train_label_path
        :param val_path: : val_label_path
        :param info_path:  video_info list path
        """
        self.video_info = {}
        self.label_info = {}
        self.train_path = train_path
        self.val_path = val_path
        self.info_path = info_path

        self.load_video_info()
        self.count_each_label_num()
        self.load_info_into_db()

        # used for account the data
        Gen.Account += 1

    def load_video_info(self):
        """ load video info list file"""

        INFO_PATH = "/Users/macbook/pyproject/videoio/master/funclib/info.json"
        with open(INFO_PATH) as f:
            self.video_info = json.load(f)

        """
        {"height": 300.0, "frame_count": 7848.0, "idx": [0], "fps": 29.916666666666668, "labels": [424], "width": 400.0, "name": "lsvc129730.avi"}
        """
        # try:
        conn = sqlite3.connect('video.db')
        cursor = conn.cursor()
        cursor.execute(
            '''CREATE TABLE video_info (height FLOAT,frame_count FLOAT,idx INT,fps FLOAT,label INT,width FLOAT,video_name VARCHAR(40),PRIMARY KEY(label,idx))''')
        for video in self.video_info["videos"]:
            for i in range(0, len(video["idx"])):
                cmd = "insert into video_info(height,frame_count,idx,fps,label,width,video_name) values ({},{},{},{},{},{},'{}')".format(
                    video["height"], video["frame_count"], video["idx"][i], video["fps"], video["labels"][i],
                    video["width"], video["name"])
                cursor.execute(cmd)

        cursor.close()
        conn.commit()
        conn.close()

    def count_each_label_num(self):
        """count label num"""
        with open(self.train_path, 'r') as train_f, open(self.val_path) as val_f:
            for f in [train_f, val_f]:
                for line in train_f.readlines():
                    name_label = line.strip("\n").split(",")
                    for label in name_label[1:]:
                        if label in self.label_info:
                            self.label_info[label] = self.label_info[label] + 1
                        else:
                            self.label_info[label] = 1

    def load_info_into_db(self):
        """load data into the database for search"""
        try:
            conn = sqlite3.connect('video.db')
            cursor = conn.cursor()
            cursor.execute('''CREATE TABLE label_info (label VARCHAR(20) PRIMARY KEY,num INTEGER)''')

            for key, item in self.label_info.items():
                cursor.execute('insert into label_info (label,num) values ("{}",{})'.format(key, item))

            cursor.close()
            conn.commit()
        except Exception as e:
            print(e)
        finally:
            conn.close()

    def __iter__(self):
        return self

    def gen(self):

        try:
            conn = sqlite3.connect("video.db")
            cursor = conn.cursor()

            for row in cursor.execute("SELECT * FROM label_info").fetchall():
                msgs = []

                label = row[0]
                sum = row[1]

                yield label, sum

            cursor.close()
            conn.commit()
        except Exception as e:
            print(e)

        finally:
            conn.close()


def main():
    if os.path.exists("video.db"):
        os.remove("video.db")

    parser = ConfigParser()
    parser.read('config.ini')

    nsqd_addr = parser.get("nsqd", "address")
    nsq_port = int(parser.get("nsqd", "port"))
    topic = parser.get("nsqd", "topic")
    offset = parser.get("task", "offset")
    size = parser.get("task", "size")
    flow = parser.get("task", "flow")
    task_name = parser.get("task", "task_name")

    pub = pb.Publisher(nsqd_addr=nsqd_addr, port=nsq_port, topic=topic)

    conn = sqlite3.connect('video.db')
    cursor = conn.cursor()
    index = 0
    for label, sum in Gen().gen():
        msgs = []

        cmd = "select * from video_info where label={} order by idx".format(label)
        for row in cursor.execute(cmd):
            index += 1
            msg = {"id": util.gen_id(),
                   "label": label,
                   "idx": row[2],
                   "cmd": "",
                   "video_name": row[6],
                   "offset": offset,
                   "frames": 10,
                   "task_topic": task_name}
            print(msg)
            msgs.append(str(msg))

        pub.put_msgs(msgs[10])

    print("total input:", index)


if __name__ == "__main__":
    main()
