#include "dbus_vehicle/dbus_interpreter.h"
#include <cmath>

DbusInterpreter::DbusInterpreter(double max_vel, double max_omega, double aim_sens, double deadzone)
    : max_vel(max_vel), max_omega(max_omega), aim_sens(aim_sens), deadzone(deadzone)
{
    // initialize buttons and axes
    active = false;
    ls_x = ls_y = rs_x = rs_y = wheel = 0;
    lsw = rsw = "";

    // initialize move, shoot, aim, and chassis state
    move_ = std::make_shared<Move>();
    shoot_ = std::make_shared<Shoot>();
    aim_ = std::make_shared<Aim>();
    chassis_ = std::make_shared<Chassis>();

    // Last Update Time
    last_update_time_ = rclcpp::Clock().now();
    // initialize update thread
    update_thread = std::thread([this](){
        while (rclcpp::ok())
        {
            update();
            std::this_thread::sleep_for(std::chrono::milliseconds(PERIOD));
        }
    });
}

DbusInterpreter::~DbusInterpreter()
{
    if (update_thread.joinable()) update_thread.join();
}

void DbusInterpreter::input(const operation_interface::msg::DbusControl::SharedPtr msg)
{
    ls_x = msg->ls_x; apply_deadzone(ls_x); // forward is positive
    ls_y = msg->ls_y; apply_deadzone(ls_y); // left is positive
    rs_x = msg->rs_x; apply_deadzone(rs_x); // up is positive
    rs_y = msg->rs_y; apply_deadzone(rs_y); // left is positive
    wheel = msg->wheel; apply_deadzone(wheel);
    lsw = msg->lsw;
    rsw = msg->rsw;
}

void DbusInterpreter::input_key(const operation_interface::msg::KeyMouse::SharedPtr msg)
{
    w_ = msg->w;
    a_ = msg->a;
    s_ = msg->s;
    d_ = msg->d;
    shift_ = msg->shift;
    ctrl_ = msg->ctrl;
    q_ = msg->q;
    e_ = msg->e;
    r_ = msg->r;
    f_ = msg->f;
    g_ = msg->g;
    z_ = msg->z;
    x_ = msg->x;
    c_ = msg->c;
    v_ = msg->v;
    b_ = msg->b;

}

void DbusInterpreter::update()
{
    active = (lsw == "MID");
    if (!active)
    {
        return; // do not update if not active, this prevents yaw and pitch from accumulating in standby
    }

    move_->vel_x = max_vel * ls_x;
    move_->vel_y = max_vel * ls_y;
    aim_->pitch += aim_sens * rs_x * PERIOD / 1000; curb(aim_->pitch, M_PI_4);
    move_->omega = max_omega * wheel;
    aim_->yaw += aim_sens * rs_y * PERIOD / 1000;
    if(wheel > 0.01){      
        chassis_->mode = behavior_interface::msg::Chassis::CHASSIS;
    }else{
        chassis_->mode = behavior_interface::msg::Chassis::CHASSIS_FOLLOW;
    }
    
    if (rsw == "UP")
    {
        shoot_->fric_state = false;
        shoot_->feed_state = false;
        shoot_->feed_speed = 0;
    }
    else if (rsw == "MID")
    {
        shoot_->fric_state = true;
        shoot_->feed_state = false;
        shoot_->feed_speed = 0;
    }
    else if (rsw == "DOWN")
    {
        shoot_->fric_state = true;
        shoot_->feed_state = true;
        shoot_->feed_speed = -4.0;
    }

    if(lsw == "DOWN"){
        // TODO: Implement Keyboard Actions
    }
    
    
}

void DbusInterpreter::apply_deadzone(double &val)
{
    if (val < deadzone && val > -deadzone)
    {
        val = 0;
    }
}

Move::SharedPtr DbusInterpreter::get_move() const
{
    return move_;
}

geometry_msgs::msg::Twist DbusInterpreter::get_move_ros2_control() const
{
    geometry_msgs::msg::Twist move_msg_ros2_control;
    move_msg_ros2_control.linear.x = move_->vel_x;
    move_msg_ros2_control.linear.y = move_->vel_y;
    move_msg_ros2_control.angular.z = move_->omega;
    return move_msg_ros2_control;
}

Shoot::SharedPtr DbusInterpreter::get_shoot() const
{
    return shoot_;
}

Aim::SharedPtr DbusInterpreter::get_aim() const
{
    return aim_;
}

Chassis::SharedPtr DbusInterpreter::get_chassis() const
{
    return chassis_;
}

void DbusInterpreter::curb(double &val, double max_val)
{
    if (val > max_val)
    {
        val = max_val;
    }
    else if (val < -max_val)
    {
        val = -max_val;
    }
}