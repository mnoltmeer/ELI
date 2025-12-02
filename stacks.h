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

#ifndef stacksH
#define stacksH

#include "defines.h"

//константы определяющие свойство ресурса
enum {
       indx = 0,
       obj_cath = 1,
       obj_id = 2,
       prop_id = 3,
       val_v = 4,
       is_keep = 5,
       is_save = 6
     };

//структура определяющая параметры для выборки для ф-й SelectRes() и Aquire()
struct CONDITION
{
  UINT type;
  std::wstring value;
};

struct RESOURCE
{
  UINT Index;                    //индекс
  std::wstring ObjectCathegory;  //категория объекта-владельца
  std::wstring ObjectID;         //ID объекта-владельца
  std::wstring PropertyID;       //ID свойства
  std::wstring Value;            //значение
  std::wstring KeepInStack;      //хранить ресурс в стеке после компакта
  std::wstring SaveInFile;       //сохранять ли значение ресурса в файл на диске
};

//определение для набора указателей на ресурсы стека
typedef std::vector<RESOURCE*> (RESRECORDSET);

class RESOURCESTACK
{
  private:
	UINT next_id;
	std::vector<RESOURCE> Stack;
	UINT GetNextID();
//делает выборку из стека ресурсов по определенному значению, возвращает вектор указателей
//на те элементы, которые удовлетворяют аргументу value
	RESRECORDSET SelectRes(CONDITION cond);
//уточняет выборку из ф-ии SelectRes по значению value, возвращает вектор указателей
	RESRECORDSET Aquire(RESRECORDSET source, CONDITION cond);
//выгружает в список только те ресурсы, у которых SaveInFile = 1
	StrList *ExportStrings();
	StrList *ExportStrings(std::wstring cath);

  public:
	RESOURCESTACK();
	RESOURCESTACK(const wchar_t *filename);
	virtual ~RESOURCESTACK();

	UINT StackSize();

	const wchar_t *StackInString();

//делает выборку ресурсов по заданому набору условий
	RESRECORDSET Get(std::vector<CONDITION> *conditions);
	RESRECORDSET Get(UINT type, std::wstring val);
	RESRECORDSET Get(std::wstring obj_name, std::wstring prop_name);
	std::vector<int> *Get(UINT type, const wchar_t *val);

	int Add(RESOURCE newres);
	int Delete(std::wstring ObjectCathegory, std::wstring ObjectID, std::wstring ResourceID);
	int Delete(UINT index);
	int CreateResFile(const wchar_t *filepath, bool overwrite);
	int CreateResFile(const wchar_t *filepath, const wchar_t *res_cath);
	int LoadResFile(const wchar_t *filepath);
	void Compact();
	void Clear();
};

struct EXTFUNC
{
  wchar_t inname[MAXNAMELEN]; //внутреннее имя
  HINSTANCE exthinst;      //дескриптор вн. либы
};

class PARAM
{
  private:
	std::wstring val; //значение

  public:
	PARAM(const wchar_t* p_name, const wchar_t *p_value);
	inline virtual ~PARAM(){};

	std::wstring name; //имя параметра

	inline void Set(const wchar_t *new_val){val = new_val;}

//преобразует параметр в integer и возвращает его
	int ToInt();

//преобразует параметр в float и возвращает его
	inline float ToFloat(){return _wtof(val.c_str());}
//-------------------------------------------------------------------------------

//преобразует параметр в строку и возвращает указатель на нее
//в случае ошибки возвращает NULL
	inline const wchar_t *ToStr(){return val.c_str();}
};
//-------------------------------------------------------------------------------

class PARAMSTACK
{
  private:
	std::vector<PARAM*> Stack;

//ищет параметр в стеке по имени и возвращает индекс
//-1 - не найдено
	int GetParamInd(const wchar_t *name);

  public:
    inline PARAMSTACK(){};
    inline virtual ~PARAMSTACK(){Clear();}

//добавляет параметр в конец стека
	void Add(const wchar_t *name, const wchar_t *val);

//возвращает количество параметров в стеке
	inline UINT Count(){return Stack.size();}

//возвращает форматированную строку со списком всех ф-й
	wchar_t *ShowInString();

	PARAM *Get(const wchar_t *name);

//очищает стек параметров
	void Clear();
};
//-------------------------------------------------------------------------------

class FUNC
{
  private:
    wchar_t *name;        //имя ф-ии
    func_ptr ptr;      	  //указатель на ф-ю из хост-приложения или библиотеки
	wchar_t *prms_format; //строка определяющая имена и тип параметров ф-и
						  //num val,sym val
	wchar_t *return_val;  //возвращаемое значение

  public:
	FUNC(const wchar_t *name, const wchar_t *params, func_ptr fptr);
	virtual ~FUNC();

	inline wchar_t *GetName(){return name;}
	inline wchar_t *GetParams(){return prms_format;}
	inline func_ptr GetPointer(){return ptr;}
//вызывает функцию, возвращает 1 в случае успеха
	inline void Call(void *e_ptr){ptr(e_ptr);}
//преобразует возвращаемое значение ф-ии name в строку и возвращает указатель на нее
//в случае ошибки возвращает NULL
	inline wchar_t *GetResult(){return return_val;}
//устанавливает возвращаемое значение ф-ии
    inline void SetResult(const wchar_t* result){wcscpy(return_val, result);}
};
//-------------------------------------------------------------------------------

class FUNCSTACK
{
  private:
	std::vector<FUNC*> Stack;

//ищет ф-ю в стеке, возвращает ее индекс
//возвращает -1 в случае неудачи
	int GetInd(const wchar_t *fname);

