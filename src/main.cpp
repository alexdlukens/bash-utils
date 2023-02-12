#include "main.h"

using boost::asio::ip::tcp;

void sigintHandler(int signum)
{
    endwin();			/* End curses mode		  */
    system("clear");
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "\033[?47l";
    std::cout << "\033[u";
    std::cout.flush();

    exit(0);
}


void display_cur_time(std::chrono::_V2::system_clock::time_point& start_time)
{

    auto cur_time = std::chrono::high_resolution_clock().now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(cur_time - start_time);
    auto hhmmss = std::chrono::hh_mm_ss(cur_time - start_time);
    auto time_t_time = std::chrono::system_clock::to_time_t(cur_time);
    
    
    // std::cout << w.ws_row << ", " << w.ws_col << "\n";
    
    char days_since[3];
    char hours_since[3];
    char min_since[3];
    char sec_since[3];
    std::snprintf(days_since, 3, "%01ld", (hhmmss.hours().count()/24)%100);
    std::snprintf(hours_since, 3, "%02ld", hhmmss.hours().count()%24);
    std::snprintf(min_since, 3, "%02ld", hhmmss.minutes().count()%60);
    std::snprintf(sec_since, 3, "%02ld", hhmmss.seconds().count()%60);

    auto cur_time_str = std::ctime(&time_t_time);
        cur_time_str = strtok(cur_time_str, "\n");

    std::string time_since_string = std::string(days_since) + " days, " \
                    + hours_since + " hours, " \
                    + min_since + " minutes, " \
                    + sec_since + " seconds since starting";
                    
    mvprintw(0,0, "%s", cur_time_str);

}

void display_row_col()
{
    // get terminal size
    int rows, cols;
    getmaxyx(stdscr,rows,cols);

    std::string row_msg = "Rows: " + std::to_string(rows);
    std::string col_msg = "Cols: " + std::to_string(cols);
    // display row, col data
    attron(A_STANDOUT);
    mvprintw(0,cols-row_msg.size()-col_msg.size() - 1, "%s", row_msg.c_str());
    mvprintw(0,cols-col_msg.size(), "%s", col_msg.c_str());
    attroff(A_STANDOUT);
}

std::string call_docker_cmd(std::string url_path)
{
        auto sockAddress = std::string("/var/run/docker.sock");
        boost::asio::io_service io_service;
        boost::asio::local::stream_protocol::socket socket(io_service);
        socket.connect(sockAddress.c_str());

        boost::asio::streambuf request;
        std::ostream req_stream(&request);

        req_stream << "GET " <<  "http://localhost" << url_path << " HTTP/1.0\r\n";  // note that you can change it if you wish to HTTP/1.0
        req_stream << "Host: " << "localhost" << "\r\n";
        req_stream << "Accept: */*\r\n";
        req_stream << "Connection: close\r\n\r\n";
        // http://localhost/v1.42/images/json

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        std::string resp_str;
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            resp_str.append("FAILED: Invalid response\n");
            std::cout << resp_str;
            return resp_str;
        }
        if (status_code != 200)
        {
            resp_str.append("FAILED: Response returned with status code ");
            resp_str.append(std::to_string(status_code));
            resp_str.append("\n");
            std::cout << resp_str;
            return resp_str;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
        {
            // std::cout << header << "\n";
        }

        // Write whatever content we already have to output.
        if (response.size() > 0)
        {
            std::stringstream tmp_stream;
            tmp_stream << &response;
            resp_str.append(tmp_stream.str());
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(socket, response,boost::asio::transfer_at_least(1), error))
        {
            std::stringstream tmp_stream;
            tmp_stream << &response;
            resp_str.append(tmp_stream.str());
        }

        if (error != boost::asio::error::eof)
        {
              throw boost::system::system_error(error);
        }
        return resp_str;

}

