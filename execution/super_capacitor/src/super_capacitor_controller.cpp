#include <rclcpp/rclcpp.hpp>
#include <linux/can.h>
#include <memory>
#include <string>
#include <pluginlib/class_loader.hpp>
#include <chrono>
#include <unordered_map>

#include "super_capacitor_base.h"
#include "super_capacitor_controller.h"
#include "device_interface/msg/super_capacitor_goal.hpp"
#include "device_interface/msg/super_capacitor_state.hpp"
#include "can_driver/can_driver.hpp"


SuperCapacitorController::SuperCapacitorController() : Node("SuperCapacitorController")
{

    // get parameters
    std::string can_interface = this->declare_parameter("can_interface", std::string("can0"));

    // create subscriptions
    goal_sub_ = this->create_subscription<device_interface::msg::SuperCapacitorGoal>(
        "super_capacitor_goal", 10, [this](const device_interface::msg::SuperCapacitorGoal::SharedPtr msg){
            goal_sub_callback(msg);
        });

    // create a timer for publishing super capacitor state
    state_pub_ = this->create_publisher<device_interface::msg::SuperCapacitorState>("super_capacitor_state", 10);
    pub_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100), [this](){
            pub_state();
        });

    // complete initialization
    tx_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100), [this](){
            send_command();
        });


    pluginlib::ClassLoader<SuperCapacitorBase> capacitor_loader("super_capacitor_base","super_capacitor_base::SuperCapacitorBase");

    try
    {
        capacitor_ = capacitor_loader.createSharedInstance("SuperCapacitorBase");
    }
    catch(pluginlib::PluginlibException& ex)
    {
        printf("The plugin failed to load for some reason. Error: %s\n", ex.what());
    }
    RCLCPP_INFO(this->get_logger(), "SuperCapacitorController initialized");
}
std::shared_ptr<SuperCapacitorBase> capacitor_; 
double target_power_, referee_power_;
void SuperCpacitorController::goal_sub_callback(const device_interface::msg::SuperCapacitorGoal::SharedPtr msg){
    // RCLCPP_INFO(this->get_logger(), "Received super capacitor goal");
    target_power_ = msg->target_power;
    referee_power_ = msg->referee_power;
    capacitor_->set_target_power(target_power_);
    capacitor_->set_referee_power(referee_power_);
}

void SuperCapacitorController::send_command() {
    capacitor_->tx();
}

void SuperCapacitorController:pub_state() {
    std::unordered_map<std::string, double> state = capacitor_->get_state();
    device_interface::msg::SuperCapacitorState msg;
    if state.find("input_voltage")  msg.input_voltage = state.at("input_voltage");  
    if state.find("capacitor_voltage") msg.capacitor_voltage = state.at("capacitor_voltage");   
    if state.find("input_current") msg.input_current = state.at("input_current");
    if state.find("target_power") msg.target_power = state.at("target_power");
    if state.find("max_discharge_power") msg.max_discharge_power = state.at("max_discharge_power");
    if state.find("base_power") msg.base_power = state.at("base_power");
    if state.find("cap_energy_percentage") msg.cap_energy_percentage = state.at("cap_energy_percentage");
    if state.find("cap_state") msg.cap_state = state.at("cap_state");
    state_pub_->publish(msg);
}