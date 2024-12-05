#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <map>

using namespace std;
using namespace cv;
using boost::asio::ip::udp;

#define CHUNK_SIZE 64000 // 64 KB

int main() {
    // Initialize UDP socket
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 5000));

    // 設置接收緩衝區大小為 128 KB
    boost::asio::socket_base::receive_buffer_size option(128 * 1024); // 128 KB
    socket.set_option(option);

    
    udp::endpoint sender_endpoint;

    map<int, vector<uchar>> frame_chunks; // Store chunks by frame number
    map<int, int> chunk_count;            // Store the number of received chunks per frame
    map<int, int> total_chunks;           // Store total number of chunks expected per frame

    Mat frame;

    // Create a window for displaying the video
    cv::namedWindow("Receiver", cv::WINDOW_NORMAL);
    resizeWindow("Receiver", 1280, 720); // Static resolution for 16:9 videos

    while (true) {
        vector<uchar> chunk_data(CHUNK_SIZE + 100); // Add extra space for metadata
        boost::system::error_code error;

        // Receive a chunk
        size_t bytes_received = socket.receive_from(boost::asio::buffer(chunk_data), sender_endpoint, 0, error);

        if (error && error != boost::asio::error::message_size) {
            cerr << "Receive failed: " << error.message() << endl;
            break;
        }

        chunk_data.resize(bytes_received);

        // Parse metadata: [frame_number, total_chunks, current_chunk]
        //format 0,7,0,chunk_data
        string metadata1(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);
        string metadata2(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);
        string metadata3(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ','));
        chunk_data.erase(chunk_data.begin(), find(chunk_data.begin(), chunk_data.end(), ',') + 1);
        
        //cout << "Received chunk for frame #" << metadata1 << metadata2 << metadata3 << endl;
        string metadata = metadata1 + "," + metadata2 + "," + metadata3;
        //break;
        istringstream metadata_stream(metadata);
        int frame_number, total, current_chunk;
        char delimiter;

        metadata_stream >> frame_number >> delimiter >> total >> delimiter >> current_chunk;
        cout << "Received chunk for frame #" << frame_number << " (" << current_chunk + 1 << "/" << total << ")" << endl;
        // Store total chunks expected for this frame
        total_chunks[frame_number] = total;

        // Append chunk to the frame buffer
        frame_chunks[frame_number].insert(frame_chunks[frame_number].end(), chunk_data.begin(), chunk_data.end());
        chunk_count[frame_number]++;

        // Check if all chunks for this frame are received
        if (abs (chunk_count[frame_number] -total_chunks[frame_number] ) < 7 && chunk_count[frame_number]>0) {
            // Decode the complete frame
            //sort(frame_chunks[frame_number].begin(), frame_chunks[frame_number].end());
            frame = imdecode(frame_chunks[frame_number], IMREAD_COLOR);

            if (!frame.empty()) {
                // Dynamically adjust the window size to match the frame size (if needed)
                resizeWindow("Receiver", frame.cols, frame.rows);

                // Display the decoded frame
                imshow("Receiver", frame);
            } else {
                cerr << "Error decoding frame #" << frame_number << endl;
            }

            // Clear data for this frame
            frame_chunks.erase(frame_number);
            chunk_count.erase(frame_number);
            total_chunks.erase(frame_number);
        }

        // Break the loop on a key press
        if (waitKey(33) >= 0) break; // ~30 FPS
    }

    return 0;
}
