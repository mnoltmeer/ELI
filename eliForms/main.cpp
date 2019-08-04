#include <vcl.h>
#include <windows.h>
#include "eli_interface.h"
#include "service.h"

#pragma hdrstop
#pragma argsused

TList *eWindows; //������ ������ id + ���������

void DeleteWndList()
{
  for (int i = 0; i < eWindows->Count; i++)
	{
	  delete (TObject*)eWindows->operator [](i);
	  eWindows->Delete(i);
	}

  delete eWindows;
  eWindows = NULL;
}
//------------------------------------------------------------------------------


extern "C"
{
__declspec(dllexport) void __stdcall ewCreateForm(void *p)
{
  ELI_INTERFACE *ELI = (ELI_INTERFACE*)p;

  ShowMessage("Entry point");
  TForm *frm = new TForm(Application->Handle);
  frm->Width = ELI->GetParamToInt(L"pWidth");
  frm->Height = ELI->GetParamToInt(L"pHeight");
  frm->Name = String(ELI->GetParamToStr(L"pFormName"));
  frm->Show();

  ELI->SetFunctionResult(ELI->GetCurrentFuncName(), L"1");
}
//------------------------------------------------------------------------------

__declspec(dllexport) void __stdcall ewTest(void *p)
{
  ShowMessage("Entry point");

  ELI_INTERFACE *ELI = (ELI_INTERFACE*)p;

  if (!ELI)
	ShowMessage("Null pointer!");

  int x = ELI->GetParamToInt(L"pX");
  int y = ELI->GetParamToInt(L"pY");

  ShowMessage(IntToStr(x) + " " + IntToStr(y));
  ShowMessage(ELI->GetCurrentFuncName());

  ELI->SetFunctionResult(ELI->GetCurrentFuncName(), L"1");
}
//------------------------------------------------------------------------------
}


int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  eWindows = new TList();

  switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
			// detach from process
			DeleteWndList();
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
			// detach from thread
			DeleteWndList();
            break;
    }


  return 1;
}

