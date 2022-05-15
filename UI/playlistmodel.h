#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "playlist.h"
#include "recorder.h"

#include <QObject>
#include <QStandardItemModel>

class PlayListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit PlayListModel(QObject *parent = nullptr);
    PlayList* GetPlayList() const;

signals:

private:
    void OnPlayItemAdd(int start, int end);
    void OnPlayItemDelete(int start, int end);

private:
    PlayList* playlist_ = nullptr;
    recorder* recorder_;

};

#endif // PLAYLISTMODEL_H
