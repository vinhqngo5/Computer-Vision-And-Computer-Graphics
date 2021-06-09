#include "CV/camera_pose.h"
#include "CV/utils.h"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
using namespace cv;


CameraPose ourCameraPose("camera.yml");
VideoCapture cap(0);

int main()
{
    Mat frame;

    // Create Project Matrix
    createProjectionMatrix(0.01f, 1000.0f);

}