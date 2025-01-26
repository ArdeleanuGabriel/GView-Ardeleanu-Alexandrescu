#include "png.hpp"
#include <iostream>

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
        if (buf.GetLength() < sizeof(uint32)) 
        {
            return false;
        }
        return true;
    }

    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new PNG::PNGFile;
    }

    void CreateBufferView(Reference<GView::View::WindowInterface> win, Reference<PNG::PNGFile> png)
    {
        BufferViewer::Settings settings;

        auto& data            = png->obj->GetData();
        const uint64 dataSize = data.GetSize();

        settings.AddZone(0, 8, ColorPair{ Color::Magenta, Color::DarkBlue }, "PNG Signature");
        uint64 offset = 8;

        while (offset + 12 <= dataSize)
        {
            uint32 chunkLength, chunkType;
            if (!data.Copy(offset, chunkLength) || !data.Copy(offset + 4, chunkType))
                break;

            chunkLength            = Endian::BigToNative(chunkLength);
            std::string chunkLabel = "Chunk: " + std::string(reinterpret_cast<char*>(&chunkType), 4);

            settings.AddZone(offset, 8 + chunkLength + 4, ColorPair{ Color::Yellow, Color::DarkBlue }, chunkLabel.c_str());
            offset += 8 + chunkLength + 4;
        }

        png->SetSelectionZoneInterface(win->GetSelectionZoneInterfaceFromViewerCreation(settings));
    }

    void CreateImageView(Reference<GView::View::WindowInterface> win, Reference<PNG::PNGFile> png)
    {
        GView::View::ImageViewer::Settings settings;
        settings.SetLoadImageCallback(png.ToBase<View::ImageViewer::LoadImageInterface>());
        settings.AddImage(0, win->GetObject()->GetData().GetSize());
        win->CreateViewer(settings);
    }

    PLUGIN_EXPORT bool PopulateWindow(Reference<GView::View::WindowInterface> win)
    {
        auto png = win->GetObject()->GetContentType<PNG::PNGFile>();
        png->Update();

        CreateImageView(win, png);
        CreateBufferView(win, png);
        win->AddPanel(Pointer<TabPage>(new PNG::Panels::Information(png)), true);

        return true;
    }

    PLUGIN_EXPORT void UpdateSettings(IniSection sect)
    {
        sect["Pattern"] = "magic:89 50 4E 47";
        sect["Priority"] = 1;
        sect["Description"] = "PNG image file (*.png)";
    }
}

int main()
{
    return 0;
}