/*!
Copyright 2023 Maxim Noltmeer (m.noltmeer@gmail.com)

This file is part of Extern Logic Interpreter.

    Extern Logic Interpreter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Extern Logic Interpreter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Extern Logic Interpreter.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma hdrstop

#include "eli_script.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

ELIScript::ELIScript()
{
  FELIPath = "ELI.dll";
}
//---------------------------------------------------------------------------

ELIScript::ELIScript(const String &interpreter_path)
{
  FELIPath = interpreter_path;
}
//---------------------------------------------------------------------------

ELIScript::ELIScript(ELI_INTERFACE *instance)
{
  FEIface = instance;
  FExistInstance = true;
}
//---------------------------------------------------------------------------

bool ELIScript::ConnectELI()
{
  bool res;

  if (FDllHandle)
    ReleaseELI();

  if (!FDllHandle)
	{
      try
		 {
		   FDllHandle = LoadLibrary(FELIPath.c_str());
		 }
	  catch (Exception &e)
		 {
		   Log = "Error connecting to ELI library";
		   res = false;
		 }
	}

  if (FDllHandle)
	{
	  try
		 {
		   FGetELI = (GETELIINTERFACE) GetProcAddress(FDllHandle, "GetELIInterface");
		   FFreeELI = (FREEELIINTERFACE) GetProcAddress(FDllHandle, "FreeELIInterface");
		 }
	  catch (Exception &e)
		 {
		   Log = "ConnectELI: " + e.ToString();
		   res = false;
		 }

	  if (!FGetELI)
		{
		  Log = "Error initialisation of GetELIInterface()";
		  res = false;
		}
	  else if (!FFreeELI)
		{
		  Log = "Error initialisation of FreeELIInterface()";
		  res = false;
		}
	  else if (!FGetELI(&FEIface))
		{
		  Log = "Error initialisation of ELI_INTERFACE";
		  res = false;
		}
	  else
		res = true;
	}
  else
	{
      Log = "Error connecting to ELI library";
	  res = false;
    }

  return res;
}
//---------------------------------------------------------------------------

bool ELIScript::ReleaseELI()
{
  bool res;

  if (FEIface)
	{
	  if (!FFreeELI(&FEIface))
		res = false;
	  else
		res = true;
	}

  FreeLibrary(FDllHandle);
  FDllHandle = NULL;

  return res;
}
//---------------------------------------------------------------------------

void ELIScript::LoadFromFile(const String &file)
{
  if (FileExists(file))
	{
	  std::unique_ptr<TStringStream> srv_file(new TStringStream("", TEncoding::UTF8, true));

	  srv_file->LoadFromFile(file);
	  srv_file->Position = 0;
	  Text = srv_file->ReadString(srv_file->Size);
    }
  else
	Text = "";

//проверим, вдруг текст грузится из файла с кодировкой UTF8-BOM,
//тогда нулевой символ будет ''
//удалим его
  if (Text[1] == wchar_t(65279))
	Text.Delete(1, 1);
}
//---------------------------------------------------------------------------

void ELIScript::SaveToFile(const String &file)
{
  std::unique_ptr<TStringStream> ms(new TStringStream(Text, TEncoding::UTF8, true));

  if (ms)
	 {
	   ms->Position = 0;
	   ms->SaveToFile(file);
	 }
}
//---------------------------------------------------------------------------

bool ELIScript::Run()
{
  bool res;

  if (Text == "")
	{
	  Log = "Empty script text!";
	  res = false;
	}
  else
	{
	  if (!FExistInstance)
		ConnectELI();

	  if (FEIface)
		{
          Result = FEIface->RunScript(Text.c_str(), Params.c_str(), true);
		  Log = FEIface->ShowInfoMessages();

		  if (Result != "-err-")
			res = true;
		  else
			res = false;
		}
	  else
		res = false;

      if (!FExistInstance)
		ReleaseELI();
    }

  return res;
}
//---------------------------------------------------------------------------
