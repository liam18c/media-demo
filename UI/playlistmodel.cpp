#include "playlistmodel.h"

PlayListModel::PlayListModel(QObject *parent)
    : QStandardItemModel{parent}
{
    playlist_ = new PlayList(this);

    connect(playlist_, &PlayList::PlayItemAdd,this,&PlayListModel::OnPlayItemAdd);
    connect(playlist_, &PlayList::PlayItemDel,this,&PlayListModel::OnPlayItemDelete);

    recorder_ = new recorder(this);
    recorder_->ReadDataRecord(*playlist_);
}

PlayList* PlayListModel::GetPlayList() const {
    return playlist_;
}

void PlayListModel::OnPlayItemAdd(int start, int end) {
    for(int index = start; index <= end; index++) {
        const PlayItem& play_item = playlist_->PlayItemAt(index);
        QIcon icon = QIcon(play_item.cover_path_);
        QSize icon_size(960,720);
        icon.actualSize(icon_size);
        QStandardItem* item = new QStandardItem(icon,play_item.file_name_);
        this->insertRow(index,item);
    }
    recorder_->WriteDataRecord(*playlist_);
}

void PlayListModel::OnPlayItemDelete(int start, int end) {
    for(int index = end; index >= start; index--) {
        this->removeRow(index);
    }
    recorder_->WriteDataRecord(*playlist_);
}
