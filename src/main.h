#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <csignal>
#include <boost/asio.hpp>
#include "json.hpp"
#include <set>

void sigintHandler(int signum);
void display_cur_time(std::chrono::_V2::system_clock::time_point& start_time, int& rownum);
std::string list_docker_images();
void display_img_list(const nlohmann::json& json_imgs, int& rownum);
