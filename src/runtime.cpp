#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

struct AVDeleter {
    void operator()(AVFormatContext* p) const {
        if (p) {
            if (p->iformat) {
                avformat_close_input(&p);
            } else {
                avformat_free_context(p);
            }
        }
    }
    void operator()(AVDictionary* p) const {
        if (p) {
            av_dict_free(&p);
        }
    }
    void operator()(AVPacket* p) const {
        if (p) {
            av_packet_free(&p);
        }
    }
};

using FormatContextPtr = std::unique_ptr<AVFormatContext, AVDeleter>;
using DictionaryPtr = std::unique_ptr<AVDictionary, AVDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, AVDeleter>;
