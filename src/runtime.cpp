#include "runtime.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

void AVDeleter::operator()(AVFormatContext* p) const {
    if (p) {
        if (p->iformat) {
            avformat_close_input(&p);
        } else {
            avformat_free_context(p);
        }
    }
}

void AVDeleter::operator()(AVDictionary* p) const {
    if (p) {
        av_dict_free(&p);
    }
}

void AVDeleter::operator()(AVPacket* p) const {
    if (p) {
        av_packet_free(&p);
    }
}
