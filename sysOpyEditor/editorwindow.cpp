#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "updateddownloader.h"
#include <QMessageBox>
#include <QTextCursor>
#include <QString>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
namespace imported {
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <endian.h>
}

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);

    // Read configuration
    char* IPstring = NULL;
    char* portString = NULL;
    FILE* config = fopen("configSOE", "r");
    if(config == NULL){
        QMessageBox::critical(this, tr("Critical Error"), tr("File configSOE which should contain server's IP in the first line and port number in second line, could not be opened."));
        exit(1);
    }
    size_t lineLength = 0;
    getline(&IPstring, &lineLength, config);
    if (lineLength <= 0){
        QMessageBox::critical(this, tr("Critical Error"), tr("File configSOE does not contain an IP address in its first line."));
        QApplication::quit();
    }
    lineLength = 0;
    getline(&portString, &lineLength, config);
    if (lineLength <= 0){
        QMessageBox::critical(this, tr("Critical Error"), tr("File configSOE does not contain a port number in its first line."));
        exit(1);
    }
    fclose(config);

    // Establish connection
    sock = imported::socket(AF_INET, imported::SOCK_STREAM, 0);
    imported::sockaddr_in sockAddress;
    sockAddress.sin_family = AF_INET;
    int res = inet_aton(IPstring, &(sockAddress.sin_addr));
    if (!res){
        QMessageBox::critical(this, tr("Critical Error"), tr("IP address in configSOE is invalid."));
        exit(1);
    }
    sockAddress.sin_port = atoi(portString);
    free(IPstring);
    free(portString);
    int result = imported::connect(sock, (imported::sockaddr*) &sockAddress, sizeof(sockAddress));
    if (result == -1){
        QMessageBox::critical(this, tr("Critical Error"), tr("Unable to connect with a server."));
        exit(1);
    }

    // Receive initial state of a document.
    QString document = receiveMessage(sock);
    ui->textEdit->setText(document);

    // Start updater in a thread:
    UpdatedDownloader *updater = new UpdatedDownloader();
    updater->moveToThread(&updaterThread);
    connect(&updaterThread, &QThread::finished, updater, &QObject::deleteLater);
    connect(this, &EditorWindow::operate, updater, &UpdatedDownloader::update);
    connect(updater, &UpdatedDownloader::connectionLost, this, &EditorWindow::handleLostConnection);
    connect(updater, &UpdatedDownloader::updateAvailable, this, &EditorWindow::handleUpdate);
    updaterThread.start();
    emit operate(sock);

    appStarted = true;
}

EditorWindow::~EditorWindow()
{
    appStarted = false;
    shutdown(sock, imported::SHUT_RDWR);
    imported::close(sock);
    updaterThread.quit();
    updaterThread.wait();
    delete ui;
}

QString receiveMessage(int fd){
  QString document = "";
  char* message = (char*) malloc(255);
  int received = 255;
  int newReceived;
  while (received > 0){
    newReceived = imported::read(fd, message, received);
    message[newReceived] = '\0';
    document += message;
    if (newReceived < received) break;
    else received = newReceived;
  }
  free(message);
  return document;
}

void EditorWindow::on_textEdit_textChanged()
{
    QString document = ui->textEdit->toPlainText();
    if (appStarted && !externalChange) sendMessage(document.toStdString().c_str(), 0);
}

void EditorWindow::sendMessage(const char* message, int flags){
  unsigned int total = 0;
  const char* sendable = message;
  while(total < strlen(message)){
    int cur = imported::send(sock, sendable, strlen(sendable), flags);
    sendable += cur;
    total += cur;
  }
}

void EditorWindow::handleUpdate(const QString document){
    QTextCursor cursor = ui->textEdit->textCursor();
    int pos = cursor.position();
    externalChange = true;
    ui->textEdit->setText(document);
    externalChange = false;
    cursor.setPosition(pos);
    ui->textEdit->setTextCursor(cursor);
}

void EditorWindow::handleLostConnection(){
    QMessageBox::critical(this, tr("Critical Error"), tr("Connection lost. Please save your work externally."));
}
