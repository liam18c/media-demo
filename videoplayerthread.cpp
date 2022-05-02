#include "videoplayerthread.h"

int WIDTH=944;
int HEIGHT=728;

VideoPlayerThread::VideoPlayerThread(){}

void VideoPlayerThread::Init(AVDecoder* decoder){
    this->m_decoder=decoder;
    int ret=0;
    ret=SDL_Init(SDL_INIT_VIDEO);
    if(ret==-1){
        printf("SDL init fail:%s\n",SDL_GetError());
        return;
    }
    m_sdl_window=SDL_CreateWindow("PLAYER",SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,WIDTH,HEIGHT,SDL_WINDOW_RESIZABLE);
    if(m_sdl_window==nullptr){
        printf("fail to create window\n");
        return;
    }
    m_sdl_render = SDL_CreateRenderer(m_sdl_window, -1, 0);
    if (m_sdl_render == NULL){
      printf("fail to create render:%s\n",SDL_GetError());
      return;
    }
    m_sdl_texture = SDL_CreateTexture(m_sdl_render,SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING,WIDTH,HEIGHT);
    if (m_sdl_texture == NULL){
      printf("fail to create texture:%s\n",SDL_GetError());
      return;
    }
    m_sdl_rect.x = 0;
    m_sdl_rect.y = 0;
    m_sdl_rect.w = WIDTH;
    m_sdl_rect.h = HEIGHT;
}



void VideoPlayerThread::Start(){
    this->start();
}

void VideoPlayerThread::Resume(){
    stop_mutex.unlock();
}

void VideoPlayerThread::Stop(){
    stop_mutex.lock();
}

void VideoPlayerThread::Close(){
    m_exit.store(true);
}

void VideoPlayerThread::SetSpeed(float speed){

}

//int thread_exit=0;
//int  fresh(void *data){

//  while (thread_exit==0){

//    SDL_Event event;
//    event.type  = SDL_DISPLAYEVENT;
//    SDL_PushEvent(&event);
//    printf("sending\n");
//    SDL_Delay(40);
//  }
//  return 0;
//}

void VideoPlayerThread::run(){
    m_exit.store(false);
    while(!m_exit.load()){
      stop_mutex.lock();
      if(m_decoder->HasFrame()) {
        //读取rgb数据
        VideoFrame* videoFrame=m_decoder->GetVideoFrame();
        printf("%d %d \n",videoFrame->width,videoFrame->height);
        //更新和播放
        SDL_UpdateTexture(m_sdl_texture, NULL, videoFrame->data, (videoFrame->width)*3);
        SDL_RenderClear(m_sdl_render);
        SDL_RenderCopy(m_sdl_render, m_sdl_texture, NULL, &m_sdl_rect);
        SDL_RenderPresent(m_sdl_render);
        SDL_Delay(40);
      }
      stop_mutex.unlock();
    }
    m_exit.store(true);
    release();
}

void VideoPlayerThread::release(){

}
