#include "png.hpp"
using namespace GView::Type::PNG;

PNGFile::PNGFile()
{
}

bool PNGFile::Update()
{
    memset(&header, 0, sizeof(header));

    IHDR_chunk = {};
    my_chunks.clear();

    auto& data = this->obj->GetData();

    CHECK(data.Copy<png_header>(0, header), false, "Bad header");
    CHECK(header.magic == png_sig, false, "Bad magic");

    uint64 offset = sizeof(png_header);

    while (offset < data.GetSize()) 
    {
        png_chunk chunk;
        CHECK(data.Copy<uint32>(offset, chunk.length), false, "Bad chunk length");
        chunk.length = Endian::BigToNative(chunk.length);
        CHECK(data.Copy<uint32>(offset + 4, chunk.type), false, "Bad chunk type");

        if (chunk.length > 0) {
            chunk.data.resize(chunk.length);
            auto buffer = data.Get(offset + 8, chunk.length, true);
            if (!buffer.IsValid())
                return false;
            memcpy(chunk.data.data(), buffer.GetData(), chunk.length);
        }

        CHECK(data.Copy<uint32>(offset + 8 + chunk.length, chunk.crc), false, "Bad chunk CRC");

        if (chunk.IsType(chunk_IHDR)) {
            CHECK(chunk.data.size() >= sizeof(IHDR_chunk), false, "Bad IDHR chunk size");
            auto buffer = data.Get(offset + 8, chunk.length, true);
            memcpy(&IHDR_chunk.width, buffer.GetData(), 4);
            memcpy(&IHDR_chunk.height, buffer.GetData() + 4, 4);
            memcpy(&IHDR_chunk.depth, buffer.GetData() + 8, 1);
            memcpy(&IHDR_chunk.coloring, buffer.GetData() + 9, 1);
            memcpy(&IHDR_chunk.compression, buffer.GetData() + 10, 1);
            memcpy(&IHDR_chunk.filter_type, buffer.GetData() + 11, 1);
            memcpy(&IHDR_chunk.interlace_method, buffer.GetData() + 12, 1);
            
            IHDR_chunk.width  = Endian::BigToNative(IHDR_chunk.width);
            IHDR_chunk.height = Endian::BigToNative(IHDR_chunk.height);
        }

        my_chunks.push_back(chunk);

        if (chunk.IsType(chunk_IEND)) 
        {
            break;
        }
        offset += 12 + chunk.length;
    }

    bool hasIHDR = std::any_of(my_chunks.begin(), my_chunks.end(), [](const png_chunk& c) { return c.IsType(chunk_IHDR); });
    bool hasIEND = std::any_of(my_chunks.begin(), my_chunks.end(), [](const png_chunk& c) { return c.IsType(chunk_IEND); });
    CHECK(hasIHDR && hasIEND, false, "No IHDR or IEND chunk");

    return true;
}

bool PNGFile::LoadImageToObject(Image& img, uint32 index)
{
    Buffer buf;
    auto bf = obj->GetData().GetEntireFile();
    if (!bf.IsValid()) {
        buf = this->obj->GetData().CopyEntireFile();
        CHECK(buf.IsValid(), false, "Failed to copy the entire file");
        bf = (BufferView) buf;
    }

    CHECK(img.Create(bf), false, "Cant create image from data");

    return true;
}