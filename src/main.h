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
#include <time.h>
#include <ncurses.h>
// #include <panel.h>
// #include <memory>
#include <algorithm>

void sigintHandler(int signum);
void display_cur_time(std::chrono::_V2::system_clock::time_point& start_time);
std::string call_docker_cmd(std::string url_path);
void display_img_list(WINDOW* win, const nlohmann::json& json_imgs, int& rownum);
void display_row_col();
void setup_docker_box(WINDOW* win, char selected_tab);
void display_img_panel();
void display_cont_panel();
