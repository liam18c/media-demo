#include "player.h"

#include <QStyle>

Player::Player(QWidget *parent)
    : QWidget{parent}
{
     playButton_ = new QToolButton(this); //播放按钮
     playButton_->setIcon(QIcon(":/iconsource/pause.png"));
     playButton_->setIconSize(QSize(36,36));

     forwardButton_ = new QToolButton(this); //下一个视频按钮
     forwardButton_->setIcon(QIcon(":/iconsource/next.png"));
     forwardButton_->setIconSize(QSize(30,30));
     backwardButton_ = new QToolButton(this); //上一个视频按钮
     backwardButton_->setIcon(QIcon(":/iconsource/previous.png"));
     backwardButton_->setIconSize(QSize(30,30));

     previousButton_ = new QToolButton(this);//进度后退按钮
     previousButton_->setIcon(QIcon(":/iconsource/seekbackward.png"));
     previousButton_->setIconSize(QSize(30,30));
     nextButton_ = new QToolButton(this);//进度前调按钮
     nextButton_->setIcon(QIcon(":/iconsource/seekforward.png"));
     nextButton_->setIconSize(QSize(30,30));

     rateButton_ = new QPushButton(this);
     rateButton_->setText("×1.0");
     rateButton_->setFlat(true);
     rateButton_->setStyleSheet("QPushButton{color:white}");

     listButton_ = new QPushButton(this);//列表控制按钮
     listButton_->setText("隐藏列表");
     listButton_->setIconSize(QSize(50,20));
     listButton_->setStyleSheet("QPushButton{color:white}");

     playmodeButton_ = new QPushButton(this);
     playmodeButton_->setText("顺序播放");
     playmodeButton_->setFlat(true);
     playmodeButton_->setStyleSheet("QPushButton{color:white}");
     curplaymode_ = 1;

     fullscreen_ = new QToolButton(this);
     fullscreen_->setIcon(QIcon(":/iconsource/fullscreen.png"));
     fullscreen_->setIconSize(QSize(25,25));

     label_ = new QLabel(this); // 时间指示器
     label_->setText("00:00 / 00:00");
     label_->setStyleSheet("QLabel{color:white}");
     slider_ = new QSlider(Qt::Horizontal, this); //进度条
     slider_->setValue(0); //进度条初始化为0

     audioslider_ = new QSlider(Qt::Horizontal, this);
     audioslider_->setMaximum(100);
     audioslider_->setValue(100);
     audiolabel_  = new QLabel(this);
     audiolabel_->setText("音量");
     audiolabel_->setStyleSheet("QLabel{color:white}");
     audiolabelmax_ = new QLabel(this);
     audiolabelmax_->setText("100");
     audiolabelmax_->setStyleSheet("QLabel{color:white}");

     playlistView_ = new QListView(this); //播放列表
     playlistModel_ = new PlayListModel();
     playlistView_->setModel(playlistModel_);
     playlistView_->setStyleSheet("QListView{background-color:darkslategrey;color:white}");
     playlistView_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

     player_ = AVPlayer::getInstance();
     videoOutput_ = new MyVideoWidget(this); //视频播放器

     SetupLayout();
     SetupPlayList();
     setAcceptDrops(true);
     this->grabKeyboard();
     this->setMouseTracking(true);
}
void Player::SetupLayout()
{

    QBoxLayout* bottom_layout = new QVBoxLayout();//播放控制块和进度条纵向布局

    QBoxLayout* tool_layout = new QHBoxLayout();//播放控制块
    tool_layout->addStretch();
    tool_layout->addWidget(audiolabel_);
    tool_layout->addWidget(audioslider_);
    tool_layout->addWidget(audiolabelmax_);
    tool_layout->addSpacing(45);
    tool_layout->addWidget(backwardButton_);
    tool_layout->addSpacing(30);
    tool_layout->addWidget(previousButton_);
    tool_layout->addSpacing(30);
    tool_layout->addWidget(playButton_);
    tool_layout->addSpacing(30);
    tool_layout->addWidget(nextButton_);
    tool_layout->addSpacing(30);
    tool_layout->addWidget(forwardButton_);
    tool_layout->addSpacing(25);
    tool_layout->addWidget(rateButton_);
    tool_layout->addSpacing(25);
    tool_layout->addWidget(playmodeButton_);
    tool_layout->addSpacing(25);
    tool_layout->addWidget(listButton_);
    tool_layout->addSpacing(25);
    tool_layout->addWidget(fullscreen_);
    tool_layout->addStretch();

    QBoxLayout* slider_layout = new QHBoxLayout();
    slider_layout->addWidget(slider_);
    slider_layout->addWidget(label_);

    bottom_layout->addLayout(slider_layout);
    bottom_layout->addLayout(tool_layout);

    this->videoOutput_->GetControlWidget()->setLayout(bottom_layout);

    QBoxLayout* layout = new QVBoxLayout();

    QBoxLayout* top_layout = new QHBoxLayout();//播放器和视频列表横向布局
    top_layout->addWidget(videoOutput_,17);
    top_layout->addWidget(playlistView_,5);

    layout->addLayout(top_layout);
    this->setLayout(layout);

    videoOutput_->setMinimumSize(960,480);
    playlistView_->setMinimumSize(180,480);

}

