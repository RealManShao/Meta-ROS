#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <linux/can.h>
#include <linux/can/raw.h>
// #include <rclcpp/parameter.hpp>
// #include <rclcpp/subscription.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

class CanDriver
{
private:
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

public:
    CanDriver()
    {
        s = socket(PF_CAN, SOCK_RAW, CAN_RAW); // open the CAN socket

        strcpy(ifr.ifr_name, "can0");
        ioctl(s, SIOCGIFINDEX, &ifr); // set the interface name

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex; // set the socket address

        int bind_result = bind(s, (struct sockaddr *)&addr, sizeof(addr)); // bind the socket to the CAN interface
        if (bind_result == -1) {
            perror("Error binding socket to CAN interface");
        }
    }

    ~CanDriver()
    {
        // close the CAN socket
        close(s);
    }

    void send_frame(can_frame frame)
    {
        write(s, &frame, sizeof(frame));
    }
};

#endif // CAN_DRIVER_H