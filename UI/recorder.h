#ifndef RECORDER_H
#define RECORDER_H

#include "playlist.h"

#include <QObject>

class recorder : public QObject
{
    Q_OBJECT
public:
    explicit recorder(QObject *parent = nullptr);

    void ReadDataRecord(PlayList& playlist);
    void WriteDataRecord(const PlayList& playlist);
signals:

private:

};

#endif // RECORDER_H
