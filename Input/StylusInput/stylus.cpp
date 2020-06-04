
#include "Input/StylusInput/stylus.h"

#ifdef _WIN32

class FWindowsRealTimeStylusPlugin * g_pRTSHandler = nullptr;
IRealTimeStylus * g_pRTSStylus = nullptr;



void ReleaseRTS()
{
    if( g_pRTSStylus )
    {
        g_pRTSStylus->Release();
        g_pRTSStylus = nullptr;
    }

    if( g_pRTSHandler )
    {
        g_pRTSHandler->Release();
        g_pRTSHandler = nullptr;
    }
}

bool CreateRTS(HWND hWnd, InputManager* input_manager)
{
    // Release, just in case
    ReleaseRTS();

    // Create stylus
    HRESULT hr = CoCreateInstance(CLSID_RealTimeStylus, NULL, CLSCTX_ALL, IID_PPV_ARGS(&g_pRTSStylus));
    if (FAILED(hr))
        return false;

    // Attach RTS object to a window
    hr = g_pRTSStylus->put_HWND((HANDLE_PTR)hWnd);
    if (FAILED(hr))
    {
        ReleaseRTS();
        return false;
    }

    // Create eventhandler
    g_pRTSHandler = new FWindowsRealTimeStylusPlugin(input_manager);

    // Create free-threaded marshaller for this object and aggregate it.
    hr = CoCreateFreeThreadedMarshaler(g_pRTSHandler, &g_pRTSHandler->FreeThreadedMarshaller);
    if (FAILED(hr))
    {
        ReleaseRTS();
        return false;
    }

    // Add handler object to the list of synchronous plugins in the RTS object.
    hr = g_pRTSStylus->AddStylusSyncPlugin(0,g_pRTSHandler);
    if (FAILED(hr))
    {
        ReleaseRTS();
        return false;
    }

    // Set data we want - we're not actually using all of this, but we're gonna get X and Y anyway so might as well set it
    GUID lWantedProps[] = {GUID_PACKETPROPERTY_GUID_X,
                           GUID_PACKETPROPERTY_GUID_Y,
                           GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE,
                           GUID_PACKETPROPERTY_GUID_X_TILT_ORIENTATION,
                           GUID_PACKETPROPERTY_GUID_Y_TILT_ORIENTATION,
                          };
    g_pRTSStylus->SetDesiredPacketDescription(5, lWantedProps);
    g_pRTSStylus->put_Enabled(true);

    return true;
}

//////
/////
///
///

HRESULT FWindowsRealTimeStylusPlugin::QueryInterface(const IID& InterfaceID, void** Pointer)
{
    if ((InterfaceID == __uuidof(IStylusSyncPlugin)) || (InterfaceID == IID_IUnknown))
    {
        *Pointer = this;
        AddRef();
        return S_OK;
    }
    else if ((InterfaceID == IID_IMarshal) && (FreeThreadedMarshaller != nullptr))
    {
        return FreeThreadedMarshaller->QueryInterface(InterfaceID, Pointer);
    }

    *Pointer = nullptr;
    return E_NOINTERFACE;
}

HRESULT FWindowsRealTimeStylusPlugin::StylusDown(IRealTimeStylus*, const StylusInfo* StylusInfo, ULONG, LONG*, LONG**)
{
    FTabletContextInfo* TabletContext = FindTabletContext(StylusInfo->tcid);
    if (TabletContext != nullptr)
    {
        TabletContext->WindowsState.IsTouching = true;
    }

    INPUT_DEVICE device = INPUT_DEVICE::MOUSE;


    inputs[StylusInfo->cid] = std::make_shared<IStroke>(tbb::concurrent_vector<IInputState>(), device);
    m_input_manager->FireEvent(INPUTEVENT::STROKE_START, std::reinterpret_pointer_cast<void*>(inputs[StylusInfo->cid]));
    return S_OK;
}

