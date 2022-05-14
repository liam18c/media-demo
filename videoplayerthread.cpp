#include "videoplayerthread.h"


VideoPlayerThread::VideoPlayerThread(){
    m_audio_player=new AudioPlayer();
}

void VideoPlayerThread::Init(AVDecoder* decoder,void* winId){
    this->m_decoder=decoder;
    m_audio_player->Init(decoder);
    m_information=m_decoder->GetAVInfomation();
    m_videoFrame=nullptr;
    int ret=0;
    ret=SDL_Init(SDL_INIT_VIDEO);
    if(ret==-1){
        printf("SDL init fail:%s\n",SDL_GetError());
        return;
    }
    m_sdl_window=SDL_CreateWindowFrom(winId);
//    m_sdl_window=SDL_CreateWindow("PLAYER",SDL_WINDOWPOS_CENTERED,
//                                  SDL_WINDOWPOS_CENTERED,m_information->width,m_information->height,SDL_WINDOW_RESIZABLE);
    SDL_ShowWindow(m_sdl_window);
    if(m_sdl_window==nullptr){
        printf("fail to create window\n");
        return;
    }
    m_sdl_render = SDL_CreateRenderer(m_sdl_window, -1, 0);
    if (m_sdl_render == NULL){
      printf("fail to create render:%s\n",SDL_GetError());
      return;
    }
    m_sdl_texture = SDL_CreateTexture(m_sdl_render,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,m_information->width,m_information->height);
    if (m_sdl_texture == NULL){
      printf("fail to create texture:%s\n",SDL_GetError());
      return;
    }
    int width,height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);
    m_sdl_rect.x = 0;
    m_sdl_rect.y = 0;
    m_sdl_rect.w = width;
    m_sdl_rect.h = height;
}



void VideoPlayerThread::Start(){
    m_exit.store(false);
    m_stop.store(false);
    m_play_mode=1;
    this->start();
    m_audio_player->Start();
}

void VideoPlayerThread::Resume(){
    m_stop.store(false);
    m_audio_player->Resume();
}

void VideoPlayerThread::Stop(){
    m_stop.store(true);
    m_audio_player->Stop();
}

void VideoPlayerThread::Close(){
    m_play_mode=1;
    m_stop.store(true);
    m_exit.store(true);
    m_audio_player->Close();
    release();
}

void VideoPlayerThread::SetPlayMode(int flag){
    m_play_mode=flag;
}

VideoFrame* VideoPlayerThread::GetCurrentFrame(){
    return m_videoFrame;
}

void VideoPlayerThread::sdlResize(){
    int width,height;
    SDL_GetWindowSize(m_sdl_window, &width, &height);
    if(m_sdl_rect.w!=width||m_sdl_rect.h!=height){
        SDL_DestroyRenderer(m_sdl_render);
        m_sdl_render = SDL_CreateRenderer(m_sdl_window, -1, 0);
        SDL_DestroyTexture(m_sdl_texture);
        m_sdl_texture=SDL_CreateTexture(m_sdl_render,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,m_information->width,m_information->height);
        m_sdl_rect.w=width;
        m_sdl_rect.h=height;
        int ret=SDL_RenderSetViewport(m_sdl_render, &m_sdl_rect);
        printf("%d %d %d\n",width,height,ret);
    }
}

void VideoPlayerThread::run(){
    while(!m_exit.load()){
        if(!m_stop.load()){
            m_mutex.lock();
            //读取rgb数据
            VideoFrame* videoFrame=m_decoder->GetVideoFrame();
            if(videoFrame==nullptr){
                m_mutex.unlock();
                continue;
            }
            delete m_videoFrame;
            m_videoFrame=videoFrame;
            //更新和播放
            sdlResize();
            SDL_UpdateTexture(m_sdl_texture, NULL, videoFrame->data, (videoFrame->width)*3);
            SDL_RenderClear(m_sdl_render);
            SDL_RenderCopy(m_sdl_render, m_sdl_texture, NULL, &m_sdl_rect);
            SDL_RenderPresent(m_sdl_render);
            SDL_Delay(1000*videoFrame->duration);
            m_mutex.unlock();
            if((m_play_mode==1&&videoFrame->pos+videoFrame->duration>=m_information->duration-0.1)
                    ||(m_play_mode==-1&&videoFrame->pos-videoFrame->duration<=0.1)){
                //可捕获该信号作为播放结束的标志
                emit PlayFinish();
            }
        }
        else{
            QThread::msleep(1);
        }
    }
}

void VideoPlayerThread::release(){
    m_mutex.lock();
    if(m_sdl_texture){
        SDL_DestroyTexture(m_sdl_texture);
        m_sdl_texture=nullptr;
    }
    if(m_sdl_render){
        SDL_DestroyRenderer(m_sdl_render);
        m_sdl_render=nullptr;
    }
    if(m_sdl_window){
        SDL_DestroyWindow(m_sdl_window);
        m_sdl_window=nullptr;
    }
    m_mutex.unlock();
}
