#include <jni.h>
#include <memory>
#include <thread>
#include <chrono>
#include "depthai/depthai.hpp"
#include <opencv2/opencv.hpp>

using namespace std;

static int currentWidth = 416;
static int currentHeight = 416;
static shared_ptr<dai::Device> device = nullptr;
static shared_ptr<dai::DataOutputQueue> qRgb = nullptr;
static cv::Mat rgbMat;

extern "C" JNIEXPORT jint JNICALL
Java_com_example_depthai_1android_1jni_1example_MainActivity_nativeStartDevice(
        JNIEnv* env, jobject thiz, jstring modelPath, jint width, jint height)
{
    currentWidth = width;
    currentHeight = height;

    int retries = 5;
    while (retries-- > 0) {
        try {
            dai::Pipeline pipeline;
            auto camRgb = pipeline.create<dai::node::ColorCamera>();
            auto xoutRgb = pipeline.create<dai::node::XLinkOut>();
            xoutRgb->setStreamName("rgb");
            camRgb->setPreviewSize(width, height);
            camRgb->setInterleaved(false);
            camRgb->setColorOrder(dai::ColorCameraProperties::ColorOrder::BGR);
            camRgb->preview.link(xoutRgb->input);

            device = make_shared<dai::Device>(pipeline);
            qRgb = device->getOutputQueue("rgb", 8, false);
            rgbMat = cv::Mat(height, width, CV_8UC3);

            return 0;
        } catch (const std::exception& e) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    return -1;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_example_depthai_1android_1jni_1example_MainActivity_imageFromJNI(
        JNIEnv* env, jobject thiz)
{
    if (!qRgb) return NULL;

    auto frame = qRgb->tryGet<dai::ImgFrame>();
    if (!frame) return NULL;

    rgbMat = cv::Mat(frame->getHeight(), frame->getWidth(), CV_8UC3, const_cast<uint8_t*>(frame->getData().data()));
    jintArray result = env->NewIntArray(currentWidth * currentHeight);
    if (!result) return NULL;
    jint* pixels = env->GetIntArrayElements(result, nullptr);

    for (int y = 0; y < currentHeight; y++) {
        for (int x = 0; x < currentWidth; x++) {
            cv::Vec3b color = rgbMat.at<cv::Vec3b>(y, x);
            pixels[y * currentWidth + x] = (0xFF << 24) | (color[2] << 16) | (color[1] << 8) | color[0];
        }
    }
    env->ReleaseIntArrayElements(result, pixels, 0);
    return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_depthai_1android_1jni_1example_MainActivity_nativeStopDevice(
        JNIEnv* env, jobject thiz)
{
    qRgb.reset();
    device.reset();
}
