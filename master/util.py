import datetime
import random


def gen_id():
    return datetime.datetime.now().strftime("%s")+str(random.randint(0,100))

