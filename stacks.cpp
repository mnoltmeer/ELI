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
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

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
  std::vector<CONDITION> conds;
  CONDITION cond;

//создадим условия
  if (ObjectCathegory != L"")
    {
      cond.type = obj_cath;
      cond.value = ObjectCathegory;
      conds.push_back(cond);
    }

  if (ObjectID != L"")
    {
      cond.type = obj_id;
      cond.value = ObjectID;
      conds.push_back(cond);
    }

  if (ResourceID != L"")
    {
      cond.type = prop_id;
      cond.value = ResourceID;
      conds.push_back(cond);
    }

//найдем соотв. условиям набор свойств
  RESRECORDSET rs = Get(&conds);
  std::vector<int> inds;

  if (rs.size() == 0)
    return 0;

  for (auto itm : rs)
	inds.push_back(itm->Index);

  for (auto itm : inds)
	{
	  if (Delete(itm) < 1)
        return -1;
    }

  Compact();

  return 1;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(std::vector<CONDITION> *conditions)
{
  RESRECORDSET result;

  if (conditions->size() > 1)
    {
      result = SelectRes(conditions->at(0));

      for (UINT i = 1; i < conditions->size(); i++)
		result = Aquire(result, conditions->at(i));
    }
  else
    result = SelectRes(conditions->at(0));

  return result;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(UINT type, std::wstring val)
{
  CONDITION cond;

  cond.type = type;
  cond.value = val;

  return SelectRes(cond);
}
//-------------------------------------------------------------------------------

std::vector<int> *RESOURCESTACK::Get(UINT type, const wchar_t *val)
{
  CONDITION cond;

  cond.type = type;
  cond.value = val;

  static std::vector<int> res;
  res.clear();

  RESRECORDSET rs = SelectRes(cond);

  for (UINT i = 0; i < rs.size(); i++)
     res.push_back(rs[i]->Index);

  return &res;
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::Get(std::wstring obj_name, std::wstring prop_name)
{
  std::vector<CONDITION> conds;
  CONDITION cond;

  cond.type = obj_id;
  cond.value = obj_name;
  conds.push_back(cond);
  cond.type = prop_id;
  cond.value = prop_name;
  conds.push_back(cond);

  return Get(&conds);
}
//-------------------------------------------------------------------------------

RESRECORDSET RESOURCESTACK::SelectRes(CONDITION cond)
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
  std::wofstream fout; //создаем объект потоковой записи

  if (!overwrite)
    fout.open(AnsiOf(filepath), std::ios_base::trunc); //открываем файл для дозаписи
  else
    fout.open(AnsiOf(filepath));

  if (!fout.is_open())
    return -1;
  else
    {
//пишем инфу в файл
	  StrList *vecExp = ExportStrings();

      try
         {
		   for (auto itm : *vecExp)
			 fout << itm.c_str();
         }
	  catch (Exception &e)
         {
		   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
         }

      fout.close();

      return 1;
    }
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::CreateResFile(const wchar_t *filepath, const wchar_t *res_cath)
{
  std::wofstream fout; //создаем объект потоковой записи

  fout.open(AnsiOf(filepath), std::ios_base::trunc); //открываем файл для дозаписи

  if (!fout.is_open())
    return -1;
  else
    {
//пишем инфу в файл
      StrList *vecExp = ExportStrings(res_cath);

      try
		 {
		   for (auto itm : *vecExp)
			 fout << itm.c_str();
         }
	  catch (Exception &e)
         {
		   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
         }

      fout.close();

      return 1;
	}
}
//-------------------------------------------------------------------------------

int RESOURCESTACK::LoadResFile(const wchar_t *filepath)
{
  std::wifstream fin; //создаем объект потокового чтения

  fin.open(AnsiOf(filepath)); //открываем файл

  if (!fin.is_open())
    return -1;
  else
    {
      String impstr;
      wchar_t buf[MAXBUF];
      StrList fields;
      RESOURCE res;

      while (!fin.eof())
        {
          try
             {
               fin.getline(buf, sizeof(fin)); //получаем строку из файла
             }
		  catch (Exception &e)
             {
			   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
             }

//разбиваем на части по символу разделителю
		  StrToListW(&fields, std::wstring(buf), FIELDSDELIM, NODELIMEND);

          if (fields.size() < SAVEFIELDCOUNT)
            break;

//заполняем поля по умолчанию
          res.KeepInStack = YES;
          res.SaveInFile = YES;
 //заполняем оставшиеся поля из файла
          res.ObjectCathegory = fields[0];
          res.ObjectID = fields[1];
          res.PropertyID = fields[2];
          res.Value = fields[3];
//добавляем ресурс в стек
          Add(res);
        }

      fin.close();

      return 1;
	}
}
//-------------------------------------------------------------------------------

void RESOURCESTACK::Compact()
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
//-------------------------------------------------------------------------------

void RESOURCESTACK::Clear()
{
  Stack.clear();
  next_id = Stack.size();
}
//-------------------------------------------------------------------------------

StrList *RESOURCESTACK::ExportStrings()
{
  static StrList explist;
  explist.clear();

  for (auto itm : Stack)
    {
	  if (itm.SaveInFile == YES)
        {
		  std::wstring str = itm.ObjectCathegory +
							 FIELDSDELIM +
							 itm.ObjectID +
							 FIELDSDELIM +
							 itm.PropertyID +
							 FIELDSDELIM +
							 itm.Value +
							 L"\r\n";

           explist.push_back(str);
        }
    }

  return &explist;
}
//-------------------------------------------------------------------------------

StrList *RESOURCESTACK::ExportStrings(std::wstring cath)
{
  static StrList explist;
  explist.clear();
  std::wstring str;

  for (auto itm : Stack)
    {
	  if (itm.ObjectCathegory == cath)
        {
		  str = itm.ObjectCathegory +
                FIELDSDELIM +
				itm.ObjectID +
                FIELDSDELIM +
				itm.PropertyID +
                FIELDSDELIM +
				itm.Value +
				L"\r\n";

           explist.push_back(str);
        }
    }

  return &explist;
}
//-------------------------------------------------------------------------------

const wchar_t *RESOURCESTACK::StackInString()
{
  wchar_t ind[NUMSIZE];
  static std::wstring str;
  str = L"";

  for (auto itm : Stack)
    {
	  swprintf(ind, L"[%d] ", itm.Index);

	  str += std::wstring(ind) +
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

  if (Stack.size() == 0)
    str = L"<empty>\r\n";

  return str.c_str();
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
  UINT pos = val.find(L".");

  if (pos != std::wstring::npos)
	val.erase(pos, val.length() - pos);

  return _wtoi(val.c_str());
}
//-------------------------------------------------------------------------------

//добавляет параметр в конец стека
void PARAMSTACK::Add(const wchar_t *name, const wchar_t *val)
{
  int ind = GetParamInd(name);

  if (ind < 0)
	Stack.push_back(new PARAM(name, val));
  else
	Stack[ind]->Set(val);
}
//-------------------------------------------------------------------------------

//возвращает форматированную строку со списком всех ф-й
wchar_t *PARAMSTACK::ShowInString()
{
  static wchar_t output[STRBUFSTACK];
  UINT cnt = 0;

  for (int i = 0; i < Stack.size(); i++)
	cnt += swprintf(output + cnt, L"[%d] %s = %s\r\n",
					i, Stack[i]->name.c_str(), Stack[i]->ToStr());

  return output;
}
//-------------------------------------------------------------------------------

PARAM *PARAMSTACK::Get(const wchar_t *name)
{
  int ind = GetParamInd(name);

  if (ind < 0)
	return NULL;
  else
	return Stack[ind];
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
int PARAMSTACK::GetParamInd(const wchar_t *name)
{
  for (int i = 0; i < Stack.size(); i++)
	{
	  if (0 == _wcsicmp(Stack[i]->name.c_str(), name))
		return i;
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
  for (int i = 0; i < Stack.size(); i++)
    {
	  if (0 == _wcsicmp(Stack[i]->GetName(), fname))
		return i;
	}

  return -1;
}
//-------------------------------------------------------------------------------

//возвращает указатель на свойства функции name из стека
//в случае ошибки возвращает NULL
FUNC *FUNCSTACK::Get(const wchar_t *fname)
{
  for (int i = 0; i < Stack.size(); i++)
	{
	  if (0 == _wcsicmp(Stack[i]->GetName(), fname))
		return Stack[i];
	}

  return NULL;
}
//-------------------------------------------------------------------------------

//добавляет ф-ю в стек
void FUNCSTACK::Add(const wchar_t *name, const wchar_t *params, func_ptr fptr)
{
  if (!Get(name))
	Stack.push_back(new FUNC(name, params, fptr));
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
wchar_t *FUNCSTACK::ShowInString()
{
  static wchar_t output[STRBUFSTACK];
  UINT cnt = 0;

  for (int i = 0; i < Stack.size(); i++)
	{
	  cnt += swprintf(output + cnt, L"[%d] %s(%s), ptr = %d\r\n",
									i,
									Stack[i]->GetName(),
									Stack[i]->GetParams(),
									(int)Stack[i]->GetPointer());
	}

  return output;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetFirstFree(UINT type)
{
  for (int i = 0; i < stMain.size(); i++)
	{
	  if (stMain[i].isfree && stMain[i].type == type)
		return &stMain[i];
	}

  return NULL;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::Get(const wchar_t *varname)
{
  for (int i = 0; i < stMain.size(); i++)
	{
	  if ((0 == wcscmp(stMain[i].varname, varname)) && !stMain[i].isfree)
		return &stMain[i];
	}

  return NULL;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetByValue(std::wstring val)
{
  for (UINT i = 0; i < stStr.size(); i++)
    {
	  if (stStr.at(i) == val)
        {
		  for (int j = 0; j < stMain.size(); j++)
            {
			  if ((stMain[j].ind == i) && (2 == stMain[j].type))
				return &stMain[j];
			}
		}
    }

  return NULL;
}
//-------------------------------------------------------------------------------

VARIABLE *VARSTACK::GetByValue(float val)
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

  return NULL;
}
//-------------------------------------------------------------------------------

bool VARSTACK::Add(wchar_t *name, std::wstring val)
{
  if (Get(name)) //элемент с таким именем есть в стеке и не свободен
	return false;

  VARIABLE *vfree = GetFirstFree(SCSTR);

  if (vfree) //есть свободные элементы нужного типа
    {
	  stStr[vfree->ind] = val;
	  wcscpy(vfree->varname, name);
      vfree->isfree = false;

      return true;
    }
  else
	{
	  VARIABLE var;

      stStr.push_back(val);
      UINT ind = stStr.size() - 1; //узнаем индекс элемента в стеке
      wcscpy(var.varname, name);
      var.type = SCSTR;
	  var.ind = ind;
	  var.isfree = false;
	  stMain.push_back(var);

      return true;
    }
}
//-------------------------------------------------------------------------------

bool VARSTACK::Add(wchar_t *name, float val)
{
  if (Get(name)) //элемент с таким именем есть в стеке и не свободен
    return false;

  VARIABLE *vfree = GetFirstFree(SCNUM);

  if (vfree) //есть свободные элементы нужного типа
    {
      stNum[vfree->ind] = val;
      wcscpy(vfree->varname, name);
      vfree->isfree = false;

      return true;
	}
  else
    {
      VARIABLE var;

	  stNum.push_back(val);
      UINT ind = stNum.size() - 1; //узнаем индекс элемента в стеке
	  wcscpy(var.varname, name);
      var.type = SCNUM;
      var.ind = ind;
	  var.isfree = false;

      stMain.push_back(var);

	  return true;
    }
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

const wchar_t *VARSTACK::ShowInString()
{
  wchar_t frmt[512];
  static std::wstring res;
  res = L"";
  wchar_t out[4096];

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

		   res += out;
         }
	 }

  return res.c_str();
}
//-------------------------------------------------------------------------------

FRAGMENTCODE::FRAGMENTCODE(const wchar_t *mark, SCRIPTLINES *code, bool global)
{
  FMark = mark;
  FCode = *code;
  FGlobal = global;
}
//-------------------------------------------------------------------------------

const wchar_t *FRAGMENTCODE::GetCodeStrings()
{
  static String res;
  res = "";

  for (auto itm : FCode)
	 {
	   res += _wltrim(itm.c_str());
	   res += L"\r\n";
	 }

  return res.c_str();
}
 //-------------------------------------------------------------------------------

 FRAGMENTCODE *FRAGMENTSTACK::ReadFragments(int ind)
{
  if (ind < 0)
	throw Exception("FRAGMENTSTACK::Fragments: Out of bounds!");
  else if (ind < FStack.size())
	return FStack[ind];
  else
	throw Exception("FRAGMENTSTACK::Fragments: Out of bounds!");
}
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::Add(std::wstring frg_str, std::wstring mark, bool global)
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
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::Remove(std::wstring mark)
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
//-------------------------------------------------------------------------------

SCRIPTLINES *FRAGMENTSTACK::GetFragmentCode(std::wstring mark)
{
  for (auto itm : FStack)
	{
	  if (wcscmp(mark.c_str(), itm->GetMark()) == 0)
		return itm->GetCode();
	}

  return NULL;
}
//-------------------------------------------------------------------------------

FRAGMENTCODE *FRAGMENTSTACK::Get(std::wstring mark)
{
  for (auto itm : FStack)
    {
	  if (wcscmp(mark.c_str(), itm->GetMark()) == 0)
		return itm;
    }

  return NULL;
}
//-------------------------------------------------------------------------------

void FRAGMENTSTACK::ClearFragments(bool all)
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
//-------------------------------------------------------------------------------

const wchar_t *FRAGMENTSTACK::ShowInString()
{
  wchar_t frmt[512];
  static std::wstring res;
  res = L"";

  for (int i = 0; i < FStack.size(); i++)
     {
       swprintf(frmt, L"[%d] {%s} GLOBAL = %d TEXT = \r\n%s\r\n",
			   i,
			   FStack[i]->GetMark(),
			   (UINT)FStack[i]->IsGlobal(),
			   FStack[i]->GetCodeStrings());

       res += frmt;
     }

  return res.c_str();
}
//-------------------------------------------------------------------------------
