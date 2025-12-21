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
#include <cstdlib>
#include <fstream>
#include <vector>
#include <iterator>
#include <string.h>

#pragma hdrstop

#include "..\work-functions\Logs.h"
#include "..\work-functions\Data.h"
#include "oldfuncs.h"
#include "stacks.h"

extern String LogPath;

RESOURCESTACK::RESOURCESTACK()
{
  next_id = Stack.size();
}
//-------------------------------------------------------------------------------

RESOURCESTACK::RESOURCESTACK(const wchar_t *filename)
{
  LoadResFile(filename);
  next_id = Stack.size();
}
//-------------------------------------------------------------------------------

RESOURCESTACK::~RESOURCESTACK()
{
  Stack.clear();
}
//-------------------------------------------------------------------------------

UINT RESOURCESTACK::GetNextID()
{
  return next_id;
}
//-------------------------------------------------------------------------------

UINT RESOURCESTACK::StackSize()
{
  return Stack.size();
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::Add(RESOURCE newres)
{
  try
     {
       newres.Index = next_id;
       newres.KeepInStack = YES;
       newres.SaveInFile = YES;
	   Stack.push_back(newres);

       next_id++;
     }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::Add: " + e.ToString());

       return 0;
     }

  return 1;
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::Delete(UINT index)
{
  for (auto itm : Stack)
    {
	  if (itm.Index == index)
        {
          itm.KeepInStack = NO;

          return 1;
        }
    }

  return 0;
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::Delete(std::wstring ObjectCathegory,
						  std::wstring ObjectID,
						  std::wstring ResourceID)
{
  try
	 {
	   std::vector<CONDITION> conds;

//создадим условия
	   if (ObjectCathegory != L"")
		 conds.push_back({obj_cath, ObjectCathegory});

	   if (ObjectID != L"")
		 conds.push_back({obj_id, ObjectID});

	   if (ResourceID != L"")
		 conds.push_back({prop_id, ResourceID});

//найдем соотв. условиям набор свойств
	   RESRECORDSET rs = Get(&conds);
	   std::vector<int> inds;

	   if (rs.size() == 0)
		 throw Exception("No matching results");

	   for (auto itm : rs)
		  inds.push_back(itm->Index);

	   for (auto itm : inds)
		  {
			if (Delete(itm) < 1)
			  return -1;
		  }

	   Compact();
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::Delete: " + e.ToString());

	   return 0;
	 }

  return 1;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(std::vector<CONDITION> *conditions)
{
  RESRECORDSET result;

  try
	 {
	   if (conditions->size() > 1)
		 {
		   result = SelectRes(conditions->at(0));

		   for (UINT i = 1; i < conditions->size(); i++)
			  result = Aquire(result, conditions->at(i));
		 }
		else
		  result = SelectRes(conditions->at(0));
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::Get: " + e.ToString());

	   result.clear();
	 }

  return result;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(UINT type, std::wstring val)
{
  return SelectRes({type, val});
}
//-------------------------------------------------------------------------------

std::vector<int> RESOURCESTACK::Get(UINT type, const wchar_t *val)
{
  std::vector<int> res;

  try
	 {
	   res.clear();

	   RESRECORDSET rs = SelectRes({type, val});

	   for (UINT i = 0; i < rs.size(); i++)
		  res.push_back(rs[i]->Index);
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::Get: " + e.ToString());

       res.clear();
	 }

  return res;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(std::wstring obj_name, std::wstring prop_name)
{
  std::vector<CONDITION> conds;

  conds.push_back({obj_id, obj_name});
  conds.push_back({prop_id, prop_name});

  return Get(&conds);
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::SelectRes(CONDITION cond)
{
  RESRECORDSET result;

  try
	 {
	   switch (cond.type)
		 {
		   case indx:
			 {
			   UINT intval;

			   try
				  {
					intval = _wtoi(cond.value.c_str());
				  }
			   catch (Exception &e)
				  {
					throw e;
					break;
				  }

			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].Index == intval)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case obj_cath:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].ObjectCathegory == cond.value)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case obj_id:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].ObjectID == cond.value)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case prop_id:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].PropertyID == cond.value)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case val_v:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].Value == cond.value)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case is_keep:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].KeepInStack == cond.value)
					  result.push_back(&Stack[i]);
				  }

			   break;
			 }
		   case is_save:
			 {
			   for (int i = 0; i < Stack.size(); i++)
				  {
					if (Stack[i].SaveInFile == cond.value)
					  result.push_back(&Stack[i]);
				  }

		  break;
			 }
		 }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::SelectRes: " + e.ToString());

	   result.clear();
	 }

  return result;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Aquire(RESRECORDSET source, CONDITION cond)
{
  RESRECORDSET result;

  switch (cond.type)
    {
      case indx:
		{
		  UINT intval;

		  try
			 {
			   intval = _wtoi(cond.value.c_str());
			 }
		  catch (Exception &e)
			 {
			   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
			   break;
			 }

		  for (int i = 0; i < source.size(); i++)
			 {
			   if (source[i]->Index == intval)
				 result.push_back(source[i]);
			 }

		  break;
		}
      case obj_cath:
		{
		  for (int i = 0; i < source.size(); i++)
			 {
			   if (source[i]->ObjectCathegory == cond.value)
				 result.push_back(source[i]);
			 }

		  break;
		}
      case obj_id:
		{
		  for (int i = 0; i < source.size(); i++)
			{
			  if (source[i]->ObjectID == cond.value)
				result.push_back(source[i]);
			}

		  break;
		}
      case prop_id:
		{
		  for (int i = 0; i < source.size(); i++)
			{
			  if (source[i]->PropertyID == cond.value)
				result.push_back(source[i]);
			}

		  break;
		}
      case val_v:
		{
		  for (int i = 0; i < source.size(); i++)
			 {
			   if (source[i]->Value == cond.value)
				 result.push_back(source[i]);
			 }

		  break;
		}
      case is_keep:
		{
		  for (int i = 0; i < source.size(); i++)
			 {
			   if (source[i]->KeepInStack == cond.value)
				 result.push_back(source[i]);
			 }

		  break;
		}
      case is_save:
		{
		  for (int i = 0; i < source.size(); i++)
			 {
			   if (source[i]->SaveInFile == cond.value)
				 result.push_back(source[i]);
			 }

		  break;
		}
    }

  return result;
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::CreateResFile(const wchar_t *filepath, bool overwrite)
{
  return CreateResFile(filepath, L"", overwrite);
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::CreateResFile(const wchar_t *filepath, const wchar_t *res_cath, bool overwrite)
{
  int res = 0;

  try
	 {
	   std::wofstream fout; //создаем объект потоковой записи

	   if (!overwrite)
		 fout.open(AnsiOf(filepath), std::ios_base::trunc); //открываем файл для дозаписи
	   else
		 fout.open(AnsiOf(filepath));

	   if (!fout.is_open())
		 throw Exception("Error opening file: " + String(filepath));

//пишем инфу в файл
	   StrList *vecExp;

	   if (wcscmp(res_cath, L"") == 0)
		 vecExp = ExportStrings();
	   else
		 vecExp = ExportStrings(res_cath);

	   for (auto itm : *vecExp)
		 fout << itm.c_str();

	   fout.close();

	   res = 1;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::CreateResFile: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::LoadResFile(const wchar_t *filepath)
{
  int res = 0;

  try
	 {
	   std::wifstream fin; //создаем объект потокового чтения

	   fin.open(AnsiOf(filepath)); //открываем файл

	   if (!fin.is_open())
		 throw Exception("Error opening file: " + String(filepath));

	   String impstr;
	   wchar_t buf[MAXBUF];
	   StrList fields;

	   while (!fin.eof())
		 {
		   fin.getline(buf, sizeof(fin)); //получаем строку из файла

//разбиваем на части по символу разделителю
		   StrToListW(&fields, std::wstring(buf), FIELDSDELIM, NODELIMEND);

		   if (fields.size() < SAVEFIELDCOUNT)
			 break;

//добавляем ресурс в стек
		   Add({0, fields[0], fields[1], fields[2], fields[3], YES, YES});
		 }

	   fin.close();

	   res = 1;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::LoadResFile: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------------

void RESOURCESTACK::Compact()
{
  try
	 {
	   UINT ind = 0;

	   while (ind < Stack.size())
		 {
		   if (Stack[ind].KeepInStack == NO)
			 {
			   Stack.erase(Stack.begin() + ind);
			   ind = 0;
			 }
		   else
			 ind++;
		 }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::Compact: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

void RESOURCESTACK::Clear()
{
  Stack.clear();
  next_id = Stack.size();
}
//-------------------------------------------------------------------------------

StrList *RESOURCESTACK::ExportStrings()
{
  return ExportStrings(L"");
}
//-------------------------------------------------------------------------------

StrList *RESOURCESTACK::ExportStrings(std::wstring cath)
{
  FExportData.clear();

  try
	 {
	   std::wstring str;

	   RESRECORDSET res = SelectRes({is_save, YES});

	   if (cath != L"")
		 res = Aquire(res, {obj_cath, cath});

	   for (auto itm : res)
		  {
			str = itm->ObjectCathegory +
				  FIELDSDELIM +
				  itm->ObjectID +
				  FIELDSDELIM +
				  itm->PropertyID +
				  FIELDSDELIM +
				  itm->Value +
				  L"\r\n";

			FExportData.push_back(str);
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::ExportStrings: " + e.ToString());

	   FExportData.clear();
	 }

  return &FExportData;
}
//-------------------------------------------------------------------------------

const wchar_t *RESOURCESTACK::GetString()
{
  wchar_t ind[NUMSIZE];

  FStrBuffer = L"";

  try
	 {
	   for (auto itm : Stack)
		  {
			swprintf(ind, L"[%d] ", itm.Index);

			FStrBuffer += std::wstring(ind) +
					   itm.ObjectCathegory +
					   FIELDSDELIM +
					   itm.ObjectID +
					   FIELDSDELIM +
					   itm.PropertyID +
					   FIELDSDELIM +
					   itm.Value +
					   FIELDSDELIM +
					   L"Keep=" + itm.KeepInStack +
					   FIELDSDELIM +
					   L"Save=" + itm.SaveInFile +
					   L"\r\n";
		  }

	   if (Stack.empty())
		 FStrBuffer = L"<empty>\r\n";
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "RESOURCESTACK::GetString: " + e.ToString());

	   FStrBuffer = L"<error>\r\n";
	 }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

PARAM::PARAM(const wchar_t* p_name, const wchar_t *p_value)
{
  name = p_name;
  Set(p_value);
}
//-------------------------------------------------------------------------------

//преобразует параметр в integer и возвращает его
int PARAM::ToInt()
{
  int res = 0;

  try
	 {
	   UINT pos = val.find(L".");

	   if (pos != std::wstring::npos)
		 val.erase(pos, val.length() - pos);

	   res = _wtoi(val.c_str());
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAM::ToInt: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------------

//преобразует параметр в float и возвращает его
float PARAM::ToFloat()
{
  float res = 0;

  try
	 {
	   res = _wtof(val.c_str());
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAM::ToFloat: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------------

//добавляет параметр в конец стека
void PARAMSTACK::Add(const wchar_t *name, const wchar_t *val)
{
  try
	 {
	   int ind = GetInd(name);

	   if (ind < 0)
		 Stack.push_back(new PARAM(name, val));
	   else
		 Stack[ind]->Set(val);
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::Add: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

//возвращает форматированную строку со списком всех ф-й
const wchar_t *PARAMSTACK::GetString()
{
  wchar_t frmt[STRBUFSTACK];
  FStrBuffer = L"";

  try
	 {
	   UINT cnt = 0;

	   for (int i = 0; i < Stack.size(); i++)
		  {
			swprintf(frmt, L"[%d] %s = %s\r\n", i, Stack[i]->name.c_str(), Stack[i]->ToStr());
			FStrBuffer.append(frmt);
		  }

	   if (Stack.empty())
		 FStrBuffer = L"<empty>\r\n";
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::ShowInString: " + e.ToString());

	   FStrBuffer = L"<error>\r\n";
	 }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

PARAM *PARAMSTACK::Get(const wchar_t *name)
{
  PARAM *res = nullptr;

  try
	 {
	   int ind = GetInd(name);

	   if (ind >= 0)
		 res = Stack[ind];
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::Get: " + String(name));
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::Get: " + e.ToString());

	   res = nullptr;
	 }

  return res;
}
//-------------------------------------------------------------------------------

//очищает стек параметров
void PARAMSTACK::Clear()
{
  for (auto itm : Stack)
	 {
	   delete itm;
	   itm = NULL;
	 }

  Stack.clear();
}
//-------------------------------------------------------------------------------

//ищет параметр в стеке по имени и возвращает индекс
//-1 - не найдено
int PARAMSTACK::GetInd(const wchar_t *name)
{
  try
	 {
	   for (int i = 0; i < Stack.size(); i++)
		  {
			if (0 == _wcsicmp(Stack[i]->name.c_str(), name))
			  return i;
		  }
	 }
  catch(Exception &e)
	 {
       SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::GetInd: " + String(name));
	   SaveLogToUserFolder("ELI.log", "ELI", "PARAMSTACK::GetInd: " + e.ToString());
	 }

  return -1;
}
//-------------------------------------------------------------------------------

FUNC::FUNC(const wchar_t *name, const wchar_t *params, func_ptr fptr)
{
  this->name = new wchar_t[FNAMESIZE];
  this->prms_format = new wchar_t[CHARSIZE];
  return_val = new wchar_t[CHARSIZE];

  wcscpy(this->name, name);
  this->ptr = fptr;
  wcscpy(this->prms_format, params);
}
//-------------------------------------------------------------------------------

FUNC::~FUNC()
{
  delete [] name;
  name = NULL;
  delete [] prms_format;
  prms_format = NULL;
  delete [] return_val;
  return_val = NULL;
  ptr = NULL;
}
//-------------------------------------------------------------------------------

//вызывает функцию, возвращает 1 в случае успеха
int FUNC::Call(void *e_ptr)
{
  int res = 0;

  try
	 {
	   ptr(e_ptr);

	   res = 1;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNC::Call: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------------

FUNCSTACK::~FUNCSTACK()
{
  for (auto itm : Stack)
	 {
	   delete itm;
	   itm = NULL;
	 }

  Stack.clear();
}
//-------------------------------------------------------------------------------

//ищет ф-ю в стеке, возвращает ее индекс
//возвращает -1 в случае неудачи
int FUNCSTACK::GetInd(const wchar_t *fname)
{
  try
	 {
	   for (int i = 0; i < Stack.size(); i++)
		  {
			if (_wcsicmp(Stack[i]->GetName(), fname) == 0)
			  return i;
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::GetInd: " + String(fname));
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::GetInd: " + e.ToString());
	 }

  return -1;
}
//-------------------------------------------------------------------------------

//возвращает указатель на свойства функции name из стека
//в случае ошибки возвращает NULL
FUNC *FUNCSTACK::Get(const wchar_t *fname)
{
  FUNC *res = nullptr;

  try
	 {
	   int ind = GetInd(fname);

	   if (ind >= 0)
		 res = Stack[ind];
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::Get: " + String(fname));
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::Get: " + e.ToString());

	   res = nullptr;
	 }

  return res;
}
//-------------------------------------------------------------------------------

//добавляет ф-ю в стек
void FUNCSTACK::Add(const wchar_t *name, const wchar_t *params, func_ptr fptr)
{
  try
	 {
	   if (Get(name))
		 throw Exception("Function with name " + String(name) + " already exists");

	   Stack.push_back(new FUNC(name, params, fptr));
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::Add: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

//удаляет ф-ю из стека
void FUNCSTACK::Delete(const wchar_t *name)
{
  int del_ind = GetInd(name);

  if (del_ind > -1)
    {
	  delete Stack[del_ind];
	  Stack[del_ind] = NULL;
	  Stack.erase(Stack.begin() + del_ind);
	}
}
//-------------------------------------------------------------------------------

//возвращает форматированную строку со списком всех ф-й
const wchar_t *FUNCSTACK::GetString()
{
  wchar_t frmt[STRBUFSTACK];
  FStrBuffer = L"";

  try
	 {
	   UINT cnt = 0;

	   for (int i = 0; i < Stack.size(); i++)
		  {
			swprintf(frmt,
					 L"[%d] %s(%s), ptr = %d\r\n",
					 i,
					 Stack[i]->GetName(),
					 Stack[i]->GetParams(),
					 reinterpret_cast<int>(Stack[i]->GetPointer()));

			FStrBuffer.append(frmt);
		  }

	   if (Stack.empty())
		 FStrBuffer = L"<empty>\r\n";
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FUNCSTACK::ShowInString: " + e.ToString());

	   FStrBuffer = L"<error>\r\n";
	 }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetFirstFree(UINT type)
{
  try
	 {
	   for (int i = 0; i < stMain.size(); i++)
		  {
			if (stMain[i].isfree && stMain[i].type == type)
			  return &stMain[i];
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::GetFirstFree: " + e.ToString());
	 }

  return nullptr;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::Get(const wchar_t *varname)
{
  try
	 {
	   for (int i = 0; i < stMain.size(); i++)
		  {
			if ((0 == wcscmp(stMain[i].varname, varname)) && !stMain[i].isfree)
			  return &stMain[i];
		  }
	 }
  catch(Exception &e)
	 {
       SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::Get: " + String(varname));
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::Get: " + e.ToString());
	 }

  return nullptr;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetByValue(std::wstring val)
{
  try
	 {
	   for (UINT i = 0; i < stStr.size(); i++)
		  {
			if (stStr.at(i) == val)
			  {
				for (int j = 0; j < stMain.size(); j++)
				   {
					 if ((stMain[j].ind == i) && (stMain[j].type == SCSTR))
					   return &stMain[j];
				   }
			  }
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::GetByValue: " + e.ToString());

	   return nullptr;
	 }

  return nullptr;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetByValue(float val)
{
  try
	 {
	   for (UINT i = 0; i < stNum.size(); i++)
		  {
			if (stNum.at(i) == val)
			  {
				for (int j = 0; j < stMain.size(); j++)
				   {
					 if (stMain[j].ind == i)
					   return &stMain[j];
				   }
			  }
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::GetByValue: " + e.ToString());

	   return nullptr;
	 }

  return nullptr;
}
//-------------------------------------------------------------------------------

bool VARSTACK::Add(wchar_t *name, std::wstring val)
{
  bool res = false;

  try
	 {
	   if (Get(name)) //элемент с таким именем есть в стеке и не свободен
		 throw Exception("Element with name " + String(name) + " already exists");

	   VARIABLE *vfree = GetFirstFree(SCSTR);

	   if (vfree) //есть свободные элементы нужного типа
		 {
		   stStr[vfree->ind] = val;
		   wcscpy(vfree->varname, name);
		   vfree->isfree = false;
		 }
	   else
		 {
		   stStr.push_back(val);
		   UINT ind = stStr.size() - 1; //узнаем индекс элемента в стеке

           VARIABLE var;

		   wcscpy(var.varname, name);
		   var.type = SCSTR;
		   var.ind = ind;
		   var.isfree = false;

		   stMain.push_back(var);
		 }

	   res = true;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::Add: " + e.ToString());

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool VARSTACK::Add(wchar_t *name, float val)
{
  bool res = false;

  try
	 {
	   if (Get(name)) //элемент с таким именем есть в стеке и не свободен
		 throw Exception("Element with name " + String(name) + " already exists");;

	   VARIABLE *vfree = GetFirstFree(SCNUM);

	   if (vfree) //есть свободные элементы нужного типа
		 {
		   stNum[vfree->ind] = val;
		   wcscpy(vfree->varname, name);
		   vfree->isfree = false;
		 }
	   else
		 {
		   stNum.push_back(val);
		   UINT ind = stNum.size() - 1; //узнаем индекс элемента в стеке

		   VARIABLE var;

		   wcscpy(var.varname, name);
		   var.type = SCNUM;
		   var.ind = ind;
		   var.isfree = false;

		   stMain.push_back(var);
		 }

	   res = true;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::Add: " + e.ToString());

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool VARSTACK::Remove(const wchar_t *name)
{
  VARIABLE *var = Get(name);

  if (!var)
    return false;
  else
    {
//помечаем этот элемент стека как свободный
	  var->isfree = true;

      return true;
    }
}
//-------------------------------------------------------------------------------

const wchar_t *VARSTACK::GetString()
{
  wchar_t frmt[512];
  wchar_t out[4096];

  FStrBuffer = L"";

  try
	 {
	   wcscpy(frmt, L"[%d] Stack[%d] %s %s = ");
	   wcscat(frmt, FRMTNUM);
	   wcscat(frmt, L"\r\n");

	   for (int i = 0; i < stMain.size(); i++)
		  {
			if (!stMain[i].isfree)
			  {
				if (SCNUM == stMain[i].type)
				  swprintf(out, frmt, i, stMain[i].ind, NUMTYPE, stMain[i].varname, stNum[stMain[i].ind]);
				else if (SCSTR == stMain[i].type)
				  swprintf(out, L"[%d] Stack[%d] %s %s = %s\r\n",
						   i, stMain[i].ind, STRTYPE, stMain[i].varname, stStr[stMain[i].ind].c_str());

				FStrBuffer.append(out);
			  }
		  }

	   if (stMain.empty())
		 FStrBuffer = L"<empty>\r\n";
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::ShowInString: " + e.ToString());

	   FStrBuffer = L"<error>\r\n";
	 }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

float VARSTACK::GetNumElement(VARIABLE *var)
{
  float res = 0.0f;

  try
	 {
	   res = stNum[var->ind];
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::GetNumElement: " + e.ToString());

	   res = 0.0f;
	 }

  return res;
}
//-------------------------------------------------------------------------------

void VARSTACK::SetNumElement(VARIABLE *var, float val)
{
  try
	 {
	   stNum[var->ind] = val;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::SetNumElement: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

std::wstring VARSTACK::GetStrElement(VARIABLE *var)
{
  std::wstring res = L"";

  try
	 {
	   res = stStr[var->ind];
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::GetStrElement: " + e.ToString());

	   res = L"";
	 }

  return res;
}
//-------------------------------------------------------------------------------

void VARSTACK::SetStrElement(VARIABLE *var, std::wstring val)
{
  try
	 {
	   stStr[var->ind] = val;
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "VARSTACK::SetStrElement: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

FRAGMENTCODE::FRAGMENTCODE(const wchar_t *mark, SCRIPTLINES *code, bool global)
{
  FMark = mark;
  FCode = *code;
  FGlobal = global;
}
//-------------------------------------------------------------------------------

const wchar_t *FRAGMENTCODE::GetString()
{
  FStrBuffer = L"";

  for (auto itm : FCode)
	 {
	   FStrBuffer.append(_wltrim(itm.c_str()));
	   FStrBuffer.append(L"\r\n");
	 }

  return FStrBuffer.c_str();
}
 //-------------------------------------------------------------------------------

 FRAGMENTCODE *FRAGMENTSTACK::ReadFragments(int ind)
{
  FRAGMENTCODE *res = NULL;

  try
	 {
	   if ((ind < 0) || (ind >= FStack.size()))
		 throw Exception("Out of bounds!");

	   res = FStack[ind];
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::ReadFragments: " + e.ToString());

	   res = nullptr;
	 }

  return res;
}
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::Add(std::wstring frg_str, std::wstring mark, bool global)
{
  try
	 {
	   FRAGMENTCODE *new_frg = new FRAGMENTCODE();

	   new_frg->SetMark(mark.c_str());

	   if (global)
		 new_frg->SetGlobal();
	   else
		 new_frg->SetLocal();

	   StrToListW(new_frg->GetCode(), frg_str, ENDLNSTR, DELIMEND);

	   FStack.push_back(new_frg);
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::Add: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::Remove(std::wstring mark)
{
  try
	 {
	   for (UINT i = 0; i < FStack.size(); i++)
		  {
			if (wcscmp(mark.c_str(), FStack[i]->GetMark()) == 0)
			  {
				delete FStack[i];
				FStack[i] = NULL;
		  		FStack.erase(FStack.begin() + i);
			  }
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::Remove: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

SCRIPTLINES *FRAGMENTSTACK::GetFragmentCode(std::wstring mark)
{
  try
	 {
	   for (auto itm : FStack)
		  {
			if (wcscmp(mark.c_str(), itm->GetMark()) == 0)
			  return itm->GetCode();
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::GetFragmentCode: " + e.ToString());
	 }

  return NULL;
}
//-------------------------------------------------------------------------------

FRAGMENTCODE *FRAGMENTSTACK::Get(std::wstring mark)
{
  try
	 {
	   for (auto itm : FStack)
		  {
			if (wcscmp(mark.c_str(), itm->GetMark()) == 0)
			  return itm;
		  }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::Get: " + String(mark.c_str()));
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::Get: " + e.ToString());
	 }

  return NULL;
}
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::ClearFragments(bool all)
{
  try
	 {
	   if (all)
		 {
		   for (auto itm : FStack)
			  {
				delete itm;
				itm = NULL;
			  }

		   FStack.clear();
		 }
	   else
		 {
		   for (UINT i = 0; i < FStack.size(); i++)
			  {
				if (!FStack[i]->IsGlobal())
				  {
					delete FStack[i];
					FStack[i] = NULL;
					FStack.erase(FStack.begin() + i);
				  }
			  }
		 }
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::ClearFragments: " + e.ToString());
	 }
}
//-------------------------------------------------------------------------------

const wchar_t *FRAGMENTSTACK::GetString()
{
  wchar_t frmt[512];
  FStrBuffer = L"";

  try
	 {
	   for (int i = 0; i < FStack.size(); i++)
		  {
			swprintf(frmt, L"[%d] {%s} GLOBAL = %d TEXT = \r\n",
					 i,
					 FStack[i]->GetMark(),
					 (UINT)FStack[i]->IsGlobal());

			FStrBuffer.append(frmt);
			FStrBuffer.append(FStack[i]->GetString());
			FStrBuffer.append(L"\r\n");
		  }

	   if (FStack.empty())
		 FStrBuffer = L"<empty>\r\n";
	 }
  catch(Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FRAGMENTSTACK::GetString: " + e.ToString());

	   FStrBuffer = L"<error>\r\n";
	 }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------
