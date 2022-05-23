#ifndef PLAYER_H
#define PLAYER_H

#include "playlist.h"
#include "framegrabber.h"
#include "playlistmodel.h"
#include "myvideowidget.h"
#include "VideoDecoder/avplayer.h"

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>
#include <QMediaPlayer>
#include <QListView>
#include <QAudioOutput>
#include <QStandardItemModel>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QTimer>

class Player : public QWidget
{
    Q_OBJECT
public:
    explicit Player(QWidget *parent = nullptr);
    void SetupLayout();
    void SetupPlayList();
    void Move();

signals:


private slots:
    void OnPlayItemClick(const QModelIndex& index);
    void OnMenuRequest(const QPoint& pos);
    void OnAddPlayItem();
    void OnDeletePlayItem();

    void OnAudioSlider(qint64 position);
    void OnPlay(const PlayItem& playitem);
    void OnCurrentIndexChanged(int index);
    void AddToPlayList(QList<QString> files);
    void AddToPlayList(QList<QUrl> files);
    void OnPauseOrPlayButton();
    void OnPlayStatuChange(AVPlayer::AVPlayerState newstate);
    void OnMediaFinish();
    void OnForwardButton();
    void OnBackwardButton();
    void OnRateButton();
    void OnListButton();
    void OnFullScreenButton();
    void OnPlayModeButton();
    void OnBackRunButton();

    void OnSeek();
    void OnSeek(qint64 position);
    void OnSeekPervious();
    void OnSeekNext();
    void OnVideoDurationChange(qint64 duration);
    void OnVideoPositionChange();
    void OnPlayBackRateChange(qreal value);
    void OnAudioVolumeChange(float volume);
    void OnUrlError();
    void OnBackRun(int flag);

private:
    QToolButton* playButton_;
    QToolButton* forwardButton_;
    QToolButton* backwardButton_;
    QToolButton* previousButton_;
    QToolButton* nextButton_;
    QToolButton* fullscreen_;
    QPushButton* playmodeButton_;
    QPushButton* backRunButton_;
    QPushButton* rateButton_;
    QPushButton* listButton_;

    QLabel* label_;
    QSlider* slider_;
    QLabel* audiolabel_;
    QLabel* audiolabelmax_;
    QSlider* audioslider_;
    AVPlayer* player_;
    MyVideoWidget* videoOutput_;
    QListView* playlistView_ ;
    PlayListModel* playlistModel_;

    int curplaymode_;
    int runflag_;
    QTimer* timer_;
protected:
    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
};

#endif // PLAYER_H