  public:
	inline FUNCSTACK(){};
	virtual ~FUNCSTACK();

//возвращает количество функций в стеке
	inline UINT CountFuncs(){return Stack.size();};

//возвращает указатель на свойства функции с индексом st_ind из стека
//в случае ошибки возвращает NULL
	inline FUNC *Get(UINT st_ind)
	{
	  if (st_ind >= Stack.size())
		return NULL;
	  else
		return Stack[st_ind];
	};

//возвращает указатель на свойства функции name из стека
//в случае ошибки возвращает NULL
	FUNC *Get(const wchar_t *fname);

//добавляет ф-ю в стек
	void Add(const wchar_t *name, const wchar_t *params, func_ptr fptr);

//удаляет ф-ю из стека
	void Delete(const wchar_t *name);

//возвращает форматированную строку со списком всех ф-й
	wchar_t *ShowInString();
};
//-------------------------------------------------------------------------------

//структура описывающая ассоциации скриптовых переменных
//и элементов стеков интерпретатора
struct VARIABLE
{
  wchar_t varname[MAXNAMELEN]; //имя переменной из скрипта
  UINT type;                   //тип стека: 1 - num, 2 - sym
  UINT ind;                    //индекс в стеке
  bool isfree;                 //признак того действующая переменная или нет
};
//-------------------------------------------------------------------------------

//структура описывающая ссылку
struct REFERENCE
{
  wchar_t refname[MAXNAMELEN];
  wchar_t refobj[CHARSIZE];
};
//-------------------------------------------------------------------------------

//структура що описує тригер
struct TRIGGER
{
  wchar_t condition[CHARSIZE]; //$x > 0, &obj.X == 0
  wchar_t fragment[CHARSIZE];
};
//-------------------------------------------------------------------------------

//структура у якій містяться налаштування інтерпретатора для зміни рантайм
struct SETTINGS
{
  bool ParseNumConst; //парсити числові константи перед трансляцією рядка
  bool ParseSymConst; //парсити символьні константи перед трансляцією рядка
  bool KeepObjects;  //зберігати вміст стеку об'єктів до кінця роботи ELI
  bool KeepClasses;  //зберігати вміст стеку класів до кінця роботи ELI
};
//-------------------------------------------------------------------------------

class VARSTACK
{
  private:
	std::vector<VARIABLE> stMain;
    std::vector<float> stNum;
	std::vector<std::wstring> stStr;

  public:
    inline VARSTACK(){};
    inline virtual ~VARSTACK(){ClearStack();}

	VARIABLE *GetFirstFree(UINT type);
	VARIABLE *Get(const wchar_t *varname);
	VARIABLE *GetByValue(std::wstring val);
	VARIABLE *GetByValue(float val);
	bool Add(wchar_t *name, std::wstring val);
	bool Add(wchar_t *name, float val);
	bool Remove(const wchar_t *name);
	const wchar_t *ShowInString();

	inline void ClearStack()
	{
	  stMain.clear();
	  stNum.clear();
	  stStr.clear();
	}

	inline float GetNumElement(VARIABLE *var){return stNum[var->ind];}
	inline void SetNumElement(VARIABLE *var, float val){stNum[var->ind] = val;}
	inline std::wstring GetStrElement(VARIABLE *var){return stStr[var->ind];}
	inline void SetStrElement(VARIABLE *var, std::wstring val){stStr[var->ind] = val;}
};
//-------------------------------------------------------------------------------

//структура описывающая часть выражения (символ операции и аргумент)
//пример: "+2" -> arg=2;oper='+'
struct SUBEXP
{
  float arg;  //значение аргумента
  wchar_t oper;  //символ операции
};

//структура хранит позиции соотв друг другу открывающей и закрывающей скобок
struct SCPOS
{
  int oppos;  //позиция открывающей скобки
  int clpos;  //позиция закрывающей
};

class FRAGMENTCODE
{
  private:
	SCRIPTLINES FCode;
	std::wstring FMark;
	bool FGlobal;

  public:
	FRAGMENTCODE(const wchar_t *mark, SCRIPTLINES *code, bool global);
	inline FRAGMENTCODE(){};
	inline virtual ~FRAGMENTCODE(){FCode.clear();}

	inline void SetMark(const wchar_t *new_mark){FMark += new_mark;}
	inline const wchar_t *GetMark(){return FMark.c_str();}
	inline void SetGlobal(){FGlobal = true;}
	inline void SetLocal(){FGlobal = false;}
	inline bool IsGlobal(){return FGlobal;}
	inline SCRIPTLINES *GetCode(){return &FCode;};

	const wchar_t *GetCodeStrings();
};
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

class FRAGMENTSTACK
{
  private:
	std::vector<FRAGMENTCODE*> FStack;

	inline int GetSize(){return FStack.size();}

	FRAGMENTCODE *ReadFragments(int ind);

  public:
	inline FRAGMENTSTACK(){};
	inline virtual ~FRAGMENTSTACK(){};

	void Add(std::wstring frg_str, std::wstring mark, bool global);
	void Remove(std::wstring mark);
	SCRIPTLINES *GetFragmentCode(std::wstring mark);
	FRAGMENTCODE *Get(std::wstring mark);
	void ClearFragments(bool all);
	const wchar_t *ShowInString();

	__property FRAGMENTCODE *Fragments[int ind] = {read = ReadFragments};
	__property int Count = {read = GetSize};
};
//-------------------------------------------------------------------------------
#endif
