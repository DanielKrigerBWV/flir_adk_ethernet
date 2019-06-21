#include <pluginlib/class_list_macros.h>
#include "flir_boson_ethernet/BaseCameraController.h"

using namespace cv;
using namespace flir_boson_ethernet;

BaseCameraController::BaseCameraController() : _cvImage()
{
}

BaseCameraController::~BaseCameraController()
{
    if(_camera) {
        _camera->closeCamera();
        delete _camera;
    }
}

void BaseCameraController::onInit()
{
    nh = getNodeHandle();
    pnh = getPrivateNodeHandle();
    
    it = std::shared_ptr<image_transport::ImageTransport>(new image_transport::ImageTransport(nh));
    _imagePublisher = it->advertiseCamera("image_raw", 1);
    setupExtraPubSub();

    bool exit = false;

    std::string ip, cameraInfoStr;
    bool grayscale, bit16;

    pnh.param<std::string>("frame_id", frame_id, "boson_camera");
    pnh.param<std::string>("ip_addr", ip, "");
    pnh.param<std::string>("camera_info_url", cameraInfoStr, "");
    pnh.param<bool>("grayscale", grayscale, false);
    pnh.param<bool>("raw_16", bit16, false);

    ROS_INFO("flir_boson_ethernet - Got frame_id: %s.", frame_id.c_str());
    ROS_INFO("flir_boson_ethernet - Got IP: %s.", ip.c_str());
    ROS_INFO("flir_boson_ethernet - Got camera_info_url: %s.", 
        cameraInfoStr.c_str());
    ROS_INFO("flir_boson_ethernet - Got grayscale: %s.", 
        grayscale ? "TRUE" : "FALSE");
    ROS_INFO("flir_boson_ethernet - Got %s Bit.", 
        bit16 ? "16" : "8");

    EthernetCameraInfo info;
    info.ip = ip;
    info.camInfoPath = cameraInfoStr;
    info.width = 800;
    info.height = 600;
    // info.grayscale = grayscale;
    // info.bit16 = bit16;
    auto sys = std::make_shared<SystemWrapper>(
        SystemWrapper(System::GetInstance()));

    _camera = new EthernetCamera(info, sys, nh);

    if (!exit) {
        exit = !_camera->openCamera() || exit;
    }

    if (exit)
    {
        ros::shutdown();
        return;
    }
    
    setupFramePublish();
}

void BaseCameraController::setupExtraPubSub() {
    // do nothing for base class;
}

void BaseCameraController::publishImage(ros::Time timestamp) {
        sensor_msgs::CameraInfoPtr
        ci(new sensor_msgs::CameraInfo(_camera->getCameraInfo()));

    auto thermalMat = _camera->getImageMatrix();
    _cvImage.image = thermalMat;
    _cvImage.encoding = "rgb8";
    _cvImage.header.stamp = timestamp;
    _cvImage.header.seq = _seq;
    _cvImage.header.frame_id = frame_id;

    auto publishedImage = _cvImage.toImageMsg();

    ci->header.stamp = publishedImage->header.stamp;
    _imagePublisher.publish(publishedImage, ci);

    _seq++;
}