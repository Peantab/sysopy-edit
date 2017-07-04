#define _BSD_SOURCE
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

using namespace std;

const string FILENAME = "document.txt";
int sockListener; // Initialized before setting handler
string* documentForSigint = NULL;

void sendMessage(int sock, const char* message, int flags);
string receiveMessage(int fd);
void saveDocument(string document);

void sigint(int signal){
    // Close a listener for IPv4 connections
    shutdown(sockListener, SHUT_RDWR);
    close(sockListener);

    // Save document
    saveDocument(*documentForSigint);

    // Everything is fine, it was planned.
    exit(0);
}

int main(int argc, char *argv[])
{
    // Parsing input
    if(argc < 2)
    {
        cout << "Expected: " << argv[0] << "[TCP port number]" << endl;
        return 1;
    }
    int port = atoi(argv[1]);

    // Accesing file with saved(?) text.
    cout << "Opening file...";

    fstream documentFile;
    documentFile.open(FILENAME, fstream::in);

    // Read entire file (if exists) to a String.
    string document(static_cast<stringstream const&>(stringstream() << documentFile.rdbuf()).str());
    documentForSigint = &document;
    if (document.length() == 0) document = "***Empty document***";

    // Networking.
    cout << "OK" << endl;
    cout << "Running socket...";
    vector<struct pollfd> polled;

    // Prepare a listener for IPv4 connections
    sockListener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addressIP4;
    addressIP4.sin_family = AF_INET;
    addressIP4.sin_port = port;
    addressIP4.sin_addr.s_addr = INADDR_ANY;
    bind(sockListener,(struct sockaddr*) &addressIP4, sizeof(addressIP4));
    listen(sockListener, 10);
    struct pollfd sIP4;
    sIP4.fd = sockListener;
    sIP4.events = POLLIN;
    sIP4.revents = 0;
    polled.push_back(sIP4);
    cout << "OK" << endl;

    // Register SIGINT handler.
    struct sigaction act;
    act.sa_handler = sigint;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    bool keepRunning = true;
    while(keepRunning){
        int changedAmount = poll((struct pollfd*) &polled[0], polled.size(), -1);
        if (changedAmount > 0){
          // Accepting new client.
          if(polled[0].revents != 0){
            // clear flag
            polled[0].revents = 0;
            // create and register socket
            struct sockaddr_in cliAddress;
            socklen_t sizeCli = sizeof(cliAddress);
            int cliSock = accept(polled[0].fd, (struct sockaddr*) &cliAddress, &sizeCli);
            struct pollfd* sCli = (struct pollfd*) malloc(sizeof(struct pollfd));
            sCli->fd = cliSock;
            sCli->events = POLLIN | POLLRDHUP;
            sCli->revents = 0;
            polled.push_back(*sCli);
            cout << "Client connected" << endl;
            sendMessage(sCli->fd, document.c_str(), 0);
          }
          // Iterate trough clients.
          for (unsigned int i=1; i < polled.size(); i++){
              if(polled[i].revents != 0){
                  // Client disconnected
                  if ((polled[i].revents & POLLRDHUP) != 0){
                      polled.erase(polled.begin()+i);
                      cout << "Client disconnected." << endl;
                      if (polled.size() == 1) keepRunning = false; //There are no clients, so we can leave.
                      break;
                  }else{ // Client sent something.
                      polled[i].revents = 0;
                      document = receiveMessage(polled[i].fd);
                      // Deliver new text to other clients.
                      for (unsigned int j=1; j < polled.size(); j++){
                          if (j != i){
                              sendMessage(polled[j].fd, document.c_str(), 0);
                          }
                      }
                  }
              }
          }
        }
    }

    // Close a listener for IPv4 connections
    shutdown(sockListener, SHUT_RDWR);
    close(sockListener);

    // Save document
    saveDocument(document);

    return 0;
}

void sendMessage(int sock, const char* message, int flags){
  unsigned int total = 0;
  const char* sendable = message;
  while(total < strlen(message)){
    int cur = send(sock, sendable, strlen(sendable), flags);
    sendable += cur;
    total += cur;
  }
}

string receiveMessage(int fd){
  string document = "";
  char* message = (char*) malloc(255);
  int received = 255;
  int newReceived;
  while (received > 0){
    newReceived = read(fd, message, received);
    message[newReceived] = '\0';
    document += message;
    if (newReceived < received) break;
    else received = newReceived;
  }
  free(message);
  return document;
}

void saveDocument(string document){
    fstream documentFile;
    documentFile.open(FILENAME, fstream::out | fstream::trunc);
    documentFile << document;
    documentFile.close();
}
