// Copyright 2021 Andre Pool
// SPDX-License-Identifier: Apache-2.0

// Extreme simple application to demonstrate use of darknet/YOLO in custom application

#include <stdio.h>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>

#include <darknet.h>

using namespace cv;

#define BALL_COLOR Scalar(0,255,255)
#define OBSTACLE_COLOR Scalar(255,51,255)
#define HUMAN_COLOR Scalar(0,153,255)
#define OUTSIDE_COLOR Scalar(255,0,0)
#define GOAL_POST_COLOR Scalar(255,255,0)

typedef struct{
	uint32_t class_id;
	float x;
	float y;
	float w;
	float h;
	float confidence;
} object_t;

typedef struct{
	uint32_t frame_id;
	float calcTime; // in seconds
	std::vector<object_t> obj;
} frame_t;

frame_t detection_to_struct(detection *dets, int nboxes, int classes, double calcTime) {
	frame_t objects;

	static uint32_t frame_id = 0;

	objects.frame_id = frame_id++;
	objects.calcTime = float(calcTime / 1000000.0);

	for (int ii = 0; ii < nboxes; ++ii) {
		for (int jj = 0; jj < classes; ++jj) {
			if (dets[ii].prob[jj] > 0.005) { // function get_network_boxes() has already filtered dets by actual threshold
				object_t obj;
				obj.class_id = (uint32_t)jj;
				obj.x = dets[ii].bbox.x;
				obj.y = dets[ii].bbox.y;
				obj.w = dets[ii].bbox.w;
				obj.h = dets[ii].bbox.h;
				obj.confidence = dets[ii].prob[jj];
				objects.obj.push_back(obj);
			}
		}
	}
	return objects;
}

void print_objects(const frame_t objects) {
	for( size_t ii = 0; ii < objects.obj.size(); ii++ ) {
		object_t obj = objects.obj[ii];
		if( ii == 0 ) {
			printf("%8u %3.0f c %1u x %4.2f y %4.2f w %4.2f h %4.2f conf %3.0f%%\n",
					objects.frame_id, objects.calcTime * 1000.0, obj.class_id, obj.x, obj.y, obj.w, obj.h, 100.0*obj.confidence);
		} else {
			printf("             c %1u x %4.2f y %4.2f w %4.2f h %4.2f conf %3.0f%%\n",
					obj.class_id, obj.x, obj.y, obj.w, obj.h, 100.0*obj.confidence);

		}
	}
}

// copied from darknet
extern "C" image mat_to_image(Mat input) {
	int w = input.cols;
	int h = input.rows;
	int c = input.channels();
	image im = make_image(w, h, c);
	unsigned char *data = (unsigned char *)input.data;
	int step = input.step;
	for (int y = 0; y < h; ++y) {
		for (int k = 0; k < c; ++k) {
			for (int x = 0; x < w; ++x) {
				im.data[k*w*h + y*w + x] = data[y*step + x*c + k] / 255.0f;
			}
		}
	}
	return im;
}

void show_objects(const Mat image, const frame_t objects ){
	char buf[256];
	sprintf(buf, "frame %5u    %5.0f ms     fps %4.2f", objects.frame_id, objects.calcTime*1000.0, 1/objects.calcTime);
	putText(image, buf, Point(10, 14), 1, 1, Scalar(255,255,255), 1);

	for( size_t ii = 0; ii < objects.obj.size(); ii++ ) {
		object_t obj = objects.obj[ii];

		Scalar color = {0,0,255};
		if( obj.class_id == 0 ) {
			color = BALL_COLOR;
		} else if( obj.class_id == 1 ) {
			color = OBSTACLE_COLOR;
		} else if( obj.class_id == 2 ) {
			color = HUMAN_COLOR;
		} else if( obj.class_id == 3 ) {
			color = OUTSIDE_COLOR;
		} else if( obj.class_id == 4 ) {
			color = GOAL_POST_COLOR;
		} else  {
			color = {0,0,255};
		}

		int x = (obj.x - obj.w/2.0)* 608;
		int y = (obj.y - obj.h/2.0) * 800;
		int w = obj.w * 608;
		int h = obj.h * 800;

		// rectangle(image, rectScaled, color);
		rectangle(image, Rect(x,y,w,h), color);

		sprintf(buf, "%2.0f%%", 100.0 * obj.confidence);
		putText(image, buf, Point(x, y), 1, 1, Scalar(255,255,255), 1);
	}

	imshow("Display window", image);
	waitKey(1);
}

int main ( ) {
	printf("INFO   stand alone darknet detector\n");

	const int classes = 5; // net->layers[last yolo layer].classes (../robocup_ml/yolov4.cfg)
	network *net;
	const NMS_KIND nms_kind = GREEDY_NMS; // net->layers[last yolo layer].nms_kind (../robocup_ml/yolov4.cfg)
	const float nms_thresh = 0.45; // threshold related to intersection over union, default value demo
	frame_t objects;
	objects.frame_id = 0;
	const float thresh = 0.2; // threshold to accept object, default value run_detector 0.25
	Mat orig_mat;

	// NOTE load_network_custom also performs fuse_conv_batchnorm
	// load_network_custom(cfg_file_name, weights_file_name, clear, batch_size);
	net = load_network_custom((char *) "../robocup_ml/yolov4.cfg", (char *)"./yolov4_final.weights", 1, 1); // clear and batch size 1

	calculate_binary_weights(*net);

	srand(2222222); // set the seed, but is the random function used during detect?

	while( 1 ) {
		// use opencv to read image
		orig_mat = imread("../robocup_ml/20200123/r1/cam0_20200123_210013.jpg", IMREAD_COLOR);

		// convert and resize to darknet image type (int w, int h, int c, float *data) required by darknet
		image origImg = mat_to_image(orig_mat);
		image res_img = resize_image(origImg, net->w, net->h); // resize to 416x416

		// perform the object detection
		double timeStart = get_time_point();
		network_predict_ptr(net, (float *)res_img.data);
		double calcTime = get_time_point() - timeStart;

		// create bounding boxes (with relative size) for the detected objects
		int nboxes = 0;
		// get_network_boxes(net, width, height, threshold, hier_threshold, map, relative, num_boxes, letter);
		// NOTE: hier_threshold not used, custom_get_region_detections
		detection *dets = get_network_boxes(net, 1, 1, thresh, 0, 0, 1, &nboxes, 0);

		// one detected box can be classified for multiple classes, find the best class for each box
		diounms_sort(dets, nboxes, classes, nms_thresh, nms_kind, 0); // https://github.com/Zzh-tju/DIoU-darknet

		// copy the objects to a struct
		objects = detection_to_struct(dets, nboxes, classes, calcTime);
		if( objects.frame_id % 10 == 0 ) {
			print_objects(objects);
		}

		show_objects( orig_mat, objects );

		free_detections(dets, nboxes);
		free_image(origImg);
		free_image(res_img);
	}

	printf("INFO   done with testing\n");

	return 0;
}
