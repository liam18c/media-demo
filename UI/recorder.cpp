#include "recorder.h"

#include <QFile>

recorder::recorder(QObject *parent)
    : QObject{parent}
{

}

void recorder::WriteDataRecord(const PlayList &playlist) {
    QFile record_file("record");
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    if(record_file.open(QIODeviceBase::WriteOnly)) {
        QDataStream record_stream(&record_file);
        record_stream<<playlist;
        record_file.close();
    }
    else {
        msgBox.setText("cannot open file");
        msgBox.exec();
    }
}

void recorder::ReadDataRecord(PlayList &playlist) {
    QFile record_file("record");
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);

    if(record_file.open(QIODeviceBase::ReadOnly)) {
        QDataStream record_stream(&record_file);
        record_stream>>playlist;
        record_file.close();
    }
    else {
        msgBox.setText("cannot open file");
        msgBox.exec();
    }
}
