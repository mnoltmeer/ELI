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

#ifndef mainH
#define mainH

#include "stacks.h"

String LogPath;
wchar_t initdir[4096], path[4096];
std::wstring debugfile;

class ELI: public ELI_INTERFACE
{
  private:
    bool write_log;
    bool debug_eli;
	bool use_return;
	std::wstring scrtext; //текст скрипта, с которым будет работать dll
	UINT CstrInd; //глобальный индекс присваиваемый конст. строкам
	UINT CnumInd; //глобальний індекс, що призначається числовим константам
	UINT FrgmntNum; //глобальный индекс нумерации фрагментов кода
    UINT TmpObjInd; //глобальный индекс нумерации временных объектов
	std::wstring InfStack; //стек сообщений интерпретатора
	std::wstring ScriptResult; //значение возвращаемое скриптом
	bool use_false; //флаг, сообщающий, что нужно использовать секцию else
	std::wstring current_func_name; //имя текущей вызванной ф-ии
	std::wstring current_class; //имя текущего объявляемого класса
	std::wstring return_val; //значение возвращаемое методом
	bool trigger_check; //флаг, який повідомляє, що відбувається перевірка тригеру
						//тож не треба писати повідомлення в лог
	bool settings_change; //флаг що визначає момент, коли код вносить зміни
						  //у налаштування інтерпретатора
	SETTINGS InterpreterSettings; //налаштування інтерпретатора
	std::vector<std::wstring> vecScList; //вектор для хранения строк скрипта
    FRAGMENTSTACK *frgStack;
	std::vector<VARSTACK*> vecVSt; //вектор указателей на стеки переменных
								   //нулевой элемент указывает на глобальный стек
	std::vector<HINSTANCE> vecLibs; //вектор для дескрипторов внешних библиотек
	std::vector<EXTFUNC> vecExtFuncs; //вектор со списком импортированных ф-й
	std::vector<REFERENCE> vecRefs; //вектор со списком ссылок
	std::vector<TRIGGER> vecTriggers; //вектор з переліком доступних тригерів
	std::wstring FStrBuffer; //буфер для обміну текстовою інформацією між методами класу
	SCRIPTLINES FCodeBuffer; //буфер для обміну рядками скрипту
    float FNumBuffer;

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
	void InitTranslatorFuncs();
//подготавливает интерпретатор к запуску нового скрипта
	void FreeRes();
//компилирует фрагмент кода (тело цикла или условия)
	bool TranslateFragment(std::vector<std::wstring> *vecFragment, UINT index);
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
//парсить текст скрипту на наявність числових констант
//вносить їх у стек як змінні та замінює у тексті на імена змінних
	const wchar_t *ParseConstNumbers(wchar_t *text);
//убирает пробелы
	const wchar_t *RemoveSpaces(const wchar_t *text);
//прибирає переноси рядків
	std::wstring RemoveEndlines(std::wstring text);
//заменяет директивы #include в исходном тексте скрипта на содержимое инклуд-файлов
	std::vector<std::wstring> GetInclude(const wchar_t *str);
//разбивает скрипт на строки и заполняет вектор
//разделителем строк является ENDLN
	void PrepareScript();
//проверяет на корректность и компилирует каждую строку скрипта
//возвращает 1, если скрипт скомпилирован, 0 - если произошла ошибка
	bool TranslateScriptLines();
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
	bool ExpIf(wchar_t *line, UINT index); //if (lval == rval){};
	bool ExpElseIf(wchar_t *line, UINT index); //else if (lval == rval){};
	bool ExpElse(wchar_t *line, UINT index);
	bool ExpFor(wchar_t *line, UINT index); //for ($var, <= 10, +1) for (0, <= 10, +1) {};
	bool ExpCount(wchar_t *line, UINT index); //count(10) {};
	bool ExpWhile(wchar_t *line, UINT index); //while(lval != rval) {};
	bool ExpSelect(wchar_t *line, UINT index); //select ($var){when 10 then {_return(1)}}
	bool ExpWhen(wchar_t *line, UINT index);