void Player::SetupPlayList() {

    playlistView_->setIconSize(QSize(100,60));
    playlistView_->setFont(QFont("Times",12));
    playlistView_->setDragEnabled(false); //暂不支持item拖动
    playlistView_->setSelectionMode(QAbstractItemView::ExtendedSelection); //开启多选模式
    playlistView_->setEditTriggers(QAbstractItemView::NoEditTriggers); //不可编辑

    playlistView_->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(playlistView_, &QWidget::customContextMenuRequested, this, &Player::OnMenuRequest);
    connect(playlistModel_->GetPlayList(), &PlayList::CurIndexChange, this, &Player::OnCurrentIndexChanged);
    connect(playlistModel_->GetPlayList(), &PlayList::CurPlayItemChange, this, &Player::OnPlay);
    connect(playlistView_, &QListView::activated,this,&Player::OnPlayItemClick);

    connect(playButton_, &QToolButton::clicked,this,&Player::OnPauseOrPlayButton);
    connect(forwardButton_, &QToolButton::clicked,this,&Player::OnForwardButton);
    connect(backwardButton_, &QToolButton::clicked,this,&Player::OnBackwardButton);
    connect(previousButton_,&QToolButton::clicked,this,&Player::OnSeekPervious);
    connect(nextButton_,&QToolButton::clicked,this,&Player::OnSeekNext);
    connect(rateButton_,&QPushButton::clicked,this,&Player::OnRateButton);
    connect(listButton_,&QPushButton::clicked,this,&Player::OnListButton);
    connect(playmodeButton_, &QPushButton::clicked,this,&Player::OnPlayModeButton);//改变播放模式
    connect(fullscreen_,&QToolButton::clicked,this,&Player::OnFullScreenButton);

    connect(player_, &AVPlayer::PlayStateChange, this, &Player::OnPlayStatuChange);
    connect(player_, &AVPlayer::VideoPositionChange, this, &Player::OnVideoPositionChange);
    connect(player_, &AVPlayer::PlayFinish, this, &Player::OnMediaFinish);
    connect(player_, &AVPlayer::urlError, this, &Player::OnUrlError);
    connect(player_, &AVPlayer::VideoSpeedChange, this, &Player::OnPlayBackRateChange);

    connect(slider_, &QSlider::sliderMoved, this, qOverload<qint64>(&Player::OnSeek));
    connect(slider_, &QSlider::sliderPressed, this, qOverload<>(&Player::OnSeek));
    connect(audioslider_, &QSlider::sliderMoved,this,&Player::OnAudioSlider);

}

void Player::OnPlayItemClick(const QModelIndex& index) {
    int row = index.row();
    playlistModel_->GetPlayList()->SetCurIndex(row);
}

void Player::OnMenuRequest(const QPoint& pos) {

    std::shared_ptr<QMenu> menu = std::make_shared<QMenu>();
    QAction* addAction = menu->addAction("添加视频");
    QAction* delAction = menu->addAction("删除视频");
    connect(addAction,&QAction::triggered, this, &Player::OnAddPlayItem);
    connect(delAction,&QAction::triggered, this, &Player::OnDeletePlayItem);
    menu->exec(QCursor::pos());
}

void Player::OnPlayModeButton(){
    std::shared_ptr<QMenu> menu = std::make_shared<QMenu>();
    QAction* onlycur = menu->addAction("仅播放当前");
    QAction* loopcur = menu->addAction("循环播放");
    QAction* randmode = menu->addAction("随机播放");
    QAction* listmode = menu->addAction("顺序播放");
    connect(onlycur,&QAction::triggered, this, [&](){
        this->curplaymode_ = -1;
        this->playmodeButton_->setText("仅播放当前");
    });
    connect(loopcur,&QAction::triggered, this, [&](){
        this->curplaymode_ = 0;
        this->playmodeButton_->setText("循环播放");
    });
    connect(listmode,&QAction::triggered, this, [&](){
        this->curplaymode_ = 1;
        this->playmodeButton_->setText("顺序播放");
    });
    connect(randmode,&QAction::triggered, this, [&](){
        this->curplaymode_ = 2;
        this->playmodeButton_->setText("随机播放");
    });
    menu->setStyleSheet("QMenu{background-color:black;color:white}");
    menu->exec(QCursor::pos()-QPoint(0,90));
}

