#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "framegrabber.h"

#include <QObject>
#include <QFileInfo>
#include <QStandardItem>
class PlayItem{
public:
    int ItemID_;
    QString path_;
    QString file_name_;
    QString cover_path_;
    bool lost;

    PlayItem():ItemID_(-1),path_(""), file_name_(""), cover_path_(),lost(false){}
    friend QDataStream& operator<<(QDataStream& datastream,const PlayItem& item)
    {
        datastream<<item.ItemID_<<item.path_<<item.file_name_<<item.cover_path_;
        return datastream;
    }

    friend QDataStream& operator>>(QDataStream& datastream,PlayItem& item)
    {
        datastream>>item.ItemID_>>item.path_>>item.file_name_>>item.cover_path_;
        return datastream;
    }
};

class PlayList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int curIndex READ GetCurIndex WRITE SetCurIndex NOTIFY CurIndexChange);
    Q_PROPERTY(PlayItem curPlayitem READ GetCurPlayItem NOTIFY CurPlayItemChange);
public:
    explicit PlayList(QObject *parent = nullptr);

    void AddMedia(const QString& path, int pos = -1);
    void AddMedias(const QList<PlayItem> playitems);
    void DeleteMedia(int& index);
    void DeleteMedias(QList<int> indexs);
    const PlayItem& PlayItemAt(int index) const;
    bool HasPlayItem(int index) const;
    void Previous();
    void Next(int mode=1);

    int GetCurIndex() const;
    void SetCurIndex(int newCurIndex);
    void SetCurItemLost();
    const PlayItem& GetCurPlayItem() const;
    const int GetCurListSize() const;

    friend QDataStream &operator<<(QDataStream & , const PlayList &);
    friend QDataStream &operator>>(QDataStream & , PlayList &);

private:
    QList<PlayItem> playlist_;

    int curIndex_;
    int oldIndex_;

    PlayItem curPlayitem_;

signals:
    void PlayItemAdd(int beginpos, int endpos);
    void PlayItemDel(int beginpos, int endpos);
    void CurIndexChange(int newindex);
    void CurPlayItemChange(const PlayItem& newplayitem);

};

#endif // PLAYLIST_H
