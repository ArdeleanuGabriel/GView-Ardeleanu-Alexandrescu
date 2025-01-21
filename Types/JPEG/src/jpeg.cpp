#include "jpeg.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace GView::Utils;
using namespace GView::Type;
using namespace GView;
using namespace GView::View;

extern "C"
{
    PLUGIN_EXPORT bool Validate(const AppCUI::Utils::BufferView& buf, const std::string_view& extension)
    {
        if (buf.GetLength() < sizeof(JPEG::Header) + sizeof(JPEG::App0MarkerSegment)) {
            return false;
        }
        auto header = buf.GetObject<JPEG::Header>();
        if (header->soi != JPEG::JPEG_SOI_MARKER || header->app0 != JPEG::JPEG_APP0_MARKER) {
            return false;
        }
        auto app0MarkerSegment = buf.GetObject<JPEG::App0MarkerSegment>(sizeof(JPEG::Header));
        if (memcmp(app0MarkerSegment->identifier, "JFIF", 5) != 0) {
            return false;
        }
        return true;
    }

    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new JPEG::JPEGFile;
    }

    void CreateBufferView(Reference<GView::View::WindowInterface> win, Reference<JPEG::JPEGFile> jpeg)
    {
        BufferViewer::Settings settings;

        const std::vector<ColorPair> colors = { ColorPair{ Color::Teal, Color::DarkBlue }, ColorPair{ Color::Yellow, Color::DarkBlue } };

        auto& data            = jpeg->obj->GetData();
        const uint64 dataSize = data.GetSize();
        uint64 offset         = 0;
        uint32 colorIndex     = 0;
        uint32 segmentCount   = 1;

        settings.AddZone(0, sizeof(JPEG::Header), ColorPair{ Color::Magenta, Color::DarkBlue }, "SOI Segment");
        offset += sizeof(JPEG::Header);

        settings.AddZone(offset, sizeof(JPEG::App0MarkerSegment), ColorPair{ Color::Olive, Color::DarkBlue }, "APP0 Segment");
        offset += sizeof(JPEG::App0MarkerSegment);

        while (offset < dataSize - JPEG::MARKER_SIZE) {
            uint8 marker_prefix;
            uint8 marker_type;

            if (!data.Copy<uint8>(offset, marker_prefix) || marker_prefix != JPEG::JPEG_START_MARKER_BYTE) {
                break;
            }

            if (!data.Copy<uint8>(offset + 1, marker_type) || marker_type == 0x00 || marker_type == JPEG::JPEG_START_MARKER_BYTE) {
                offset++;
                continue;
            }

            if (marker_type == JPEG::JPEG_EOI_BYTE) {
                settings.AddZone(offset, JPEG::MARKER_SIZE, ColorPair{ Color::Magenta, Color::DarkBlue }, "EOI Segment");
                break;
            }

            uint16 length;
            if (!data.Copy<uint16>(offset + JPEG::MARKER_SIZE, length)) {
                offset++;
                continue;
            }

            length = Endian::BigToNative(length);
            uint16 segmentLength = length + JPEG::MARKER_SIZE;

            if (offset + segmentLength > data.GetSize()) {
                offset++;
                continue;
            }

            std::string label = "Marker " + std::to_string(segmentCount);
            settings.AddZone(offset, segmentLength, colors[colorIndex], label.c_str());
            offset += segmentLength;
            colorIndex = (colorIndex + 1) % colors.size();
            segmentCount++;

            if (marker_type == JPEG::JPEG_SOS_MARKER) {
                if (offset + segmentLength < dataSize - JPEG::MARKER_SIZE) {
                    settings.AddZone(offset, dataSize - JPEG::MARKER_SIZE - offset, ColorPair{ Color::Red, Color::DarkBlue }, "Compressed Data");
                    offset = dataSize - JPEG::MARKER_SIZE;
                }
                break;
            }
        }

        if (offset < dataSize - JPEG::MARKER_SIZE) {
            settings.AddZone(offset, dataSize - JPEG::MARKER_SIZE - offset, ColorPair{ Color::Red, Color::DarkBlue }, "Compressed Data");
            offset = dataSize - JPEG::MARKER_SIZE;
        }

        if (offset < dataSize) {
            uint8 byte1, byte2;
            if (data.Copy<uint8>(offset, byte1) && data.Copy<uint8>(offset + 1, byte2)) {
                if ((byte2 << 8 | byte1) == JPEG::JPEG_EOI_MARKER) {
                    settings.AddZone(offset, JPEG::MARKER_SIZE, ColorPair{ Color::Magenta, Color::DarkBlue }, "EOI Segment");
                }
            }
        }

        jpeg->selectionZoneInterface = win->GetSelectionZoneInterfaceFromViewerCreation(settings);
    }

    void CreateImageView(Reference<GView::View::WindowInterface> win, Reference<JPEG::JPEGFile> jpeg)
    {
        GView::View::ImageViewer::Settings settings;
        settings.SetLoadImageCallback(jpeg.ToBase<View::ImageViewer::LoadImageInterface>());
        settings.AddImage(0, jpeg->obj->GetData().GetSize());
        win->CreateViewer(settings);
    }

    PLUGIN_EXPORT bool PopulateWindow(Reference<GView::View::WindowInterface> win)
    {
        auto jpeg = win->GetObject()->GetContentType<JPEG::JPEGFile>();
        jpeg->Update();

        // add viewer
        CreateImageView(win, jpeg);
        CreateBufferView(win, jpeg);

        // add panels
        win->AddPanel(Pointer<TabPage>(new JPEG::Panels::Information(jpeg)), true);

        return true;
    }

    PLUGIN_EXPORT void UpdateSettings(IniSection sect)
    {
        sect["Pattern"]     = "magic:FF D8";
        sect["Priority"]    = 1;
        sect["Description"] = "JPEG image file (*.jpeg)";
    }
}

int main()
{
    return 0;
}