HRESULT FWindowsRealTimeStylusPlugin::StylusUp(IRealTimeStylus*, const StylusInfo* StylusInfo, ULONG, LONG*, LONG**)
{
    FTabletContextInfo* TabletContext = FindTabletContext(StylusInfo->tcid);
    if (TabletContext != nullptr)
    {
        // we know this is not touching
        TabletContext->WindowsState.IsTouching = false;
        TabletContext->WindowsState.NormalPressure = 0;
    }
    inputs[StylusInfo->cid]->completed.store(true);
    inputs.erase(StylusInfo->cid);

    return S_OK;
}

static void SetupPacketDescriptions(IRealTimeStylus* RealTimeStylus, FTabletContextInfo& TabletContext)
{
    ULONG NumPacketProperties = 0;
    PACKET_PROPERTY* PacketProperties = nullptr;
    HRESULT hr = RealTimeStylus->GetPacketDescriptionData(TabletContext.ID, nullptr, nullptr, &NumPacketProperties, &PacketProperties);
    if (SUCCEEDED(hr) && PacketProperties != nullptr)
    {
        for (ULONG PropIdx = 0; PropIdx < NumPacketProperties; ++PropIdx)
        {
            PACKET_PROPERTY CurrentProperty = PacketProperties[PropIdx];

            EWindowsPacketType PacketType = EWindowsPacketType::None;
            if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_X)
            {
                PacketType = EWindowsPacketType::X;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_Y)
            {
                PacketType = EWindowsPacketType::Y;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_Z)
            {
                PacketType = EWindowsPacketType::Z;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_PACKET_STATUS)
            {
                PacketType = EWindowsPacketType::Status;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE)
            {
                PacketType = EWindowsPacketType::NormalPressure;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_TANGENT_PRESSURE)
            {
                PacketType = EWindowsPacketType::TangentPressure;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_BUTTON_PRESSURE)
            {
                PacketType = EWindowsPacketType::ButtonPressure;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_ALTITUDE_ORIENTATION)
            {
                PacketType = EWindowsPacketType::Altitude;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_AZIMUTH_ORIENTATION)
            {
                PacketType = EWindowsPacketType::Azimuth;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_TWIST_ORIENTATION)
            {
                PacketType = EWindowsPacketType::Twist;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_X_TILT_ORIENTATION)
            {
                PacketType = EWindowsPacketType::XTilt;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_Y_TILT_ORIENTATION)
            {
                PacketType = EWindowsPacketType::YTilt;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_WIDTH)
            {
                PacketType = EWindowsPacketType::Width;
            }
            else if (CurrentProperty.guid == GUID_PACKETPROPERTY_GUID_HEIGHT)
            {
                PacketType = EWindowsPacketType::Height;
            }

            TabletContext.PacketDescriptions.emplace_back();
            FPacketDescription& PacketDescription = TabletContext.PacketDescriptions.back();
            PacketDescription.Type = PacketType;
            PacketDescription.Minimum = CurrentProperty.PropertyMetrics.nLogicalMin;
            PacketDescription.Maximum = CurrentProperty.PropertyMetrics.nLogicalMax;
            PacketDescription.Resolution = CurrentProperty.PropertyMetrics.fResolution;
        }

        ::CoTaskMemFree(PacketProperties);
    }
}

