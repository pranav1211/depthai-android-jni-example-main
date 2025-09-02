#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include "depthai/depthai.hpp"

// Only needed if you use NN models from assets
void readModelFromAsset(const char* modelPath, std::vector<uint8_t>& modelBuf,
                        JNIEnv* env, jobject obj);

// Minimal helpers
cv::Mat imgframeToCvMat(const std::shared_ptr<dai::ImgFrame>& frame);
jintArray cvToBmpArray(JNIEnv* env, const cv::Mat& img);

#endif
