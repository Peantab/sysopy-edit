#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QThread>

QString receiveMessage(int fd);

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT
    QThread updaterThread;

public:
    explicit EditorWindow(QWidget *parent = 0);
    ~EditorWindow();

public slots:
    void handleUpdate(const QString document);
    void handleLostConnection();

private slots:
    void on_textEdit_textChanged();

signals:
    void operate(int sock);

private:
    void sendMessage(const char* message, int flags);
    Ui::EditorWindow *ui;
    int sock;
    bool externalChange = false;
    bool appStarted = false;
};

#endif // EDITORWINDOW_H
