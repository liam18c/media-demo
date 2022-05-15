#include "avplayer.h"

AVPlayer::AVPlayer()
{
    mutex.lock();
    m_state=CLOSE;
    m_av_type=AVType::TYPENONE;
    m_play_mode=1;
    m_play_speed=1.0;
    mutex.unlock();

    m_decoder=AVDecoder::GetInstance();
    m_video_player_thread=new VideoPlayerThread();
    m_audio_player=new AudioPlayer();

    connect(m_decoder,&AVDecoder::Ready,this,&AVPlayer::GetDecoderReady);
    connect(m_video_player_thread,&VideoPlayerThread::PlayFinish,this,&AVPlayer::GetPlayFinish);
    connect(m_audio_player,&AudioPlayer::PlayFinish,this,&AVPlayer::GetPlayFinish);
    connect(m_video_player_thread,&VideoPlayerThread::VideoPositionChange,this,[&](qint64 pos){
        emit this->VideoPositionChange(pos);
    });
    connect(m_decoder,&AVDecoder::urlError,this,[&](){
        emit this->urlError();
    });
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
    m_play_mode=1;
    m_play_speed=1.0;
    m_av_type=AVType::TYPENONE;
    m_winId=winId;
    m_decoder->Open(url);
    emit PlayStateChange(m_state);
    mutex.unlock();
}

void AVPlayer::Resume(){
    mutex.lock();
    if(m_state!=STOP){
        mutex.unlock();
        return;
    }
    m_state=START;
    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->Resume();
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->Resume();
    }
    emit PlayStateChange(m_state);
    mutex.unlock();
}

void AVPlayer::Stop(){
    mutex.lock();
    if(m_state!=START){
        mutex.unlock();
        return;
    }
    m_state=STOP;
    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->Stop();
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->Stop();
    }
    emit PlayStateChange(m_state);
    mutex.unlock();
}

void AVPlayer::Close(){
    mutex.lock();
    if(m_state==CLOSE){
        mutex.unlock();
        return;
    }

    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->Close();
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->Close();
    }
    m_decoder->Close();
    m_state=CLOSE;
    m_play_mode=1;
    m_play_speed=1.0;
    m_av_type=AVType::TYPENONE;
    mutex.unlock();
    emit PlayStateChange(m_state);
}

void AVPlayer::SetPlayMode(int flag){
    mutex.lock();
    if(m_play_mode==flag){
        mutex.unlock();
        return;
    }
    m_play_mode=flag;
    if(m_av_type&AVType::TYPEVIDEO){
        VideoFrame* frame=m_video_player_thread->GetCurrentFrame();
        if(!frame){
            mutex.unlock();
            return;
        }
        m_decoder->SetPlayMode(flag);
        m_decoder->SetPos(frame->pos,0);
    }
    else if(m_av_type&AVType::TYPEAUDIO){
        AudioFrame* frame=m_audio_player->GetCurrentFrame();
        if(!frame){
            mutex.unlock();
            return;
        }
        m_decoder->SetPlayMode(flag);
        m_decoder->SetPos(frame->pos,0);
    }

    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->SetPlayMode(flag);
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->SetPlayMode(flag);
    }
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

void AVPlayer::SetVolume(double volume){
    mutex.lock();
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->SetVolume(volume);
    }
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

double AVPlayer::GetCurrentPos(){
    mutex.lock();
    if(m_state==CLOSE){
        mutex.unlock();
        return 0;
    }
    double ret=0;
    if(m_av_type&AVType::TYPEVIDEO){
        VideoFrame* videoFrame=m_video_player_thread->GetCurrentFrame();
        if(videoFrame){
            ret=videoFrame->pos;
        }
    }
    else if(m_av_type&AVType::TYPEAUDIO){
        AudioFrame* audioFrame=m_audio_player->GetCurrentFrame();
        if(audioFrame){
            ret=audioFrame->pos;
        }
    }
    mutex.unlock();
    return ret;
}

AVPlayer::AVPlayerState AVPlayer::GetPlayState() const
{
    return m_state;
}

//slots
void AVPlayer::GetDecoderReady(){
    //接收 m_decoder已启动解码 信号
    m_av_type|=m_decoder->GetAVInfomation()->type;
    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->Init(m_decoder,m_winId);
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->Init(m_decoder);
    }
    if(m_av_type&AVType::TYPEVIDEO){
        m_video_player_thread->Start();
    }
    if(m_av_type&AVType::TYPEAUDIO){
        m_audio_player->Start();
    }
    //可捕获该信号作为播放开始的标志
    emit PlayStart();
}

void AVPlayer::GetPlayFinish(){
    this->Close();
    //可捕获该信号作为播放结束的标志
    emit PlayFinish();
}
