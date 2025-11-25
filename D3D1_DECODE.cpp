#include "D3D1_DECODE.h"

#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#pragma comment(lib, "d3d11.lib")

static void print_av_error(const char* msg, int err)
{
    char errbuf[128];
    av_strerror(err, errbuf, sizeof(errbuf));
    std::cerr << msg << " error: " << errbuf << std::endl;
}

static enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    std::cerr << "Failed to get HW surface format." << std::endl;
    return AV_PIX_FMT_NONE;
}

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;
    AVBufferRef *hw_device_ctx = NULL;

    AVDictionary* dec_options = NULL;
    av_dict_set(&dec_options, "debug", "1", 0);
    av_dict_set(&dec_options, "v", "debug", 0);
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) {
        std::cerr << "Failed to create specified HW device." << std::endl;
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    av_buffer_unref(&hw_device_ctx);
    return 0;
}

FFmpegD3D11Decoder::FFmpegD3D11Decoder() : fmt_ctx(nullptr), dec_ctx(nullptr), video_stream_idx(-1), width(0), height(0)
{
}
FFmpegD3D11Decoder::~FFmpegD3D11Decoder() { cleanup(); }


bool FFmpegD3D11Decoder::open(const std::string &filename)
{
    int ret = 0;
    av_log_set_level(AV_LOG_ERROR);

    AVDictionary* options = NULL;

    // 设置输入选项
    av_dict_set(&options, "debug", "1", 0);

    if ((ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL)) < 0) {
        print_av_error("avformat_open_input", ret);
        return false;
    }
    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        print_av_error("avformat_find_stream_info", ret);
        return false;
    }

    // find video stream
    for (unsigned i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }
    if (video_stream_idx < 0) {
        std::cerr << "No video stream found." << std::endl;
        return false;
    }

    AVStream *video_stream = fmt_ctx->streams[video_stream_idx];
    const AVCodec *decoder = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!decoder) {
        std::cerr << "Failed to find codec." << std::endl;
        return false;
    }

    dec_ctx = avcodec_alloc_context3(decoder);
    if (!dec_ctx) return false;

    if ((ret = avcodec_parameters_to_context(dec_ctx, video_stream->codecpar)) < 0) {
        print_av_error("avcodec_parameters_to_context", ret);
        return false;
    }

    // try to use d3d11va
    enum AVHWDeviceType type = av_hwdevice_find_type_by_name("d3d11va");
    if (type == AV_HWDEVICE_TYPE_NONE) {
        std::cerr << "Device type d3d11va not found." << std::endl;
    } else {
        // choose supported hw pix fmt
        for (int i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
            if (!config) break;
            if ((config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) && config->device_type == type) {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }

        if (hw_pix_fmt != AV_PIX_FMT_NONE) {
            dec_ctx->get_format = get_hw_format;
            if ((ret = hw_decoder_init(dec_ctx, type)) < 0) {
                print_av_error("hw_decoder_init", ret);
            } else {
                std::cout << "Using hardware decoding: d3d11va" << std::endl;
            }
        }
    }

    // 设置输入选项
    AVDictionary* dec_options = NULL;
    av_dict_set(&dec_options, "debug", "1", 0);
    av_dict_set(&dec_options, "v", "debug", 0);
    if ((ret = avcodec_open2(dec_ctx, decoder, NULL)) < 0) {
        print_av_error("avcodec_open2", ret);
        return false;
    }

    width = dec_ctx->width;
    height = dec_ctx->height;
    return true;
}

bool FFmpegD3D11Decoder::decode_loop()
{
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    AVFrame *sw_frame = av_frame_alloc();
    int ret = 0;

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_idx) {
            ret = avcodec_send_packet(dec_ctx, pkt);
            if (ret < 0) {
                print_av_error("avcodec_send_packet", ret);
                break;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) {
                    print_av_error("avcodec_receive_frame", ret);
                    goto cleanup;
                }

                AVFrame *upload_frame = nullptr;

                if (frame->format == hw_pix_fmt) {
                    // transfer to system memory
                    ret = av_hwframe_transfer_data(sw_frame, frame, 0);
                    if (ret < 0) {
                        print_av_error("av_hwframe_transfer_data", ret);
                        goto cleanup;
                    }
                    upload_frame = sw_frame;
                } else {
                    upload_frame = frame;
                }

                std::cout << "Decoded and uploaded frame." << std::endl;

                av_frame_unref(frame);
                av_frame_unref(sw_frame);
            }
        }
        av_packet_unref(pkt);
    }

cleanup:
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_frame_free(&sw_frame);
    return true;
}

void FFmpegD3D11Decoder::cleanup()
{
    if (dec_ctx) { avcodec_free_context(&dec_ctx); dec_ctx = nullptr; }
    if (fmt_ctx) { avformat_close_input(&fmt_ctx); fmt_ctx = nullptr; }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 启用控制台输出以便调试
    //SetDllDirectoryA(".");
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);


    KMTInterceptor::Initialize();


    avformat_network_init();

    FFmpegD3D11Decoder decoder;
    if (!decoder.open("D:\\AVSYNC.mp4")) {
        std::cerr << "Failed to open decoder." << std::endl;
        return -1;
    }

    decoder.decode_loop();
   
    KMTInterceptor::Shutdown();
    std::cout << "Application exiting..." << std::endl;
    return 0;
}

int main(int argc, char **argv)
{


    std::cout << "Decoding finished." << std::endl;
    return 0;
}
