/*!
Copyright 2017-2025 Maxim Noltmeer (m.noltmeer@gmail.com)

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

#ifndef ELI_INTERFACE_H_INCLUDED
#define ELI_INTERFACE_H_INCLUDED

typedef void (__stdcall *func_ptr)(void*);   //describes pointer to function from main app

struct ELI_INTERFACE
{
//adds function to stack
	virtual void __stdcall AddFunction(const wchar_t *name, const wchar_t *params, func_ptr fptr) = 0;
//removes function from stack
	virtual void __stdcall DeleteFunction(const wchar_t *name) = 0;
//calls a function
	virtual void __stdcall CallFunction(const wchar_t *name) = 0;
//converts return value of function to a string and returns pointer to it
//return NULL in case of error
	virtual wchar_t* __stdcall GetFunctionResult(const wchar_t *name) = 0;
//sets function return value
	virtual void __stdcall SetFunctionResult(const wchar_t *name, const wchar_t* result) = 0;
//sets new parameter value or adds new parameter in stack
	virtual void __stdcall SetParam(const wchar_t *name, const wchar_t *new_val) = 0;
//converts parameter to an integer value and returns pointer to it
	virtual int __stdcall GetParamToInt(const wchar_t *name) = 0;
//converts parameter to a float value and returns pointer to it
	virtual float __stdcall GetParamToFloat(const wchar_t *name) = 0;
//converts parameter to a string and returns pointer to it
//return NULL in case of error
	virtual const wchar_t* __stdcall GetParamToStr(const wchar_t *name) = 0;
//returns value of object's property (property must be public)
	virtual const wchar_t * __stdcall GetObjectProperty(const wchar_t *obj_name,
														const wchar_t *prop_name) = 0;
//sets value of object's property (property must be public)
	virtual bool __stdcall SetObjectProperty(const wchar_t *obj_name,
											 const wchar_t *prop_name,
											 const wchar_t *val) = 0;

	virtual const wchar_t* __stdcall GetVersion() = 0;
	virtual const wchar_t* __stdcall ShowVarStack() = 0;
	virtual const wchar_t* __stdcall ShowObjStack() = 0;
	virtual const wchar_t* __stdcall ShowClassStack() = 0;
	virtual const wchar_t* __stdcall ShowProcStack() = 0;
	virtual const wchar_t* __stdcall ShowInfoMessages() = 0;
	virtual const wchar_t* __stdcall RunScript(const wchar_t *imptext,
											   const wchar_t *parameter,
											   bool log) = 0;
	virtual const wchar_t* __stdcall RunScriptFromFile(const wchar_t *filepath,
													   const wchar_t *parameter,
													   bool log) = 0;
	virtual const wchar_t* __stdcall ShowFuncStack() = 0;
	virtual const wchar_t* __stdcall ShowParamStack() = 0;
	virtual const wchar_t* __stdcall ShowFragmentStack() = 0;
	virtual const wchar_t* __stdcall GetCurrentFuncName() = 0;
	virtual void __stdcall SetDebug(bool enable_dbg, bool in_file) = 0;
	virtual bool __stdcall DebugEnabled() = 0;
//returns path to ELI work directory
	virtual const wchar_t* __stdcall GetInitDir() = 0;
    virtual void __stdcall AddToLog(const wchar_t *msg) = 0;
};
//-------------------------------------------------------------------------------

#ifdef BUILD_DLL
	#define ELI_API __declspec(dllexport)
#else
	#define ELI_API __declspec(dllimport)
#endif

extern "C"
{
  ELI_API int __stdcall GetELIInterface(ELI_INTERFACE **eInterface);
  ELI_API int __stdcall FreeELIInterface(ELI_INTERFACE **eInterface);
}

typedef int (__stdcall *GETELIINTERFACE)(ELI_INTERFACE **eInterface);
typedef int (__stdcall *FREEELIINTERFACE)(ELI_INTERFACE **eInterface);
#endif // ELI_INTERFACE_H_INCLUDED
