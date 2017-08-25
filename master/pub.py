import gnsq
import functools


class Publisher():
    def __init__(self, nsqd_addr="127.0.0.1",port=4151, topic="default"):
        """

        :param nsqd_addrs: nsqd地址
        :param topic: topic
        """
        self.nsq = gnsq.Nsqd(address=nsqd_addr, http_port=port)
        self.topic = topic

    def put_msgs(self, msgs):
        """

        :param msgs: msgs to put
        :return: 
        """
        callback = functools.partial(self.finish_pub, msgs=msgs)
        self.nsq.multipublish(topic=self.topic, messages=msgs)

    def finish_pub(self, conn, data, msgs):
        if isinstance(data, Error):
            self.nsq.mpub(self.topic, msgs)
