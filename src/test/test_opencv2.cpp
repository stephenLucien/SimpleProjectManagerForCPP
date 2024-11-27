#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "opencv2/highgui.hpp"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"

//
#include <opencv2/opencv.hpp>

static int test_opencv2(int reason, void* userdata)
{
    //
    auto path = "/usr/share/icons/gnome/32x32/apps/octopi.png";
    //
    auto img = cv::imread(path);
    //
    if (!img.empty())
    {
        cv::imshow(path, img);
        cv::waitKey();
    }

    return 0;
}

REG_TEST_FUNC(test_opencv2, test_opencv2, NULL)
