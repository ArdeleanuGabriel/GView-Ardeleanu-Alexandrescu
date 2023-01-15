#include "zip.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace GView::Utils;
using namespace GView;
using namespace GView::View;

constexpr uint32 PK{ 0x04034B50 };
constexpr uint32 PK_EMPTY{ 0x06054B50 };
constexpr uint32 PK_SPANNED{ 0x08074B50 };

constexpr string_view ZIP_ICON = "................"  // 1
                                 "................"  // 2
                                 "................"  // 3
                                 "................"  // 4
                                 "WWWWW.WW.WWWWWWW"  // 5
                                 "....W.WW.W.....W"  // 6
                                 "....W....W.....W"  // 7
                                 "....W.WW.W.....W"  // 8
                                 "WWWWW.WW.WWWWWWW"  // 9
                                 "W.....WW.W......"  // 10
                                 "W.....WW.W......"  // 11
                                 "WWWWW.WW.W......"  // 12
                                 "................"  // 13
                                 "................"  // 14
                                 "................"  // 15
                                 "................"; // 16

extern "C"
{
    PLUGIN_EXPORT bool Validate(const AppCUI::Utils::BufferView& buf, const std::string_view& extension)
    {
        CHECK(buf.GetLength() >= sizeof(PK), false, "");

        struct Magic
        {
            uint32 value;
        };
        auto magic = buf.GetObject<Magic>(0);
        CHECK(magic.IsValid(), false, "");

        if (magic->value == PK)
        {
            return true;
        }

        if (magic->value == PK_EMPTY)
        {
            return true;
        }

        if (magic->value == PK_SPANNED)
        {
            return true;
        }

        RETURNERROR(false, "Unknown ZIP format/standard!");
    }

    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new GView::Type::ZIP::ZIPFile();
    }

    void CreateBufferView(Reference<GView::View::WindowInterface> win, Reference<GView::Type::ZIP::ZIPFile> zip)
    {
        BufferViewer::Settings settings;
        win->CreateViewer("BufferView", settings);
    }

    void CreateContainerView(Reference<GView::View::WindowInterface> win, Reference<GView::Type::ZIP::ZIPFile> zip)
    {
        ContainerViewer::Settings settings;

        settings.SetIcon(ZIP_ICON);
        settings.SetColumns({
              "n:&Filename,a:l,w:100",
              "n:&Type,a:l,w:20",
              "n:&Compressed Size,a:r,w:20",
              "n:&Uncompressed Size,a:r,w:20",
              "n:&Compression Method,a:r,w:20",
              "n:&Disk Number,a:r,w:20",
              "n:&Disk Offset,a:r,w:20",
        });

        settings.SetEnumerateCallback(
              win->GetObject()->GetContentType<GView::Type::ZIP::ZIPFile>().ToObjectRef<ContainerViewer::EnumerateInterface>());
        settings.SetOpenItemCallback(
              win->GetObject()->GetContentType<GView::Type::ZIP::ZIPFile>().ToObjectRef<ContainerViewer::OpenItemInterface>());

        win->CreateViewer("ContainerViewer", settings);
    }

    PLUGIN_EXPORT bool PopulateWindow(Reference<GView::View::WindowInterface> win)
    {
        auto zip = win->GetObject()->GetContentType<GView::Type::ZIP::ZIPFile>();
        zip->Update();

        // add views
        CreateContainerView(win, zip);
        CreateBufferView(win, zip);

        // add panels
        win->AddPanel(Pointer<TabPage>(new GView::Type::ZIP::Panels::Information(win->GetObject(), zip)), true);
        win->AddPanel(Pointer<TabPage>(new GView::Type::ZIP::Panels::Objects(zip, win)), false);

        return true;
    }

    PLUGIN_EXPORT void UpdateSettings(IniSection sect)
    {
        static const std::initializer_list<std::string> patterns = {
            "magic:50 4b 03 04",
            "magic:50 4b 05 06", // empty archive
            "magic:50 4b 07 08", // spanned archive
        };

        sect["Pattern"]     = patterns;
        sect["Extension"]   = "zip";
        sect["Priority"]    = 1;
        sect["Description"] = "Archive file format (*.zip)";
    }
}
