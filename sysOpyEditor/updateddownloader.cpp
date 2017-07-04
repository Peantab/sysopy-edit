#define _BSD_SOURCE
#include "updateddownloader.h"
#include "editorwindow.h"
#include <QString>
#include <QMessageBox>
#include <QThread>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>

UpdatedDownloader::UpdatedDownloader(QObject *parent) : QObject(parent)
{
    //Nothing here.
}

void UpdatedDownloader::update(int sock){
//    char probe;
//    int amountRead;
//    while(true){
//        QThread::usleep(100);
//        amountRead = recv(sock, &probe, 1, MSG_DONTWAIT | MSG_PEEK | MSG_NOSIGNAL);
//        if (amountRead == -1 && errno == EBADF) break;
//        if (amountRead > 0){
//            QString document = receiveMessage(sock);
//            emit updateAvailable(document);
//        }
//    }
    struct pollfd theOnly;
    theOnly.fd = sock;
    theOnly.events = POLLIN | POLLRDHUP;
    theOnly.revents = 0;
    while (true){
        int changedAmount = poll(&theOnly, 1, -1);
        // printf("changedAmount: %d\n", changedAmount);
        if (changedAmount > 0){
            if(theOnly.revents & POLLIN){
                QString document = receiveMessage(sock);
                if (!document.size()){
                    emit connectionLost();
                    break;
                }
                emit updateAvailable(document);
                theOnly.revents = 0;
            }else if(theOnly.revents & POLLRDHUP){
                emit connectionLost();
                break;
            }else{
                break;
            }
        }
    }
    return;
}
