#ifndef METAV_CHASSIS_CONTROLLER__OMNI_WHEEL_KINEMATICS_HPP
#define METAV_CHASSIS_CONTROLLER__OMNI_WHEEL_KINEMATICS_HPP

#include "Eigen/LU"

namespace meta_chassis_controller {

class OmniWheelKinematics {
  public:
    OmniWheelKinematics(const std::vector<double> &omni_wheel_angles,
                        double omni_wheel_distance, double omni_wheel_radius);
    ~OmniWheelKinematics() = default;

    std::vector<double> forward(const std::vector<double> &wheels_vel);
    Eigen::VectorXd inverse(Eigen::Vector3d twist) const;

  private:
    void init();
    std::vector<double> omni_wheel_angles_;
    double omni_wheel_distance_;
    double omni_wheel_radius_;
    Eigen::MatrixXd motion_mat_;
};

} // namespace meta_chassis_controller

#endif // METAV_CHASSIS_CONTROLLER__OMNI_WHEEL_KINEMATICS_HPP