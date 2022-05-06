#include "audioplayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(){}

void AudioPlayer::Init(AVDecoder* decoder){
    this->m_decoder=decoder;
    m_information=m_decoder->GetAVInfomation();
    //初始化缓冲区大小
    m_extra_data=(uint8_t*)malloc(1024*8);
    memset(m_extra_data,0,8*1024);
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
    free(m_extra_data);
    m_extra_data=nullptr;
    SDL_CloseAudio();
}


void AudioPlayer::fillAudioBuffer(void *userdata, Uint8 * stream, int len)
{
    int size=len;
    SDL_memset(stream, 0, len);
    AudioPlayer* m_audio_player=(AudioPlayer*)userdata;

    uint8_t* data=new uint8_t[len];
    memset(data,0,len);

    uint8_t* cur_pos=data;

    if(m_audio_player->m_extra_len<=len){
        memcpy(cur_pos,m_audio_player->m_extra_data,m_audio_player->m_extra_len);
        cur_pos=cur_pos+m_audio_player->m_extra_len;
        len-=m_audio_player->m_extra_len;
        m_audio_player->m_extra_len=0;
    }
    else{
        memcpy(cur_pos,m_audio_player->m_extra_data,len);
        cur_pos=cur_pos+len;
        m_audio_player->m_extra_data-=len;
        uint8_t* temp=new uint8_t[m_audio_player->m_extra_len-len];
        memcpy(temp,m_audio_player->m_extra_data+len,m_audio_player->m_extra_len);
        memcpy(m_audio_player->m_extra_data,temp,m_audio_player->m_extra_len);
        delete[] temp;
        len=0;
    }



    while(len>0){
            AudioFrame* audio_frame=m_audio_player->m_decoder->GetAudioFrame();
            if(audio_frame==nullptr)continue;
            uint8_t* audio_data=audio_frame->data;

            if(audio_frame->out_buffer_size>=len){
                memcpy(cur_pos,audio_data,len);
                audio_data+=len;
                m_audio_player->m_extra_len=audio_frame->out_buffer_size-len;
                memcpy(m_audio_player->m_extra_data,audio_data,m_audio_player->m_extra_len);
                len=0;
            }
            else{
                memcpy(cur_pos,audio_data,audio_frame->out_buffer_size);
                cur_pos+=audio_frame->out_buffer_size;
                len-=audio_frame->out_buffer_size;
            }
    }

    SDL_MixAudio(stream, data, size, SDL_MIX_MAXVOLUME);
}

void AudioPlayer::release(){

}
