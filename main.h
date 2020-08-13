/*!
Copyright 2017-2019 Maxim Noltmeer (m.noltmeer@gmail.com)

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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <iterator>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define BUILD_DLL

/*! somefunc.h copyright 2015-2018 Maxim Noltmeer (m.noltmeer@gmail.com) */
#include "..\work-functions\MyFunc.h"
/*! efstring.h copyright 2016-2018 Elsa Fenrich (elsa.fenrich@gmail.com) */
#include "..\work-functions\efstring.h"
#include "eli_interface.h"
#include "scdefines.h"
#include "resourcestack.h"
#include "fn_prm.h"

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

class ELI: public ELI_INTERFACE
{
  private:

    bool write_log;
    bool debug_eli;
	bool use_return;
	std::wstring logfile;
	std::wstring scrtext; //текст скрипта, с которым будет работать dll
    UINT CstrInd; //глобальный индекс присваиваемый конст. строкам
	UINT FrgmntNum; //глобальный индекс нумерации фрагментов кода
    UINT TmpObjInd; //глобальный индекс нумерации временных объектов
	std::wstring InfStack; //стек сообщений интерпретатора
	std::wstring ScriptResult; //значение возвращаемое скриптом
    wchar_t initdir[128];
	bool use_false; //флаг, сообщающий, что нужно использовать секцию else
	std::wstring current_func_name; //имя текущей вызванной ф-ии
	std::wstring current_class; //имя текущего объявляемого класса
	std::wstring return_val; //значение возвращаемое методом
    SCRIPTLINES vecScList; //вектор для хранения строк скрипта
    FRAGMENTSTACK *frgStack;
	std::vector<VARSTACK*> vecVSt; //вектор указателей на стеки переменных
                              //нулевой элемент указывает на глобальный стек
	std::vector<HINSTANCE> vecLibs; //вектор для дескрипторов внешних библиотек
	std::vector<EXTFUNC> vecExtFuncs; //вектор со списком импортированных ф-й
	std::vector<REFERENCE> vecRefs; //вектор со списком ссылок

///стеки для рантайм переменных скрипта
    VARSTACK *vStack; //стек переменных для скрипта
    VARSTACK *st; //глобальный указатель для работы со стеками переменных
                  //может указывать на главный стек или на лок. стек процедуры
    PARAMSTACK *pStack; //стек параметров для скриптовых ф-й
    FUNCSTACK *fStack; //стек функций для совместного использования
    RESOURCESTACK *objStack; //стек ресурсов - свойств объектов, которыми оперирует интерпретатор
    RESOURCESTACK *clStack; //стек классов
    RESOURCESTACK *procStack; //стек процедур
    RESOURCESTACK *tmpObj; //стек временных объектов для объектов-свойств

///служебные функции
//начальная инициализация всех ресурсов интерпретатора
void InitRes(bool init);
//ф-я инициализирующая встроенные ф-ии интерпретатора
void InitCompilerFuncs();
//подготавливает интерпретатор к запуску нового скрипта
void FreeRes();
//очищает стек фрагментов кода, оставляя только те фрагменты
//которые содержат код процедур
//void ClearFragments(bool all);
//компилирует фрагмент кода (тело цикла или условия)
bool CompileFragment(SCRIPTLINES *vecFragment, UINT index);
//по метке находит фрагмент в стеке и возвращает указатель на строки кода
//SCRIPTLINES *GetFragmentCode(const wchar_t *mark);
//проверяет содержится ли такое значение в стеке
bool IsStackContent(std::vector<std::wstring> *strStack, std::wstring val);
bool IsStackContent(std::vector<float> *numstack, float val);
//проверяет является ли выражение простым
//т.е. содержит ли математические операции одинакового приоритета
bool IsSimple(wchar_t *expr);
//проверяет, что строка содержит только цифры, точку и знаки операций
bool IsNumExpression(const wchar_t *expr);
//находит в строке символ операции и возвращает его позицию либо -1
UINT OperSymbPos(std::wstring str);
//убирает скобки из выражения
//приводит его к простому списку арифметических операций (сложное выражение)
const wchar_t *RemoveScopes(wchar_t *in_exp, UINT index);
//приводит сложное выражение к простому
const wchar_t *SetExpToSimple(wchar_t *in_exp, UINT index);
//проверяет на корректность имя переменной
//недопустимы символ _ знаки препинания и цифры
bool IsCorrectVarName(const wchar_t *varname);
//вычисляет тип выражения, возвращает 0, если произошла ошибка
//index - индекс строки в скрипте
UINT CheckExprType(const wchar_t *expr, UINT index);
//парсит строку с выражением, заменяет имена переменных на их значения
bool ParseVarInExp(wchar_t *expr, UINT index);
//парсит строку с выражением, заменяет вызовы ф-й их возвращаемыми значениями
bool ParseFuncsInExp(wchar_t *expr, UINT index);
//парсит строку с выражнеием, заменяет обращения к свойствам или методам объектов на возвр. значения
bool ParseObjectsInExp(wchar_t *expr, UINT index);
//парсит строку на наличие ссылок
bool ParseRefsInExp(wchar_t *expr, UINT index);
//парсит строку, разворачивая вложенные объекты-свойства, приводит строку к виду &obj.property
bool ParseIncObjects(wchar_t *expr, UINT index);
//функции производящие расчет выражения
//выполняют операции с его частями и сводит к одному значению
float *CalcExpNum(wchar_t *expr, UINT index);
const wchar_t *CalcExpStr(wchar_t *expr, UINT index);
//парсит текст скрипта на предмет константных строк (типа 'константная строка')
//загоняет их в стек как переменные и меняет на соотв. имена переменных
const wchar_t *ParseConstStrings(wchar_t *text);
//убирает пробелы
const wchar_t *RemoveSpaces(const wchar_t *text);
//заменяет директивы #include в исходном тексте скрипта на содержимое инклуд-файлов
SCRIPTLINES GetInclude(const wchar_t *str);
//разбивает скрипт на строки и заполняет вектор
//разделителем строк является ENDLN
void PrepareScript();
//проверяет на корректность и компилирует каждую строку скрипта
//возвращает 1, если скрипт скомпилирован, 0 - если произошла ошибка
bool CompileScriptLines();
//возвращает результат сравнения левой и правой части выражения exp
//1 - выражение истинно
//0 - ложно
//-1 - произошла ошибка
int ExpTrue(std::wstring exp, UINT index);

///функции для выполнения команд содержащихся в скрипте
//выполняет инициализацию переменной
//st - указатель на стек, в котором будет создана переменная
//defvalue - строка со значением по умолчанию
//index - индекс строки в скрипте
bool VarInit(wchar_t *name, UINT type, wchar_t* defvalue, UINT index);
//производят расчет условия if-then-else и циклов
//возвращают ложь, если произошла ошибка
bool ExpIf(wchar_t *line, UINT index);    //if (lval == rval){};
bool ExpElseIf(wchar_t *line, UINT index);    //else if (lval == rval){};
bool ExpElse(wchar_t *line, UINT index);
bool ExpFor(wchar_t *line, UINT index);   //for ($var, <= 10, +1) for (0, <= 10, +1) {};
bool ExpCount(wchar_t *line, UINT index); //count(10) {};
bool ExpWhile(wchar_t *line, UINT index); //while(lval != rval) {};
bool ExpSelect(wchar_t *line, UINT index);//select ($var){when 10 then {_return(1)}}
bool ExpWhen(wchar_t *line, UINT index);

bool RunFunc(wchar_t *str_with_func, wchar_t *result, UINT index);
bool CreateProcedure(wchar_t *str_with_proc, UINT index);
bool DropProcedure(const wchar_t *proc_name);
bool RunProcedure(const wchar_t *name, const wchar_t *params, UINT index);
bool WorkWithObject(wchar_t *str_with_obj, wchar_t *result, UINT index);
bool RunMethod(const wchar_t *objname, const wchar_t *cl_name, wchar_t *str_with_method, UINT index);
bool AddRef(const wchar_t *name, const wchar_t *val);
REFERENCE *GetRef(const wchar_t *name);
bool ImportParentClass(std::wstring child, std::wstring parent, bool type);

