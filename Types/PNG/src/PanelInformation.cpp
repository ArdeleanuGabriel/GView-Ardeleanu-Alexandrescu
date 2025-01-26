#include "png.hpp"
#include <iostream>
using namespace GView::Type::PNG;
using namespace AppCUI::Controls;

Panels::Information::Information(Reference<GView::Type::PNG::PNGFile> _png) : TabPage("&Information")
{
    png     = _png;
    general = Factory::ListView::Create(this, "x:0,y:0,w:100%,h:10", { "n:Field,w:12", "n:Value,w:100" }, ListViewFlags::None);

    issues  = Factory::ListView::Create(this, "x:0,y:21,w:100%,h:10", { "n:Info,w:200" }, ListViewFlags::HideColumns);

    this->Update();
}

void Panels::Information::UpdateGeneralInformation()
{
    LocalString<256> tempStr;
    NumericFormatter n;

    general->DeleteAllItems();
    general->AddItem("File");
    // size
    general->AddItem({ "Size", tempStr.Format("%s bytes", n.ToString(png->obj->GetData().GetSize(), { NumericFormatFlags::None, 10, 3, ',' }).data()) });
    general->AddItem({ "IHDR_chunk Size", tempStr.Format("%d bytes", sizeof(IHDR_chunk)) });
    // size
    general->AddItem({ "Size", tempStr.Format("%u x %u", png->IHDR_chunk.width, png->IHDR_chunk.height) });

    //extra info
    general->AddItem({ "Bit Depth", tempStr.Format("%u", png->IHDR_chunk.depth) });
    general->AddItem({ "Color Type", tempStr.Format("%u", png->IHDR_chunk.coloring) });

    general->AddItem({ "Compression Method", tempStr.Format("%u", png->IHDR_chunk.compression) });
    general->AddItem({ "Filter Method", tempStr.Format("%u", png->IHDR_chunk.filter_type) });
    general->AddItem({ "Interlace Method", tempStr.Format("%u", png->IHDR_chunk.interlace_method) });
}

void Panels::Information::UpdateIssues()
{
}

void Panels::Information::RecomputePanelsPositions()
{
    int py = 0;
    int w  = this->GetWidth();
    int h  = this->GetHeight();

    if ((!general.IsValid()) || (!issues.IsValid())){
        return;
    }

    issues->SetVisible(false);
    this->general->Resize(w, h);
}


void Panels::Information::Update()
{
    UpdateGeneralInformation();
    UpdateIssues();
    RecomputePanelsPositions();
}