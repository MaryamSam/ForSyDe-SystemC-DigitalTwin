#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>
#include <sstream>

#define INPUT_PIPE "input_pipe"
#define OUTPUT_PIPE "output_pipe"
#define BUFFER_SIZE 1024

int main() {
    // Create named pipes if they do not exist
    mkfifo(INPUT_PIPE, 0666);
    mkfifo(OUTPUT_PIPE, 0666);

    // Open the input pipe
    int inp_pipe_fd = open(INPUT_PIPE, O_RDONLY);
    if (inp_pipe_fd < 0) {
        perror("Failed to open input pipe");
        return EXIT_FAILURE;
    }

    // Open the output pipe
    int out_pipe_fd = open(OUTPUT_PIPE, O_WRONLY);
    if (out_pipe_fd < 0) {
        perror("Failed to open output pipe");
        close(inp_pipe_fd);
        return EXIT_FAILURE;
    }

    char buf[BUFFER_SIZE];
    ssize_t bytes_read;
    std::string leftover;

    // Loop to read and process data
    while (true) {
        bytes_read = read(inp_pipe_fd, buf, sizeof(buf) - 1);

        if (bytes_read > 0) {
            buf[bytes_read] = '\0'; // Null-terminate the string
            leftover += buf; // Append new data to the leftover buffer

            std::istringstream stream(leftover);
            std::string line;
            int output_value = 0;
            while (std::getline(stream, line)) {
                if (stream.eof()) {
                    leftover = line; // Store the last incomplete line
                    break;
                }

                try {
                    // Convert string to integer and increment
                    int input_value = std::stoi(line);
                    output_value += input_value;

                    // Print the read value and the output
                    std::cout << "Read from input pipe: " << input_value << ", Output: " << output_value << std::endl;

                    // Write the output value to the output pipe
                    std::string output_str = std::to_string(output_value) + "\n";
                    if (write(out_pipe_fd, output_str.c_str(), output_str.size()) < 0) {
                        perror("Failed to write to output pipe");
                        break;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid input received (" << line << ")" << std::endl;
                }
            }
        } else if (bytes_read == 0) {
            std::cout << "End of input pipe reached." << std::endl;
            break;
        } else {
            perror("Error reading from input pipe");
            break;
        }
    }

    // Close the pipes
    close(inp_pipe_fd);
    close(out_pipe_fd);

    std::cout << "Pipes closed. Program finished successfully." << std::endl;
    return 0;
}