  public:
    ELI();
	inline virtual ~ELI(){InitRes(false);}

//добавляет функцию в стек
	virtual void __stdcall AddFunction(const wchar_t *name, const wchar_t *params, func_ptr fptr);
//удаляет функцию из стека
	virtual void __stdcall DeleteFunction(const wchar_t *name);
//вызывает функцию
	virtual void __stdcall CallFunction(const wchar_t *name);
//преобразует возвращаемое значение функции в строку и возвращает указатель на нее
//в случае ошибки возвращает NULL
	virtual wchar_t * __stdcall GetFunctionResult(const wchar_t *name);
//устанавливает возвращаемое значение функции
	virtual void __stdcall SetFunctionResult(const wchar_t *name, const wchar_t* result);
//устанавливает новое значение параметра или добавляет новый параметр
	virtual void __stdcall SetParam(const wchar_t *name, const wchar_t *new_val);
//преобразует параметр в integer и возвращает его
	virtual int __stdcall GetParamToInt(const wchar_t *name);
//преобразует параметр в float и возвращает его
	virtual float __stdcall GetParamToFloat(const wchar_t *name);
//преобразует параметр в строку и возвращает указатель на нее
//в случае ошибки возвращает NULL
	virtual const wchar_t * __stdcall GetParamToStr(const wchar_t *name);

	virtual const wchar_t * __stdcall GetVersion();
	virtual const wchar_t * __stdcall ShowVarStack();
	virtual const wchar_t * __stdcall ShowObjStack();
	virtual const wchar_t * __stdcall ShowClassStack();
	virtual const wchar_t * __stdcall ShowProcStack();
	virtual const wchar_t * __stdcall ShowInfoMessages();
	virtual const wchar_t * __stdcall RunScript(const wchar_t *imptext, const wchar_t *parameter, bool log);
	virtual const wchar_t * __stdcall RunScriptFromFile(const wchar_t *filepath, const wchar_t *parameter, bool log);
	virtual const wchar_t * __stdcall ShowFuncStack();
	virtual const wchar_t * __stdcall ShowParamStack();
	virtual const wchar_t * __stdcall ShowFragmentStack();
	virtual const wchar_t * __stdcall GetCurrentFuncName();
	virtual void __stdcall SetDebug(bool enable_dbg, bool in_file);
	virtual bool __stdcall DebugEnabled(){return debug_eli;}
	virtual const wchar_t* __stdcall GetInitDir(){return initdir;}

