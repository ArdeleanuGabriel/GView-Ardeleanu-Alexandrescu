#include "jpeg.hpp" 

using namespace GView::Type::JPEG; 

JPEGFile::JPEGFile()
{
}

bool JPEGFile::Update()
{
    memset(&header, 0, sizeof(header));
    memset(&app0MarkerSegment, 0, sizeof(app0MarkerSegment));
    memset(&sof0MarkerSegment, 0, sizeof(sof0MarkerSegment));

    auto& data = this->obj->GetData();

    CHECK(data.Copy<Header>(0, header), false, "");
    CHECK(data.Copy<App0MarkerSegment>(sizeof(Header), app0MarkerSegment), false, "");

    uint64 offset = sizeof(Header) + sizeof(App0MarkerSegment);
    bool found    = false;
    while (offset < data.GetSize())
    {
        uint16 marker;
        CHECK(data.Copy<uint16>(offset, marker), false, "");
        
        // markers for the JPEG  type
        if (marker == JPEG::JPEG_SOF0_MARKER || marker == JPEG::JPEG_SOF1_MARKER || 
            marker == JPEG::JPEG_SOF2_MARKER || marker == JPEG::JPEG_SOF3_MARKER)
        {
            CHECK(data.Copy<SOF0MarkerSegment>(offset + 5, sof0MarkerSegment), false, "");
            found = true;
            break;
        }
        offset += 1;
    }
    return found;
}

bool JPEGFile::LoadImageToObject(Image& img, uint32 index)
{
    Buffer buf;
    auto bf = obj->GetData().GetEntireFile();
    
    // check  valid buffer / handle
    if (bf.IsValid() == false) {
        buf = this->obj->GetData().CopyEntireFile();
        CHECK(buf.IsValid(), false, "Failed to copy Entire file");
        bf = (BufferView) buf;
    }
    
    // create the image from the data buffer
    CHECK(img.Create(bf), false, "");

    return true;
}