void display_img_list(WINDOW* win,const nlohmann::json& json_imgs, int& rownum)
{
    std::set<std::string> repo_tags;
    for(auto elem : json_imgs)
    {
        if(elem.contains("RepoTags"))
        {
            for(auto tag: elem["RepoTags"])
            {
                repo_tags.insert(tag);
            }
        }
    }

    // put a space between previous output and images
    rownum++;
    for(auto& str : repo_tags)
    {
        // std::cout << "\033[" << rownum++ << ";0H" << str;
       mvwprintw(win, rownum++, 1, "%s", str.c_str());
    }
    wrefresh(win);
}

void setup_docker_box(WINDOW* win, char selected_tab)
{
    std::string images_header = "IMAGES";
    std::string containers_header = "CONTAINERS";
    std::string stats_header = "STATS";

    int image_offset = containers_header.size() + 1;
    int stats_offset = image_offset + images_header.size() + 1;

    box(win, 0, 0);

    // setup tabs
    // wattr_on(win, A_STANDOUT, NULL);
    wattr_off(win, A_STANDOUT, NULL);
    mvwprintw(win, 0, 0, "%s", containers_header.c_str());
    mvwprintw(win, 0, image_offset, "%s", images_header.c_str());
    mvwprintw(win, 0, stats_offset, "%s", stats_header.c_str());

    // std::cerr << int(selected_tab) << '\n';
    switch(selected_tab)
    {
        case 'i':
            wattr_on(win, A_STANDOUT, NULL);
            mvwprintw(win, 0, image_offset, "%s", images_header.c_str());
            wattr_off(win, A_STANDOUT, NULL);
            break;
        case 'c':
            wattr_on(win, A_STANDOUT, NULL);
            mvwprintw(win, 0, 0, "%s", containers_header.c_str());
            wattr_off(win, A_STANDOUT, NULL);
            break;
        case 's':
            wattr_on(win, A_STANDOUT, NULL);
            mvwprintw(win, 0, stats_offset, "%s", stats_header.c_str());
            wattr_off(win, A_STANDOUT, NULL);
            break;
    }
    // wattr_off(win, A_STANDOUT, NULL);
    wrefresh(win);
}

void display_img_panel()
{
    
}
void display_cont_panel()
{

}

int main()
{
    
    auto start_time = std::chrono::high_resolution_clock().now();

    // save cursor position and terminal output
    std::cout << "\033[s";
    std::cout.flush();

    std::cout << "\033[?47h";
    std::cout.flush();

    system("clear");
    std::cout.flush();

    // add sigint handler to restore cursor pos and terminal output
    signal(SIGINT, sigintHandler);
    initscr();

    

    auto docker_imgs = call_docker_cmd("/v1.42/images/json");
    if(docker_imgs.starts_with("FAILED"))
    {
        std::cout << "received error from list docker images: " << docker_imgs;
        exit(1);
    }
    // std::cout << "output: " << docker_imgs << '\n';

    nlohmann::json json_imgs = nlohmann::json::parse(docker_imgs);


    int ch;
    struct timespec hund_sleep;
    hund_sleep.tv_nsec = 1000 * 1000 * 1000;
    hund_sleep.tv_sec = 1.0/100;

    // get terminal size before main loop
    int rows, cols;
    getmaxyx(stdscr,rows,cols);

    
    WINDOW *docker_window;
    int startx, starty, width, height;
    startx = 3;
    starty = 2;
    width = cols - 5;
    height = rows - 10;

    docker_window = newwin(height, width, starty, startx);
    
    char selected_tab = '\n';
    
    timeout(50);
    while(true)
    {
        // when terminal is resized, we need to react accordingly
        bool resized = is_term_resized(rows, cols);
        if(resized)
        {
            clear();
            delwin(docker_window);
            getmaxyx(stdscr,rows,cols);
            docker_window = newwin(rows-10, cols-5, starty, startx);
        }
        display_cur_time(start_time);
        display_row_col();
        
        char new_ch = getch();
        std::vector<char> accepted_vals = {'c','i','s'};
        if(std::count(accepted_vals.begin(), accepted_vals.end(), new_ch)) selected_tab = new_ch;
        
        int rownum=1;
        setup_docker_box(docker_window, selected_tab);
        display_img_list(docker_window, json_imgs, rownum);
        refresh();
        char input = getch();
        
    }
	
    endwin();
    return 0;
}