	bool RunFunc(wchar_t *str_with_func, wchar_t *result, UINT index);
	bool CallFunc(FUNC *fn_ptr, wchar_t *result, UINT index);
	bool CreateProcedure(wchar_t *str_with_proc, UINT index);
	bool DropProcedure(const wchar_t *proc_name);
	bool RunProcedure(const wchar_t *name, const wchar_t *params, UINT index);
	bool WorkWithObject(wchar_t *str_with_obj, wchar_t *result, UINT index);
	bool RunMethod(const wchar_t *objname, const wchar_t *cl_name, wchar_t *str_with_method, UINT index);
	bool AddRef(const wchar_t *name, const wchar_t *val);
	REFERENCE *GetRef(const wchar_t *name);
	bool ImportParentClass(std::wstring child, std::wstring parent, bool type);
//выполняет защищенную трансляцию кода, код будет выполнен даже при наличии исключений
	bool ProtectTranslate(wchar_t *str, UINT index);
//робота з тригерами
	bool CreateTrigger(wchar_t *str, UINT index);
    bool RemoveTrigger(wchar_t *str, UINT index);
	bool CheckTrigger(TRIGGER *trigger);
	void RunTrigger(TRIGGER *trigger);
	bool TriggerExists(TRIGGER *trigger);
	void CheckTriggers();
//обробка директив
	bool ProcessDirective(wchar_t *str, UINT index);
//створює нові записи в стеку класів, на основі рядка з ProcessDirective()
	bool AddClassProperty(std::wstring &cl_name, std::wstring &prop_str, bool is_public, UINT index);
	bool AddClassMethod(std::wstring &cl_name, std::wstring &method_str, bool is_public, UINT index);
	void SearchAndMarkGlobalFragments();
	void MarkGlobalFragment(FRAGMENTCODE *frg);

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
	virtual void __stdcall SetFunctionResult(const wchar_t *name, const wchar_t *result);
//устанавливает новое значение параметра или добавляет новый параметр
	virtual void __stdcall SetParam(const wchar_t *name, const wchar_t *new_val);
//преобразует параметр в integer и возвращает его
	virtual int __stdcall GetParamToInt(const wchar_t *name);
//преобразует параметр в float и возвращает его
	virtual float __stdcall GetParamToFloat(const wchar_t *name);
//преобразует параметр в строку и возвращает указатель на нее
//в случае ошибки возвращает NULL
	virtual const wchar_t * __stdcall GetParamToStr(const wchar_t *name);
//повертає значення властивості об'єкта (властивість має бути публічною)
	virtual const wchar_t * __stdcall GetObjectProperty(const wchar_t *obj_name,
														const wchar_t *prop_name);
//встановлює значення властивості об'єкта (властивість має бути публічною)
	virtual bool __stdcall SetObjectProperty(const wchar_t *obj_name,
											 const wchar_t *prop_name,
											 const wchar_t *val);

	virtual const wchar_t * __stdcall GetVersion();
	virtual const wchar_t * __stdcall ShowVarStack();
	virtual const wchar_t * __stdcall ShowObjStack();
	virtual const wchar_t * __stdcall ShowClassStack();
	virtual const wchar_t * __stdcall ShowProcStack();
	virtual const wchar_t * __stdcall ShowInfoMessages();
	virtual const wchar_t * __stdcall RunScript(const wchar_t *imptext,
												const wchar_t *parameter,
												bool log);
	virtual const wchar_t * __stdcall RunScriptFromFile(const wchar_t *filepath,
														const wchar_t *parameter,
														bool log);
	virtual const wchar_t * __stdcall ShowFuncStack();
	virtual const wchar_t * __stdcall ShowParamStack();
	virtual const wchar_t * __stdcall ShowFragmentStack();
	virtual const wchar_t * __stdcall GetCurrentFuncName();
	virtual void __stdcall SetDebug(bool enable_dbg, bool in_file);
	virtual bool __stdcall DebugEnabled();
	virtual const wchar_t* __stdcall GetInitDir();
	virtual void __stdcall AddToLog(const wchar_t *msg);

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
	bool IsClassMember(const wchar_t *cl_name, const wchar_t *mb_name);
//проверяет является ли член класса публичным
    bool IsPublicMember(const wchar_t *cl_name, const wchar_t *mb_name);
//проверяет доступность члена
    bool IsAccessibleMember(const wchar_t *obj_name, const wchar_t *mb_name);
//компилирует строку
	bool TranslateLine(const wchar_t *line, UINT index);
//возвращает категорию объекта с указанным именем
    const wchar_t *GetObjCathegory(const wchar_t *obj_name);
//маркирует начало и конец фрагмента в коде
//заменяет открывающую и закрывающую фигурную скобку на соотв метки
	std::wstring MarkFragments(std::wstring &operstr);
	bool MakeCodeInVar(wchar_t *str, UINT index);
	bool TranslateCodeFromVar(const wchar_t *name, UINT index);
	HINSTANCE LoadExtLib(std::wstring &path);
	bool FreeExtLib(HINSTANCE hnd);
	const wchar_t *CreateTempObject(std::wstring str, std::wstring owner, UINT index);
//видаляє дані об'єкта, за необхідності також викликає деструктор класу
	bool DestroyObject(std::wstring &obj_name, UINT index);
//імпортує властивості/методи з об'єкта/класу
	bool ImportMemberFromObject(std::wstring &obj_name, std::wstring &src_name, std::wstring &mb_name, UINT index);
    bool ImportMemberFromClass(std::wstring &obj_name, std::wstring &cl_name, std::wstring &mb_name, UINT index);

