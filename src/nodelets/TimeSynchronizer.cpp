#include <pluginlib/class_list_macros.h>
#include "flir_boson_ethernet/TimeSynchronizer.h"

PLUGINLIB_EXPORT_CLASS(flir_boson_ethernet::TimeSynchronizer, nodelet::Nodelet)

using namespace flir_boson_ethernet;

TimeSynchronizer::TimeSynchronizer() {}
TimeSynchronizer::~TimeSynchronizer() {}

void TimeSynchronizer::onInit() {
    _nh = getNodeHandle();
    _pnh = getPrivateNodeHandle();

    _publisher = _nh.advertise<std_msgs::Time>("image_sync", 1);

    _pnh.param<float>("frame_rate", _frameRate, 60.0);
    NODELET_INFO("flir_boson_ethernet - Got frame rate: %f.", _frameRate);

    _timer = _nh.createTimer(ros::Duration(1.0 / _frameRate),
                    boost::bind(&TimeSynchronizer::publishTime, this, _1));
}

void TimeSynchronizer::publishTime(const ros::TimerEvent& evt) {
    std_msgs::Time timeToPublish;
    timeToPublish.data = ros::Time::now();
    _publisher.publish(timeToPublish);
}