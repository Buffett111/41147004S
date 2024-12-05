#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <map>

using namespace std;
using namespace cv;
using boost::asio::ip::udp;

#define CHUNK_SIZE 64000 // 64 KB

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <PORT_NUM>" << endl;
        return 1;
    }

    int port = stoi(argv[1]);

    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), port));
    

    // 設置接收緩衝區大小為 128 KB
    boost::asio::socket_base::receive_buffer_size option(128 * 1024); // 128 KB
    socket.set_option(option);

    udp::endpoint sender_endpoint;

    map<int, vector<uchar>> frame_chunks;
    map<int, int> chunk_count;
    map<int, int> total_chunks;

    Mat frame;
    cv::namedWindow("Receiver", cv::WINDOW_NORMAL);
    resizeWindow("Receiver", 1280, 720);

    while (true) {
        vector<uchar> chunk_data(CHUNK_SIZE + 100); // Add extra space for metadata
        boost::system::error_code error;

        // 接收封包
        size_t bytes_received = socket.receive_from(boost::asio::buffer(chunk_data), sender_endpoint, 0, error);

        if (error && error != boost::asio::error::message_size) {
            cerr << "Receive failed: " << error.message() << endl;
            break;
        }

        chunk_data.resize(bytes_received);

        // 回傳 ACK
        string ack = "ACK";
        socket.send_to(boost::asio::buffer(ack), sender_endpoint);

        // 解析 metadata
        string metadata1(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);
        string metadata2(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);
        string metadata3(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);

        istringstream metadata_stream(metadata1 + "," + metadata2 + "," + metadata3);
        int frame_number, total, current_chunk;
        char delimiter;

        metadata_stream >> frame_number >> delimiter >> total >> delimiter >> current_chunk;

        //cout << "Received chunk #" << current_chunk << " for frame #" << frame_number << endl;

        total_chunks[frame_number] = total;
        frame_chunks[frame_number].insert(frame_chunks[frame_number].end(), chunk_data.begin(), chunk_data.end());
        chunk_count[frame_number]++;

        if (chunk_count[frame_number] == total_chunks[frame_number]) {
            frame = imdecode(frame_chunks[frame_number], IMREAD_COLOR);

            if (!frame.empty()) {
                resizeWindow("Receiver", frame.cols, frame.rows);
                imshow("Receiver", frame);
            } else {
                cerr << "Error decoding frame #" << frame_number << endl;
            }

            frame_chunks.erase(frame_number);
            chunk_count.erase(frame_number);
            total_chunks.erase(frame_number);
        }

        if (waitKey(16) >= 0) break; // ~60 FPS
    }
    string ack = "END";
    socket.send_to(boost::asio::buffer(ack), sender_endpoint);
    cout << "Video receiver terminated." << endl;
    return 0;
}
