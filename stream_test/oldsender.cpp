#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

using namespace std;
using namespace cv;
using boost::asio::ip::udp;

#define CHUNK_SIZE 64000 // 64 KB

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: sender <video_file>" << endl;
        return -1;
    }

    string video_file = argv[1];
    VideoCapture cap(video_file);

    if (!cap.isOpened()) {
        cerr << "Error: Cannot open video file." << endl;
        return -1;
    }

    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));
    udp::endpoint receiver_endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5000);

    Mat frame;
    vector<uchar> buffer;
    int frame_number = 0;

    while (true) {
        cap >> frame;
        if (frame.empty())
            break;

        imencode(".jpg", frame, buffer);

        size_t total_size = buffer.size();
        size_t num_chunks = (total_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

        for (size_t i = 0; i < num_chunks; ++i) {
            size_t start = i * CHUNK_SIZE;
            size_t end = min(start + CHUNK_SIZE, total_size);
            size_t chunk_size = end - start;

            vector<uchar> chunk_data(buffer.begin() + start, buffer.begin() + start + chunk_size);
            string metadata = to_string(frame_number) + "," + to_string(num_chunks) + "," + to_string(i) + ",";
            chunk_data.insert(chunk_data.begin(), metadata.begin(), metadata.end());

            // Retry logic for sending chunks
            bool sent = false;
            for (int retry = 0; retry < 3; ++retry) {
                try {
                    socket.send_to(boost::asio::buffer(chunk_data), receiver_endpoint);
                    sent = true;
                    break; // Exit retry loop on success
                } catch (...) {
                    cerr << "Retrying to send chunk #" << i << endl;
                }
            }
            if (!sent) {
                cerr << "Failed to send chunk #" << i << " after retries." << endl;
                return -1;
            }
        }


        //cout << "Sent frame #" << frame_number << " with " << num_chunks << " chunks." << endl;
        frame_number++;

        this_thread::sleep_for(chrono::milliseconds(33)); // Assuming ~30fps
    }

    cout << "Streaming complete!" << endl;
    return 0;
}
