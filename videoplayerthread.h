#ifndef VIDEOPLAYERTHREAD_H
#define VIDEOPLAYERTHREAD_H

#include <QThread>
#include "avdecoder.h"
#include "audioplayerthread.h"

#include <SDL.h>


class VideoPlayerThread : public QThread
{
    Q_OBJECT
public:
    VideoPlayerThread();
    void Init(AVDecoder*);

    void Start();
    void Resume();
    void Stop();
    void Close();
    void SetSpeed(float);

protected:
    void run() override;

private:
    void release();

private:
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_render;
    SDL_Texture* m_sdl_texture;
    SDL_Rect m_sdl_rect;

    QMutex stop_mutex;
    std::atomic_bool m_exit = false;

    AVDecoder* m_decoder;
};

#endif // VIDEOPLAYERTHREAD_H