void Player::OnAddPlayItem(){

    const QStringList& files = QFileDialog::getOpenFileNames(
          this,
          tr("Choose Video"),
          QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0,QDir::homePath()),
          tr("Video Files (*.mp4 *.mov)")
          );

    if (!files.isEmpty()) {
            AddToPlayList(files);
        }
}

void Player::OnDeletePlayItem() {
    if (playlistView_->selectionModel()->selectedIndexes().isEmpty() == false) {

        QModelIndexList selectedIndexes(playlistView_->selectionModel()->selectedIndexes());
        QList<int> delete_rows;

        for (auto iter = selectedIndexes.crbegin();iter != selectedIndexes.crend(); ++iter) {
            delete_rows.append((*iter).row());
        }

        playlistModel_->GetPlayList()->DeleteMedias(delete_rows);
        int index = playlistModel_->GetPlayList()->GetCurIndex();
        playlistView_->setCurrentIndex(playlistModel_->index(index,0));
    }
}

void Player::OnAudioSlider(qint64 position)
{
    this->player_->SetVolume(double(position) / 100);
    OnAudioVolumeChange(position);
}

void Player::OnPlay(const PlayItem& playitem) {
    if(playitem.ItemID_<0) {
        player_->Stop();
        return;
    }
    player_->Start(playitem.path_, (void*)this->videoOutput_->winId());
    double durationtime_ = (player_->GetAVInformation()->duration - 1) * 1000;
    OnVideoDurationChange(durationtime_);
}

void Player::OnSeek()
{
    this->player_->SetPos(this->slider_->sliderPosition());
}

void Player::OnSeek(qint64 position)
{
    OnVideoPositionChange(position);
    this->player_->SetPos(position / 1000);
}

void Player::OnVideoDurationChange(qint64 duration) {
    slider_->setMaximum(duration);
}

void Player::OnPlayStatuChange(AVPlayer::AVPlayerState newstate) {
    if(newstate == AVPlayer::AVPlayerState::START) {
        playButton_->setIcon(QIcon(":/iconsource/play.png"));
    }
    else {
        playButton_->setIcon(QIcon(":/iconsource/pause.png"));
    }
}

void Player::OnVideoPositionChange(qint64 position) {
    if (!slider_->isSliderDown()) {
         slider_->setValue(position);
         audioslider_->setValue(player_->GetVideoVolume() * 100);
    }

    if(position <= slider_->maximum()) {
        qint64 durationSeconds = slider_->maximum() / 1000 ;
        qint64 positionSeconds = position / 1000;

        QTime currentTime = QTime((positionSeconds/3600) % 60,(positionSeconds/60) % 60,positionSeconds % 60,0);
        QTime durationTime = QTime((durationSeconds/3600) % 60,(durationSeconds/60) % 60,durationSeconds % 60,0);
        QString format = "mm:ss";
        if (durationSeconds > 3600)
            format = "hh:mm:ss";

        label_->setText(currentTime.toString(format)+" / "+durationTime.toString(format));
    }
}
void Player::OnMediaFinish() {
    if(curplaymode_ > 0)
        playlistModel_->GetPlayList()->Next(curplaymode_);
    else if(curplaymode_ == 0){
        OnPlay(this->playlistModel_->GetPlayList()->GetCurPlayItem());
    }
}

void Player::OnPlayBackRateChange(qreal value) {
    if(value>0) {
    this->rateButton_->setText("×"+QString::number(value));
    }
}


void Player::OnAudioVolumeChange(float volume) {
    this->audiolabelmax_->setText(QString::number(volume));
}

void Player::OnUrlError() {
    QMessageBox msgBox; // 弹窗提示
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Open file failed:No such file or directory");
    msgBox.exec();
    playlistModel_->GetPlayList()->SetCurItemLost();
}

void Player::OnPauseOrPlayButton() {
    if(player_->GetPlayState() == AVPlayer::AVPlayerState::START) {
        player_->Stop();
    }
    else if(player_->GetPlayState() == AVPlayer::AVPlayerState::STOP) {
         player_->Resume();
    }
}

void Player::OnSeekPervious() {
    this->player_->SetPos(this->slider_->sliderPosition() / 1000 - 3 );
}

void Player::OnSeekNext() {
    this->player_->SetPos(this->slider_->sliderPosition() / 1000 + 3 );
}

