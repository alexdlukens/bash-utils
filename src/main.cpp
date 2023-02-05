#include "main.h"

using boost::asio::ip::tcp;

void sigintHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "\033[?47l\n";
    std::cout << "\033[u";
    
    std::cout.flush();

    exit(0);
}


void display_cur_time(std::chrono::_V2::system_clock::time_point& start_time, int& rownum)
{

    auto cur_time = std::chrono::high_resolution_clock().now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(cur_time - start_time);
    auto hhmmss = std::chrono::hh_mm_ss(cur_time - start_time);
    auto time_t_time = std::chrono::system_clock::to_time_t(cur_time);
    
    
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

    std::cout << "\033[" << rownum++ << ";0H" << cur_time_str << "\033[" << rownum++ << ";0H" \
                    << days_since << " days, " \
                    << hours_since << " hours, " \
                    << min_since << " minutes, " \
                    << sec_since << " seconds since starting";
    std::cout.flush();
}

std::string list_docker_images()
{
    // try
    // {
        // unix:///var/run/docker.sock
        auto sockAddress = std::string("/var/run/docker.sock");

        boost::asio::io_service io_service;
        
        boost::asio::local::stream_protocol::socket socket(io_service);


        socket.connect(sockAddress.c_str());

        boost::asio::streambuf request;
        std::ostream req_stream(&request);

        req_stream << "GET " <<  "http://localhost/v1.42/images/json" << " HTTP/1.0\r\n";  // note that you can change it if you wish to HTTP/1.0
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
            std::cout << header << "\n";
        }

        std::cout << "\n";

        // Write whatever content we already have to output.
        if (response.size() > 0)
        {
            // std::cout << &response;
            std::stringstream tmp_stream;
            tmp_stream << &response;
            std::cout << tmp_stream.str();
            resp_str.append(tmp_stream.str());
        }

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(socket, response,boost::asio::transfer_at_least(1), error))
        {
            // std::cout << &response;
            std::stringstream tmp_stream;
            tmp_stream << &response;
            std::cout << tmp_stream.str();
            resp_str.append(tmp_stream.str());
        }

        if (error != boost::asio::error::eof)
        {
              throw boost::system::system_error(error);
        }
        return resp_str;
    
}

void display_img_list(const nlohmann::json& json_imgs, int& rownum)
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
        std::cout << "\033[" << rownum++ << ";0H" << str;
    }

}

int main()
{

    auto docker_imgs = list_docker_images();
    if(docker_imgs.starts_with("FAILED"))
    {
        std::cout << "received error from list docker images: " << docker_imgs;
        exit(1);
    }
    // std::cout << "output: " << docker_imgs << '\n';

    nlohmann::json json_imgs = nlohmann::json::parse(docker_imgs);
    // std::cout << json_imgs.dump(1);

    // exit(0);
    signal(SIGINT, sigintHandler);

    auto start_time = std::chrono::high_resolution_clock().now();
    std::cout << "\033[?47h";
    std::cout << "\033[s";
    std::cout.flush();
    system("clear");
    std::cout.flush();

    
    while(true)
    {
        system("clear");
        std::cout.flush();
        int rownum = 1;
        display_cur_time(start_time, rownum);
        display_img_list(json_imgs, rownum);
        sleep(1);
    }
    
    
    return 0;
}
