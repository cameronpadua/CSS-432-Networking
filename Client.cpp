// --------------- Client Implementation File --------------------------
// Cameron Padua CSS343 C
// April 7, 2018
//
// ---------------------------------------------------------------------------
// Purpose - This program is meant to simulate a Client Server TCP connection
//  with data being transferred between them. This program will incorporate
//  multiple threads for the reading of data.
// -------------------------------------------------------------------------
// Specification: This program will user user input to specify the following
    //  port: server's port number
    //  repetition: the repetition of sending a set of data buffers
    //  nbufs: the number of data buffers
    //  bufsize: the size of each data buffer (in bytes)
    //  serverIp: server's IP address
    //  type: the type of transfer scenario: 1, 2, or 3 (see below)
        //1: multiple writes, 2: writev, or 3: single write
//  After this information is provided, the program will error check the input
//  for any inconsistencies with the variable type
//  Special Algorithms: N/A
//  Assumptions: N/A
//  Additional Notes:
//                  nbufs * bufsize = 1500
//                  Compiled using C++11
//                  Compile command: g++ Client.cpp -o client
//                  run Command: ./client port repetition nbuf bufsize
//                  serverIP type
//                  example: ./client 1522 20000 15 100 127.0.0.1 1
// -----------------------------------------------------------------------


#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <iostream>       //cout
#include <stdlib.h>       //atoi
#include <stdexcept>      //Excpetions
#include <sys/time.h>     // time

using namespace std;

class Client {
private:
    int port;
    int repetition;
    int nbufs;
    int bufsize;
    const char *serverIP;
    int type;
    int DATABUFMAX = 1500;
public:
/*----------------------Default Constructor-----------------------------
 * This constructor will error check most inputs from the user and throw
 * an invalid_arguement error if it is not correct. In addition, it will
 * assign the inputted data to the internal private data of the class.
 *
 * Inputs: int _port, int _repetition, int _nbufs, int _bufsize, const
 * char *_serverIP, int _type
 *
 * Returns: N/A
 *
 * Notes: N/A
 * ---------------------------------------------------------------------
 */
    Client(int _port, int _repetition, int _nbufs,
           int _bufsize, const char *_serverIP, int _type) {
        //error check the passed in variables
        if (_port < 1025 || _port > 65535)
            throw invalid_argument(
                    "Port is need to be greater than 1024 and less than 65535");
        if (_repetition < 0)
            throw invalid_argument("Repitition must be larger than 0");
        if (_nbufs * _bufsize != 1500)
            throw invalid_argument("nbufs*_bufsize needs to equal 1500");
        if (_type <= 0 || _type > 3)
            throw invalid_argument("Type must be 1, 2, or 3");
        //initialize global varibles
        port = _port;
        repetition = _repetition;
        nbufs = _nbufs;
        bufsize = _bufsize;
        nbufs = _nbufs;
        serverIP = _serverIP;
        type = _type;

    }
/*-------------------------Client Run method-------------------------
 *  This method is resposible for creating the socket and connecting to the
 *  server. In addition, it will write to the databuff array and send it to
 *  the server. Lastly, it will compute the time it takes to send, and make
 *  a round trip. In addition, it will print the count it gets from the server
 *
 *  Inputs: N/A
 *
 *  Returns: N/A
 *
 *  Notes: A portion of the code was provided on the Canvas site from the
 *  additional help section
 */
    void run() {
        //get hostent structure from the server name
        struct hostent *host = gethostbyname(serverIP);

        //creation of a socket for input
        sockaddr_in sendSockAddr;
        bzero((char *) &sendSockAddr, sizeof(sendSockAddr));
        sendSockAddr.sin_family = AF_INET; // Address Family Internet
        sendSockAddr.sin_addr.s_addr =
                inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
        sendSockAddr.sin_port = htons(port);

        //Open a stream-oriented socket with the Internet address family.
        int clientSD = socket(AF_INET, SOCK_STREAM, 0);
        //error check the Socket Descriptor, below 0 is error
        if (clientSD < 0) {
            close(clientSD);
            throw logic_error("Socket Error");
        }

        //Connect input socket to the server
        connect(clientSD, (sockaddr *) & sendSockAddr, sizeof(sendSockAddr));

        //Data storage that will be written
        char databuf[nbufs][bufsize];

        struct timeval startTime;
        struct timeval lap;
        struct timeval endTime;

        //initialize startTime to current time
        gettimeofday(&startTime, NULL);

        //switch between types of data transfers
        //writev
        if (type == 1) {
            for (int i = 0; i < repetition; i++) {
                for (int j = 0; j < nbufs; j++) {
                    write(clientSD, databuf[j], bufsize);
                }
            }
        //singlewrite
        } else if (type == 2) {
            for (int i = 0; i < repetition; i++) {
                struct iovec vector[nbufs];
                for (int j = 0; j < nbufs; j++) {
                    vector[j].iov_base = databuf[j];
                    vector[j].iov_len = bufsize;
                }
                writev(clientSD, vector, nbufs);
            }
        //write all
        } else {
            for (int i = 0; i < repetition; i++) {
                write(clientSD, databuf, nbufs * bufsize);
            }
        }
        //initialize lap to current time
        gettimeofday(&lap, NULL);

        int count;      //read from the server.
        read(clientSD, &count, sizeof(count));
        //initialize startTime to current time
        gettimeofday(&endTime, NULL);
        //Test 1: data-sending time = xxx usec, round-trip time = yyy usec, #reads = zzz
        cout << "Test" << type << ": data-sending time =:"
             << (lap.tv_sec - startTime.tv_sec) * 1000000 +
                (lap.tv_usec - startTime.tv_usec) << " usec ";
        cout << "Round-trip time = "
             << (endTime.tv_sec - startTime.tv_sec) * 1000000 +
                (endTime.tv_usec - startTime.tv_usec) << " usec ";
        cout << "#reads = " << count << endl;

        //Close the Socket
        close(clientSD);
    }
};
/*-------------------------Main----------------------------------
 * Takes in the 6 require parameters from the user and passes it to the client
 * constructor.
 *
 * Returns 1 on success, -1 on failure
 *
 * Usage [int port, int repetition, int nbufs, int bufsize, serverIp, int type]
 * Notes: nbufs * bufsize = 1500
 ----------------------------------------------------------------------
 */
int main(int argc, char *argv[]) {
    //quick check for the correct number of arguements
    if (argc > 7) {
        cout << "Client requires 6 arguements" << endl;
        return -1;
    } else {
      //converts all values to ints except serverIP and pass to constructor
        Client(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]),
               atoi(argv[4]), argv[5], atoi(argv[6])).run();
    }
    return 1;
}