static void SetupTabletSupportedPackets(IRealTimeStylus* RealTimeStylus, FTabletContextInfo& TabletContext)
{
    IInkTablet* InkTablet;
    RealTimeStylus->GetTabletFromTabletContextId(TabletContext.ID, &InkTablet);

    short Supported;

    BSTR GuidBSTR;

    GuidBSTR = SysAllocString(STR_GUID_X);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::X);
        TabletContext.AddSupportedInput(EWindowsPacketType::X);
    }

    SysFreeString(GuidBSTR);
    GuidBSTR = SysAllocString(STR_GUID_Y);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::Y);
        TabletContext.AddSupportedInput(EWindowsPacketType::Y);
    }

    SysFreeString(GuidBSTR);
    GuidBSTR = SysAllocString(STR_GUID_PAKETSTATUS);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::Status);
    }

    SysFreeString(GuidBSTR);
    GuidBSTR = SysAllocString(STR_GUID_NORMALPRESSURE);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::NormalPressure);
        TabletContext.AddSupportedInput(EWindowsPacketType::NormalPressure);
    }

    SysFreeString(GuidBSTR);
    GuidBSTR = SysAllocString(STR_GUID_XTILTORIENTATION);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::XTilt);
        TabletContext.AddSupportedInput(EWindowsPacketType::XTilt);
    }

    SysFreeString(GuidBSTR);
    GuidBSTR = SysAllocString(STR_GUID_YTILTORIENTATION);

    InkTablet->IsPacketPropertySupported(GuidBSTR, &Supported);
    if (Supported)
    {
        TabletContext.SupportedPackets.emplace_back(EWindowsPacketType::YTilt);
        TabletContext.AddSupportedInput(EWindowsPacketType::YTilt);
    }

    SysFreeString(GuidBSTR);
}

FTabletContextInfo* FWindowsRealTimeStylusPlugin::FindTabletContext(TABLET_CONTEXT_ID TabletID)
{
    for (FTabletContextInfo& TabletContext : TabletContexts)
    {
        if (TabletContext.ID == TabletID)
        {
            return &TabletContext;
        }
    }
    return nullptr;
}

void FWindowsRealTimeStylusPlugin::AddTabletContext(IRealTimeStylus* RealTimeStylus, TABLET_CONTEXT_ID TabletID)
{
    FTabletContextInfo* FoundContext = FindTabletContext(TabletID);
    if (FoundContext == nullptr)
    {
        TabletContexts.emplace_back();
        FoundContext = &TabletContexts.back();
        FoundContext->ID = TabletID;
    }

    SetupTabletSupportedPackets(RealTimeStylus, *FoundContext);
    SetupPacketDescriptions(RealTimeStylus, *FoundContext);
}

void FWindowsRealTimeStylusPlugin::RemoveTabletContext(IRealTimeStylus*, TABLET_CONTEXT_ID TabletID)
{
    for (size_t ExistingIdx = 0; ExistingIdx < TabletContexts.size(); ++ExistingIdx)
    {
        if (TabletContexts[ExistingIdx].ID == TabletID)
        {
            TabletContexts.erase(TabletContexts.begin() + ExistingIdx);
            break;
        }
    }
}

HRESULT FWindowsRealTimeStylusPlugin::RealTimeStylusEnabled(IRealTimeStylus* RealTimeStylus, ULONG Num, const TABLET_CONTEXT_ID* InTabletContexts)
{
    for (ULONG TabletIdx = 0; TabletIdx < Num; ++TabletIdx)
    {
        AddTabletContext(RealTimeStylus, InTabletContexts[TabletIdx]);
    }
    return S_OK;
}

HRESULT FWindowsRealTimeStylusPlugin::RealTimeStylusDisabled(IRealTimeStylus* RealTimeStylus, ULONG Num, const TABLET_CONTEXT_ID* InTabletContexts)
{
    for (ULONG TabletIdx = 0; TabletIdx < Num; ++TabletIdx)
    {
        RemoveTabletContext(RealTimeStylus, InTabletContexts[TabletIdx]);
    }
    return S_OK;
}

HRESULT FWindowsRealTimeStylusPlugin::TabletAdded(IRealTimeStylus* RealTimeStylus, IInkTablet* InkTablet)
{
    TABLET_CONTEXT_ID TabletID;
    if (SUCCEEDED(RealTimeStylus->GetTabletContextIdFromTablet(InkTablet, &TabletID)))
    {
        AddTabletContext(RealTimeStylus, TabletID);
    }
    return S_OK;
}

HRESULT FWindowsRealTimeStylusPlugin::TabletRemoved(IRealTimeStylus*, LONG iTabletIndex)
{
    TabletContexts.erase(TabletContexts.begin() + iTabletIndex);
    return S_OK;
}

