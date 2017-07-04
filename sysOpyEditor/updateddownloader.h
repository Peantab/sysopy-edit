#ifndef UPDATEDDOWNLOADER_H
#define UPDATEDDOWNLOADER_H

#include <QObject>
#include <QString>

class UpdatedDownloader : public QObject
{
    Q_OBJECT
public:
    explicit UpdatedDownloader(QObject *parent = 0);

signals:
    void updateAvailable(const QString &document);
    void connectionLost();

public slots:
    void update(int sock);
};

#endif // UPDATEDDOWNLOADER_H
