/*!
Copyright 2017-2019, 2021 Maxim Noltmeer (m.noltmeer@gmail.com)

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

#ifndef RESOURCESTACK_H_INCLUDED
#define RESOURCESTACK_H_INCLUDED

#include <cstdlib>
#include <fstream>
#include <vector>
#include <string.h>
#include "..\work-functions\MyFunc.h"

#define UINT unsigned int
#define NUMSIZE 24
#define MAXBUF 4096      //макс. буфер для чтения строк из файла ресурсов
#define FIELDSDELIM L"|"  //разделитель значений полей в файле ресурсов
#define SAVEFIELDCOUNT 4 //кол-во полей структуры RESOURCE сохраняемых в файл

#define YES L"1"
#define NO L"0"

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

typedef std::vector<std::wstring> (StrList);
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

#endif // RESOURCESTACK_H_INCLUDED
