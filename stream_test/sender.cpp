#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

using namespace std;
using namespace cv;
using boost::asio::ip::udp;

#define CHUNK_SIZE 64000 // 64 KB

int key_pressed() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Usage: sender <video_file> <IP_ADDRESS> <PORT_NUM>" << endl;
        return -1;
    }

    string video_file = argv[1];
    string ip_address = argv[2];
    int port = stoi(argv[3]);

    VideoCapture cap(video_file);

    if (!cap.isOpened()) {
        cerr << "Error: Cannot open video file." << endl;
        return -1;
    }

    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));
    udp::endpoint receiver_endpoint(boost::asio::ip::address::from_string(ip_address), port);

    Mat frame;
    vector<uchar> buffer;
    int frame_number = 0;
    char ack[10];

    while (true) {
        cap >> frame;
        if (frame.empty())
            break;
        if (key_pressed()) {
            char c = getchar(); // Read the key pressed
            if (c == ' ') {     // Check if it's the space key
                printf("Space key pressed. Exiting program.\n");
                break; // Exit the loop
            }
        }
        // 將影片壓縮品質設定為 50%
        vector<int> params = {IMWRITE_JPEG_QUALITY, 50};
        imencode(".jpg", frame, buffer, params);

        size_t total_size = buffer.size();
        size_t num_chunks = (total_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

        for (size_t i = 0; i < num_chunks; ++i) {
            size_t start = i * CHUNK_SIZE;
            size_t end = min(start + CHUNK_SIZE, total_size);
            size_t chunk_size = end - start;

            vector<uchar> chunk_data(buffer.begin() + start, buffer.begin() + start + chunk_size);
            string metadata = to_string(frame_number) + "," + to_string(num_chunks) + "," + to_string(i) + ",";
            chunk_data.insert(chunk_data.begin(), metadata.begin(), metadata.end());

            try {
                //cout << "Sending chunk #" << i << " for frame #" << frame_number << endl;
                socket.send_to(boost::asio::buffer(chunk_data), receiver_endpoint);

                // 等待 ACK
                boost::system::error_code error;
                socket.receive(boost::asio::buffer(ack), 0, error);
                if (error) {
                    cerr << "Error receiving ACK: " << error.message() << endl;
                    return -1;
                }
                if (string(ack).find("ACK") == string::npos) {
                    cerr << "Invalid ACK received:" << string(ack)<< endl;
                    return -1;
                }
            } catch (const boost::system::system_error& e) {
                cerr << "Error sending chunk: " << e.what() << endl;
                return -1;
            }
        }

        frame_number++;
        this_thread::sleep_for(chrono::milliseconds(17)); // Assuming ~60fps

        // Check if space key is pressed
        if (waitKey(1) == 32) { // ASCII code for space key is 32
            cout << "Space key pressed. Terminating sender." << endl;
            break;
        }
    }

    cout << "Streaming complete!" << endl;
    return 0;
}
