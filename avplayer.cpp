#include "avplayer.h"

AVPlayer::AVPlayer()
{
    mutex.lock();
    m_state=CLOSE;
    m_play_mode=1;
    m_play_speed=1.0;
    mutex.unlock();

    m_decoder=AVDecoder::GetInstance();
    m_video_player_thread=new VideoPlayerThread();

    connect(m_decoder,&AVDecoder::Ready,this,&AVPlayer::GetDecoderReady);
    connect(m_video_player_thread,&VideoPlayerThread::PlayFinish,this,&AVPlayer::GetPlayFinish);

}

AVPlayer::~AVPlayer(){

}

void AVPlayer::Start(const QString& url,void* winId){
    mutex.lock();
    if(m_state==START||m_state==STOP){
        mutex.unlock();
        Close();
        mutex.lock();
    }
    m_state=START;
    m_winId=winId;
    m_decoder->Open(url);
    mutex.unlock();
}

void AVPlayer::Resume(){
    mutex.lock();
    if(m_state!=STOP){
        mutex.unlock();
        return;
    }
    m_state=START;
    m_video_player_thread->Resume();
    mutex.unlock();
}

void AVPlayer::Stop(){
    mutex.lock();
    if(m_state!=START){
        mutex.unlock();
        return;
    }
    m_state=STOP;
    m_video_player_thread->Stop();
    mutex.unlock();
}

void AVPlayer::Close(){
    mutex.lock();
    if(m_state==CLOSE){
        mutex.unlock();
        return;
    }
    m_state=CLOSE;
    m_play_mode=1;
    m_play_speed=1.0;
    m_video_player_thread->Close();
    m_decoder->Close();
    mutex.unlock();
}

void AVPlayer::SetPlayMode(int flag){
    mutex.lock();
    if(m_play_mode==flag){
        mutex.unlock();
        return;
    }
    m_play_mode=flag;
    VideoFrame* frame=m_video_player_thread->GetCurrentFrame();
    if(!frame){
        mutex.unlock();
        return;
    }
    m_decoder->SetPos(frame->pos,0);
    m_decoder->SetPlayMode(flag);
    m_video_player_thread->SetPlayMode(flag);
    mutex.unlock();
}

void AVPlayer::SetPlaySpeed(double speed){
    mutex.lock();
    if(qAbs(m_play_speed-speed)<0.001){
        mutex.unlock();
        return;
    }
    m_play_speed=speed;
    m_decoder->SetPlaySpeed(speed);
    mutex.unlock();
}

void AVPlayer::SetPos(double sec){
    mutex.lock();
    m_decoder->SetPos(sec,1);
    mutex.unlock();
}

AVInfomation* AVPlayer::GetAVInformation(){
    mutex.lock();
    if(m_state==CLOSE){
        mutex.unlock();
        return nullptr;
    }
    AVInfomation* ret=m_decoder->GetAVInfomation();
    mutex.unlock();
    return ret;
}

VideoFrame* AVPlayer::GetCurrentFrame(){
    mutex.lock();
    if(m_state==CLOSE){
        mutex.unlock();
        return nullptr;
    }
    VideoFrame* ret=m_video_player_thread->GetCurrentFrame();
    mutex.unlock();
    return ret;
}

//slots
void AVPlayer::GetDecoderReady(){
    //接收 m_decoder已启动解码 信号
    m_video_player_thread->Init(m_decoder,m_winId);
    m_video_player_thread->Start();
    //可捕获该信号作为播放开始的标志
    emit PlayStart();
}

void AVPlayer::GetPlayFinish(){
    this->Close();
    //可捕获该信号作为播放结束的标志
    emit PlayFinish();
}
