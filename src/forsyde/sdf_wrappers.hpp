#ifndef SDF_WRAPPERS_HPP
#define SDF_WRAPPERS_HPP

#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include "sdf_process.hpp"  
#include "abssemantics.hpp" 

namespace ForSyDe {
namespace SDF {

//! Process constructor for a socket wrapper with variable token rates for one input and one output
/*! 
 * This class implements a socket wrapper process in the SDF model of computation.
 * It communicates with an external simulator via a TCP socket.
 * 
 * In each evaluation cycle, the process performs the following:
 * - If offset <= 0, it reads a specified number (i1_tokens) of tokens from its input port,
 *   concatenates them into a single string (with tokens separated by spaces), and sends this
 *   string through the socket.
 * - If offset >= 0, it reads a line of data from the socket, parses it into a specified number
 *   (o1_tokens) of tokens, and writes these tokens to the output port.
 * 
 * The 'offset' parameter determines the synchronization behavior between sending and receiving.
 * The process is parameterized by the output data type (T0) and input data type (T1).
 */


template <typename T0, typename T1>
class socketwrap : public sdf_process {
public:
    SDF_in<T1> iport1;  ///< Input port
    SDF_out<T0> oport1; ///< Output port

    /// Constructor
    //! The constructor requires the module name
    /*! It creates an SC_THREAD which reads i1_tokens number of data from its input port,
     * provides it to the external model, collects the o1_tokens numer of produced outputs
     * and writes them using the output port
     */

    socketwrap(sc_module_name _name,       // Module name (process)
               const int& offset,          // Determines if data is sent first (offset <= 0) or received from the socket (offset >= 0)
               const std::string& ip_addr, // IP address of the external model
               const int& port,            // Port number of the external model
               unsigned int i1_tokens,     // Number of tokens to consume from the input port per cycle
               unsigned int o1_tokens)     // Number of tokens to produce on the output port per cycle
        : sdf_process(_name), iport1("iport1"), oport1("oport1"),
          offset(offset), ip_addr(ip_addr), port(port),
          i1_tokens(i1_tokens), o1_tokens(o1_tokens)
    {

#ifdef FORSYDE_INTROSPECTION
        arg_vec.push_back(std::make_tuple("ip_addr", ip_addr));
        arg_vec.push_back(std::make_tuple("port", std::to_string(port)));
        arg_vec.push_back(std::make_tuple("offset", std::to_string(offset)));
        arg_vec.push_back(std::make_tuple("i1_tokens", std::to_string(i1_tokens)));
        arg_vec.push_back(std::make_tuple("o1_tokens", std::to_string(o1_tokens)));
#endif

    }

private:
    int offset;
    std::string ip_addr;
    int port;
    int sockfd;
    struct sockaddr_in serv_addr;
    bool initiated;
    unsigned int i1_tokens, o1_tokens;  // Number of tokens to consume and produce per cycle

    // Vectors to hold tokens
    std::vector<abst_ext<T1>> i1vals;
    std::vector<T0> o1vals;

    // String streams for token conversion
    std::ostringstream ival_str;  ///< Used to convert input tokens to string
    std::istringstream oval_str;  ///< Used to convert received string into output tokens

