## Detector with libdarknet.so
Extreme simple .cpp detector to demonstrate use of darknet/YOLO in custom application

Based on darket test_detector and functions

* compiles against libdarknet.so
* makes use of the config files in https://github.com/andrepool/robocup_ml.git
* tested on Ubuntu 20.04
* requires opencv4
* tested with cuda-11.1


## Howto
* compile darknet with LIBSO=1
* sudo cp libdarknet.so /usr/local/lib
* sudo cp include/darknet.h /usr/local/include
* git clone https://github.com/andrepool/robocup_ml.git ~/robocup_ml
* git clone https://github.com/andrepool/detect_with_darknet.git ~/detect_with_darknet
* cd ~/detect_with_darknet
* fileid="12-D5W1UotDhShUEjGuipH3R7uE9YL4zq"
* curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}" > /dev/null
* curl -Lb ./cookie "https://drive.google.com/uc?export=download&confirm=`awk '/download/ {print $NF}' ./cookie`&id=${fileid}" -o yolov4_final.weights
* make


tested with darknet version

```
commit 795474f0ed022fd73af4e70cac79751f672d8af9 (HEAD -> master, origin/master, origin/HEAD)
Merge: 46dea82 070ed01
Author: AlexeyAB <kikots@mail.ru>
Date:   Tue Jun 22 00:31:53 2021 +0300

    Merge branch 'master' of https://github.com/AlexeyAB/darknet
```
