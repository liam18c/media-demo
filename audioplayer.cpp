#include "audioplayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(){}

double AudioPlayer::m_volume=1.0;

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
    /*** 初始化初始化SDL_AudioSpec结构体 ***/
    m_sdl_audio_spec.freq = m_information->sample_rate;
    // 音频数据的格式
    m_sdl_audio_spec.format = AUDIO_S16SYS;
    // 声道数。例如单声道取值为1，立体声取值为2
    m_sdl_audio_spec.channels = m_information->channels;
    // 设置静音的值
    m_sdl_audio_spec.silence = 0;
    // 音频缓冲区中的采样个数，要求必须是2的n次方
    m_sdl_audio_spec.samples = 2048;
    // 填充音频缓冲区的回调函数
    m_sdl_audio_spec.callback = fillAudioBuffer;
    // 用户数据
    m_sdl_audio_spec.userdata=this;

    ret=SDL_OpenAudio(&m_sdl_audio_spec, nullptr);
    if(ret==-1){
        printf("SDL openAudio fail:%s\n",SDL_GetError());
        return;
    }

}



void AudioPlayer::Start(){
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
    m_audio_frame=nullptr;
}

void AudioPlayer::SetPlayMode(int flag){
    m_play_mode=flag;
}

void AudioPlayer::SetVolume(double volume){
    m_volume=volume;
}

void AudioPlayer::fillAudioBuffer(void *userdata, Uint8 * stream, int len)
{
    int size=len;
    SDL_memset(stream, 0, len);
    AudioPlayer* m_audio_player=(AudioPlayer*)userdata;

    uint8_t* data=new uint8_t[len];
    memset(data,0,len);

    if(len==0)return;

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
            if(frame==nullptr)continue;
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
                        ||(m_audio_player->m_play_mode==-1&&frame->pos-frame->duration<=0.1)){
                    //可捕获该信号作为播放结束的标志
                    emit m_audio_player->PlayFinish();
                }
            }
        }
    }

    SDL_MixAudio(stream, data, size, SDL_MIX_MAXVOLUME*m_volume);
}
