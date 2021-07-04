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

#include "resourcestack.h"

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
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (Stack[i].Index == index)
        {
          Stack[i].KeepInStack = NO;

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

  for (UINT i = 0; i < rs.size(); i++)
    inds.push_back(rs[i]->Index);

  for (UINT i = 0; i < inds.size(); i++)
    {
      if (Delete(inds[i]) < 1)
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
        {
          result = Aquire(result, conditions->at(i));
        }
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
					   SaveLog("exceptions.log", e.ToString());
                       break;
                     }

				  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].Index == intval)
                        result.push_back(&Stack[i]);
                    }
                  break;
				}
      case obj_cath:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].ObjectCathegory == cond.value)
						result.push_back(&Stack[i]);
                    }
                  break;
                }
      case obj_id:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].ObjectID == cond.value)
                        result.push_back(&Stack[i]);
                    }
                  break;
                }
      case prop_id:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].PropertyID == cond.value)
                        result.push_back(&Stack[i]);
                    }
                  break;
                }
      case val_v:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].Value == cond.value)
                        result.push_back(&Stack[i]);
                    }
                  break;
                }
      case is_keep:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
                    {
                      if (Stack[i].KeepInStack == cond.value)
                        result.push_back(&Stack[i]);
                    }
                  break;
                }
      case is_save:
                {
                  for (UINT i = 0; i < Stack.size(); i++)
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

                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->Index == intval)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case obj_cath:
                {
                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->ObjectCathegory == cond.value)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case obj_id:
                {
                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->ObjectID == cond.value)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case prop_id:
                {
                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->PropertyID == cond.value)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case val_v:
                {
                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->Value == cond.value)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case is_keep:
                {
                  for (UINT i = 0; i < source.size(); i++)
                    {
                      if (source[i]->KeepInStack == cond.value)
                        result.push_back(source[i]);
                    }

                  break;
                }
      case is_save:
                {
                  for (UINT i = 0; i < source.size(); i++)
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
           for (UINT i = 0; i < vecExp->size(); i++)
             {
               fout << vecExp->at(i).c_str();
             }
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
           for (UINT i = 0; i < vecExp->size(); i++)
             {
               fout << vecExp->at(i).c_str();
             }
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

  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (Stack[i].SaveInFile == YES)
        {
		  std::wstring str = Stack[i].ObjectCathegory +
							 FIELDSDELIM +
							 Stack[i].ObjectID +
							 FIELDSDELIM +
							 Stack[i].PropertyID +
							 FIELDSDELIM +
							 Stack[i].Value +
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

  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (Stack[i].ObjectCathegory == cath)
        {
          str = Stack[i].ObjectCathegory +
                FIELDSDELIM +
                Stack[i].ObjectID +
                FIELDSDELIM +
                Stack[i].PropertyID +
                FIELDSDELIM +
                Stack[i].Value +
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

  for (UINT i = 0; i < Stack.size(); i++)
    {
      swprintf(ind, L"[%d] ", Stack[i].Index);
	  str += std::wstring(ind) +
			 Stack[i].ObjectCathegory +
			 FIELDSDELIM +
			 Stack[i].ObjectID +
			 FIELDSDELIM +
			 Stack[i].PropertyID +
			 FIELDSDELIM +
			 Stack[i].Value +
			 FIELDSDELIM +
			 L"Keep=" + Stack[i].KeepInStack +
			 FIELDSDELIM +
			 L"Save=" + Stack[i].SaveInFile +
			 L"\r\n";
    }

  if (Stack.size() == 0)
    str = L"<empty>\r\n";

  return str.c_str();
}
//-------------------------------------------------------------------------------
