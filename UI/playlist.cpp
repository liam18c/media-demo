#include "playlist.h"

static int ID=1000;

PlayList::PlayList(QObject *parent)
    : QObject{parent},
      curIndex_(-1),oldIndex_(-1) {}

void PlayList::AddMedia(const QString &path, int pos) {
    QFileInfo file = QFileInfo(path);
    const QString& file_name = file.fileName();
    QList<QString> file_path = file_name.split(".");
    if(file_path[file_path.size()-1] == "mp3") {
        PlayItem new_item;
        new_item.ItemID_ = ID++;
        new_item.path_ = path;
        new_item.file_name_ = file_name;
        new_item.cover_path_ = ":/iconsource/audio.jpg";
        int insertpos = pos==-1? this->playlist_.size():pos; //是否随机插入
        this->playlist_.insert(insertpos,new_item);
        emit this->PlayItemAdd(insertpos,insertpos); //完成插入
        return;
    }
    FrameGrabber* grabber = new FrameGrabber();
    if(grabber->GrabFrame(QUrl(path))) { //item插入m_playlist
        connect(grabber,&FrameGrabber::framebeFetched,this, [=](const QImage& image) {
            QString image_name = file_name.split(".").at(0)+".png";
            QFileInfo info(image_name);
            int index=0;
            while(info.exists()) { //已存在同名文件
                image_name = file_name.split(".").at(0)+QString::number(++index)+".png";
                info = QFileInfo(image_name);
            }

            bool save_reslut = image.save(image_name,"PNG");
            if(save_reslut) {
                PlayItem new_item;
                new_item.ItemID_ = ID++;
                new_item.path_ = path;
                new_item.file_name_ = file_name;
                new_item.cover_path_ = image_name;
                int insertpos = pos==-1? this->playlist_.size():pos; //是否随机插入
                this->playlist_.insert(insertpos,new_item);
                emit this->PlayItemAdd(insertpos,insertpos); //完成插入
            }
            else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("image save failed");
                msgBox.exec();
            }
            grabber->Stop();
            grabber->deleteLater();
        });
        connect(grabber, &FrameGrabber::error,this,[=]() {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("grab failed");
            msgBox.exec();
            grabber->Stop();
            grabber->deleteLater();
        });
    }
    else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("add media failed");
        msgBox.exec();
        grabber->deleteLater();//延时析构，防止未完成动作崩溃
    }
}

void PlayList::AddMedias(const QList<PlayItem> playitems) {
    if(playitems.empty()) {
        return;
    }
    int startpos = this->playlist_.size();
    int endpos = startpos+playitems.size()-1;
    this->playlist_.append(playitems);
    emit PlayItemAdd(startpos, endpos);
}

void PlayList::DeleteMedia(int &index){

    DeleteMedias(QList<int>()<<index);

}

void PlayList::DeleteMedias(QList<int> indexs){
    std::sort(indexs.begin(),indexs.end());
    int change = 0;
    for(auto iter = indexs.constEnd()-1; iter>=indexs.constBegin();iter--) {
        int index = *iter;
        QFile::remove(playlist_.at(index).cover_path_);
        this->playlist_.remove(index);
        emit PlayItemDel(index,index);
        if(index<curIndex_) {
            change++;
        }
    }
    if(curIndex_ !=-1) {
        SetCurIndex(this->playlist_.size()>0?(curIndex_-change+this->playlist_.size())%this->playlist_.size():-1);
    }
}

int PlayList::GetCurIndex() const
{
    return curIndex_;
}

void PlayList::SetCurIndex(int newCurIndex)
{
    const PlayItem& newitem = HasPlayItem(newCurIndex) ? PlayItemAt(newCurIndex) : PlayItem();

    if (newitem.ItemID_ != curPlayitem_.ItemID_) {//播放项发生改变
        curPlayitem_ = newitem;
        emit CurPlayItemChange(newitem);
    }

    if (curIndex_ == newCurIndex) {
        return;
    }

    curIndex_ = newCurIndex;
    emit CurIndexChange(newCurIndex);
}

const PlayItem &PlayList::GetCurPlayItem() const {
    return curPlayitem_;
}

const int PlayList::GetCurListSize() const
{
    return playlist_.size();
}

const PlayItem &PlayList::PlayItemAt(int index) const {
    return this->playlist_.at(index);
}

bool PlayList::HasPlayItem(int index) const {
    if(index<0||index>=playlist_.size()) {
        return false;
    }
    return true;
}

void PlayList::Next(int mode) {
    int next_index;
    switch(mode){
    case 0:
        next_index = curIndex_;
        break;
    case 1:
        next_index = curIndex_<playlist_.size()-1?(curIndex_+1)%playlist_.size():0;
        break;
    case 2:
        next_index = (curIndex_+rand())%playlist_.size();
        break;
      default:
        next_index = curIndex_;
        break;
    }
        SetCurIndex(next_index);
}

void PlayList::Previous() {
    int next_index = curIndex_>0?(curIndex_-1+playlist_.size())%playlist_.size():playlist_.size()-1;
    SetCurIndex(next_index);
}

QDataStream& operator<<(QDataStream& datastream, const PlayList& playlist) {
    datastream<<playlist.playlist_;
    return datastream;
}

QDataStream& operator>>(QDataStream& datastream, PlayList& playlist) {
    QList<PlayItem> playitems;
    datastream>>playitems;
    for(int index=0;index<playitems.size();index++) {//防止出现ID相同
       playitems[index].ItemID_ = ID++;
    }
    playlist.AddMedias(playitems);
    return datastream;
}
