#include "jpeg.hpp"

using namespace GView::Type::JPEG; 
using namespace AppCUI::Controls;

Panels::Information::Information(Reference<GView::Type::JPEG::JPEGFile> _jpeg) : TabPage("&Information") 
{
    jpeg     = _jpeg;
    general = Factory::ListView::Create(this, "x:0,y:0,w:100%,h:10", { "n:Field,w:12", "n:Value,w:100" }, ListViewFlags::None);

    issues = Factory::ListView::Create(this, "x:0,y:21,w:100%,h:10", { "n:Info,w:200" }, ListViewFlags::HideColumns);

    this->Update();
}

void Panels::Information::UpdateGeneralInformation()
{
    LocalString<256> tempStr;
    NumericFormatter n;

    general->DeleteAllItems();
    general->AddItem("File");
    
    // size info
    general->AddItem({ "Size", tempStr.Format("%s bytes", n.ToString(jpeg->obj->GetData().GetSize(), { NumericFormatFlags::None, 10, 3, ',' }).data()) });

    // dimensions 
    const auto width  = Endian::BigToNative(jpeg->sof0MarkerSegment.width);  
    general->AddItem({ "Dimensions", tempStr.Format("%u x %u", width, height) });

    // extras
    general->AddItem({ "Density Units", tempStr.Format("%u", jpeg->app0MarkerSegment.densityUnits) });  

    const auto xDensity = Endian::BigToNative(jpeg->app0MarkerSegment.xDensity);  
    const auto yDensity = Endian::BigToNative(jpeg->app0MarkerSegment.yDensity);  
    general->AddItem({ "X Density", tempStr.Format("%u", xDensity) }); 
    general->AddItem({ "Y Density", tempStr.Format("%u", yDensity) }); 

    general->AddItem({ "X Thumbnail", tempStr.Format("%u", jpeg->app0MarkerSegment.xThumbnail) }); 
    general->AddItem({ "Y Thumbnail", tempStr.Format("%u", jpeg->app0MarkerSegment.yThumbnail) }); 
}

void Panels::Information::UpdateIssues()
{
}

void Panels::Information::RecomputePanelsPositions()
{
    int py = 0;
    int w  = this->GetWidth();
    int h  = this->GetHeight();

    if ((!general.IsValid()) || (!issues.IsValid())) {
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
