import os
import sys
import random
import numpy as np
import json
import cv2


def get_video_info(video_file):
    try:
        cap = cv2.VideoCapture(video_file)
        frames = cap.get(cv2.CAP_PROP_FRAME_COUNT)
        fps = cap.get(cv2.CAP_PROP_FPS)
        width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        cap.release()
        return frames, fps, width, height
    except:
        print("Open Video Error")
        return 0, 0, 0, 0


# list folder get classlist, filelist
def list_video_folder(folder_name):
    label_list = []
    file_list = []

    files = os.listdir(folder_name)
    clsidx = 0
    for fs in files:
        sub_folder = os.path.join(folder_name, fs)
        if not os.path.isdir(sub_folder):
            continue
        # get classlist
        label_list.append(fs)
        subfile_list = []
        # get video files from sub folder
        filelist = os.listdir(sub_folder)
        vididx = 0
        folder_json = {"clsidx": clsidx, "clsname": fs, "root": sub_folder, "videos": []}
        clsidx += 1
        for f in filelist:
            full_filename = os.path.join(sub_folder, f)
            if os.path.isdir(full_filename):
                continue
            fext = os.path.splitext(full_filename)[1]
            if fext == ".avi":
                nFrames, fps, width, height = get_video_info(full_filename)
                if nFrames == 0:
                    continue
                folder_json["videos"].append({"idx": vididx, "videoname": f, "nFrames": nFrames, \
                                              "fps": fps, "iWidth": width, "iHeight": height})
                vididx += 1
        file_list.append(folder_json)
    return label_list, file_list


def rand_gen(epoch_length, rand_type=1):
    if rand_type == 1:
        # random 3 float list
        rand_seed = np.random.random((epoch_length, 3))
        return rand_seed


def save_filejson(file_json, output_name):
    fp = open(output_name, "w")
    # for fj in file_json:
    json.dump(file_json, fp, indent=2)
    fp.close()


def save_randseed(rd_seed, output_name):
    np.savetxt(output_name + ".txt", rd_seed)
    # np.save(output_name+".npy",rd_seed)


if __name__ == '__main__':

    lable_list, file_list = list_video_folder(sys.argv[1])
    print(lable_list)
    file_count = 0
    frame_count = 0
    for f in file_list:
        file_count += len(f["videos"])
        for ff in f["videos"]:
            frame_count += ff["nFrames"]
    print(file_count)
    print(frame_count)
    rand_seed = rand_gen(file_count)
    save_filejson(file_list, sys.argv[2])
    save_randseed(rand_seed, sys.argv[3])
