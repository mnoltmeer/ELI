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

#ifndef eli_scriptH
#define eli_scriptH

#include <System.Classes.hpp>
#include <windows.h>
#include "eli_interface.h"

class ELIScript
{
  private:
	String FELIPath; //за замовчуванням "ELI.dll" тобто з поточного каталогу
	ELI_INTERFACE *FEIface;
	HINSTANCE FDllHandle;
	GETELIINTERFACE FGetELI;
	FREEELIINTERFACE FFreeELI;
	bool FExistInstance;
	bool FSaveLogInFile;
	String FParams;
	String FText;
    String FResult;
	String FLog;

	bool ConnectELI();
	bool ReleaseELI();

    void Prepare();

  public:
    ELIScript();
	ELIScript(const String &interpreter_path);
	ELIScript(ELI_INTERFACE *instance); //ініціалізація з наявним і налаштованим інтерператором
	inline virtual ~ELIScript(){if (!FExistInstance) ReleaseELI();}

	void LoadFromFile(const String &file);
    void SaveToFile(const String &file);
	bool Run();

	__property String InterpreterPath = {read = FELIPath, write = FELIPath};
    __property String Params = {read = FParams, write = FParams};
	__property String Text = {read = FText, write = FText};
	__property String Result = {read = FResult, write = FResult};
	__property String Log = {read = FLog, write = FLog};
	__property ELI_INTERFACE *Interpreter = {read = FEIface};
	__property bool SaveLogInFile = {read = FSaveLogInFile, write = FSaveLogInFile};
};
//---------------------------------------------------------------------------
#endif
