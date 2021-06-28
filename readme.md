## Extreme simple .cpp application to demonstrate use of darknet/YOLO in custom application

TODO have to find a way provide the large yolov4_final.weights file

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
cd detect_with_darknet
make


tested with darknet version

```
commit 795474f0ed022fd73af4e70cac79751f672d8af9 (HEAD -> master, origin/master, origin/HEAD)
Merge: 46dea82 070ed01
Author: AlexeyAB <kikots@mail.ru>
Date:   Tue Jun 22 00:31:53 2021 +0300

    Merge branch 'master' of https://github.com/AlexeyAB/darknet
```