void Player::OnCurrentIndexChanged(int index) {
    playlistView_->setCurrentIndex(playlistModel_->index(index,0));
    int curIndex = playlistModel_->GetPlayList()->GetCurIndex();
    for(int index = 0; index < playlistModel_->GetPlayList()->GetCurListSize(); index++) {
        if (playlistModel_->GetPlayList()->PlayItemAt(index).lost == false) {
             playlistModel_->item(index)->setForeground(QBrush(QColor(255, 255, 255)));
        }
        else {
             playlistModel_->item(index)->setForeground(QBrush(QColor(123,123,123)));
        }
    }
    if (curIndex != -1) {
        if (playlistModel_->GetPlayList()->PlayItemAt(curIndex).lost == false) {
            playlistModel_->item(curIndex)->setForeground(QBrush(QColor(230, 255, 20)));
        }
    }
}

void Player::OnForwardButton() {
    this->playlistModel_->GetPlayList()->Next();
}

void Player::OnBackwardButton() {
    this->playlistModel_->GetPlayList()->Previous();
}

void Player::OnRateButton() {

    std::shared_ptr<QMenu> menu = std::make_shared<QMenu>();
    QAction* midrate = menu->addAction("×0.5");
    QAction* originrate = menu->addAction("×1.0");
    QAction* doublerate = menu->addAction("×2.0");
    QAction* fourthrate = menu->addAction("×4.0");
    connect(midrate,&QAction::triggered, this,[&](){
        this->player_->SetPlaySpeed(0.5);
    });
    connect(originrate,&QAction::triggered, this,[&](){
        this->player_->SetPlaySpeed(1.0);
    });
    connect(doublerate,&QAction::triggered, this, [&](){
        this->player_->SetPlaySpeed(2.0);
    });
    connect(fourthrate,&QAction::triggered, this, [&](){
        this->player_->SetPlaySpeed(4.0);
    });
    menu->setStyleSheet("QMenu{background-color:transparent;color:white}");
    menu->setWindowOpacity(0.5);
    menu->exec(QCursor::pos()-QPoint(0,90));

}

void Player::OnFullScreenButton() {
    if(!this->videoOutput_->isFullScreen()) {
        videoOutput_->setWindowFlags(Qt::Window);
        videoOutput_->showFullScreen();
        videoOutput_->ResizeControlWidget();
    }
    else {
        videoOutput_->setWindowFlags(Qt::SubWindow);
        videoOutput_->showNormal();
        videoOutput_->ResizeControlWidget();
    }
}

void Player::OnListButton() {
    if(playlistView_->isVisible()) {
        playlistView_->setVisible(false);
        listButton_->setText("显示列表");
    }
    else {
        playlistView_->setVisible(true);
        listButton_->setText("隐藏列表");
    }
    this->videoOutput_->SetControlWidgetUnVisible();
}

void Player::AddToPlayList(QList<QString> files) {
    for (auto iter = files.cbegin();iter != files.cend();iter++) {
        const QString& path = *iter;
        playlistModel_->GetPlayList()->AddMedia(path);
    }

    int index = playlistModel_->GetPlayList()->GetCurIndex();
    playlistView_->setCurrentIndex(playlistModel_->index(index,0));

}

void Player::AddToPlayList(QList<QUrl> files) {
    QList<QString> pathlist;
    for(QUrl& url:files){
       pathlist<<url.toString();
    }
    AddToPlayList(pathlist);
}
void Player::dragEnterEvent(QDragEnterEvent *event) {

    event->acceptProposedAction();

}

void Player::dropEvent(QDropEvent *event) {

    auto fileurls = event->mimeData()->urls();
    AddToPlayList(fileurls);
    event->acceptProposedAction();

}

void Player::Move() {
    this->videoOutput_->ResizeControlWidget();
}

void Player::keyPressEvent(QKeyEvent* event){
     event->accept();
     if(event->key() == Qt::Key_Space){
         OnPauseOrPlayButton();
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_F){
         OnFullScreenButton();
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_L){
         OnMenuRequest(QPoint(500,500));
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_Up){
         this->OnAudioSlider(audioslider_->sliderPosition() > 95 ? 100 : audioslider_->sliderPosition() + 5);
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_Down){
         this->OnAudioSlider(audioslider_->sliderPosition() < 5 ? 0 : audioslider_->sliderPosition() - 5);
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_Left){
         this->playlistView_->clearSelection();
         this->playlistModel_->GetPlayList()->Previous();
     }
     else if(event->modifiers()==Qt::ControlModifier&&event->key() == Qt::Key_Right){
         this->playlistView_->clearSelection();
         this->playlistModel_->GetPlayList()->Next();
     }
}
