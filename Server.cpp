/*--------------- Server Implementation File --------------------------
 * Purpose: This program is meant to act as a server in a client server. It
 * will read in the data sent by the client and send an acknowledgement of
 * the number of reads done.
 *
 * Specification: This program is broken down into 4 logical parts. The
 * first one is the main method that will read in the parameters and ensure
 * tha there is the correct amount. It will then pass that data into the
 * second logical part. The constructor for the server will check if the
 * data is valid return to main. After it will use the run method to create
 * and accept socket request from the client. Should one come in, it will
 * create a new thread and read in the data from the client. At the end, it
 * will close the socket with the client.
 *
 * Additional Notes:
 *                  DATABUFMAX = 1500
//                  Compiled using C++11
//                  Compile command: g++ Server.cpp -o server -pthreads
//                  run Command: ./server port repetition
//                  example: ./server 1522 20000
 */

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
#include <pthread.h>      //pthread


using namespace std;

class Server {
private:
    int port;
    int repetition;
public:
/*----------------------Default Constructor-----------------------------
 * This constructor will error that the input data is within acceptable ranges
     * For the port, it need to be between 1024 and 65535. For the
     * repetition, it needs to be greater than 0.
 *
 * Inputs: int _port, int _repetition
 *
 * Returns: N/A
 *
 * Notes: N/A
 * -------------------------------------------------------------------------
 */
    Server(int _port, int _repetition) {
        //error check the passed in variables
        if (_port < 1024 || _port > 65535)
            throw invalid_argument(
                    "Port is need to be greater than 1024 and less than 65535");
        if (_repetition <= 0)
            throw invalid_argument("Repitition must be larger than 0");
        //initialize global varibles
        port = _port;
        repetition = _repetition;

    }
/*------------------------------Run -------------------------
 * This method is responsible for create a socket and listening for incoming
 * requests. Upon request, the method will create a new pthreads and call a
 * new method upon creation. This server will run indefinitely until the
 * server is manually stopped
 *
 * Inputs: Nothing
 * Returns Nothing
 *
 ----------------------------------------------------------------------------
 */
    void run() {
        pthread_t thread1;
        //creation of socket
        sockaddr_in acceptSockAddr;
        bzero((char *) &acceptSockAddr, sizeof(acceptSockAddr));
        acceptSockAddr.sin_family = AF_INET; // Address Family Internet
        acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        acceptSockAddr.sin_port = htons(port);
        //set up socket for going in
        int serverSD = socket(AF_INET, SOCK_STREAM, 0);

        const int on = 1;
        setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
                   sizeof(int));
        //bind socket descirptor and infaststructure
        bind(serverSD, (sockaddr * ) & acceptSockAddr, sizeof(acceptSockAddr));

        //Looping for new clients attemping to connect
        while (true) {
            listen(serverSD, 4);

            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);
            int newSd = accept(serverSD, (sockaddr * ) & newSockAddr,
                           &newSockAddrSize);
            int arr[2] = {newSd, repetition};
            pthread_create(&thread1, NULL, readData, (void *) arr);
        }
        close(serverSD);
    }
/*------------------------------Read Data -------------------------
 * This is the static method that will be called when a new pthread is created.
 * This method is responsible for reading in the data based on the
 * repetitions and socket descriptor that is passed from the previous method
 * .  In addition, this method will computer the total data-receiving time to
 * get the data from the client
 *
 *
 * Returns Nothing
 *
 * Usage [void* value]
 * I passed in an array with Repetition and the SD
 ----------------------------------------------------------------------------
 */
    static void *readData(void *vptr_value) {
        int *arr = (int *) vptr_value;
        char dataBuff[1500];

        struct timeval startTime;
        struct timeval stopTime;

        gettimeofday(&startTime, NULL);

        //reading the data from the client
        int count = 0;
        for (int i = 0; i < arr[1]; i++) {
            int nRead = 0;
            while (nRead < 1500) {
                int bytesRead = read(arr[0], dataBuff, 1500 - nRead);
                nRead += bytesRead;
                count++;
            }
        }

        //End time
        gettimeofday(&stopTime, NULL);

        cout << "data-receiving time = "
             << (stopTime.tv_sec - startTime.tv_sec) * 1000000 +
                (stopTime.tv_usec - startTime.tv_usec) << " usec" << endl;

        //Send the number of reads to the client
        write(arr[0], &count, sizeof(count));

        //close socket
        close(arr[0]);
    }

};
/*-------------------------Main--------------------------------------------
 * Takes in the 2 require parameters from the user and passes it to the server
 * constructor.
 *
 * Returns 1 on success, -1 on failure
 *
 * Usage [int port, int repetition]
 ----------------------------------------------------------------------------
 */
int main(int argc, char *argv[]) {
    if (argc > 3) {
        cout << "Server requires 2 arguments" << endl;
        return -1;
    } else {
        Server(atoi(argv[1]), atoi(argv[2])).run();
    }
    return 1;
}
