#include "avplayer.h"

AVPlayer::AVPlayer()
{
    mutex.lock();
    m_state=CLOSE;
    m_order=true;
    m_speed=1.0;
    mutex.unlock();

    m_decoder=AVDecoder::GetInstance();
    m_video_player_thread=new VideoPlayerThread();

    connect(m_decoder,&AVDecoder::Ready,this,&AVPlayer::GetDecoderReady);

}

AVPlayer::~AVPlayer(){

}

void AVPlayer::Start(const QString& url){
    mutex.lock();
    if(m_state==START||m_state==STOP){
        mutex.unlock();
        Close();
        mutex.lock();
    }
    m_state=START;
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
    emit m_decoder->resume();
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
    emit m_decoder->stop();
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
    emit m_decoder->exit();
    m_video_player_thread->Close();
    mutex.unlock();
}

void AVPlayer::SetOrder(bool order){
    mutex.lock();
    if(m_order==order){
        mutex.unlock();
        return;
    }
    m_order=order;
    //    emit m_decoder->SetOrder(order);
    mutex.unlock();
}

void AVPlayer::SetSpeed(float speed){
    mutex.lock();
    if(qAbs(m_speed-speed)<0.001){
        mutex.unlock();
        return;
    }
    m_speed=speed;
    m_video_player_thread->SetSpeed(speed);
    mutex.unlock();
}

void AVPlayer::SeekPos(float sec){
    mutex.lock();
    m_video_player_thread->Stop();
    m_decoder->SeekPos(sec);
    m_video_player_thread->Resume();
    mutex.unlock();
}

//slots
void AVPlayer::GetDecoderReady(){
    //接收 m_decoder已初始化完成并启动解码 信号
    m_video_player_thread->Init(m_decoder);
    m_video_player_thread->Start();
}