    /// init phase: Allocate memory, resize vectors, and establish the socket connection.
    void init() {
        // Resize vectors based on token counts.
        i1vals.resize(i1_tokens);
        o1vals.resize(o1_tokens);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            SC_REPORT_ERROR(name(), "Error opening socket");

        struct hostent* server = gethostbyname(ip_addr.c_str());
        if (server == NULL)
            SC_REPORT_ERROR(name(), "No such host");

        bzero((char*)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy((char*)&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            SC_REPORT_ERROR(name(), "Error connecting");
        else
        printf("\n>>>>>Connected<<<<<\n");    

        initiated = false;
    }

    /// prep phase: If offset <= 0, read the specified number of tokens from the input port and send them over the socket.
    void prep() {
        if (offset <= 0) {
            // Read i1_tokens tokens from the input port.
            for (unsigned int i = 0; i < i1_tokens; i++) {
                i1vals[i] = iport1.read();
            }
            // Convert the tokens to a string (tokens separated by spaces)
            ival_str.str("");
            for (unsigned int i = 0; i < i1_tokens; i++) {
                ival_str << unsafe_from_abst_ext(i1vals[i]);
                if (i < i1_tokens - 1)
                    ival_str << " ";
            }
            std::string data = ival_str.str();

            // Write the entire string to the socket.
            size_t total_written = 0;
            while (total_written < data.size()) {
                // printf("writing in socket...\n");
                ssize_t n = write(sockfd, data.c_str() + total_written, data.size() - total_written);
                // std:: cout << "sent : " <<data << std::endl;
                if (n < 0)
                    SC_REPORT_ERROR(name(), "Error writing to socket");
                total_written += n;
            }
        }
    }

    /// exec phase: No internal processing is performed in this example.
    void exec() {
        // No additional processing in this phase.
    }

    /// prod phase: If offset >= 0, read a line from the socket, parse it into o1_tokens tokens, and write them to the output port.
    void prod() {
        if (offset >= 0) {
            // Read data from the socket into a buffer.
            char buf[256];
            int n = 0;
            int counter = 0;
            while (counter < 1) {
                ssize_t count = read(sockfd, buf + n, 256 - n);
                counter ++;
                if (count < 0)
                    SC_REPORT_ERROR(name(), "Error reading from socket");
                else if (count == 0)
                    break;
                else
                {
                    // printf("reading from socket...\n"); 
                }
                n += count;
                // Assume the end of data is indicated by a newline or when buffer is full.
                if (buf[n - 1] == ' ' || n >= 256)
                    break;
            }
            std::string data(buf, n);
            // std::cout << "Received data : " << data << std::endl;
            oval_str.clear();
            oval_str.str(data);

            // Parse the string into o1_tokens tokens.
            for (unsigned int i = 0; i < o1_tokens; i++) {
                oval_str >> o1vals[i];
            }
            // Write the vector of output tokens to the output port.
            write_vec_multiport(oport1, o1vals);
            // std::cout <<"oport1 :" << oport1 << std::endl;
            // std::cout <<"o1vals :" << o1vals << std::endl;

        }
        // Adjust the offset for synchronization with the external model.
        if (offset < 0)
            offset++;
        else if (offset > 0)
            offset--;
    }

    /// clean phase: Close the socket and free allocated resources.
    void clean() {
        printf("done with the soket\n");
        close(sockfd);
    }

#ifdef FORSYDE_INTROSPECTION
    void bindInfo() {
        boundInChans.resize(1);
        boundInChans[0].port = &iport1;
        boundOutChans.resize(1);
        boundOutChans[0].port = &oport1;
    }
#endif
virtual std::string forsyde_kind() const override {
    return "SDF::socketwrap";
}


};


//! Process constructor for a socket wrapper with variable token rates for two input ports (of different types) and one output port
/*! 
 * This class implements a socket wrapper process in the SDF model of computation.
 * It communicates with an external simulator via a TCP socket.
 * 
 * In each evaluation cycle, the process performs the following:
 * - If offset <= 0, it reads a specified number (i1_tokens) of tokens from its first input port
 *   (of type T1) and a specified number (i2_tokens) of tokens from its second input port
 *   (of type T2), concatenates them into a single string (with tokens within each group separated by spaces
 *   and the two groups separated by " | "), and sends this string through the socket.
 * - If offset >= 0, it reads a line of data from the socket, parses it into a specified number (o1_tokens)
 *   of tokens, and writes these tokens to the output port.
 * 
 * The 'offset' parameter determines the synchronization behavior between sending and receiving.
 * The process is parameterized by the output data type (T0), the type of tokens from the first input (T1),
 * and the type of tokens from the second input (T2).
 */
template <typename T0, typename T1, typename T2>
class socketwrap2 : public sdf_process {
public:
    SDF_in<T1> iport1;  ///< First input port (type T1)
    SDF_in<T2> iport2;  ///< Second input port (type T2)
    SDF_out<T0> oport1; ///< Output port

    /// Constructor
    //! The constructor requires the module name and connection/token parameters.
    /*! It creates an SC_THREAD which reads i1_tokens tokens from the first input port and
     * i2_tokens tokens from the second input port, sends the concatenated tokens to the external model,
     * collects the produced outputs (o1_tokens tokens) from the external model, and writes them using the output port.
     */
    
    socketwrap2(sc_module_name _name,       // Module name (process)
               const int& offset,          // Determines if data is sent first (offset <= 0) or received from the socket (offset >= 0)
               const std::string& ip_addr, // IP address of the external model
               const int& port,            // Port number of the external model
               unsigned int i1_tokens,     // Number of tokens to consume from the first input port per cycle
               unsigned int i2_tokens,     // Number of tokens to consume from the second input port per cycle
               unsigned int o1_tokens)     // Number of tokens to produce on the output port per cycle

        : sdf_process(_name), iport1("iport1"), iport2("iport2"), oport1("oport1"),
          offset(offset), ip_addr(ip_addr), port(port),
          i1_tokens(i1_tokens), i2_tokens(i2_tokens), o1_tokens(o1_tokens)
    {
#ifdef FORSYDE_INTROSPECTION
        arg_vec.push_back(std::make_tuple("ip_addr", ip_addr));
        arg_vec.push_back(std::make_tuple("port", std::to_string(port)));
        arg_vec.push_back(std::make_tuple("offset", std::to_string(offset)));
        arg_vec.push_back(std::make_tuple("i1_tokens", std::to_string(i1_tokens)));
        arg_vec.push_back(std::make_tuple("i2_tokens", std::to_string(i2_tokens)));
        arg_vec.push_back(std::make_tuple("o1_tokens", std::to_string(o1_tokens)));
#endif
    }

private:
    int offset;
    std::string ip_addr;
    int port;
    int sockfd;
    struct sockaddr_in serv_addr;
    bool initiated;
    unsigned int i1_tokens, i2_tokens, o1_tokens;  // Token counts for first input, second input, and output

    // Vectors to hold tokens from the two input ports and output tokens
    std::vector<abst_ext<T1>> i1vals;
    std::vector<abst_ext<T2>> i2vals;
    std::vector<T0> o1vals;

    // String streams for token conversion
    std::ostringstream ival_str;  ///< Used to convert input tokens to a string
    std::istringstream oval_str;  ///< Used to convert received string into output tokens

    /// init phase: Allocate memory, resize vectors, and establish the socket connection.
    void init() {
        // Resize vectors based on token counts.
        i1vals.resize(i1_tokens);
        i2vals.resize(i2_tokens);
        o1vals.resize(o1_tokens);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            SC_REPORT_ERROR(name(), "Error opening socket");

        struct hostent* server = gethostbyname(ip_addr.c_str());
        if (server == NULL)
            SC_REPORT_ERROR(name(), "No such host");

        bzero((char*)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy((char*)&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            SC_REPORT_ERROR(name(), "Error connecting");
        else
            printf("\n>>>>>Connected<<<<<\n");

        initiated = false;
    }

    /// prep phase: If offset <= 0, read tokens from both input ports and send them over the socket.
    void prep() {
        if (offset <= 0) {
            // Read i1_tokens tokens from the first input port.
            for (unsigned int i = 0; i < i1_tokens; i++) {
                i1vals[i] = iport1.read();
            }
            // Read i2_tokens tokens from the second input port.
            for (unsigned int i = 0; i < i2_tokens; i++) {
                i2vals[i] = iport2.read();
            }
            // Convert the tokens to a string.
            ival_str.str("");
            // Convert tokens from the first input port (separated by spaces).
            for (unsigned int i = 0; i < i1_tokens; i++) {
                ival_str << unsafe_from_abst_ext(i1vals[i]);
                if (i < i1_tokens - 1)
                    ival_str << " ";
            }
            // Insert a delimiter between the two input groups.
            ival_str << " | ";
            // Convert tokens from the second input port.
            for (unsigned int i = 0; i < i2_tokens; i++) {
                ival_str << unsafe_from_abst_ext(i2vals[i]);
                if (i < i2_tokens - 1)
                    ival_str << " ";
            }
            std::string data = ival_str.str();

            // Write the entire string to the socket.
            size_t total_written = 0;
 
            // printf("writing in socket...\n");
            while (total_written < data.size()) {
                
                ssize_t n = write(sockfd, data.c_str() + total_written, data.size() - total_written);
                // std:: cout << "sent : " << data << std::endl;
                if (n < 0)
                    SC_REPORT_ERROR(name(), "Error writing to socket");
 
                total_written += n;
            }
        }
    }

    /// exec phase: No internal processing is performed in this example.
    void exec() {
        // No additional processing in this phase.
    }

    /// prod phase: If offset >= 0, read a line from the socket, parse it into o1_tokens tokens, and write them to the output port.
    void prod() {
        if (offset >= 0) {
            // Read data from the socket into a buffer.
            char buf[256];
            int n = 0;
            int counter = 0;
            while (counter == 0) {
                counter++;
                ssize_t count = read(sockfd, buf + n, 256 - n);
                if (count < 0)
                    SC_REPORT_ERROR(name(), "Error reading from socket");
                else if (count == 0)
                    break;
                n += count;
                // Assume the end of data is indicated by a newline or when buffer is full.
                if (buf[n - 1] == '\n' || n >= 256)
                    break;
                else
                {
                    // printf("reading from socket...\n"); 
                }       
            }
            std::string data(buf, n);
            // std::cout << "Received data : " << data << std::endl;
            oval_str.clear();
            oval_str.str(data);

            // Parse the string into o1_tokens tokens.
            for (unsigned int i = 0; i < o1_tokens; i++) {
                oval_str >> o1vals[i];
            }
            // Write the vector of output tokens to the output port.
            write_vec_multiport(oport1, o1vals);
            // std::cout <<"oport1 :" << oport1 << std::endl;
            // std::cout <<"o1vals :" << o1vals << std::endl;
        }
        // Adjust the offset for synchronization with the external model.
        if (offset < 0)
            offset++;
        else if (offset > 0)
            offset--;
    }

    /// clean phase: Close the socket and free allocated resources.
    void clean() {
        // printf("done with soket \n");
        close(sockfd);
    }

#ifdef FORSYDE_INTROSPECTION
    void bindInfo() {
        // Bind both input ports.
        boundInChans.resize(2);
        boundInChans[0].port = &iport1;
        boundInChans[1].port = &iport2;
        // Bind the output port.
        boundOutChans.resize(1);
        boundOutChans[0].port = &oport1;
    }
#endif

    virtual std::string forsyde_kind() const override {
        return "SDF::socketwrap2";
    }
};

//! Helper function to construct a socketwrap process
/*! This function is used to construct a socket wrapper process (SystemC
 * module) and connect its input and output signals.
 * It provides a more functional style definition of a ForSyDe process.
 * It also removes bilerplate code by using type-inference feature of
 * C++ and automatic binding to the input and output FIFOs.
 */
template <class T0, template <class> class OIf,
          class T1, template <class> class I1If>
inline socketwrap<T0,T1>* make_socketwrap(const std::string& pName,
    const int& offset,
    const std::string& ip_addr,
    const int& port,
    unsigned int i1_tk,
    unsigned int o1_tk,
    OIf<T0>& outS,
    I1If<T1>& inp1S
    )
{
    auto p = new socketwrap<T0,T1>(pName.c_str(), offset, ip_addr, port , i1_tk , o1_tk);
    
    (*p).iport1(inp1S);
    (*p).oport1(outS);
    
    return p;
}

//! Helper function to construct a socketwrap2 process
/*! This function is used to construct a socket wrapper process (SystemC
 * module) with two input ports (of different types) and one output port.
 * It provides a more functional style definition of a ForSyDe process.
 * It also removes boilerplate code by using C++ type inference and automatic
 * binding to the input and output FIFOs.
 */
template <class T0, template <class> class OIf,
          class T1, template <class> class I1If,
          class T2, template <class> class I2If>
inline socketwrap2<T0, T1, T2>* make_socketwrap2(const std::string& pName,
    const int& offset,
    const std::string& ip_addr,
    const int& port,
    unsigned int i1_tk,   // Number of tokens to consume from the first input port per cycle
    unsigned int i2_tk,   // Number of tokens to consume from the second input port per cycle
    unsigned int o1_tk,   // Number of tokens to produce on the output port per cycle
    OIf<T0>& outS,
    I1If<T1>& inp1S,
    I2If<T2>& inp2S)
{
    auto p = new socketwrap2<T0, T1, T2>(pName.c_str(), offset, ip_addr, port, i1_tk, i2_tk, o1_tk);
    
    // Bind input and output ports to the corresponding FIFOs
    (*p).iport1(inp1S);
    (*p).iport2(inp2S);
    (*p).oport1(outS);
    
    return p;
}

} // end namespace SDF
} // end namespace ForSyDe

#endif