	std::wstring LastErr;
    bool debug_in_file;

///служебные функции
    void WriteLog(const wchar_t *rec);
    void WriteELIDebug(const wchar_t *event, const wchar_t *rec);
//добавляет в стек сообщений новую строку
    void AddInfoMsg(const wchar_t *msg, const wchar_t *type, UINT index);
    void AddInfoMsg(const wchar_t *msg);
//сохраняет информацию о содержимом стеков в файл state.log рядом с dll
    void SaveELIState();
//сохраняет в файл varstack.log содержимое стека переменных
//level = 0 - глобальный стек
//level = 1 - текущий стек
    void SaveVStState(UINT level);
//проверяет корректность имени
    bool IsCorrectName(const wchar_t *str);
//проверяет имя на соответствие члену из стека классов
    bool IsClassMember(const wchar_t *cl_name, const wchar_t *mt_name);
//проверяет является ли член класса публичным
    bool IsPublicMember(const wchar_t *cl_name, const wchar_t *mb_name);
//проверяет доступность члена
    bool IsAccessibleMember(const wchar_t *obj_name, const wchar_t *mb_name);
//компилирует строку
    bool CompileLine(const wchar_t *line, UINT index);
//возвращает категорию объекта с указанным именем
    const wchar_t *GetObjCathegory(const wchar_t *obj_name);
//маркирует начало и конец фрагмента в коде
//заменяет открывающую и закрывающую фигурную скобку на соотв метки
    std::wstring MarkFragments(std::wstring &operstr);
    bool MakeCodeInVar(wchar_t *str, UINT index);
    bool CompileCodeFromVar(const wchar_t *name, UINT index);
	HINSTANCE LoadExtLib(std::wstring &path);
    bool FreeExtLib(HINSTANCE hnd);
	bool AddClassProperty(std::wstring &cl_name, std::wstring &prop_str, bool is_public, UINT index);
	bool AddClassMethod(std::wstring &cl_name, std::wstring &method_str, bool is_public, UINT index);
    const wchar_t *CreateTempObject(std::wstring str, std::wstring owner, UINT index);

    inline PARAMSTACK *GetParamStack(){return pStack;}
    inline FUNCSTACK *GetFuncStack(){return fStack;}
	inline RESOURCESTACK *GetObjStack(){return objStack;}
	inline RESOURCESTACK *GetClassStack(){return clStack;}
	inline RESOURCESTACK *GetProcStack(){return procStack;}
	inline VARSTACK *GetVarStack(){return st;}
	inline FRAGMENTSTACK *GetFragmentStack(){return frgStack;}
	inline std::vector<EXTFUNC> *GetExtFnStack(){return &vecExtFuncs;}

	inline void SetScriptResult(const wchar_t *res){ScriptResult = res;}
	inline void ReturnEnabled(bool res){use_return = res;}
	inline void SetInitDir(const wchar_t *path){wcscpy(initdir, path);}
};
//------------------------------------------------------

