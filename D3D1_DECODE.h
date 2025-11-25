// D3D1_MESA_DEMO.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。
#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11_3.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "KMTInterceptor.h"


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if(p){ (p)->Release(); (p)=nullptr; }
#endif

EXTERN_C_START
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
EXTERN_C_END

using Microsoft::WRL::ComPtr;

class FFmpegD3D11Decoder {

public:
    FFmpegD3D11Decoder();
    ~FFmpegD3D11Decoder();

    bool open(const std::string& filename);

    bool decode_loop();

    void cleanup();

private:
    AVFormatContext* fmt_ctx;
    AVCodecContext* dec_ctx;
    int video_stream_idx;
    int width, height;
};

