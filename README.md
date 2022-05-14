# media-demo



以老岑test1.1版本（已同步）为基础

实现了视频播放与音频播放的基础功能

包括倍速、快进某一位置、倒放（出错待完善）、经过测试基本没有问题

在avplayer中进行了基本封装

前端可完成以下调用

- ```
  void Start(const QString& url,void*winId)
  ```

  指定播放的文件地址与绑定的label的winId

- ```
  void Resume()
  ```

  暂停后从当前位置重启，仅针对播放过程，解码过程根据队列长度独立控制

- ```
  void Stop();
  ```

  暂停播放，仅针对播放过程

- ```
  void Close();
  ```

  主动关闭视频播放，对于播放完成则内部自动调用该函数，不需要主动关闭

- ```
  void SetPlayMode(int flag)
  ```

  设置播放的模式，即正反，flag分别取1或-1

- ```
  void SetPlaySpeed(double speed)
  ```

  设置播放速度，取值0~4

- ```c++
  void SetPos(double sec)
  ```

  设置播放位置，sec以秒作单位

- ```c++
  void SetVolume(double volume)
  ```

  设置音量范围 0~1

- ```c++
  AVInfomation* GetAVInformation()
  ```

  获取当前音视频信息

- ```c++
  VideoFrame* GetCurrentFrame()
  ```

  获取当前播放帧的信息

