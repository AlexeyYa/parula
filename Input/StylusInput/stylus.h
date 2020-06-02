
#ifndef STYLUS_H
#define STYLUS_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"
#include "Input/input_manager.h"

#include <Ole2.h>
#include <RTSCOM.h>
#include <RTSCOM_i.c>

#include <tbb/concurrent_vector.h>
#include <memory>
#include <map>

struct StylusState : public IInputState
{
//    float Z;
//    float Twist;
//    float TangentPressure;
    bool IsTouching : 1;
    bool IsInverted : 1;

    StylusState() :
        IInputState(0, 0, 0, 0, 0), IsTouching(false), IsInverted(false)
    {}

    StylusState(float X, float Y, float TiltX, float TiltY, float NormalPressure, bool IsTouching, bool IsInverted) :
        IInputState(X, Y, TiltX, TiltY, NormalPressure),
        IsTouching(IsTouching),
        IsInverted(IsInverted)
    {}
};

bool CreateRTS(HWND hWnd, InputManager* input_manager);

/**
 * Packet types as derived from IRealTimeStylus::GetPacketDescriptionData.
 */
enum class EWindowsPacketType
{
    None,
    X,
    Y,
    Z,
    Status,
    NormalPressure,
    TangentPressure,
    ButtonPressure,
    Azimuth,
    Altitude,
    Twist,
    XTilt,
    YTilt,
    Width,
    Height,
};

/**
 * Description of a packet's information, as derived from IRealTimeStylus::GetPacketDescriptionData.
 */
struct FPacketDescription
{
    EWindowsPacketType Type { EWindowsPacketType::None };
    int Minimum { 0 };
    int Maximum { 0 };
    float Resolution { 0 };
};

struct FTabletContextInfo
{
    int Index;

    TABLET_CONTEXT_ID ID;
    std::vector<FPacketDescription> PacketDescriptions;
    std::vector<EWindowsPacketType> SupportedPackets;

    StylusState WindowsState;

    void AddSupportedInput(EWindowsPacketType Type) { SupportedInputs.emplace_back(Type); }
    void SetDirty() { Dirty = true; }

//    virtual void Tick()
//    {
//        PreviousState = CurrentState;
//        CurrentState = WindowsState.ToPublicState();
//        Dirty = false;
//    }

private:
    StylusState CurrentState;
    StylusState PreviousState;
    std::vector<EWindowsPacketType> SupportedInputs;
    bool Dirty : 1;
};

/**
 * An implementation of an IStylusSyncPlugin for use with the RealTimeStylus API.
 */
class FWindowsRealTimeStylusPlugin : public IStylusSyncPlugin
{
public:
    FWindowsRealTimeStylusPlugin(InputManager* input_manager) :
        m_input_manager(input_manager)
    {}
    virtual ~FWindowsRealTimeStylusPlugin()
    {
        if (FreeThreadedMarshaller != nullptr)
        {
            FreeThreadedMarshaller->Release();
        }
    }

    virtual ULONG AddRef() override { return ++RefCount; }
    virtual ULONG Release() override
    {
        int NewRefCount = --RefCount;
        if (NewRefCount == 0)
            delete this;

        return NewRefCount;
    }

    virtual HRESULT QueryInterface(const IID& InterfaceID, void** Pointer) override;

    virtual HRESULT TabletAdded(IRealTimeStylus* RealTimeStylus, IInkTablet* InkTablet) override;
    virtual HRESULT TabletRemoved(IRealTimeStylus* RealTimeStylus, LONG iTabletIndex) override;

    virtual HRESULT RealTimeStylusEnabled(IRealTimeStylus* RealTimeStylus, ULONG Num, const TABLET_CONTEXT_ID* InTabletContexts) override;
    virtual HRESULT RealTimeStylusDisabled(IRealTimeStylus* RealTimeStylus, ULONG Num, const TABLET_CONTEXT_ID* InTabletContexts) override;

    virtual HRESULT StylusInRange(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) override { return S_OK; }
    virtual HRESULT StylusOutOfRange(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) override { return S_OK; }

    virtual HRESULT StylusDown(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo, ULONG PacketSize, LONG* Packet, LONG** InOutPackets) override;
    virtual HRESULT StylusUp(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo, ULONG PacketSize, LONG* Packet, LONG** InOutPackets) override;

    virtual HRESULT StylusButtonDown(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) override { return S_OK; }
    virtual HRESULT StylusButtonUp(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) override { return S_OK; }

    virtual HRESULT InAirPackets(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo,
                                 ULONG PacketCount, ULONG PacketBufferLength, LONG* Packets, ULONG* NumOutPackets, LONG** PtrOutPackets) override;
    virtual HRESULT Packets(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo,
                            ULONG PacketCount, ULONG PacketBufferSize, LONG* Packets, ULONG* NumOutPackets, LONG** PtrOutPackets) override;

    virtual HRESULT CustomStylusDataAdded(IRealTimeStylus*, const GUID*, ULONG, const BYTE*) override { return S_OK; }

    virtual HRESULT SystemEvent(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID, SYSTEM_EVENT, SYSTEM_EVENT_DATA) override { return S_OK; }
    virtual HRESULT Error(IRealTimeStylus*, IStylusPlugin*, RealTimeStylusDataInterest, HRESULT, LONG_PTR*) override { return S_OK; }

    virtual HRESULT DataInterest(RealTimeStylusDataInterest* OutDataInterest) override
    {
        *OutDataInterest = RTSDI_AllData;
        return S_OK;
    }

    virtual HRESULT UpdateMapping(IRealTimeStylus*) override { return S_OK; }

    void HiMetricToPixel(FTabletContextInfo* TabletContext);

    FTabletContextInfo* FindTabletContext(TABLET_CONTEXT_ID TabletID);

    IUnknown* FreeThreadedMarshaller;
    std::vector<FTabletContextInfo> TabletContexts;
    bool HasChanges { false };

private:
    int RefCount { 1 };
    HDC m_hDC;
    InputManager* m_input_manager;
    std::map<int, std::shared_ptr<IStroke>> inputs;

    void HandlePacket(IRealTimeStylus* RealTimeStylus, const StylusInfo* StylusInfo, ULONG PacketCount, ULONG PacketBufferLength, LONG* Packets);

    void AddTabletContext(IRealTimeStylus* RealTimeStylus, TABLET_CONTEXT_ID TabletID);
    void RemoveTabletContext(IRealTimeStylus* RealTimeStylus, TABLET_CONTEXT_ID TabletID);
};

#endif