static float Normalize(int Value, const FPacketDescription& Desc)
{
    return (float) (Value - Desc.Minimum) / (float) (Desc.Maximum - Desc.Minimum);
}

static float ToDegrees(int Value, const FPacketDescription& Desc)
{
    return Value / Desc.Resolution;
}

void FWindowsRealTimeStylusPlugin::HandlePacket(IRealTimeStylus*, const StylusInfo* StylusInfo, ULONG PacketCount, ULONG PacketBufferLength, LONG* Packets)
{
    FTabletContextInfo* TabletContext = FindTabletContext(StylusInfo->tcid);
    if (TabletContext == nullptr)
    {
        return;
    }

    TabletContext->SetDirty();
    TabletContext->WindowsState.IsInverted = StylusInfo->bIsInvertedCursor;

    ULONG PropertyCount = PacketBufferLength / PacketCount;

    for (ULONG i = 0; i < PropertyCount; ++i)
    {
        const FPacketDescription& PacketDescription = TabletContext->PacketDescriptions[i];

        float Normalized = Normalize(Packets[i], PacketDescription);

        switch (PacketDescription.Type)
        {
        case EWindowsPacketType::X:
            TabletContext->WindowsState.X = Packets[i];
            break;
        case EWindowsPacketType::Y:
            TabletContext->WindowsState.Y = Packets[i];
            break;
        case EWindowsPacketType::NormalPressure:
            TabletContext->WindowsState.NormalPressure = Normalized;
            break;
        case EWindowsPacketType::XTilt:
            TabletContext->WindowsState.TiltX = ToDegrees(Packets[i], PacketDescription);
            break;
        case EWindowsPacketType::YTilt:
            TabletContext->WindowsState.TiltY = ToDegrees(Packets[i], PacketDescription);
            break;
        default:
            break;
        }
    }
    HiMetricToPixel(TabletContext);
    //    cir_inputs.push_back(TabletContext->WindowsState);
    if (TabletContext->WindowsState.IsTouching)
    {
        IInputState is(TabletContext->WindowsState.X,
                       TabletContext->WindowsState.Y,
                       TabletContext->WindowsState.NormalPressure,
                       TabletContext->WindowsState.TiltX,
                       TabletContext->WindowsState.TiltY);
        inputs.at(StylusInfo->cid)->stroke.push_back(is);
    }
}

void FWindowsRealTimeStylusPlugin::HiMetricToPixel(FTabletContextInfo* TabletContext)
{
    // Retrieve pixels/inch for screen width
    int xPixelsPerInch = GetDeviceCaps(m_hDC, LOGPIXELSX);

    if (0 == xPixelsPerInch)
    {
        // System call failed, use default
        xPixelsPerInch = 96;
    }

    // Retrieve pixels/inch for screen height from system
    int yPixelsPerInch = GetDeviceCaps(m_hDC, LOGPIXELSY);

    if (0 == yPixelsPerInch)
    {
        // System call failed, use default
        yPixelsPerInch = 96;
    }

    TabletContext->WindowsState.X = TabletContext->WindowsState.X/(2540.0/xPixelsPerInch);
    TabletContext->WindowsState.Y = TabletContext->WindowsState.Y/(2540.0/yPixelsPerInch);
}

HRESULT FWindowsRealTimeStylusPlugin::Packets(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo,
                                              ULONG PacketCount, ULONG PacketBufferLength, LONG* Packets, ULONG*, LONG**)
{
    HandlePacket(RealTimeStylus, StylusInfo, PacketCount, PacketBufferLength, Packets);
    return S_OK;
}

HRESULT FWindowsRealTimeStylusPlugin::InAirPackets(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo,
                                                   ULONG PacketCount, ULONG PacketBufferLength, LONG* Packets, ULONG*, LONG**)
{
    HandlePacket(RealTimeStylus, StylusInfo, PacketCount, PacketBufferLength, Packets);
    return S_OK;
}
#endif