///встроенные ф-ии для скриптового языка
inline void __stdcall scRandom(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRandom", L"[start]");

  int gen, area;
  wchar_t res[NUMSIZE];

  area = e_ptr->GetParamToInt(L"pArea");

  srand(time(NULL));
  gen = rand() % area;
  swprintf(res, L"%d", gen);

  e_ptr->SetFunctionResult(L"_random", res);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRandom", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scRound(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRound", L"[start]");

  wchar_t res[NUMSIZE];
  float n;
  int prec = e_ptr->GetParamToInt(L"pPrecision");

  switch (prec)
    {
	  case 0: n = _roundf(e_ptr->GetParamToFloat(L"pNumber")); break;
	  case 1: n = _roundf(e_ptr->GetParamToFloat(L"pNumber") * 10) / 10; break;
	  case 2: n = _roundf(e_ptr->GetParamToFloat(L"pNumber") * 100) / 100; break;
      default: n = _roundf(e_ptr->GetParamToFloat(L"pNumber"));
    }

  swprintf(res, FRMTNUM, n);
  e_ptr->SetFunctionResult(L"_round", res);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRound", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scInt(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scInt", L"[start]");

  wchar_t res[NUMSIZE];

  swprintf(res, L"%d", e_ptr->GetParamToInt(L"pNumber"));
  e_ptr->SetFunctionResult(L"_int", res);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scInt", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scStrLen(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrLen", L"[start]");

  wchar_t res[NUMSIZE];
  UINT len = wcslen(e_ptr->GetParamToStr(L"pStr"));

  swprintf(res, L"%d", len);

  e_ptr->SetFunctionResult(L"_wcslen", res);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrLen", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scStrEq(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrEq", L"[start]");

  if (wcscmp(e_ptr->GetParamToStr(L"pStr1"),
             e_ptr->GetParamToStr(L"pStr2")) == 0)
    e_ptr->SetFunctionResult(L"_streq", L"1");
  else
    e_ptr->SetFunctionResult(L"_streq", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrEq", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scIStrEq(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scIStrEq", L"[start]");

  if (_wcsicmp(e_ptr->GetParamToStr(L"pStr1"),
             e_ptr->GetParamToStr(L"pStr2")) == 0)
    e_ptr->SetFunctionResult(L"_istreq", L"1");
  else
    e_ptr->SetFunctionResult(L"_istreq", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scIStrEq", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSubStr(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSubStr", L"[start]");

  std::wstring res = e_ptr->GetParamToStr(L"pTargetStr");

  UINT pos = e_ptr->GetParamToInt(L"pPos");
  UINT cnt = e_ptr->GetParamToInt(L"pCount");

  res = res.substr(pos + 1, cnt);

  e_ptr->SetFunctionResult(L"_substr", res.c_str());

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSubStr", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scReturn(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReturn", L"[start]");

  e_ptr->SetScriptResult(e_ptr->GetParamToStr(L"pReturnVal"));
  e_ptr->ReturnEnabled(true);
  e_ptr->SetFunctionResult(L"_return", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReturn", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scThrow(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scThrow", L"[start]");

  e_ptr->AddInfoMsg(e_ptr->GetParamToStr(L"pException"), ERRMSG, e_ptr->GetParamToInt(P_IND));
  e_ptr->SetFunctionResult(L"_throw", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scThrow", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scFree(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFree", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  if (e_ptr->GetVarStack()->Remove(e_ptr->GetParamToStr(L"pVarName")))
    {
      e_ptr->SetFunctionResult(L"_free", L"1");
    }
  else
    {
      e_ptr->AddInfoMsg(UNKVARNAME, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"_free", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFree", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scLoadObjStack(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadObjStack", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFilePath");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
    {
      path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
    }

  if (e_ptr->GetParamToInt(L"pClear") > 0)
    e_ptr->GetObjStack()->Clear();

  int res = e_ptr->GetObjStack()->LoadResFile(path.c_str());

  wchar_t result[3];
  swprintf(result, L"%d", res);
  e_ptr->SetFunctionResult(L"_LoadObjStack", result);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadObjStack", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveObjStack(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
   e_ptr->WriteELIDebug(L"scSaveObjStack", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFilePath");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
    {
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
	}

  int res = e_ptr->GetObjStack()->CreateResFile(path.c_str(), true);

  wchar_t result[3];
  swprintf(result, L"%d", res);
  e_ptr->SetFunctionResult(L"_SaveObjStack", result);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveObjStack", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveObjects(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
   e_ptr->WriteELIDebug(L"scSaveObjects", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFilePath");
  std::wstring cath = e_ptr->GetParamToStr(L"pCathegory");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
    {
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
    }

  int res = e_ptr->GetObjStack()->CreateResFile(path.c_str(), cath.c_str());

  wchar_t result[3];
  swprintf(result, L"%d", res);
  e_ptr->SetFunctionResult(L"_SaveObjects", result);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveObjects", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scCompactObjStack(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scCompactObjStack", L"[start]");

  e_ptr->GetObjStack()->Compact();

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scCompactObjStack", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scClearObjStack(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scClearObjStack", L"[start]");

  e_ptr->GetObjStack()->Clear();

  e_ptr->SetFunctionResult(L"_ClearObjStack", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scClearObjStack", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scRemoveObjects(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRemoveObjects", L"[start]");

  std::wstring cath = e_ptr->GetParamToStr(L"pCathegory");

  wchar_t result[3];

  int res = e_ptr->GetObjStack()->Delete(cath, L"", L"");

  swprintf(result, L"%d", res);
  e_ptr->SetFunctionResult(L"_RemoveObjects", result);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRemoveObjects", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scRun(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRun", L"[start]");

  int index = e_ptr->GetParamToInt(P_IND);
  std::wstring vname = e_ptr->GetParamToStr(L"pVarName");

  VARIABLE *var = e_ptr->GetVarStack()->Get(vname.c_str());

  if (var)
    {
	  std::wstring oldval = e_ptr->GetVarStack()->GetStrElement(var);

	  std::wstring text = L"{" + oldval + L"}";
	  text = e_ptr->MarkFragments(text);
	  text.erase(text.length() - 1, 1);

      e_ptr->GetVarStack()->SetStrElement(var, text);

      if (e_ptr->CompileCodeFromVar(vname.c_str(), index))
        {e_ptr->SetFunctionResult(L"_Run", L"1");}
      else
        e_ptr->SetFunctionResult(L"_Run", L"0");

//да, то, что ниже это костыль. Мне стыдно, но я так и не понял, почему указатель var
//превращается в тыкву, если ф-я _run() транслирует строку с ф-ей _importfunc()
      var = e_ptr->GetVarStack()->Get(vname.c_str());

      e_ptr->GetVarStack()->SetStrElement(var, oldval);
    }
  else
    e_ptr->SetFunctionResult(L"_Run", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRun", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scGetParamAsNum(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsNum", L"[start]");

//ищем в стеке имя искомого параметра, которое хранит параметр pParam
  PARAM *prm = e_ptr->GetParamStack()->Get(e_ptr->GetParamToStr(L"pParam"));

  if (!prm)
    {
      e_ptr->SetFunctionResult(L"_GetParamAsNum", L"0");
      e_ptr->AddInfoMsg(PARAMERR, WRNMSG, e_ptr->GetParamToInt(P_IND));
    }
  else
    {
      wchar_t res[NUMSIZE];
	  swprintf(res, FRMTNUM, prm->ToFloat());
      e_ptr->SetFunctionResult(L"_GetParamAsNum", res);
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsNum", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scGetParamAsStr(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsStr", L"[start]");

//ищем в стеке имя искомого параметра, которое хранит параметр pParam
  PARAM *prm = e_ptr->GetParamStack()->Get(e_ptr->GetParamToStr(L"pParam"));

  if (!prm)
    {
      e_ptr->SetFunctionResult(L"_GetParamAsStr", L"0");
      e_ptr->AddInfoMsg(PARAMERR, WRNMSG, e_ptr->GetParamToInt(P_IND));
    }
  else
    {
      e_ptr->SetFunctionResult(L"_GetParamAsStr", prm->ToStr());
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsStr", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSetParam(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSetParam", L"[start]");

//ищем в стеке имя искомого параметра, которое хранит параметр pParam
  if (wcslen(e_ptr->GetParamToStr(L"pParam")) == 0)
    e_ptr->SetFunctionResult(L"_SetParam", L"0");
  else
    {
      e_ptr->GetParamStack()->Add(e_ptr->GetParamToStr(L"pParam"),
                                  e_ptr->GetParamToStr(L"pValue"));
      e_ptr->SetFunctionResult(L"_SetParam", L"1");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSetParam", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scLoadFileToVar(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadFileToVar", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFile");
  std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
    {
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
    }

  std::wstring text = LoadTextFile(path.c_str()).c_str();

  if (text == ERROUT)
    e_ptr->SetFunctionResult(L"_LoadFileToVar", L"0");
  else
    {
	  if (target.find(L"$") != std::wstring::npos)
        {
          VARIABLE *var = e_ptr->GetVarStack()->Get(target.c_str());

          if (var)
            {
              e_ptr->GetVarStack()->SetStrElement(var, text);
              e_ptr->SetFunctionResult(L"_LoadFileToVar", L"1");
            }
          else
            {
              e_ptr->SetFunctionResult(L"_LoadFileToVar", L"0");

              if (e_ptr->DebugEnabled())
                e_ptr->WriteELIDebug(L"scLoadFileToVar", L"[no variable!]");
            }
        }
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadFileToVar", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveVarToFile(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFile");
  std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
	}

  if (target.find(L"$") != std::wstring::npos)
    {
      VARIABLE *var = e_ptr->GetVarStack()->Get(target.c_str());

      if (var)
        {
		  std::wstring text = e_ptr->GetVarStack()->GetStrElement(var);

		  SaveToFile(text.c_str(), path.c_str());
          e_ptr->SetFunctionResult(L"_SaveVarToFile", L"1");
		}
      else
        {
          e_ptr->SetFunctionResult(L"_SaveVarToFile", L"0");

          if (e_ptr->DebugEnabled())
            e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[no variable!]");
        }
    }


  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveFragmentToFile(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFile");
  std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
	}

  if (target.find(L"$") != std::wstring::npos)
    {
      VARIABLE *var = e_ptr->GetVarStack()->Get(target.c_str());

      if (var)
        {
		  std::wstring mark = e_ptr->GetVarStack()->GetStrElement(var);

          SCRIPTLINES *ptr = e_ptr->GetFragmentStack()->GetFragmentCode(mark);

          if (!ptr)
            {
              e_ptr->SetFunctionResult(L"_SaveFragmentToFile", L"0");

              if (e_ptr->DebugEnabled())
                e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[no fragment!]");
            }
          else
            {
              SCRIPTLINES frg = *ptr;

              if (frg.size() > 0)
                {
                  for (UINT i = 0; i < frg.size(); i++)
					 frg[i] += L";";

				  SaveVectorToFileW(&frg, path);
				  e_ptr->SetFunctionResult(L"_SaveFragmentToFile", L"1");
                }
              else
                e_ptr->SetFunctionResult(L"_SaveFragmentToFile", L"0");
            }
        }
      else
        {
          e_ptr->SetFunctionResult(L"_SaveFragmentToFile", L"0");

          if (e_ptr->DebugEnabled())
            e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[no variable!]");
        }
    }


  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scGetConfig(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetConfig", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pFile");
  std::wstring line = e_ptr->GetParamToStr(L"pLine");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(e_ptr->GetInitDir()) + path;
	}

  std::wstring text = GetConfigLineW(path, line);

  if ((text == L"^no_line") || (text == L"^err_open_file"))
	e_ptr->SetFunctionResult(L"_GetConfig", L"0");
  else
    e_ptr->SetFunctionResult(L"_GetConfig", text.c_str());

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetConfig", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveState(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveState", L"[start]");

  e_ptr->SaveELIState();
  e_ptr->SetFunctionResult(L"_SaveState", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveState", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSaveVarStack(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarStack", L"[start]");

  UINT level = e_ptr->GetParamToInt(L"pLevel");

  e_ptr->SaveVStState(level);
  e_ptr->SetFunctionResult(L"_SaveVarStack", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarStack", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scWriteOut(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scWriteOut", L"[start]");

  const wchar_t *str = e_ptr->GetParamToStr(L"pStr");
  std::wstring outstr;

  if (_wcsicmp(L"#varstack", str) == 0)
    outstr =  e_ptr->ShowVarStack();
  else if (_wcsicmp(L"#funcstack", str) == 0)
    outstr = e_ptr->ShowFuncStack();
  else if (_wcsicmp(L"#prmstack", str) == 0)
    outstr = e_ptr->ShowParamStack();
  else if (_wcsicmp(L"#objstack", str) == 0)
    outstr = e_ptr->ShowObjStack();
  else if (_wcsicmp(L"#clstack", str) == 0)
    outstr = e_ptr->ShowClassStack();
  else if (_wcsicmp(L"#procstack", str) == 0)
    outstr = e_ptr->ShowProcStack();
  else if (_wcsicmp(L"#frgstack", str) == 0)
    outstr = e_ptr->ShowFragmentStack();
  else if (_wcsicmp(L"#endl", str) == 0)
    outstr = L"\r\n";
  else
    outstr = str;

    wprintf(outstr.c_str());

  e_ptr->SetFunctionResult(L"_WriteOut", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scWriteOut", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scReadIn(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReadIn", L"[start]");

  wchar_t str[CHARSIZE];

  _getws(str);

  VARIABLE *var = e_ptr->GetVarStack()->Get(e_ptr->GetParamToStr(L"pVar"));

  if (var && var->type == SCSTR)
    {
	  e_ptr->GetVarStack()->SetStrElement(var, std::wstring(str));
      e_ptr->SetFunctionResult(L"_ReadIn", L"1");
    }
  else
    e_ptr->SetFunctionResult(L"_ReadIn", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReadIn", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scSystem(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSystem", L"[start]");

  wchar_t res[NUMSIZE];

  swprintf(res, L"%d", _wsystem(e_ptr->GetParamToStr(L"pCmd")));
  e_ptr->SetFunctionResult(L"_System", res);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSystem", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scLastError(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLastError", L"[start]");

  e_ptr->SetFunctionResult(L"_LastError", e_ptr->LastErr.c_str());

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLastError", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scConnectLib(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scConnectLib", L"[start]");

  std::wstring path = e_ptr->GetParamToStr(L"pPath");

  HINSTANCE h = e_ptr->LoadExtLib(path);

  if (h)
    {
      wchar_t res[NUMSIZE];
      swprintf(res, L"%d", (UINT)h);
      e_ptr->SetFunctionResult(L"_ConnectLib", res);
    }
  else
    e_ptr->SetFunctionResult(L"_ConnectLib", L"-1");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scConnectLib", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scFreeLib(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFreeLib", L"[start]");

  HINSTANCE h = (HINSTANCE)e_ptr->GetParamToInt(L"pHandle");

  if (h)
    {
      if (e_ptr->FreeExtLib(h))
        e_ptr->SetFunctionResult(L"_FreeLib", L"1");
      else
        e_ptr->SetFunctionResult(L"_FreeLib", L"0");
    }
  else
    e_ptr->SetFunctionResult(L"_FreeLib", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFreeLib", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scImportFunc(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scImportFunc", L"[start]");

  UINT hi = e_ptr->GetParamToInt(L"pHandle");
  HINSTANCE h = (HINSTANCE)hi;
  std::wstring ext_name = e_ptr->GetParamToStr(L"pExtName");
  std::wstring in_name = e_ptr->GetParamToStr(L"pInName");
  std::wstring args = e_ptr->GetParamToStr(L"pArgList");

  if (h)
    {
      IMPORTFUNC fptr;

      fptr = (IMPORTFUNC)GetProcAddress(h, AnsiOf(ext_name.c_str()));

      if (fptr)
        {
          e_ptr->AddFunction(in_name.c_str(), args.c_str(), fptr);
          EXTFUNC ef;
          ef.exthinst = h;
          wcscpy(ef.inname, in_name.c_str());
          e_ptr->GetExtFnStack()->push_back(ef);
          e_ptr->SetFunctionResult(L"_ImportFunc", L"1");
        }
      else
        e_ptr->SetFunctionResult(L"_ImportFunc", L"-1");
    }
  else
    e_ptr->SetFunctionResult(L"_ImportFunc", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scImportFunc", L"[end]");
}
//------------------------------------------------------

