#pragma once

#include "GView.hpp"

namespace GView
{
namespace Type
{
    namespace JPEG
    {
#pragma pack(push, 2)

        constexpr uint16 JPEG_SOI_MARKER  = 0xD8FF;
        constexpr uint16 JPEG_EOI_MARKER  = 0xD9FF;
        constexpr uint16 JPEG_APP0_MARKER = 0xE0FF;
        constexpr uint16 JPEG_SOF0_MARKER = 0xC0FF;
        constexpr uint16 JPEG_SOF2_MARKER = 0xC2FF;
        constexpr uint16 JPEG_DHT_MARKER  = 0xC4FF;
        constexpr uint16 JPEG_DQT_MARKER  = 0xDBFF;
        constexpr uint16 JPEG_DRI_MARKER  = 0xDDFF;
        constexpr uint16 JPEG_SOS_MARKER  = 0xDAFF;
        constexpr uint16 JPEG_COM_MARKER  = 0xFEFF;
        constexpr uint8 JPEG_START_MARKER_BYTE = 0xFF;
        constexpr uint8 JPEG_RST_BASE_BYTE     = 0xD0;
        constexpr uint8 MARKER_SIZE = 2;

        struct Header {
            uint16 soi;  // Start of Image marker
            uint16 app0; // APP0 marker
        };

        struct App0MarkerSegment {
            uint16 length;
            char identifier[5]; // "JFIF" null-terminated
            uint8 version[2];
            uint8 densityUnits;
            uint16 xDensity;
            uint16 yDensity;
            uint8 xThumbnail;
            uint8 yThumbnail;
        };

        struct SOF0MarkerSegment {
            uint16 height;
            uint16 width;
        };

#pragma pack(pop) // Back to default packing

        class JPEGFile : public TypeInterface, public View::ImageViewer::LoadImageInterface
        {
          public:
            Header header{};
            App0MarkerSegment app0MarkerSegment{};
            SOF0MarkerSegment sof0MarkerSegment{};

            Reference<GView::Utils::SelectionZoneInterface> selectionZoneInterface;

          public:
            JPEGFile();
            virtual ~JPEGFile() {}

            bool Update();

            std::string_view GetTypeName() override
            {
                return "JPEG";
            }

            void RunCommand(std::string_view) override {}

            bool LoadImageToObject(Image& img, uint32 index) override;

            uint32 GetSelectionZonesCount() override
            {
                CHECK(selectionZoneInterface.IsValid(), 0, "");
                return selectionZoneInterface->GetSelectionZonesCount();
            }

            TypeInterface::SelectionZone GetSelectionZone(uint32 index) override
            {
                static auto d = TypeInterface::SelectionZone{ 0, 0 };
                CHECK(selectionZoneInterface.IsValid(), d, "");
                CHECK(index < selectionZoneInterface->GetSelectionZonesCount(), d, "");

                return selectionZoneInterface->GetSelectionZone(index);
            }

            bool UpdateKeys(KeyboardControlsInterface* interface) override
            {
                return true;
            }
        };

        namespace Panels
        {
            class Information : public AppCUI::Controls::TabPage
            {
                Reference<GView::Type::JPEG::JPEGFile> jpeg;
                Reference<AppCUI::Controls::ListView> general;
                Reference<AppCUI::Controls::ListView> issues;

                void UpdateGeneralInformation();
                void UpdateIssues();
                void RecomputePanelsPositions();

              public:
                Information(Reference<GView::Type::JPEG::JPEGFile> jpeg);

                void Update();
                virtual void OnAfterResize(int newWidth, int newHeight) override
                {
                    RecomputePanelsPositions();
                }
            };
        }; // namespace Panels
    }      // namespace JPEG
} // namespace Type
} // namespace GView
