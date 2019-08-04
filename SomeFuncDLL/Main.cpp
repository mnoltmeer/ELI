#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include "..\MyFunc.h"

#include "eli_interface.h"

extern "C"
{
__declspec(dllexport) void __stdcall eIsStrNum(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  UINT i = IsStrNum(ep->GetParamToStr(L"pStr"));

  wchar_t res[3];
  swprintf(res, L"%d", i);

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//-------------------------------------------------------------------------------

__declspec(dllexport) void __stdcall eGetCurrDateToStr(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  String res = DateToStr(Date());

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
}
//-------------------------------------------------------------------------------

__declspec(dllexport) void __stdcall eGetCurrTimeToStr(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  String res = TimeToStr(Time());

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
}
//-------------------------------------------------------------------------------

__declspec(dllexport) void __stdcall eParseString(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  String mstr = ep->GetParamToStr(L"pMainStr"),
  tstr = ep->GetParamToStr(L"pTargetStr"),
  istr = ep->GetParamToStr(L"pInsertStr");

  String res = ParseString(mstr, tstr, istr);

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
}
//-------------------------------------------------------------------------------
}

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}