inline void __stdcall scDebugIntoFile(void *p)
{
  ELI *e_ptr = (ELI*)p;

  e_ptr->SetDebug(true, true);

  e_ptr->SetFunctionResult(L"_DebugIntoFile", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scDebugIntoFile", L"[start]");
}
//------------------------------------------------------

inline void __stdcall scDebugIntoScreen(void *p)
{
  ELI *e_ptr = (ELI*)p;

  e_ptr->SetDebug(true, false);

  e_ptr->SetFunctionResult(L"_DebugIntoScreen", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scDebugIntoScreen", L"[start]");
}
//------------------------------------------------------

inline void __stdcall scStopDebug(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStopDebug", L"[start]");

  e_ptr->SetDebug(false, false);

  e_ptr->SetFunctionResult(L"_StopDebug", L"0");
}
//------------------------------------------------------

///ф-ции, отвечающие за методы объектов
inline void __stdcall objCreate(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objCreate", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  std::wstring cath = e_ptr->GetParamToStr(L"objCathegory");
  std::wstring obname = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring ctor_args = e_ptr->GetParamToStr(L"objCtorParams");
  RESOURCE res;

  res.ObjectCathegory = cath;
  res.ObjectID = obname;
  res.PropertyID = L"Owner";
  res.Value = L"<none>";

  std::wstring name = obname;
  name.erase(0, 1);

  if (!e_ptr->IsCorrectName(name.c_str()))
    {
      e_ptr->AddInfoMsg(OBJNAMEERR, WRNMSG, ind);
      e_ptr->AddInfoMsg(OBJNOCRT, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Create", L"0");
    }
  else if (e_ptr->GetObjStack()->Get(obj_id, obname).size() > 0)
    {
	  e_ptr->AddInfoMsg(OBJNOCRT, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Create", L"0");
    }
  else if (e_ptr->GetObjStack()->Add(res) < 1)
    {
      e_ptr->AddInfoMsg(OBJNOCRT, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Create", L"0");
    }
  else
    {
//проверим соответствует ли имя категории имени известного интерпретатору класса
      RESRECORDSET rs = e_ptr->GetClassStack()->Get(obj_id, cath);

      if (rs.size() > 0)
        {
//добавим все свойства присущие классу к созданному объекту
          for (UINT i = 0; i < rs.size(); i++)
            {
              res.PropertyID = rs[i]->PropertyID;
              res.Value = rs[i]->Value;

//если значение свойства это класс, выполним создание временного объекта
			  if (res.Value.find(L"#class") != std::wstring::npos)
                {
				  res.Value.erase(0, 6);
                  res.Value = e_ptr->CreateTempObject(res.Value, obname, ind);
                }

              e_ptr->GetObjStack()->Add(res);
            }

//и выполним конструктор класса, если он описан
		  std::wstring def_ctor = obname + OBJPROPSEPSTR + cath + L"(";

		  if (ctor_args != L"")
            def_ctor += ctor_args + L")";
          else
            def_ctor += L")";

          if (!e_ptr->CompileLine(def_ctor.c_str(), ind))
            e_ptr->AddInfoMsg(OBJNOCTOR, WRNMSG, ind);
        }

//добавим служебное свойство - имя объекта
      res.PropertyID = L"ObjectName";
      res.Value = name;

      e_ptr->GetObjStack()->Add(res);

      e_ptr->SetFunctionResult(L"Create", L"1");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objCreate", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objDestroy(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objDestroy", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  RESRECORDSET rs = e_ptr->GetObjStack()->Get(obj_id, std::wstring(e_ptr->GetParamToStr(P_OBJNAME)));

  if (rs.size() > 0)
    {
      for (UINT i = 0; i < rs.size(); i++)
        {
          rs[i]->KeepInStack = NO;
//отыщем и пометим на удаление все объекты-свойства удаляемого объекта
		  if (rs[i]->Value.find(OBJSYM) != std::wstring::npos)
            {
              RESRECORDSET chld_rs = e_ptr->GetObjStack()->Get(obj_id, rs[i]->Value);

              for (UINT j = 0; j < chld_rs.size(); j++)
                chld_rs[j]->KeepInStack = NO;
            }
        }

      e_ptr->GetObjStack()->Compact();

      e_ptr->SetFunctionResult(L"Destroy", L"1");
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNONE, WRNMSG, ind);
      e_ptr->AddInfoMsg(OBJNODESTR, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Destroy", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objDestroy", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objAdd(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objAdd", L"[start]");

  UINT ind = e_ptr->GetParamStack()->Get(P_IND)->ToInt(); //получаем индекс строки
//проверим, нет ли у объекта такого свойства
  RESRECORDSET rs = e_ptr->GetObjStack()->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											  std::wstring(e_ptr->GetParamToStr(L"objNewPropName")));

  if (rs.size() > 0)
    {
	  e_ptr->AddInfoMsg(OBJPROPDUP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Add", L"0");
	}
  else if (!e_ptr->GetObjCathegory(e_ptr->GetParamStack()->Get(P_OBJNAME)->ToStr()))
    {
      e_ptr->AddInfoMsg(OBJNONE, ERRMSG, ind);

      e_ptr->SetFunctionResult(L"Add", L"0");
    }
  else
    {
      RESOURCE res;

      res.ObjectCathegory = e_ptr->GetObjCathegory(e_ptr->GetParamStack()->Get(P_OBJNAME)->ToStr());
      res.ObjectID = e_ptr->GetParamToStr(P_OBJNAME);
      res.PropertyID = e_ptr->GetParamToStr(L"objNewPropName");
      res.Value = e_ptr->GetParamToStr(L"objNewPropVal");

 //если значение свойства это класс, выполним создание временного объекта
	  if (res.Value.find(L"#class") != std::wstring::npos)
        {
		  res.Value.erase(0, 6);
          res.Value = e_ptr->CreateTempObject(res.Value, res.ObjectID, ind);
        }

      if (e_ptr->GetObjStack()->Add(res) < 1)
        {
          e_ptr->AddInfoMsg(OBJPROPERR, WRNMSG, ind);

          e_ptr->SetFunctionResult(L"Add", L"0");
        }
      else
        e_ptr->SetFunctionResult(L"Add", L"1");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objAdd", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objRemove(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objRemove", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  int result = e_ptr->GetObjStack()->Delete(L"",
											std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											std::wstring(e_ptr->GetParamToStr(L"objPropName")));

  if (result == -1)
    {
      e_ptr->AddInfoMsg(OBJINDERR, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Remove", L"0");
    }
  else if (result == 0)
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Remove", L"0");
    }
  else
    e_ptr->SetFunctionResult(L"Remove", L"1");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objRemove", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objExist(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExist", L"[start]");

  if (e_ptr->GetObjStack()->Get(obj_id, std::wstring(e_ptr->GetParamToStr(P_OBJNAME))).size() > 0)
    e_ptr->SetFunctionResult(L"Exist", L"1");
  else
    e_ptr->SetFunctionResult(L"Exist", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExist", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objHave(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objHave", L"[start]");

  std::wstring name = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring prop = e_ptr->GetParamToStr(L"objPropName");

  if (e_ptr->GetObjStack()->Get(name, prop).size() > 0)
    e_ptr->SetFunctionResult(L"Have", L"1");
  else
    e_ptr->SetFunctionResult(L"Have", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objHave", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objKeep(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objKeep", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  RESRECORDSET rs = e_ptr->GetObjStack()->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											  std::wstring(e_ptr->GetParamToStr(L"objPropName")));

  if (rs.size() == 1)
    {
      rs[0]->KeepInStack = e_ptr->GetParamToStr(L"objBool");
      e_ptr->SetFunctionResult(L"Keep", L"1");
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Keep", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objKeep", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objSave(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objSave", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  RESRECORDSET rs = e_ptr->GetObjStack()->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											  std::wstring(e_ptr->GetParamToStr(L"objPropName")));

  if (rs.size() == 1)
    {
      rs[0]->SaveInFile = e_ptr->GetParamToStr(L"objBool");
      e_ptr->SetFunctionResult(L"Save", L"1");
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Save", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objSave", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objExecute(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExecute", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  std::wstring obj_name = e_ptr->GetParamToStr(P_OBJNAME);

  RESRECORDSET rs = e_ptr->GetObjStack()->Get(obj_name,
                                              std::wstring(e_ptr->GetParamToStr(L"objPropName")));

  if (rs.size() == 1)
    {
      wchar_t cmp_line[128];

	  if (rs[0]->Value.find(FRGMARK) != std::wstring::npos)
        {
          e_ptr->AddInfoMsg(NOPCCODE, WRNMSG, ind);
          e_ptr->SetFunctionResult(L"Execute", L"0");
        }
      else
        {
//уберем первый символ (&), потому что процедуре нужно передать имя объекта без спецсимвола
		  obj_name.erase(0, 1);
          swprintf(cmp_line, L"#procedure proc%s($this)%s", obj_name.c_str(), rs[0]->Value.c_str());

          e_ptr->CompileLine(cmp_line, ind);
          swprintf(cmp_line, L":proc%s(%s)", obj_name.c_str(), obj_name.c_str());

          if (e_ptr->CompileLine(cmp_line, ind))
            e_ptr->SetFunctionResult(L"Execute", L"1");
          else
            e_ptr->SetFunctionResult(L"Execute", L"0");

          swprintf(cmp_line, L"#drop proc%s", obj_name.c_str());
          e_ptr->CompileLine(cmp_line, ind);
        }
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Execute", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExecute", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objShow(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objShow", L"[start]");

  std::wstring obj_name = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring outstr;

  RESRECORDSET rs = e_ptr->GetObjStack()->Get(obj_id, obj_name);

  if (rs.size() > 0)
    {
      for (UINT i = 0; i < rs.size(); i++)
        outstr += rs[i]->PropertyID + L" = " + rs[i]->Value + L"\r\n";
    }

  wprintf(outstr.c_str());

  e_ptr->SetFunctionResult(L"Show", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objShow", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objExportIn(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExportIn", L"[start]");

  std::wstring obj_name = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring prop_list = e_ptr->GetParamToStr(L"pPropNames");
  std::wstring val_list = e_ptr->GetParamToStr(L"pPropVals");
  prop_list = L"&" + prop_list;
  val_list = L"&" + val_list;

//удаляем объекты-списки, если они уже были инициализированы
  e_ptr->GetObjStack()->Delete(L"ExpObjList", prop_list, L"");
  e_ptr->GetObjStack()->Delete(L"ExpObjList", val_list, L"");

  RESRECORDSET rs = e_ptr->GetObjStack()->Get(obj_id, obj_name);
  wchar_t str[NUMSIZE];
  RESOURCE prop, val;
  prop.ObjectCathegory = L"ExpObjList";
  prop.ObjectID = prop_list;
  val.ObjectCathegory = L"ExpObjList";
  val.ObjectID = val_list;

  if (rs.size() > 0)
    {
      UINT i = 0;

      for (i = 0; i < rs.size(); i++)
        {
          swprintf(str, L"%d", i);
          prop.PropertyID = str;
          prop.Value = rs[i]->PropertyID;
          val.PropertyID = str;
          val.Value = rs[i]->Value;
          e_ptr->GetObjStack()->Add(prop);
          e_ptr->GetObjStack()->Add(val);
        }

//добавим свойство в котором указано кол-во элементов
      swprintf(str, L"%d", i);
      prop.PropertyID = L"Count";
      prop.Value = str;
      val.PropertyID = L"Count";
      val.Value = str;
      e_ptr->GetObjStack()->Add(prop);
      e_ptr->GetObjStack()->Add(val);
    }

  e_ptr->SetFunctionResult(L"ExportIn", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExportIn", L"[end]");
}
//------------------------------------------------------

inline void __stdcall objGetName(void *p)
{
  ELI *e_ptr = (ELI*)p;

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objGetName", L"[start]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);
  RESRECORDSET rs = e_ptr->GetObjStack()->Get(objname, L"ObjectName");

  if (rs.size() == 1)
    {
      e_ptr->SetFunctionResult(L"GetName", rs[0]->Value.c_str());
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"GetName", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objGetName", L"[end]");
}
//------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
