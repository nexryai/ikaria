#pragma once
#include <memory>

struct AVFormatContext;
struct AVDictionary;
struct AVPacket;

struct AVDeleter {
    void operator()(AVFormatContext* p) const;
    void operator()(AVDictionary* p) const;
    void operator()(AVPacket* p) const;
};

using FormatContextPtr = std::unique_ptr<AVFormatContext, AVDeleter>;
using DictionaryPtr = std::unique_ptr<AVDictionary, AVDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, AVDeleter>;
