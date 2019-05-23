// FlashNess.h : CFlashNess ������
#pragma once
#include "rc\resource.h"       // ������
#include <atlctl.h>
#include "FlashNess_i.h"
#include "wizard\_IFlashNessEvents_CP.h"
#include <string>
#include "third_party/dynamic_library/dynamic_library.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;



// CFlashNess
class ATL_NO_VTABLE CFlashNess :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IFlashNess, &IID_IFlashNess, &LIBID_FlashNessLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IOleControlImpl<CFlashNess>,
    public IOleObjectImpl<CFlashNess>,
    public IOleInPlaceActiveObjectImpl<CFlashNess>,
    public IViewObjectExImpl<CFlashNess>,
    public IOleInPlaceObjectWindowlessImpl<CFlashNess>,
    public ISupportErrorInfo,
    public IConnectionPointContainerImpl<CFlashNess>,
    public CProxy_IFlashNessEvents<CFlashNess>,
    public IObjectWithSiteImpl<CFlashNess>,
    public IQuickActivateImpl<CFlashNess>,
#ifndef _WIN32_WCE
    public IDataObjectImpl<CFlashNess>,
#endif
    public IProvideClassInfo2Impl<&CLSID_FlashNess, &__uuidof(_IFlashNessEvents), &LIBID_FlashNessLib>,
    public IObjectSafetyImpl<CFlashNess, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
    public CComCoClass<CFlashNess, &CLSID_FlashNess>,
    public CComControl<CFlashNess>,
    public IPersistPropertyBagImpl<CFlashNess> {
public:
    CContainedWindow m_ctlStatic;

#pragma warning(push)
#pragma warning(disable: 4355) // ��this��: ���ڻ���Ա��ʼֵ�趨���б�


    CFlashNess();

#pragma warning(pop)

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_INSIDEOUT |
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST
)

DECLARE_REGISTRY_RESOURCEID(IDR_FLASHNESS)


BEGIN_COM_MAP(CFlashNess)
    COM_INTERFACE_ENTRY(IFlashNess)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IQuickActivate)
#ifndef _WIN32_WCE
    COM_INTERFACE_ENTRY(IDataObject)
#endif
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafety)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(CFlashNess)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
    PROP_DATA_ENTRY("name", name_, VT_BSTR)
    // ʾ����
    // PROP_ENTRY_TYPE("������", dispid, clsid, vtType)
    // PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CFlashNess)
    CONNECTION_POINT_ENTRY(__uuidof(_IFlashNessEvents))
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CFlashNess)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    CHAIN_MSG_MAP(CComControl<CFlashNess>)
ALT_MSG_MAP(1)
    // �����滻Ϊ���� Static ����Ϣӳ����
END_MSG_MAP()
// �������ԭ��:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lRes = CComControl<CFlashNess>::OnSetFocus(uMsg, wParam, lParam, bHandled);
        if (m_bInPlaceActive)
        {
            if(!IsChild(::GetFocus()))
                m_ctlStatic.SetFocus();
        }
        return lRes;
    }

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip) {
        return S_OK;
        IOleInPlaceObjectWindowlessImpl<CFlashNess>::SetObjectRects(prcPos, prcClip);
        int cx, cy;
        cx = prcPos->right - prcPos->left;
        cy = prcPos->bottom - prcPos->top;
        ::SetWindowPos(m_ctlStatic.m_hWnd, NULL, 0,
            0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
        return S_OK;
    }

    STDMETHOD(SetClientSite)(_Inout_opt_ IOleClientSite* pClientSite)
    {
        LRESULT lRes = IOleObjectImpl<CFlashNess>::SetClientSite(pClientSite);
        return lRes;
    }



// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        static const IID* const arr[] =
        {
            &IID_IFlashNess,
        };

        for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
        {
            if (InlineIsEqualGUID(*arr[i], riid))
                return S_OK;
        }
        return S_FALSE;
    }

// IViewObjectEx
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IFlashNess

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }


    CComBSTR name_;
	std::wstring data_;
    utils::DynamicLibrary library_;

    STDMETHOD(InitDevice)(BSTR port, SHORT* device);
    STDMETHOD(CloseDevice)(SHORT device);

    STDMETHOD(Beep)(SHORT times);
    STDMETHOD(ReadCard)(BSTR* pVal);

    STDMETHOD(WriteData)(BSTR data);
    STDMETHOD(ReadData)(BSTR* pVal);

    STDMETHOD(put_name)(BSTR data);
    STDMETHOD(get_name)(BSTR* pVal);

	STDMETHOD(GetModulePath)(BSTR* path);
    STDMETHOD(CreateService)(BSTR path);
};

OBJECT_ENTRY_AUTO(__uuidof(FlashNess), CFlashNess)