	__property PARAMSTACK *ParamStack = {read = pStack};
	__property FUNCSTACK *FuncStack = {read = fStack};
	__property RESOURCESTACK *ObjStack = {read = objStack};
	__property RESOURCESTACK *ClassStack = {read = clStack};
	__property RESOURCESTACK *ProcStack = {read = procStack};
	__property VARSTACK *VarStack = {read = st};
	__property FRAGMENTSTACK *FragmentStack = {read = frgStack};
	__property std::vector<EXTFUNC> ExtFnStack = {read = vecExtFuncs};
	__property bool ReturnEnabled = {read = use_return, write = use_return};
	__property SETTINGS *Settings = {read = InterpreterSettings};
	__property std::wstring Result = {read = ScriptResult, write = ScriptResult};
};
//-------------------------------------------------------------------------------

///встроенные ф-ии для скриптового языка
void __stdcall scRandom(void *p);
void __stdcall scRound(void *p);
void __stdcall scInt(void *p);
void __stdcall scStrLen(void *p);
void __stdcall scStrEq(void *p);
void __stdcall scIStrEq(void *p);
void __stdcall scSubStr(void *p);
void __stdcall scReturn(void *p);
void __stdcall scThrow(void *p);
void __stdcall scFree(void *p);
void __stdcall scLoadObjStack(void *p);
void __stdcall scSaveObjStack(void *p);
void __stdcall scSaveObjects(void *p);
void __stdcall scCompactObjStack(void *p);
void __stdcall scClearObjStack(void *p);
void __stdcall scRemoveObjects(void *p);
void __stdcall scRun(void *p);
void __stdcall scGetParamAsNum(void *p);
void __stdcall scGetParamAsStr(void *p);
void __stdcall scSetParam(void *p);
void __stdcall scLoadFileToVar(void *p);
void __stdcall scSaveVarToFile(void *p);
void __stdcall scSaveFragmentToFile(void *p);
void __stdcall scGetConfig(void *p);
void __stdcall scSaveState(void *p);
void __stdcall scSaveVarStack(void *p);
void __stdcall scWriteOut(void *p);
void __stdcall scReadIn(void *p);
void __stdcall scSystem(void *p);
void __stdcall scLastError(void *p);
void __stdcall scConnectLib(void *p);
void __stdcall scFreeLib(void *p);
void __stdcall scImportFunc(void *p);
void __stdcall scDebugIntoFile(void *p);
void __stdcall scDebugIntoScreen(void *p);
void __stdcall scStopDebug(void *p);
void __stdcall scSleep(void *p);
void __stdcall scShowMessage(void *p);

///ф-ции, отвечающие за методы объектов
void __stdcall objCreate(void *p);
void __stdcall objDestroy(void *p);
void __stdcall objAdd(void *p);
void __stdcall objRemove(void *p);
void __stdcall objExist(void *p);
void __stdcall objHave(void *p);
void __stdcall objKeep(void *p);
void __stdcall objSave(void *p);
void __stdcall objExecute(void *p);
void __stdcall objShow(void *p);
void __stdcall objClone(void *p);
void __stdcall objGetName(void *p);
void __stdcall objImport(void *p);

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
