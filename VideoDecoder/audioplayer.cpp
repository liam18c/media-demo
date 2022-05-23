#include "audioplayer.h"
#include <QDebug>

double AudioPlayer::m_volume=1.0;

AudioPlayer::AudioPlayer(){}

void AudioPlayer::Init(AVDecoder* decoder){
    this->m_decoder=decoder;
    m_information=m_decoder->GetAVInfomation();
    //初始化缓冲区大小
    m_audio_frame=nullptr;
    m_extra_len=0;

    int ret=0;
    ret=SDL_Init(SDL_INIT_AUDIO);
    if(ret==-1){
        printf("SDL init fail:%s\n",SDL_GetError());
        return;
    }

    m_sdl_audio_spec.freq = m_information->sample_rate;
    m_sdl_audio_spec.format = AUDIO_S16SYS;
    m_sdl_audio_spec.channels = m_information->channels;
    m_sdl_audio_spec.silence = 0;
    m_sdl_audio_spec.samples = 2048;
    m_sdl_audio_spec.callback = fillAudioBuffer;
    m_sdl_audio_spec.userdata=this;

    ret=SDL_OpenAudio(&m_sdl_audio_spec, nullptr);
    if(ret==-1){
        printf("SDL openAudio fail:%s\n",SDL_GetError());
        return;
    }

}

void AudioPlayer::Start(){
    m_play_mode=1;
    m_exit.store(false);
    SDL_PauseAudio(0);
}

void AudioPlayer::Resume(){
    SDL_PauseAudio(0);
}

void AudioPlayer::Stop(){
    SDL_PauseAudio(1);
}

void AudioPlayer::Close(){
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    m_play_mode=1;
    m_exit.store(true);
    m_audio_frame=nullptr;
}

void AudioPlayer::SetPlayMode(int flag){
    m_play_mode=flag;
}

void AudioPlayer::SetVolume(double volume){
    m_volume=volume;
}

double AudioPlayer::GetVolume()
{
    return m_volume;
}

AudioFrame* AudioPlayer::GetCurrentFrame(){
    return m_audio_frame;
}

void AudioPlayer::fillAudioBuffer(void *userdata, Uint8 * stream, int len)
{
    int size=len;
    SDL_memset(stream, 0, len);
    AudioPlayer* m_audio_player=(AudioPlayer*)userdata;

    if(m_audio_player->m_exit.load()){
        return;
    }

    uint8_t* data=new uint8_t[len];
    memset(data,0,len);

    if(m_audio_player->m_extra_len>=len){
        uint8_t* cur_pos=m_audio_player->m_audio_frame->data;
        cur_pos+=m_audio_player->m_audio_frame->out_buffer_size-m_audio_player->m_extra_len;
        memcpy(data,cur_pos,len);
        m_audio_player->m_extra_len-=len;
    }
    else{
        uint8_t* cur_pos;
        if(m_audio_player->m_audio_frame){
            cur_pos=m_audio_player->m_audio_frame->data;
            cur_pos+=m_audio_player->m_audio_frame->out_buffer_size-m_audio_player->m_extra_len;
            memcpy(data,cur_pos,m_audio_player->m_extra_len);
        }
        len-=m_audio_player->m_extra_len;
        cur_pos=data+m_audio_player->m_extra_len;
        m_audio_player->m_extra_len=0;

        while(len>0){
            AudioFrame* frame=m_audio_player->m_decoder->GetAudioFrame();
            if(frame==nullptr){
                continue;
            }
            if(m_audio_player->m_audio_frame){
                delete m_audio_player->m_audio_frame;
            }
            m_audio_player->m_audio_frame=frame;
            uint8_t* src_data=frame->data;
            if(frame->out_buffer_size>=len){
                memcpy(cur_pos,src_data,len);
                m_audio_player->m_extra_len=frame->out_buffer_size-len;
                len=0;
            }
            else{
                memcpy(cur_pos,src_data,frame->out_buffer_size);
                cur_pos+=frame->out_buffer_size;
                len-=frame->out_buffer_size;

            }
            if(m_audio_player->m_information->type&AVType::TYPEAUDIO
                    &&!(m_audio_player->m_information->type&AVType::TYPEVIDEO)){
                if((m_audio_player->m_play_mode==1&&frame->pos+frame->duration>=m_audio_player->m_information->duration-0.1)
                        ||(m_audio_player->m_play_mode==-1&&frame->pos-frame->duration<=1)){
                    delete[] data;
                    m_audio_player->m_exit.store(true);
                    QThread::msleep(10);
                    emit m_audio_player->PlayFinish();
                    return;
                }
            }
        }
    }

    SDL_MixAudio(stream, data, size, SDL_MIX_MAXVOLUME*m_audio_player->m_volume);
}
