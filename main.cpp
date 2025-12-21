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

#include <windows.h>
#include <iterator>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

#pragma hdrstop

#include "..\work-functions\Logs.h"
#include "..\work-functions\Data.h"
#include "..\work-functions\FilesDirs.h"
#include "oldfuncs.h"
#include "main.h"

ELI_API int __stdcall GetELIInterface(ELI_INTERFACE **eInterface)
{
  int res = 0;

  if (*eInterface)
    return 0;

  try
	 {
	   *eInterface = new ELI();

	   res = 1;
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "GetELIInterface: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------

ELI_API int __stdcall FreeELIInterface(ELI_INTERFACE **eInterface)
{
  int res = 0;

  try
	 {
	   delete *eInterface;
	   *eInterface = nullptr;

	   res = 1;
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("ELI.log", "ELI", "FreeELIInterface: " + e.ToString());

	   res = 0;
	 }

  return res;
}
//-------------------------------------------------------------------------

ELI::ELI()
{
  write_log = false;
  debug_eli = false;
  debug_in_file = false;
  use_return = false;
  use_false = false;
  debugfile = L"debug.log";
  InitRes(true);
};

//служебные функции
void ELI::AddInfoMsg(const wchar_t *msg, const wchar_t *type, UINT index)
{
  if (trigger_check) //відбувається перевірка тригеру, лог не потрібен
	return;

  wchar_t str[CHARSIZE];

  swprintf(str,
		  L"[%s] %s : %s, line [%d]: %s\r\n",
          pStack->Get(P_SCRNAME)->ToStr(), type, msg, index, vecScList[index].c_str());
  LastErr = msg;
  InfStack += str;

  if (write_log)
    {
	  swprintf(str, L"%c [%d] %s", '!', index, msg);
      WriteLog(str);
    }

  if (debug_eli)
    WriteELIDebug(L"Log", str);
}
//-------------------------------------------------------------------------------

void ELI::AddInfoMsg(const wchar_t *msg)
{
  if (trigger_check) //відбувається перевірка тригеру, лог не потрібен
	return;

  wchar_t str[CHARSIZE];

  swprintf(str, L"[%s] : %s\r\n", pStack->Get(P_SCRNAME)->ToStr(), msg);
  InfStack.append(str);

  if (write_log)
    {
	  swprintf(str, L"%c %s", '!', msg);
      WriteLog(str);
    }

  if (debug_eli)
    WriteELIDebug(L"Log", str);
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::GetObjCathegory(const wchar_t *obj_name)
{
  if (debug_eli)
    {
	  WriteELIDebug(L"GetObCathegory", L"[START]");
      WriteELIDebug(L"GetObjCathegory", obj_name);
    }

  RESRECORDSET rs = objStack->Get(obj_id, std::wstring(obj_name));

  if (rs.size() == 0)
    {
      if (debug_eli)
        WriteELIDebug(L"GetObCathegory", L"[FAIL]");

      return NULL;
    }

  return rs[0]->ObjectCathegory.c_str();
}
//-------------------------------------------------------------------------------

bool ELI::RunFunc(wchar_t *str_with_func, wchar_t *result, UINT index)
{
  bool res = false;

  if (debug_eli)
    {
	  WriteELIDebug(L"RunFunc", L"[START]");
      WriteELIDebug(L"RunFunc", str_with_func);
    }

  std::wstring fname, vals;
  UINT pos;

//добавляем в стек служебный параметр - индекс строки из которой вызывается ф-я
  wchar_t ind[NUMSIZE];

  swprintf(ind, L"%d", index);
  pStack->Add(P_IND, ind);

//вычленяем из строки имя ф-ии и список аргументов
  fname = str_with_func;

  pos = fname.find_first_of('(');
  vals = fname.substr(pos + 1, fname.length() - pos - 2);
  fname.erase(pos, fname.length() - pos);

  FUNC *fn = fStack->Get(fname.c_str());

  if (fn)
	{
	  std::vector<std::wstring> vecArgs, vecVals;

	  if ((wcslen(fn->GetParams()) == 0) && (vals.length() == 0)) //нет параметров - ф-я без аргументов
		res = CallFunc(fn, result, index);
	  else if ((wcslen(fn->GetParams()) != 0) && (vals.length() == 0)) //нет параметров но ф-я с аргументами
		{
          AddInfoMsg(FNARGCNTERR, ERRMSG, index);
		  AddInfoMsg(FNARGERR, ERRMSG, index);
		  res = false;
        }
	  else
        {
//получаем список аргументов
		  StrToListW(&vecVals, vals, FARGSSEP, NODELIMEND);
//получаем из описания ф-ии описание параметров
		  StrToListW(&vecArgs, std::wstring(fn->GetParams()), FARGSSEP, NODELIMEND);

          if (vecVals.size() == vecArgs.size()) //если кол-во параметров совпадает с описанием
            {
              wchar_t ptype[5], pname[PNAMESIZE], pval[CHARSIZE];

              for (UINT i = 0; i < vecVals.size(); i++)
				{
//инициализируем параметры и добавляем их в стек
                  try
                    {
					  UINT spos = vecArgs[i].find(L" ");
					  UINT namelen = vecArgs[i].length() - spos - 1;

					  wcscpy(ptype, vecArgs[i].substr(0, spos).c_str());
                      wcscpy(pname, vecArgs[i].substr(spos + 1, namelen).c_str());
                      wcscpy(pval, vecVals[i].c_str());

                      if (0 == _wcsicmp(ptype, S_NUM))
                        {
                          float *fres = CalcExpNum(pval, index);

                          if (fres)
                            swprintf(pval, FRMTNUM, *fres);
                          else
                            {
							  AddInfoMsg(NUMERR, ERRMSG, index);

                              return false;
                            }
                        }
                      else if (0 == _wcsicmp(ptype, S_SYM))
                        {
//если функция не из тех, которые используют в качестве аргументов имена переменных
//приведем содержимое аргумента к простому значению
						  if (!_wstriincl(fname.c_str(), L"Create", 0) &&
							  !_wstriincl(fname.c_str(), L"free", 1) &&
                              !_wstriincl(fname.c_str(), L"Run", 1) &&
                              !_wstriincl(fname.c_str(), L"ReadIn", 1) &&
                              (!(_wstriincl(fname.c_str(), L"LoadFileToVar", 1) && (_wcsicmp(pname, L"pTarget") == 0)))&&
                              (!(_wstriincl(fname.c_str(), L"SaveVarToFile", 1) && (_wcsicmp(pname, L"pTarget") == 0)))&&
                              (!(_wstriincl(fname.c_str(), L"SaveFragmentToFile", 1) && (_wcsicmp(pname, L"pTarget") == 0))))
                            {
                              const wchar_t *sres = CalcExpStr(pval, index);

                              if (0 != _wcsicmp(sres, ERROUT))
                                wcscpy(pval, sres);
                              else
								return false;
                            }
                        }
                      else
                        {
                          AddInfoMsg(FNARGERR, ERRMSG, index);

                          return false;
                        }

                      pStack->Add(pname, pval);
                    }
				 catch (Exception &e)
                    {
					  String msg = "RunFunc(" + String(str_with_func) + "), create params: " + e.ToString();
					  AddInfoMsg(FNARGERR, ERRMSG, index);
					  AddInfoMsg(msg.c_str(), ERRMSG, index);
					  wcscpy(result, L"0");

                      return false;
                    }
                }

//выполняем функцию
			  res = CallFunc(fn, result, index);
            }
          else
            {
			  AddInfoMsg(FNARGCNTERR, ERRMSG, index);
			  res = false;
			}
		}
    }
  else
    {
	  AddInfoMsg(FNNAMERR, ERRMSG, index);
	  res = false;
    }

  if (_wstrincl(fname.c_str(), L"return", 1))
    {
	  AddInfoMsg(SCEXIT, INFMSG, index);
	  res = false;
    }
  else if (_wstrincl(fname.c_str(), L"throw", 1))
    {
	  AddInfoMsg(SCEXCEPT, INFMSG, index);
	  res = false;
	}

  if (debug_eli)
	{
	  WriteELIDebug(L"RunFunc", result);
	  WriteELIDebug(L"RunFunc", L"[END]");
	}

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::CallFunc(FUNC *fn_ptr, wchar_t *result, UINT index)
{
  bool res = false;

  if (debug_eli)
	WriteELIDebug(L"CallFunc", fn_ptr->GetName());

  try
	 {
	   if (!fn_ptr)
		 throw Exception(FNPTRERR);

	   current_func_name = fn_ptr->GetName(); //повідомимо ELI ім'я поточної ф-ції

	   fn_ptr->Call(this);

	   if (_wcsicmp(fn_ptr->GetResult(), L"") == 0) //захист від порожньої строки у результаті ф-ї
		 throw Exception(FNEMPTYRES);
	   else
		 wcscpy(result, fn_ptr->GetResult());

	   res = true;
	 }
  catch (Exception &e)
	 {
	   String msg = "CallFunc: " + e.ToString();
	   AddInfoMsg(msg.c_str(), ERRMSG, index);
	   wcscpy(result, L"0");

	   res = false;
	 }

  current_func_name = L""; //обнулимо ім'я поточної функції для запобігання казусів

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::CreateProcedure(wchar_t *str_with_proc, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"CreateProcedure", L"[START]");
      WriteELIDebug(L"CreateProcedure", str_with_proc);
    }

  wchar_t procname[MAXNAMELEN], procprm[STRBUFSTACK], proctext[MAXNAMELEN];

  _wstrcpywc(_wsetpstr(str_with_proc, 10), procname, '(');
  int pos = _wstrcpos(str_with_proc, '(') + 1;
  _wstrcpywc(_wsetpstr(str_with_proc, pos), procprm, ')');
  pos = _wstrcpos(str_with_proc, ')') + 1;
  wcscpy(proctext, _wsetpstr(str_with_proc, pos));

  RESOURCE res;

  res.ObjectCathegory = OBJPROC;
  res.ObjectID = procname;
  res.PropertyID = OBJPROCPRM;
  res.Value = procprm;

  if (procStack->Get(obj_id, res.ObjectID).size() > 0)
    {
      AddInfoMsg(PROCNOCRT, ERRMSG, index);

      return false;
    }

  if (procStack->Add(res) < 1)
    {
      AddInfoMsg(PROCNOCRT, ERRMSG, index);

      return false;
    }

  res.ObjectCathegory = OBJPROC;
  res.ObjectID = procname;
  res.PropertyID = OBJPROCTXT;
  res.Value = proctext;

  if (procStack->Add(res) < 1)
    {
      AddInfoMsg(PROCNOCRT, ERRMSG, index);

      return false;
    }

  if (debug_eli)
    WriteELIDebug(L"CreateProcedure", L"[OK]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::DropProcedure(const wchar_t *proc_name)
{
  if (debug_eli)
    {
      WriteELIDebug(L"DropProcedure", L"[START]");
      WriteELIDebug(L"DropProcedure", proc_name);
    }

  RESRECORDSET rs = procStack->Get(obj_id, std::wstring(proc_name));

  if (rs.size() > 0)
    {
      for (UINT i = 0; i < rs.size(); i++)
        {
          rs[i]->KeepInStack = NO;

          if (rs[i]->PropertyID == OBJPROCTXT)
            frgStack->Remove(rs[i]->Value);
        }

      procStack->Compact();

      return true;
    }

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::RunProcedure(const wchar_t *name, const wchar_t *params, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"RunProcedure", L"[START]");
	  std::wstring str = std::wstring(name) + L"(" + std::wstring(params) + L")";
      WriteELIDebug(L"RunProcedure", str.c_str());
    }

  std::vector<std::wstring> prms, vals;
  RESRECORDSET rs = procStack->Get(obj_id, std::wstring(name));

  if (rs.size() < 2)
	{
	  AddInfoMsg(PROCNAMERR, ERRMSG, index);

	  return false;
	}

//приберем завершальну ',' з params, яка з'являється у випадку, коли останній параметр є порожнім рядком
  UINT len = wcslen(params),
	   argsccnt = _wstrccount(rs[0]->Value.c_str(), FARGSSEP[0]),
	   prmsccnt = _wstrccount(params, FARGSSEP[0]);

//якщо в аргументах немає ',' (один аргумент), а в параметрах останній символ це ',' (параметр - порожня строка)
  if ((params[len - 1] == FARGSSEP[0]) && (argsccnt == 0))
	prmsccnt--; //змусимо ELI ігнорувати цю кому

  if (argsccnt != prmsccnt)
	{
	  AddInfoMsg(PROCARGCNTERR, ERRMSG, index);

	  return false;
	}

//получаем списки имен указанных при объявлении параметров и полученных значений
  if (rs[0]->Value != L"")
	{
	  StrToListW(&prms, rs[0]->Value, FARGSSEP, NODELIMEND);
	  StrToListW(&vals, std::wstring(params), FARGSSEP, NODELIMEND);
	}

//получим текст процедуры, развернем его из стека фрагментов и запарсим параметры значениями
  SCRIPTLINES *proccode = frgStack->GetFragmentCode(rs[1]->Value);

  if (!proccode)
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
	}

//инициализируем стек переменных для процедуры
  st = NULL;
  VARSTACK *loc = new VARSTACK();
  vecVSt.push_back(loc);
  UINT i = vecVSt.size() - 1;
  st = vecVSt.back(); //переключаем указатель на локальный стек
  static bool res = false;
//инициализируем в локальном стеке переменные, которым будут присвоены значения аргументов
  wchar_t str[CHARSIZE];

  for (UINT i = 0; i < prms.size(); i++)
     {
	   swprintf(str, L"%s = %c%s%c", prms[i].c_str(), STRSYM, vals[i].c_str(), STRSYM);

	   if (!TranslateLine(str, index))
         {
		   AddInfoMsg(PROCARGERR, ERRMSG, index);

           return false;
         }
     }

  res = TranslateFragment(proccode, index);

  delete vecVSt[i];
  vecVSt[i] = NULL;
  loc = NULL;
  vecVSt.erase(vecVSt.begin() + i);
  st = vecVSt.back(); //возвращаем указатель на уровень выше

  if (debug_eli)
    WriteELIDebug(L"RunProcedure", L"[END]");

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::WorkWithObject(wchar_t *str_with_obj, wchar_t *result, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"WorkWithObject", L"[START]");
      WriteELIDebug(L"WorkWithObject", str_with_obj);
    }

//парсимо рядок на числові константи, щоб роздільник розрядів не сприймався як OBJPROPSEP
  wcscpy(str_with_obj, ParseConstNumbers(str_with_obj));

  wchar_t prop[MAXNAMELEN], name[MAXNAMELEN], oldname[MAXNAMELEN];

//вычленяем из строки имя объекта
  if (_wstrcpywc(str_with_obj, oldname, OBJPROPSEP) < 0)
    {
      AddInfoMsg(SYNTAXERR, ERRMSG, index);

      return false;
    }

//парсим имя объекта на имена переменных для правильного исп. методов
  wcscpy(name, oldname);

  if (!ParseVarInExp(name, index))
    {
      AddInfoMsg(OBJNOCRTNAME, ERRMSG, index);

      return false;
    }

//добавляем параметр с именем объекта в стек для использования в вызываемой ф-ии
  pStack->Add(P_OBJNAME, name);

//пишем оставшееся в имя свойства
  wcscpy(prop, _wsetpstr(str_with_obj, _wstrcpos(str_with_obj, OBJPROPSEP)) + 1);

  if (wcslen(prop) == 0)
    {
      AddInfoMsg(OBJNOPROP, ERRMSG, index);

      return false;
    }

//если в имени свойства есть точка, значит это свойство - объект
//обрезаем по точку
  if (_wstrccount(prop, OBJPROPSEP) > 0)
    {
      wchar_t tmp[MAXNAMELEN];

      _wstrcpywc(prop, tmp, OBJPROPSEP);
      wcscpy(prop, tmp);
    }

//если есть открытая скобка - предполагается метод
  if (_wstrccount(prop, '(') > 0)
    {
//получаем предполагаемое имя метода
	  std::wstring mname = prop;
	  UINT op = mname.find(L"("), cl = mname.find(L")");
//если метод - это Create(), значит категория объекта это аргумент метода
	  std::wstring cath = mname.substr(op + 1, cl - op - 1);
	  mname.erase(op, mname.length() - op);

//получаем предполагаемое имя класса - категорию вызывающего объекта
	  RESRECORDSET rs = objStack->Get(obj_id, std::wstring(name));

//если есть выборка из стека ресурсов - берем категорию из первого элемента выборки
      if (rs.size() > 0)
        cath = rs[0]->ObjectCathegory;

      if (IsClassMember(cath.c_str(), mname.c_str()))
        {
          if (!IsAccessibleMember(name, mname.c_str()))
            {
              AddInfoMsg(OBJMEMNOTACC, ERRMSG, index);

              return false;
            }

          if (RunMethod(name, cath.c_str(), prop, index))
            {
              if (return_val != L"")
                wcscpy(result, return_val.c_str());

              return true;
            }
          else
            return false;
        }

      return RunFunc(prop, result, index);
    }
  else //скобки нет - предполагается свойство
    {
	  std::wstring oldprop = prop;

      if (!ParseVarInExp(prop, index))
        {
          AddInfoMsg(OBJNOCRTPROP, ERRMSG, index);

          return false;
        }

	  RESRECORDSET rs = objStack->Get(obj_id, std::wstring(name));

      if (rs.size() == 0)
        {
          AddInfoMsg(OBJNONE, ERRMSG, index);

          return false;
        }

      rs.clear();

	  rs = objStack->Get(std::wstring(name), std::wstring(prop));

      if (rs.size() > 0) //если объект найден
        {
		  std::wstring cath = rs[0]->ObjectCathegory;

          if (IsClassMember(cath.c_str(), prop))
            {
              if (!IsAccessibleMember(name, prop))
                {
                  AddInfoMsg(OBJMEMNOTACC, ERRMSG, index);

                  return false;
                }
            }

		  std::wstring oname = std::wstring(oldname) + OBJPROPSEPSTR + oldprop;

		  std::wstring operstr = ParseStringW(std::wstring(str_with_obj), oname, rs[0]->Value);

          wcscpy(result, operstr.c_str());

		  if (debug_eli)
            {
              WriteELIDebug(L"WorkWithObject", result);
              WriteELIDebug(L"WorkWithObject", L"[OK]");
            }

          return true;
        }
      else
        {
          if (debug_eli)
            {
              WriteELIDebug(L"WorkWithObject", str_with_obj);
              WriteELIDebug(L"WorkWithObject", L"[FAIL]");
            }

          AddInfoMsg(OBJNOPROP, ERRMSG, index);

          return false;
        }
    }
}
//-------------------------------------------------------------------------------

bool ELI::RunMethod(const wchar_t* objname, const wchar_t *cl_name, wchar_t *str_with_method, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"RunMethod", L"[START]");
      WriteELIDebug(L"RunMethod", str_with_method);
    }

  std::wstring name, vals;
  UINT pos;

//вычленяем из строки имя метода и список аргументов
  name = str_with_method;

  pos = name.find(L"(");
  vals = name.substr(pos + 1, name.length() - pos - 2);

  if (vals != L"")
	vals = std::wstring(objname).erase(0, 1) + L"," + vals;
  else
	vals = std::wstring(objname).erase(0, 1);

  name.erase(pos, name.length() - pos);
  name = std::wstring(cl_name) + name;

//парсим аргументы метода на значения переменных
  wchar_t valstr[CHARSIZE];

  wcscpy(valstr, vals.c_str());

  if (!ParseVarInExp(valstr, index))
    {
      if (debug_eli)
        WriteELIDebug(L"RunMethod", L"[FAIL]");

      return false;
    }

  vals = valstr;

  if (RunProcedure(name.c_str(), vals.c_str(), index))
    {
      if (debug_eli)
        WriteELIDebug(L"RunMethod", L"[OK]");

      return true;
    }

  if (debug_eli)
    {
      WriteELIDebug(L"RunMethod", L"[FAIL]");
    }

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::MakeCodeInVar(wchar_t *str, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"MakeCodeInVar", L"[START]");
	  WriteELIDebug(L"MakeCodeInVar", str);
	}

  std::wstring vname, mark;
  UINT pos;

  vname = str;
  vname.erase(0, 5);
  pos = vname.find(FRGMARK);
  mark = vname.substr(pos, vname.length() - pos);
  vname.erase(pos, vname.length() - pos);

  wchar_t name[MAXNAMELEN], mrk[MAXNAMELEN];

  wcscpy(name, vname.c_str());
  wcscpy(mrk, mark.c_str());

  if (!VarInit(name, SCSTR, mrk, index))
	return false;

  if (debug_eli)
	WriteELIDebug(L"MakeCodeInVar", L"[END]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::CreateTrigger(wchar_t *str, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"CreateTrigger", L"[START]");
	  WriteELIDebug(L"CreateTrigger", str);
	}

  bool res;

  try
	 {
	   std::wstring cond, mark;
	   UINT mrk;

	   cond = str;
	   cond.erase(0, 8);
	   mrk = cond.find(FRGMARK);
	   mark = cond.substr(mrk, cond.length() - mrk);
	   cond.erase(mrk, cond.length() - mrk);

	   TRIGGER tr;

	   wcscpy(tr.condition, cond.c_str());
	   wcscpy(tr.fragment, mark.c_str());

	   if (!TriggerExists(&tr))
		 {
		   vecTriggers.push_back(tr);
		   res = true;
         }
	   else
		 {
		   AddInfoMsg(TRGCRTERR, ERRMSG, index);
		   res = false;
		 }

	   if (debug_eli)
		 WriteELIDebug(L"CreateTrigger", L"[END]");
	 }
  catch (Exception &e)
	 {
	   AddInfoMsg(TRGCRTERR, ERRMSG, index);

	   if (debug_eli)
		 WriteELIDebug(L"CreateTrigger", String("Exception: " + e.ToString()).c_str());

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::RemoveTrigger(wchar_t *str, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"RemoveTrigger", L"[START]");
	  WriteELIDebug(L"RemoveTrigger", str);
	}

  bool res = false;

  try
	 {
	   std::wstring cond;

	   cond = str;
	   cond.erase(0, 12);

	   for (int i = 0; i < vecTriggers.size(); i++)
		  {
			if (vecTriggers[i].condition == cond)
			  {
				vecTriggers.erase(vecTriggers.begin() + i);
				res = true;
				break;
			  }
		  }

	   if (debug_eli)
		 WriteELIDebug(L"RemoveTrigger", L"[END]");
	 }
  catch (Exception &e)
	 {
	   AddInfoMsg(TRGREMERR, ERRMSG, index);

	   if (debug_eli)
		 WriteELIDebug(L"RemoveTrigger", String("Exception: " + e.ToString()).c_str());

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::CheckTrigger(TRIGGER *trigger)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"CheckTrigger", L"[START]");
	  WriteELIDebug(L"CheckTrigger", trigger->condition);
	}

  bool res = false;

  try
	 {
	   if (ExpTrue(trigger->condition, 0) > 0)
         res = true;

	   if (debug_eli)
		 WriteELIDebug(L"CheckTrigger", L"[SUCCESS]");
	 }
  catch (Exception &e)
	 {
	   AddInfoMsg(UNKTRG, ERRMSG, 0);

	   if (debug_eli)
		 WriteELIDebug(L"CheckTrigger", String("Exception: " + e.ToString()).c_str());

       res = false;
	 }

  if (debug_eli)
	WriteELIDebug(L"CheckTrigger", L"[END]");

  return res;
}
//-------------------------------------------------------------------------------

void ELI::RunTrigger(TRIGGER *trigger)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"RunTrigger", L"[START]");
	  WriteELIDebug(L"RunTrigger", trigger->condition);
	}

  try
	 {
       try
		  {
			SCRIPTLINES *code = frgStack->GetFragmentCode(trigger->fragment);

			if (code)
			  TranslateFragment(code, 0);
			else
			  throw Exception("Fragment not found");
		  }
	   catch (Exception &e)
		  {
			AddInfoMsg(TRGRUNERR, ERRMSG, 0);

			if (debug_eli)
			  WriteELIDebug(L"RunTrigger", String("Exception: " + e.ToString()).c_str());
		  }

	   if (debug_eli)
		 WriteELIDebug(L"RunTrigger", L"[END]");
	 }
  catch (Exception &e)
	 {
	   AddInfoMsg(TRGRUNERR, ERRMSG, 0);

	   if (debug_eli)
		 WriteELIDebug(L"RunTrigger", String("Exception: " + e.ToString()).c_str());
	 }
}
//-------------------------------------------------------------------------------

bool ELI::TriggerExists(TRIGGER *trigger)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"TriggerExists", L"[START]");
	  WriteELIDebug(L"TriggerExists", trigger->condition);
	}

  bool res = false;

  try
	 {
	   for (int i = 0; i < vecTriggers.size(); i++)
		  {
			if (vecTriggers[i].condition == trigger->condition)
			  {
				res = true;
				break;
			  }
		  }

	   if (debug_eli)
		 WriteELIDebug(L"TriggerExists", L"[END]");
	 }
  catch (Exception &e)
	 {
       AddInfoMsg(TRGEXST, ERRMSG, 0);

	   if (debug_eli)
		 WriteELIDebug(L"TriggerExists", String("Exception: " + e.ToString()).c_str());

       res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

void ELI::CheckTriggers()
{
  if (debug_eli)
	WriteELIDebug(L"CheckTriggers", L"[START]");

  try
	 {
	   trigger_check = true;

	   for (int i = 0; i < vecTriggers.size(); i++)
		  {
			if (CheckTrigger(&vecTriggers[i]))
			  RunTrigger(&vecTriggers[i]);
		  }

	   trigger_check = false;

	   if (debug_eli)
		 WriteELIDebug(L"RunTrigger", L"[END]");
	 }
  catch (Exception &e)
	 {
	   AddInfoMsg(UNKTRG, ERRMSG, 0);

	   if (debug_eli)
		 WriteELIDebug(L"RunTrigger", String("Exception: " + e.ToString()).c_str());
	 }
}
//-------------------------------------------------------------------------------

bool ELI::ProtectTranslate(wchar_t *str, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"ProtectTranslate", L"[START]");
	  WriteELIDebug(L"ProtectTranslate", str);
	}

  bool res;

  try
	 {
	   try
		  {
			std::wstring mark = str;
			mark.erase(0, 8); //уберем слово #protect

			SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

			if (code)
			  TranslateFragment(code, index);
			else
			  AddInfoMsg(FRGMNTERR, ERRMSG, index);
		  }
	   catch (Exception &e)
		  {
			AddInfoMsg(FRGMNTERR, ERRMSG, index);

			if (debug_eli)
			  WriteELIDebug(L"ProtectTranslate", String("Exception: " + e.ToString()).c_str());
		  }
	 }
  __finally
	 {
	   if (debug_eli)
		 WriteELIDebug(L"ProtectTranslate", L"[END]");

	   res = true;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::ProcessDirective(wchar_t *str, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"ProcessDirective", L"[START]");
	  WriteELIDebug(L"ProcessDirective", str);
	}

  bool res;

  try
	 {
       if (0 == wcscmp(str, L"#exit")) //директива выхода из скрипта
		 {
		   AddInfoMsg(SCSTP, INFMSG, index);
		   res = false;
		 }
	   else if (_wstrincl(str, L"#procedure", 0)) //объявление процедуры
		 res = CreateProcedure(str, index);
	   else if (_wstrincl(str, L"#dropprocedure", 0)) //удаление процедуры
		 {
		   wchar_t proc[MAXNAMELEN];

		   _wstrncopy(str, proc, 14, wcslen(str) - 14);

		   if (!DropProcedure(proc))
			 {
			   AddInfoMsg(PROCNAMERR, ERRMSG, index);
			   AddInfoMsg(PROCNODEL, ERRMSG, index);
			   res = false;
			 }
		   else
			res = true;
		 }
	   else if (_wstrincl(str, L"#include", 0)) //включение и трансляция кода из др. файла
		 {
		   res = ParseRefsInExp(str, index);

		   if (res)
			 res = ParseVarInExp(str, index);

		   if (res)
			 {
			   SCRIPTLINES incl_script = GetInclude(str);
			   res = TranslateFragment(&incl_script, index);
			 }
		 }
	   else if (_wstrincl(str, L"#make", 0)) //прекомпиляция фрагмента и сохранение его метки в переменную
		 res = MakeCodeInVar(str, index);
	   else if (_wstrincl(str, L"#run", 0)) //трансляция фрагмента из переменной
		 {
		   wchar_t name[MAXNAMELEN];

		   _wstrncopy(str, name, 4, wcslen(str) - 4);

		   res = TranslateCodeFromVar(name, index);
		 }
	   else if (_wstrincl(str, L"#class", 0)) //определение нового класса объектов
		 {
		   std::wstring name = str, parent_name;
		   name.erase(0, 6);

		   std::wstring mark = name;
		   UINT pos = name.find(FRGMARK);
		   mark = name.substr(pos, mark.length() - pos);
		   name.erase(pos, name.length() - pos);

		   pos = name.find(L":");

		   if (pos != std::wstring::npos)
			 {
			   parent_name = name.substr(pos + 1, name.length() - pos - 1);
			   name.erase(pos, name.length() - pos);
			 }

		   if (clStack->Get(obj_id, name).size() > 0)
			 {
			   AddInfoMsg(CLDUP, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   current_class = name;
			   res = TranslateFragment(frgStack->GetFragmentCode(mark), index);
			   current_class = L"";

//если был указан родительский класс - добавим его члены к дочернему
			   if (clStack->Get(obj_id, parent_name).size() > 0)
				 {
				   if (!ImportParentClass(name, parent_name, true))
					 AddInfoMsg(CLNOPUBPROP, WRNMSG, index);
				   if (!ImportParentClass(name, parent_name, false))
					 AddInfoMsg(CLNOPUBMETH, WRNMSG, index);
				 }
             }
		 }
	   else if (_wstrincl(str, L"#dropclass", 0)) //удаление класса
		 {
		   wchar_t name[MAXNAMELEN];

		   _wstrncopy(str, name, 10, wcslen(str) - 10);

		   RESRECORDSET rs = clStack->Get(obj_id, std::wstring(name));

		   if (rs.size() > 0)
			 {
			   for (UINT i = 0; i < rs.size(); i++)
				  {
					if (rs[i]->ObjectCathegory == CLMETHOD)
					  procStack->Delete(L"", rs[i]->ObjectID + rs[i]->PropertyID, L"");
				  }

			   clStack->Delete(L"", name, L"");
			 }
		   else
			 {
			   AddInfoMsg(CLNODESTR, WRNMSG, index);
			   AddInfoMsg(CLNONE, WRNMSG, index);
			 }

		   res = true;
		 }
	   else if (_wstrincl(str, L"#property", 0)) //добавление нового свойства в класс
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring prop_str = str;

			   prop_str.erase(0, 9);
			   res = AddClassProperty(current_class, prop_str, false, index);
             }
		 }
	   else if (_wstrincl(str, L"#publicproperty", 0)) //добавление нового свойства в класс
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring prop_str = str;

			   prop_str.erase(0, 15);

			   res = AddClassProperty(current_class, prop_str, true, index);
             }
		 }
	   else if (_wstrincl(str, L"#method", 0)) //добавление нового метода в класс
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring meth_str = str;

			   meth_str.erase(0, 7);

			   res = AddClassMethod(current_class, meth_str, false, index);
             }
		 }
	   else if (_wstrincl(str, L"#publicmethod", 0)) //добавление нового метода в класс
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring meth_str = str;

			   meth_str.erase(0, 13);

			   res = AddClassMethod(current_class, meth_str, true, index);
             }
		 }
	   else if (_wstrincl(str, L"#modifyclass", 0)) //редактирование класса
		 {
		   std::wstring name = str;
		   name.erase(0, 12);

		   std::wstring mark = name;
		   UINT pos = name.find(FRGMARK);
		   mark = name.substr(pos, mark.length() - pos);
		   name.erase(pos, name.length() - pos);
		   current_class = name;

		   res = TranslateFragment(frgStack->GetFragmentCode(mark), index);

		   current_class = L"";
		 }
	   else if (_wstrincl(str, L"#dropproperty", 0)) //удаление свойства из класса
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring prop_str = str;

			   prop_str.erase(0, 13);

			   RESRECORDSET rs = clStack->Get(current_class, prop_str);

			   if (rs.size() == 0)
				 {
				   AddInfoMsg(CLNOPROP, ERRMSG, index);
				   res = false;
				 }
			   else
				 {
				   if (clStack->Delete(L"", current_class, prop_str) < 1)
					 AddInfoMsg(CLNOPROPREM, WRNMSG, index);

				   res = true;
				 }
             }
		 }
	   else if (_wstrincl(str, L"#dropmethod", 0)) //удаление метода из класса
		 {
		   if (current_class == L"")
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   std::wstring prop_str = str;

			   prop_str.erase(0, 11);

			   RESRECORDSET rs = clStack->Get(current_class, prop_str);

			   if (rs.size() == 0)
				 {
				   AddInfoMsg(CLNOPROP, ERRMSG, index);
				   res = false;
				 }
			   else
				 {
				   if (clStack->Delete(L"", current_class, prop_str) < 1)
					 AddInfoMsg(CLNOPROPREM, WRNMSG, index);

				   procStack->Delete(L"", current_class + prop_str, L"");
				   res = true;
				 }
             }
		 }
	   else if (_wstrincl(str, L"#return", 0)) //установка возвращаемого значения метода
		 {
		   res = ParseRefsInExp(str, index);

		   if (res)
			 res = ParseFuncsInExp(str, index);

		   if (res)
			 res = ParseObjectsInExp(str, index);

		   if (res)
			 res = ParseVarInExp(str, index);

		   if (res)
			 {
			   return_val = str;
			   return_val.erase(0, 7);
             }
		 }
	   else if (_wstrincl(str, L"#protect", 0)) //защищенная трансляция кода
		 res = ProtectTranslate(str, index);
	   else if (_wstrincl(str, L"#trigger", 0)) //створення триггеру
		 res = CreateTrigger(str, index);
	   else if (_wstrincl(str, L"#droptrigger", 0)) //видалення триггеру
		 res = RemoveTrigger(str, index);
	   else if (_wstrincl(str, L"#set", 0)) //змінюються налаштування інтерпретатора
		 {
		   std::wstring name = str;
		   name.erase(0, 4);

		   std::wstring mark = name;
		   UINT pos = name.find(FRGMARK);
		   mark = name.substr(pos, mark.length() - pos);
		   name.erase(pos, name.length() - pos);
		   current_class = name;

		   settings_change = true;
		   res = TranslateFragment(frgStack->GetFragmentCode(mark), index);
		   settings_change = false;
         }
	   else if (_wstrincl(str, L"#cnum", 0)) //вмикається парсинг числових констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.ParseNumConst = true;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#!cnum", 0)) //вимикається парсинг числових констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.ParseNumConst = false;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#cstr", 0)) //вмикається парсинг символьних констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.ParseSymConst = true;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#!cstr", 0)) //вимикається парсинг символьних констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.ParseSymConst = false;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#keepobjects", 0)) //зберігати вміст стеку об'єктів після трансляції
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.KeepObjects = true;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#!keepobjects", 0)) //не зберігати вміст стеку об'єктів після трансляції
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.KeepObjects = false;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#keepclasses", 0)) //зберігати вміст стеку класів після трансляції
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.KeepClasses = true;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#!keepclasses", 0)) //не зберігати вміст стеку класів після трансляції
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   InterpreterSettings.KeepClasses = false;
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#oldsym", 0)) //використовувати символ ' як признак строкових констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   STRSYM = wchar_t(39);
			   res = true;
			 }
		 }
	   else if (_wstrincl(str, L"#!oldsym", 0)) //використовувати символ " як признак строкових констант
		 {
		   if (!settings_change)
			 {
			   AddInfoMsg(SYNTAXERR, ERRMSG, index);
			   res = false;
			 }
		   else
			 {
			   STRSYM = wchar_t(34);
			   res = true;
			 }
		 }

	   if (debug_eli)
		 WriteELIDebug(L"ProcessDirective", L"[END]");
	 }
  catch (Exception &e)
	 {
	   if (debug_eli)
		 WriteELIDebug(L"ProcessDirective", String("Exception: " + e.ToString()).c_str());

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::TranslateCodeFromVar(const wchar_t *name, UINT index)
{
  if (debug_eli)
    {
	  WriteELIDebug(L"TranslateCodeFromVar", L"[START]");
	  WriteELIDebug(L"TranslateCodeFromVar", name);
    }

  VARIABLE *var = st->Get(name);

  if (!var)
	{
	  AddInfoMsg(UNKVARNAME, ERRMSG, index);

      return false;
	}

  std::wstring ftxt = st->GetStrElement(var);
  SCRIPTLINES *frg = frgStack->GetFragmentCode(ftxt);

  if (!frg)
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }

  if (TranslateFragment(frg, index))
    {
      frgStack->Remove(ftxt);

      return true;
    }

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::IsSimple(wchar_t *expr)
{
  if (debug_eli)
    {
      WriteELIDebug(L"IsSimple", L"[START]");
      WriteELIDebug(L"IsSimple", expr);
    }

  if (_wstrcpos(expr, '(') > 0)
    return false;
  else
    {
      UINT countplus = _wstrccount(expr, '+');  //кол-во +
      UINT countminus = _wstrccount(expr, '-'); //кол-во -
      UINT countmult = _wstrccount(expr, '*');  //кол-во *
      UINT countdiv = _wstrccount(expr, '/');   //кол-во /

      if (!countplus && !countminus && !countmult && !countdiv)
		return true;

      if ((countplus > 0) || (countminus > 0))
        {
          if ((0 == countmult) && (0 == countdiv))
            return true;
        }

      if ((countmult > 0) || (countdiv > 0))
        {
          if ((0 == countplus) && (0 == countminus))
            return true;
        }

      return false;
    }
}
//-------------------------------------------------------------------------------

bool ELI::IsNumExpression(const wchar_t *expr)
{
//65-90 - символы A-Z,
//97-122 - символы a-z,
//36 - символ $,
//95 - нижнее подчеркивание _

  if (debug_eli)
    {
      WriteELIDebug(L"IsNumExpression", L"[START]");
      WriteELIDebug(L"IsNumExpression", expr);
    }

  while (!_wstrend(expr))
    {
      if ((IsNum(*expr) < 0) &&
          (*expr != '(') &&
           (*expr != ')') &&
			(IsOper(*expr) < 0) &&
			(*expr != '.'))
        return false;

	   expr++;
    }

  if (debug_eli)
    {
      WriteELIDebug(L"IsNumExpression", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

UINT ELI::OperSymbPos(std::wstring str)
{
  if (debug_eli)
    {
      WriteELIDebug(L"OperSymbPos", L"[START]");
      WriteELIDebug(L"OperSymbPos", str.c_str());
    }

  UINT pos;

  for (UINT i = 0; i < OPSYMCNT; i++)
    {
	  pos = str.find(OperSymb[i]);

	  if (pos != std::wstring::npos)
        return pos;
    }

  if (debug_eli)
    WriteELIDebug(L"OperSymbPos", L"return FAIL");

  return -1;
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::RemoveScopes(wchar_t *in_exp, UINT index)
{
  if (debug_eli)
    WriteELIDebug(L"RemoveScopes", L"[START]");

  std::vector<SCPOS> scPos;
  FStrBuffer = L"";
  FStrBuffer = in_exp;
  std::vector<std::wstring> expScope;

  UINT len = wcslen(in_exp);
  SCPOS exp;
  int spos;

  for (UINT i = 0; i < len; i++) //находим последнюю открывающую
    {
      if ('(' == in_exp[i])
        {
          exp.oppos = i;
          exp.clpos = -1;
          scPos.push_back(exp);
        }

      if (')' == in_exp[i])
        {
          spos = scPos.size() - 1;
          do
            {
              if (scPos[spos].clpos < 0)
                {
                  scPos[spos].clpos = i;
                  break;
                }
              spos--;
            }
          while (spos != -1);
        }
    }

  spos = scPos.size() - 1;

  do
    {
	  expScope.push_back(FStrBuffer.substr(scPos[spos].oppos, scPos[spos].clpos + 1 - scPos[spos].oppos));
      spos--;
	}
  while(spos != -1);

  wchar_t strval[NUMSIZE], line[CHARSIZE];

  for (UINT i = 0; i < expScope.size(); i++)
    {
      if (1 == _wstrccount(expScope[i].c_str(), '('))
        {
		  wcscpy(line, expScope[i].substr(1, expScope[i].length() - 2).c_str());
          float *arg = CalcExpNum(line, index);

		  if (!arg)
            return ERROUT;

          swprintf(strval, FRMTNUM, *arg);
		  FStrBuffer = ParseStringW(FStrBuffer, expScope[i], strval);
        }
    }

  if (debug_eli)
    WriteELIDebug(L"RemoveScopes", L"[END]");

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::SetExpToSimple(wchar_t *in_exp, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"SetExpToSimple", L"[START]");
      WriteELIDebug(L"SetExpToSimple", in_exp);
    }

  std::vector<wchar_t> opPos;
  wchar_t outstr[CHARSIZE];
  UINT pos;

  FStrBuffer = ERROUT;

  while (_wstrccount(in_exp, '(') > 0) //уберем из выражения скобки
    {
      wcscpy(in_exp, RemoveScopes(in_exp, index));
    }

  if (0 == wcscmp(in_exp, ERROUT))
    return ERROUT;

  std::wstring operstr = in_exp;

//запомним порядок расположения символов операций
  for (UINT i = 0; i < operstr.length(); i++)
     {
       if (IsOper(operstr.c_str()[i]) > -1)
         opPos.push_back(operstr[i]);
     }

//убираем символы операций
  operstr = ParseStringW(operstr, L"+", L"=");
  operstr = ParseStringW(operstr, L"-", L"=");
  operstr = ParseStringW(operstr, L"*", L"=");
  operstr = ParseStringW(operstr, L"/", L"=");

  std::vector<std::wstring> args;

  StrToListW(&args, operstr, L"=", NODELIMEND);

  for (UINT i = opPos.size() - 1; i > 1; i--)
     {
       if (IsOper(opPos[i]) > 2) //найден символ операци с высоким приоритетом
         {
//индексу i операции соответствуют элементы i-1 и i в векторе аргументов
//если аргументы - отрицательные числа, заменим NEGNUM на -
		   if (NEGNUM == args.at(i)[0])
			 args.at(i)[0] = '-';

		   if (NEGNUM == args.at(i + 1)[0])
			 args.at(i + 1)[0] = '-';

           float larg = _wtof(args[i].c_str());
           float rarg = _wtof(args[i + 1].c_str());

           if (rarg == 0.0)
             {
               AddInfoMsg(SYNTAXERR, ERRMSG, index);

               return ERROUT;
             }

           switch (opPos[i])
             {
               case '*': larg = larg * rarg; break;
               case '/':
                 {
                   if (rarg != 0)
                     larg = larg / rarg;
                   else
                     {
                       AddInfoMsg(SYNTAXERR, ERRMSG, index);

                       return NULL;
                     }
                   break;
                 }
             }

//запишем полученное значение обратно в вектор аргументов
           wchar_t arg[NUMSIZE];
           swprintf(arg, FRMTNUM, larg);
           args[i] = arg;
           args[i + 1] = L"@"; //пометим элемент вектора аргументов на удаление
           opPos[i] = '@';    //пометим элемент вектора операций на удаление

        }
     }

//соберем содержимое векторов обратно в строку
  pos = 0;

  for (UINT i = 1; i < args.size(); i++)
     {
       if ('@' != opPos[i - 1])
         pos += swprintf(outstr + pos, L"%c%s", opPos[i - 1], args[i].c_str());
	 }

  FStrBuffer = outstr;

  if (debug_eli)
    WriteELIDebug(L"SetExpToSimple", L"[END]");

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

bool ELI::IsCorrectVarName(const wchar_t *varname)
{
//65-90 - символы A-Z,
//97-122 - символы a-z,
//36 - символ $,
//48-57 - цифры,
//91, 93 - кв. скобки

  if (debug_eli)
    {
      WriteELIDebug(L"IsCorrectVarName", L"[START]");
      WriteELIDebug(L"IsCorrectVarName", varname);
    }

  int symcode;

  while (!_wstrend(varname))
    {
      symcode = (int)*varname;

     if (symcode < 65)
       {
         if ((symcode < 48) && (symcode != 36))
           return false;
         else if (symcode > 57)
           return false;
       }

     if (symcode > 90)
       {
         if (symcode < 97)
           {
             if ((symcode != 91) && (symcode != 93))
               return false;
           }

         if (symcode > 122)
           return false;
       }

      varname++;
    }

  if (debug_eli)
    WriteELIDebug(L"IsCorrectVarName", L"[OK]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::IsCorrectName(const wchar_t *str)
{
//65-90 - символы A-Z,
//97-122 - символы a-z,
//36 - символ $,
//45 - символ -,
//48-57 - цифры,
//91, 93 - кв. скобки,
//95 - нижнее подчеркивание _
//126 - тільда ~

  if (debug_eli)
    {
      WriteELIDebug(L"IsCorrectName", L"[START]");
      WriteELIDebug(L"IsCorrectName", str);
    }

  int symcode;

  while (!_wstrend(str))
    {
      symcode = (int)*str;

     if (symcode < 65)
       {
         if (symcode < 48)
           return false;
         else if (symcode > 57)
           return false;
       }

     if (symcode > 90)
       {
         if (symcode < 97)
           {
             if ((symcode != 91) && (symcode != 93) && (symcode != 95))
               return false;
           }

		 if ((symcode > 122) && (symcode != 126))
		   return false;
       }

      str++;
    }

  if (debug_eli)
    WriteELIDebug(L"IsCorrectName", L"[OK]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::IsClassMember(const wchar_t *cl_name, const wchar_t *mb_name)
{
  if (debug_eli)
	{
	  String msg = cl_name;
	  msg += OBJPROPSEPSTR;
	  msg += mb_name;

	  WriteELIDebug(L"IsClassMember", L"[START]");
	  WriteELIDebug(L"IsClassMember", msg.c_str());
	}

  if (clStack->Get(cl_name, mb_name).size() > 0)
    {
      if (debug_eli)
        WriteELIDebug(L"IsClassMember", L"[OK]");

      return true;
    }

  if (debug_eli)
    WriteELIDebug(L"IsClassMember", L"[FAIL]");

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::IsPublicMember(const wchar_t *cl_name, const wchar_t *mb_name)
{
  if (debug_eli)
    {
      WriteELIDebug(L"IsPublicMember", L"[START]");
      WriteELIDebug(L"IsPublicMember", mb_name);
    }

  RESRECORDSET rs = clStack->Get(cl_name, mb_name);

  if (rs.size() > 0)
    {
      if ((rs[0]->ObjectCathegory == CLPUBMETHOD) || (rs[0]->ObjectCathegory == CLPUBPROP))
        {
          if (debug_eli)
            WriteELIDebug(L"IsPublicMember", L"[OK]");

          return true;
        }
	}

  if (debug_eli)
    WriteELIDebug(L"IsPublicMember", L"[FAIL]");

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::IsAccessibleMember(const wchar_t *obj_name, const wchar_t *mb_name)
{
  if (debug_eli)
    {
      WriteELIDebug(L"IsAccessibleMember", L"[START]");
      WriteELIDebug(L"IsAccessibleMember", mb_name);
    }

  RESRECORDSET rs = objStack->Get(obj_id, std::wstring(obj_name));

  std::wstring cl_name = rs[0]->ObjectCathegory;

  if (IsPublicMember(cl_name.c_str(), mb_name))
    {
      if (debug_eli)
        WriteELIDebug(L"IsAccessibleMember", L"[OK]");

      return true;
    }
  else //член не публичный
    {
	  VARIABLE *var = st->Get(OBJTHIS);

      if (var) //обращение идет внутри класса
        {
		  if (wchar_t(OBJSYM) + st->GetStrElement(var) == obj_name)
            {
			  if (debug_eli)
                WriteELIDebug(L"IsAccessibleMember", L"[OK]");

              return true;
            }
        }
    }

  if (debug_eli)
    WriteELIDebug(L"IsAccessibleMember", L"[FAIL]");

  return false;
}
//-------------------------------------------------------------------------------

UINT ELI::CheckExprType(const wchar_t *expr, UINT index)
{
  if (debug_eli)
    WriteELIDebug(L"CheckExprType", L"[START]");

  static UINT exptype;

  if (VARSYM == expr[0]) //если expr - переменная
    {
      VARIABLE *varptr = st->Get(expr);

      if (varptr != NULL)
        exptype = varptr->type;
      else
        exptype = 0;
    }
  else if (STRSYM == expr[0]) //expr - константная строка
    {
      if (STRSYM == expr[wcslen(expr) - 1]) //если последний символ тоже STRSYM
        exptype = SCSTR;
      else
        exptype = 0;
    }
  else if ((IsNum(expr[0]) > -1) || ('-' == expr[0]) || (NEGNUM == expr[0])) //expr - число
    {
	  int numtype = IsStrNum(expr); //определяем какое именно

	  if (numtype >= 0)
        exptype = SCNUM;
      else
        exptype = 0;
    }
  else
    return 0;

  if (debug_eli)
    WriteELIDebug(L"CheckExprType", L"[END]");

  return exptype;
}
//-------------------------------------------------------------------------------

bool ELI::ParseVarInExp(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseVarInExp", L"[START]");
      WriteELIDebug(L"ParseVarInExp", expr);
    }

  std::wstring operstr = expr;
  wchar_t str[MAXNAMELEN];
  std::vector<std::wstring> varPos;

  int cppos = -1;
  UINT exprlen;

  exprlen = wcslen(expr);

  for (UINT i = 0; i < exprlen; i++)
    {
      if (expr[i] == VARSYM)
        {
          if (cppos > -1)
            {
              _wstrncopy(expr, str, cppos, i - cppos);
              varPos.push_back(str);
            }

          cppos = i;
        }

      if ((IsOper(expr[i]) > -1) ||
          (')' == expr[i]) ||
          (STRCCONS == expr[i]) ||
          (',' == expr[i]) ||
          (':' == expr[i]) ||
		  //('.' == expr[i]) ||
		  (OBJPROPSEP == expr[i]))
        {
          if (cppos > -1)
            {
              _wstrncopy(expr, str, cppos, i - cppos);
              varPos.push_back(str);
            }

          cppos = -1;
        }

     if (i == exprlen - 1)
       {
         if (cppos > -1)
           {
             _wstrncopy(expr, str, cppos, i - cppos + 1);
             varPos.push_back(str);
           }

         cppos = -1;
       }
    }

  VARIABLE *var;

  for (UINT i = 0; i < varPos.size(); i++)
    {
      var = st->Get(varPos[i].c_str());

      if (var)
        {
          if (SCNUM == var->type)
            {
              float f, num;

              num = st->GetNumElement(var);

              if (modf(num, &f) == 0)
                swprintf(str, L"%.0f", num);
              else
                swprintf(str, FRMTNUM, num);

			  operstr = ParseStringW(operstr, varPos[i], str);
            }
          else if (SCSTR == var->type) //если переменная - строка, все выражение должно быть строкой
            {
			  operstr = ParseStringW(operstr, varPos[i], st->GetStrElement(var));
            }
          else
            {
              AddInfoMsg(LREXPRERR, ERRMSG, index);

              return false;
			}
        }
      else
        {
          AddInfoMsg(UNKVARNAME, ERRMSG, index);

          if (debug_eli)
            {
              WriteELIDebug(L"ParseVarInExp", varPos[i].c_str());
              WriteELIDebug(L"ParseVarInExp", L"[FAIL]");
            }

          return false;
        }
    }

  wcscpy(expr, operstr.c_str());

  if (debug_eli)
    {
      WriteELIDebug(L"ParseVarInExp", expr);
      WriteELIDebug(L"ParseVarInExp", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ParseFuncsInExp(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseFuncsInExp", L"[START]");
      WriteELIDebug(L"ParseFuncsInExp", expr);
    }

  std::wstring operstr = expr;
  UINT op, pos;

  while (operstr.find_last_of(FUNCSYM) != std::wstring::npos)
    {
	  UINT fspos = operstr.find_last_of(FUNCSYM);
	  std::wstring fstr = operstr.substr(fspos, operstr.length() - fspos);

//уберем лишние закрывающие скобки
      op = _wstrccount(fstr.c_str(), '(');

//нет закр. скобок - символ "_" не является признаком функции
//делаем возврат истины, чтобы не стопорить интепретатор
      if (op < 1)
        return true;

      while (_wstrccount(fstr.c_str(), ')') > op)
        {
		  pos = fstr.find_last_of(')');
		  fstr.erase(pos, fstr.length() - pos);
        }

//если между последней закр. скобкой и концом строки остались символы - уберем их
	  pos = fstr.find_last_of(')');

	  if (pos && (pos < fstr.length() - 1))
		fstr.erase(pos + 1, fstr.length() - pos - 1);

      wchar_t result[CHARSIZE], str[CHARSIZE];

      wcscpy(str, fstr.c_str());

	  if (RunFunc(str, result, index))
		operstr = ParseStringW(operstr, fstr, std::wstring(result));
	  else
		return false;
    }

  wcscpy(expr, operstr.c_str());

  if (debug_eli)
    {
      WriteELIDebug(L"ParseFuncsInExp", expr);
      WriteELIDebug(L"ParseFuncsInExp", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ParseObjectsInExp(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseObjectsInExp", L"[START]");
      WriteELIDebug(L"ParseObjetcsInExp", expr);
    }

  std::wstring operstr = expr, obstr;
  UINT obspos;

  while (operstr.find(OBJSYM) != std::wstring::npos)
    {
	  obspos = operstr.find(OBJSYM);
	  obstr = operstr.substr(obspos, operstr.length() - obspos);

	  if (obstr.find(OBJPROPSEP) == std::wstring::npos) //нет точки - ошибка
        {
          AddInfoMsg(ILLGLOBJOPER, ERRMSG, index);

          return false;
        }

	  wchar_t result[CHARSIZE], str[CHARSIZE];

//копируем символы из рабочей строки в строку объекта, пока не получим стоп-символ
//стоп-символ это: =
      UINT i = 0;
      UINT opcnt = 0, clcnt = 0;

	  while(i < obstr.length())
        {
		  if (obstr[i] == '(')
            opcnt++;
          else if (obstr[i] == ')')
            {
              clcnt++;

              if (opcnt < clcnt) //есть закрывающая, но нет откр. - объект внутри скобок
                break;
            }
		  else if (obstr[i] == '+' ||
				   obstr[i] == '-' ||
				   obstr[i] == '/' ||
				   obstr[i] == '*' ||
				   obstr[i] == ',')
            {
              if (opcnt == clcnt)
                break;
            }
          else if (obstr[i] == '=')
            break;

          if ((opcnt > 0) & (opcnt == clcnt))
            {
              i++;
              break;
            }

          i++;
        }

	  obstr.erase(i, obstr.length() - i);
      wcscpy(str, RemoveSpaces(obstr.c_str()));

      if (WorkWithObject(str, result, index))
		operstr = ParseStringW(operstr, obstr, std::wstring(result));
      else
        return false;
    }

  wcscpy(expr, operstr.c_str());

  if (debug_eli)
    {
      WriteELIDebug(L"ParseObjectsInExp", expr);
      WriteELIDebug(L"ParseObjectsInExp", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ParseRefsInExp(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseRefsInExp", L"[START]");
      WriteELIDebug(L"ParseRefsInExp", expr);
    }

  std::wstring operstr = expr;
  wchar_t str[MAXNAMELEN];
  std::vector<std::wstring> refPos;

  int cppos = -1;
  UINT exprlen;

  exprlen = wcslen(expr);

  for (UINT i = 0; i < exprlen; i++)
    {
      if (expr[i] == REFSYM)
        {
          if (cppos > -1)
            {
              _wstrncopy(expr, str, cppos, i - cppos);
              refPos.push_back(str);
            }

          cppos = i;
        }

      if ((IsOper(expr[i]) > -1) ||
          (')' == expr[i]) ||
          ('(' == expr[i]) ||
          (STRCCONS == expr[i]) ||
          (',' == expr[i]) ||
		  //('.' == expr[i]) ||
          VARSYM == expr[i] ||
          FUNCSYM == expr[i] ||
          PROCSYM == expr[i] ||
		  OBJSYM == expr[i] ||
		  OBJPROPSEP == expr[i])
        {
          if (cppos > -1)
            {
              _wstrncopy(expr, str, cppos, i - cppos);
              refPos.push_back(str);
            }

          cppos = -1;
        }

     if (i == exprlen - 1)
       {
         if (cppos > -1)
           {
             _wstrncopy(expr, str, cppos, i - cppos + 1);
             refPos.push_back(str);
           }

         cppos = -1;
       }
    }

  REFERENCE *rf;

  for (UINT i = 0; i < refPos.size(); i++)
    {
      rf = GetRef(refPos[i].c_str());

      if (rf)
		operstr = ParseStringW(operstr, refPos[i], std::wstring(rf->refobj));
      else
        {
          AddInfoMsg(UNKDECR, ERRMSG, index);

          return false;
        }
    }

  wcscpy(expr, operstr.c_str());

  if (debug_eli)
    {
      WriteELIDebug(L"ParseRefsInExp", expr);
      WriteELIDebug(L"ParseRefsInExp", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ParseIncObjects(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseIncObjects", L"[START]");
      WriteELIDebug(L"ParseIncObjects", expr);
    }

  std::wstring operstr, tmpobj, tmpname, tmpprop, tmpmethod;
  int dpos;
  RESRECORDSET rs;

  if (!ParseVarInExp(expr, index))
	{
	  if (debug_eli)
		WriteELIDebug(L"ParseIncObjects", L"[FAIL]");

	  return false;
	}

  while (_wstrccount(expr, OBJPROPSEP) > 1)
    {
	  rs.clear();
	  operstr = expr;
	  tmpobj = operstr;
	  dpos = tmpobj.find(OBJPROPSEPSTR);
	  tmpname = tmpobj.substr(0, dpos);
	  tmpprop = tmpobj.erase(0, dpos + 1);

	  dpos = tmpprop.find('('); //перевіримо чи є методом обраний член класу

	  if (dpos != std::wstring::npos) //якщо так - обріжемо все, що до дужок методу
		{
		  tmpmethod = tmpprop.substr(dpos, tmpprop.length() - dpos);
		  tmpprop.erase(dpos, tmpprop.length() - dpos);
		  operstr = tmpname + OBJPROPSEPSTR + tmpprop;
		}

	  dpos = tmpprop.find(OBJPROPSEPSTR);

	  if (dpos != std::wstring::npos)
		tmpprop.erase(dpos, tmpprop.length() - dpos);

	  rs = objStack->Get(obj_id, std::wstring(tmpname));

      if (rs.size() == 0)
        {
          AddInfoMsg(OBJNONE, ERRMSG, index);

          if (debug_eli)
            WriteELIDebug(L"ParseIncObjects", L"[FAIL]");

          return false;
        }

      if (IsClassMember(rs[0]->ObjectCathegory.c_str(), tmpprop.c_str()))
        {
          if (!IsAccessibleMember(tmpname.c_str(), tmpprop.c_str()))
            {
              AddInfoMsg(OBJMEMNOTACC, ERRMSG, index);

              if (debug_eli)
                WriteELIDebug(L"ParseIncObjects", L"[FAIL]");

              return false;
            }
        }

	  rs.clear();

//получаем имя объекта, которое хранится в свойстве исходного объекта и парсим его в строку
	  rs = objStack->Get(std::wstring(tmpname), std::wstring(tmpprop));

	  if (rs[0]->Value[0] == OBJSYM) //отримане значення є ім'ям вкладеного об'єкта
		operstr = ParseStringW(operstr, tmpname + OBJPROPSEPSTR + tmpprop, rs[0]->Value);

	  wcscpy(expr, operstr.c_str());
	}

  if (!tmpmethod.empty()) //якщо був задіяний метод - повертаємо в рядок все, що було в дужках
	{
	  wchar_t methodstr[CHARSIZE];

	  wcscpy(methodstr, tmpmethod.c_str());

	  if (!ParseObjectsInExp(methodstr, index))
		{
		  if (debug_eli)
			{
			  WriteELIDebug(L"ParseIncObjects", methodstr);
			  WriteELIDebug(L"ParseIncObjects", L"[FAIL]");
            }

		  return false;
		}
	  else
		tmpmethod = methodstr;

	  wcscat(expr, tmpmethod.c_str());
    }

  if (debug_eli)
    {
      WriteELIDebug(L"ParseIncObjects", expr);
      WriteELIDebug(L"ParseIncObjects", L"[OK]");
    }

  return true;
}
//-------------------------------------------------------------------------------

float *ELI::CalcExpNum(wchar_t *expr, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"CalcExpNum", L"[START]");
	  WriteELIDebug(L"CalcExpNum", expr);
    }

  std::vector<SUBEXP> vecExp;
  wchar_t exp[CHARSIZE];
  FNumBuffer = 0.0f;

  swprintf(exp, L"=%s", expr); //приводим строку выражения в формат "=строка"

//если в выражении есть символы объектов, заменим все объекты на значения свойств или методов
  if (_wstrccount(exp, OBJSYM) > 0)
    {
      if (!ParseObjectsInExp(exp, index))
        return NULL;
    }

//если в выражении есть символы ф-й, заменим все ф-ии на значения
  if (_wstrccount(exp, FUNCSYM) > 0)
    {
      if (!ParseFuncsInExp(exp, index))
        return NULL;
    }

//если в выражении есть символ переменной, заменим все переменные на значения
  if (_wstrccount(exp, VARSYM) > 0)
    {
      if (!ParseVarInExp(exp, index))
        return NULL;
    }

//проверим выражение на соответствие числовому и вернем управление, если не соответствует
  if (!IsNumExpression(exp))
    {
      if (debug_eli)
        WriteELIDebug(L"CalcExpNum", L"[NOT NUM EXPR]");

      return NULL;
    }

  if (!IsSimple(exp)) //если выражение - сложное, приведем его к простому
	wcscpy(exp, SetExpToSimple(exp, index));

  if (0 == wcscmp(exp, ERROUT))
    return NULL;

//если выражение - простое, проведем последовательно все операции
  SUBEXP se;
  UINT len = wcslen(exp);

  for (UINT i = 0; i < len; i ++) //заполняем вектор подвыражений
     {
       if ((IsOper(exp[i]) > -1) && ('-' == exp[i + 1]) && (i + 1 < len))
         {
           exp[i + 1] = NEGNUM; //заменяем минусы отрицательных чисел на спецсимвол
         }

       if ((IsOper(exp[i]) > -1))
         {
           se.oper = exp[i];
           se.arg = 0;

           vecExp.push_back(se);
         }
     }

//парсим строку заменяя символы операций на символ-разделитель
   std::wstring operstr = exp;
   operstr = ParseStringW(operstr, L"+", L"=");
   operstr = ParseStringW(operstr, L"-", L"=");
   operstr = ParseStringW(operstr, L"*", L"=");
   operstr = ParseStringW(operstr, L"/", L"=");

//разделяем строку по символу-разделителю, дописываем параметр arg в вектор vecExp
   std::vector<std::wstring> args;

   StrToListW(&args, operstr, L"=", NODELIMEND);

   for (UINT i = 0; i < vecExp.size(); i++)
	  {
        if (NEGNUM == args.at(i + 1)[0])
		  args.at(i + 1)[0] = '-';

        if (SCNUM != CheckExprType(args[i + 1].c_str(), index))
          return NULL;

        vecExp[i].arg = _wtof(args[i + 1].c_str());
      }

//вычисляем результат всех элементов vecExp
   for (UINT i = 0; i < vecExp.size(); i++)
      {
        switch (vecExp[i].oper)
           {
			 case '=': FNumBuffer = vecExp[i].arg; break;
			 case '+': FNumBuffer = FNumBuffer + vecExp[i].arg; break;
			 case '-': FNumBuffer = FNumBuffer - vecExp[i].arg; break;
			 case '*': FNumBuffer = FNumBuffer * vecExp[i].arg; break;
             case '/':
                 {
                   if (vecExp[i].arg != 0.00)
                     FNumBuffer = FNumBuffer / vecExp[i].arg;
                   else
                     return NULL;

                   break;
                 }
           }
        }

  if (debug_eli)
    WriteELIDebug(L"CalcExpNum", L"[END]");

  return &FNumBuffer;
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::CalcExpStr(wchar_t *expr, UINT index)
{
  if (debug_eli)
    WriteELIDebug(L"CalcExpStr", L"[START]");

  UINT pos;
  bool iscstr = false;

  if (0 == _wstrccount(expr, '+')) //нет символов операций - переменной присваивается конст. строка или формат. ввод
    iscstr = true;

  wcscpy(expr, ParseStringW(std::wstring(expr), L"+", STRCONS).c_str()); //заменим знак + на знак конкатенации

//если в выражении есть символы объектов, заменим все объекты на значения свойств или методов
  if (_wstrccount(expr, OBJSYM) > 0)
    {
      if (!ParseObjectsInExp(expr, index))
        return ERROUT;
    }
//если в выражении есть символы ф-й, заменим все ф-ии на значения
  if (_wstrccount(expr, FUNCSYM) > 0)
    {
      if (!ParseFuncsInExp(expr, index))
        return ERROUT;
    }
//если в выражении есть символ переменной, заменим все переменные на значения
  if (_wstrccount(expr, VARSYM) > 0)
    {
      if (!ParseVarInExp(expr, index))
		return ERROUT;
    }

  if (iscstr)
    return expr;

  std::vector<std::wstring> strparts;

  StrToListW(&strparts, std::wstring(expr), STRCONS, NODELIMEND); //разобъем строку на части

  pos = 0;
  for (UINT i = 0; i < strparts.size(); i++) //уберем признаки строки из элементов, содерж. конст. строки
    {
	  if (STRSYM == strparts.at(i)[0])
		strparts[i] = strparts[i].substr(1, strparts[i].length() - 2);

      pos += swprintf(expr + pos, L"%s", strparts[i].c_str());
    }

  if (debug_eli)
    {
      WriteELIDebug(L"CalcExpStr", expr);
      WriteELIDebug(L"CalcExpStr", L"[END]");
    }

  return expr;
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::ParseConstStrings(wchar_t *text)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ParseConstStrings", L"[START]");
      WriteELIDebug(L"ParseConstStrings", text);
    }

  int sspos = -1;
  bool op = false;
  UINT len = wcslen(text);
  wchar_t cstr[CHARSIZE];
  std::vector<std::wstring> vecStrings;

  FStrBuffer = text;

  for (UINT i = 0; i < len; i++) //найдем все конст. строки и внесем в вектор
    {
      if (STRSYM == text[i])
        {
          if (!op)
            {
              sspos = i;
              op = true;
            }
          else
            {
              _wstrncopy(text, cstr, sspos, i - sspos + 1);
              vecStrings.push_back(cstr);
              op = false;
            }
        }
    }

  std::wstring tmp;

//создадим переменые соотв. этим строкам в стеке и заменим строки ('некий текст') в скрипте на имена переменных
  for (UINT i = 0; i < vecStrings.size(); i++)
    {
	  tmp = vecStrings[i].substr(1, vecStrings[i].length() - 2); //уберем кавычки

	  VARIABLE *var = st->GetByValue(tmp);
	  wchar_t name[MAXNAMELEN];

      if (!var) //если такой строки нет в стеке - добавим
        {
          swprintf(name, CSTRF, CstrInd); //создадим имя (шаблон из #define + глоб. индекс)
          st->Add(name, tmp);
		  FStrBuffer = ParseStringW(FStrBuffer, vecStrings[i], std::wstring(name));

		  CstrInd++; //увеличим индекс конст. строк
        }
      else //если есть - используем имеющуюся переменную из стека
        {
          if (_wstrincl(var->varname, L"$CSTR", 0))
			FStrBuffer = ParseStringW(FStrBuffer, vecStrings[i], std::wstring(var->varname));
          else
			{
              swprintf(name, CSTRF, CstrInd); //создадим имя (шаблон из #define + глоб. индекс)
              st->Add(name, tmp);
			  FStrBuffer = ParseStringW(FStrBuffer, vecStrings[i], std::wstring(name));

              CstrInd++; //увеличим индекс конст. строк
            }
        }
    }

  if (debug_eli)
    {
      WriteELIDebug(L"ParseConstStrings", L"[END]");
    }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::ParseConstNumbers(wchar_t *text)
{
  if (debug_eli)
    {
	  WriteELIDebug(L"ParseConstNumbers", L"[START]");
	  WriteELIDebug(L"ParseConstNumbers", text);
    }

  int bpos = -1, epos = -1;
  wchar_t cnum[NUMSIZE];
  std::vector<std::wstring> vecNums;
  FStrBuffer = text;
  UINT len = FStrBuffer.length();

  for (int i = 0; i < len; i++) //знайдемо всі числові константи та занесемо у вектор
	{
	  if (FStrBuffer[i] == '-') //знайдено знак "-"
		{
		  if (bpos > -1)
			epos = i; //це кінець константи
		  else if (((i + 1) < len) && (IsNum(FStrBuffer[i + 1]) >= 0)) //якщо наступний символ - цифра
			bpos = i; //початок від'ємної константи
		}
	  else if (IsNum(FStrBuffer[i]) >= 0) //знайдено цифру - початок константи
		{
		  if (bpos < 0)
			{
			  if ((i - 1) >= 0) //перевіримо попередній символ
				{
				  int opsym = IsOper(FStrBuffer[i - 1]); //символ операції, значить початок константи

				  if ((opsym == 1) || (opsym == 3) || (opsym == 4)) //'+' '*' '/'
					bpos = i;
				  else if (opsym == 2) //'-'
					bpos = i - 1;
				  else if (FStrBuffer[i - 1] == ' ') //пробіл - отже початок константи
					bpos = i;
				  else if (FStrBuffer[i - 1] == '(') //відкриваюча дужка теж початок константи
					bpos = i;
				  else if (FStrBuffer[i - 1] == ')') //закриваюча дужка теж початок константи
					bpos = i;
				}
			  else if (i == 0) //цифра є першим символом рядка
				bpos = i;

              if (i == len - 1) //це останній символ
				epos = i + 1;
			}
		  else if (i == len - 1) //це останній символ
			epos = i + 1;
		  else
            continue;
		}
	  else if (IsNum(FStrBuffer[i]) < 0) //символ не є цифрою
		{
		  if (FStrBuffer[i] == '.') //може символ це десятковий роздільник
			{
			  if (((i + 1) < len) && (IsNum(FStrBuffer[i + 1]) >= 0)) //якщо далі йде цифра
				continue; //переходимо до наступного символу
			  else if (bpos > -1)
				{
				  bpos = -1;
                  epos = -1;
				}
			}
		  else if (bpos > -1)
			{
			  if (FStrBuffer[i] == ' ') //якщо це пробіл
				epos = i; //це кінець константи
			  else if (IsOper(FStrBuffer[i]) > 0) //символ операції
				epos = i;
			  else if ((FStrBuffer[i] == '(') || (FStrBuffer[i] == ')')) //дужки
                epos = i;
			  else
				{
				  bpos = -1;
                  epos = -1;
				}
			}
		}

	  if ((bpos > -1) && (epos > -1)) //якщо вдалося виділити константу
		{
		  vecNums.push_back(FStrBuffer.substr(bpos, epos - bpos));
		  FStrBuffer.insert(epos, L"~");
		  len = FStrBuffer.length();
		  bpos = -1;
		  epos = -1;
		}
	}

//створимо змінні відповідно цих чисел у стеку та замінимо числа на імена змінних
  for (UINT i = 0; i < vecNums.size(); i++)
	{
	  VARIABLE *var = st->GetByValue(vecNums[i]);
	  wchar_t name[MAXNAMELEN];

	  if (!var) //такого числа ще немає у стеку - додаєм
        {
		  swprintf(name, CNUMF, CnumInd); //створимо ім'я (шаблон з #define + глоб. індекс)
		  st->Add(name, vecNums[i]);
		  FStrBuffer = ParseStringW(FStrBuffer, vecNums[i] + L"~", std::wstring(name));

		  CnumInd++; //збільшуємо індекс числових констант
		}
	  else //таке число вже є - використовуємо константу зі стеку
        {
          if (_wstrincl(var->varname, L"$CNUM", 0))
			FStrBuffer = ParseStringW(FStrBuffer, vecNums[i] + L"~", std::wstring(var->varname));
          else
			{
			  swprintf(name, CNUMF, CnumInd); //створимо ім'я (шаблон з #define + глоб. індекс)
			  st->Add(name, vecNums[i]);
			  FStrBuffer = ParseStringW(FStrBuffer, vecNums[i] + L"~", std::wstring(name));

			  CnumInd++; //збільшуємо індекс числових констант
            }
        }
    }

  if (debug_eli)
    {
      WriteELIDebug(L"ParseConstNumbers", L"[END]");
	}

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

SCRIPTLINES ELI::GetInclude(const wchar_t *str)
{
  if (debug_eli)
	WriteELIDebug(L"GetInclude", L"[START]");

  FCodeBuffer.clear();

  std::wstring inclstr, path, text;

  inclstr = str;
  path = inclstr.substr(8, inclstr.length() - 8);

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(initdir) + path;
	}

  text = LoadTextFile(path.c_str()).c_str();

  UINT pos;
  std::wstring tmp;

  text = RemoveEndlines(text);

  if (text.find(L"{") != std::wstring::npos)
	text = MarkFragments(text);

  StrToListW(&FCodeBuffer, text, ENDLNSTR, DELIMEND);

  if (debug_eli)
	WriteELIDebug(L"GetInclude", L"[END]");

  return FCodeBuffer;
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::RemoveSpaces(const wchar_t *text)
{
  if (debug_eli)
    {
      WriteELIDebug(L"RemoveSpaces", L"[START]");
      WriteELIDebug(L"RemoveSpaces", text);
    }

  FStrBuffer = text;
  UINT pos;

  while (FStrBuffer.find(L" ") != std::wstring::npos) //убираем пробелы из текста
    {
	  pos = FStrBuffer.find(L" ");
	  FStrBuffer.erase(pos, 1);
    }

  while (FStrBuffer.find(wchar_t(9)) != std::wstring::npos) //убираем табуляцию из текста
    {
	  pos = FStrBuffer.find(wchar_t(9));
	  FStrBuffer.erase(pos, 1);
    }

  FStrBuffer = RemoveEndlines(FStrBuffer);

  if (debug_eli)
    {
	  WriteELIDebug(L"RemoveSpaces", FStrBuffer.c_str());
      WriteELIDebug(L"RemoveSpaces", L"[END]");
    }

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

std::wstring ELI::RemoveEndlines(std::wstring text)
{
  try
	{
	  UINT pos;

	  while (text.find(L"\r\n") != std::wstring::npos)
		{
		  pos = text.find(L"\r\n");
		  text.erase(pos, 2);
		}

	  while (text.find(L"\n") != std::wstring::npos)
		{
		  pos = text.find(L"\n");
		  text.erase(pos, 1);
		}
	}
  catch(Exception &e)
	{
      text = L"";

	  if (debug_eli)
		WriteELIDebug(L"RemoveEndlines", String("Exception: " + e.ToString()).c_str());
	}

  return text;
}
//-------------------------------------------------------------------------------

void ELI::PrepareScript()
{
  if (debug_eli)
    WriteELIDebug(L"PrepareScript", L"[START]");

  UINT pos;
  std::wstring tmp;

  scrtext = RemoveEndlines(scrtext);

  if (scrtext.find(L"{") != std::wstring::npos)
	scrtext = MarkFragments(scrtext);

  try
	{
	  StrToListW(&vecScList, scrtext, ENDLNSTR, DELIMEND);
	}
  catch(Exception &e)
	{
	  String msg = "PrepareScript(), StrToListW(&vecScList, scrtext, ENDLNSTR, DELIMEND): "
				   + e.ToString();
	  AddInfoMsg(SYNTAXERR, ERRMSG, 0);
	  AddInfoMsg(msg.c_str(), ERRMSG, 0);
	}

  if (debug_eli)
    WriteELIDebug(L"PrepareScript", L"[END]");
}
//-------------------------------------------------------------------------------

bool ELI::TranslateLine(const wchar_t *line, UINT index)
{
  wchar_t str[CHARSIZE];

  wcscpy(str, _wltrim(line));

  if (debug_eli)
    {
      WriteELIDebug(L"-------------------------------------------------", L"");
	  WriteELIDebug(L"TranslateLine", str);
      WriteELIDebug(L"-------------------------------------------------", L"");
	}

//проверим, какой тип действия содержится в строке
  if ((str[0] == COMSYM) && (str[1] == COMSYM)) //комментарий в строке
	return true;

//если строка не закоментированна - инициализируем конст. строки и выполним действия
  if (InterpreterSettings.ParseSymConst)
	wcscpy(str, ParseConstStrings(str));

  if (InterpreterSettings.ParseNumConst)
	wcscpy(str, ParseConstNumbers(str));

  wcscpy(str, RemoveSpaces(str));

  if (VARSYM == str[0]) //производится операция с переменной
    {
      if (!ParseRefsInExp(str, index))
        return false;

      if (1 == _wstrccount(str, '=')) //операция присваивания
        {
          wchar_t lvname[MAXNAMELEN];
          wchar_t rexpr[CHARSIZE];

          UINT eqpos = _wstrcpos(str, '=') + 1;
          wchar_t *eqptr = _wsetpstr(str, eqpos);

          _wstrcpywc(str, lvname, '='); //пишем имя переменной
          wcscpy(rexpr, eqptr); //пишем правую часть выражения

          wchar_t defvalue[CHARSIZE];

          if (!ParseObjectsInExp(rexpr, index)) //если переменная инициализируется объектом
            {
              AddInfoMsg(INITERR, ERRMSG, index);

              return false;
            }

          if (!ParseFuncsInExp(rexpr, index)) //если переменная инициализируется ф-ей
            {
              AddInfoMsg(INITERR, ERRMSG, index);

			  return false;
            }

//если после "=" идет num/sym - это объявление переменной
          if (_wstrincl(rexpr, NUMTYPE, 0)) //если в начале rexpr идет признак числового типа
            {
//копируем в defvalue все, что после названия типа
              int pos = wcslen(NUMTYPE), cnt = wcslen(rexpr) - pos;
              _wstrncopy(rexpr, defvalue, pos, cnt);

              if (0 == _wcsicmp(defvalue, L""))
                wcscpy(defvalue, L"0.000");

              if (!ParseVarInExp(defvalue, index))
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              if (!ParseObjectsInExp(defvalue, index))
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              if (!ParseFuncsInExp(defvalue, index))
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              float *defval = CalcExpNum(defvalue, index);

              if (defval)
                swprintf(defvalue, FRMTNUM, *defval);
              else
                {
                  AddInfoMsg(NUMERR, ERRMSG, index);

                  return false;
                }

              return VarInit(lvname, SCNUM, defvalue, index);
            }
          else if (_wstrincl(rexpr, STRTYPE, 0)) //если в начале rexpr идет признак строкового типа
            {
//копируем в defvalue все, что после названия типа
              int pos = wcslen(STRTYPE), cnt = wcslen(rexpr) - pos;
              _wstrncopy(rexpr, defvalue, pos, cnt);

              if (!ParseVarInExp(defvalue, index)) //на случай, если переменная инициализируется другой переменной
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              if (!ParseObjectsInExp(defvalue, index))
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              if (!ParseFuncsInExp(defvalue, index))
                {
                  AddInfoMsg(INITERR, ERRMSG, index);

                  return false;
                }

              return VarInit(lvname, SCSTR, defvalue, index);
            }

//если после = не указан тип данных - это операция присваивания
          VARIABLE *var = st->Get(lvname); //получаем переменную

          if (!var)
            {
//если правую часть можно привести к числовому типу - объявляем числовую переменную
			  if (CalcExpNum(rexpr, index))
                {
                  if (!VarInit(lvname, SCNUM, defvalue, index))
                    return false;
                }
			  else if (CalcExpStr(rexpr, index) != ERROUT)//иначе - строковую
                {
				  if (!VarInit(lvname, SCSTR, defvalue, index))
                    return false;
				}
			  else
				{
				  return false;
                }
            }

		  var = st->Get(lvname);

//пр. часть начинается с символа операции - это изменение значения переменной
//$v = ++2 -> $v = $v + 2
          if (_wstrincl(rexpr, L"++", 0) ||
              _wstrincl(rexpr, L"--", 0))
            {
              wchar_t tmp[CHARSIZE];
			  std::wstring s = std::wstring(rexpr).erase(0, 1);
              wcscpy(rexpr, s.c_str());
              swprintf(tmp, L"%s%s", lvname, rexpr);
              wcscpy(rexpr, tmp);
            }
          else if (_wstrincl(rexpr, L"*", 0) ||
                   _wstrincl(rexpr, L"/", 0))
            {
              wchar_t tmp[CHARSIZE];
              swprintf(tmp, L"%s%s", lvname, rexpr);
              wcscpy(rexpr, tmp);
            }

          if (SCNUM == var->type) //получаем тип данных lvalue-части выражения
            {
              float *nres = CalcExpNum(rexpr, index); //пытаемся получить результат из rexpr

              if (nres)
                {
                  st->SetNumElement(var, *nres);

                  return true;
                }
              else //не удалось рассчитать rexpr
                {
                  AddInfoMsg(NUMERR, ERRMSG, index);

                  return false;
                }
            }
          else if (SCSTR == var->type)
            {
              const wchar_t *sres = CalcExpStr(rexpr, index); //пытаемся получить результат из rexpr

              if (0 != _wcsicmp(sres, ERROUT))
                {
                  wchar_t sval[CHARSIZE];
                  wcscpy(sval, sres);

                  st->SetStrElement(var, sval);

                  return true;
                }
              else //не удалось рассчитать rexpr
                {
				  AddInfoMsg(STRERR, ERRMSG, index);

                  return false;
                }
            }
        }
      else if (_wstrccount(str, '=') > 1) //больше одного =, либо операция сравнения (т.е. бессмысленная), либо ошибка
        {
          AddInfoMsg(SYNTAXERR, ERRMSG, index);

          return false;
        }
      else if (0 == _wstrccount(str, '=')) //нет =, возможно ++  или --
        {
          if (2 == _wstrccount(str, '+'))
            {
               wchar_t lvname[MAXNAMELEN];

               _wstrcpywc(str, lvname, '+'); //пишем имя переменной
               VARIABLE *var = st->Get(lvname); //получаем переменную

               if (!var) //переменная не найдена - ошибка
                 {
                   AddInfoMsg(UNKVARNAME, ERRMSG, index);

                   return false;
                 }

               if (SCNUM == var->type) //получаем тип данных lvalue-части выражения
                 {
                   st->SetNumElement(var, st->GetNumElement(var) + 1);

                   return true;
                 }
               else
                 {
                   AddInfoMsg(ILLGLOPER, ERRMSG, index);

                   return false;
                 }
            }
          else if (2 == _wstrccount(str, '-'))
            {
               wchar_t lvname[MAXNAMELEN];

               _wstrcpywc(str, lvname, '-'); //пишем имя переменной
               VARIABLE *var = st->Get(lvname); //получаем переменную

               if (!var) //переменная не найдена - ошибка
                 {
                   AddInfoMsg(UNKVARNAME, ERRMSG, index);

                   return false;
                 }

               if (SCNUM == var->type) //получаем тип данных lvalue-части выражения
                 {
                   st->SetNumElement(var, st->GetNumElement(var) - 1);

                   return true;
                 }
               else
                 {
                   AddInfoMsg(ILLGLOPER, ERRMSG, index);

                   return false;
                 }
            }
          else
            {
              AddInfoMsg(SYNTAXERR, ERRMSG, index);

              return false;
            }
        }
    }
  else if (REFSYM == str[0]) //операция со ссылкой
    {
      if (1 == _wstrccount(str, '=')) //операция перегрузки (линковки)
        {
          wchar_t lvname[MAXNAMELEN];
          wchar_t rexpr[CHARSIZE];

          _wstrcpywc(str, lvname, '='); //пишем имя переменной
          UINT eqpos = _wstrcpos(str, '=') + 1;
          wchar_t *eqptr = _wsetpstr(str, eqpos);
          wcscpy(rexpr, eqptr); //пишем правую часть выражения

          return AddRef(lvname, rexpr);
        }
      else
        {
          AddInfoMsg(SYNTAXERR, ERRMSG, index);

          return false;
        }
    }
  else if (FUNCSYM == str[0]) //строка начинается с символа функции, исп-я функция без возвращаемого значения
    {
      if (!ParseRefsInExp(str, index))
        return false;

      return ParseFuncsInExp(str, index);
    }
  else if (OBJSYM == str[0]) //строка начинается с символа объекта, исп. метод не возвр. значения или присваивание значения переменной
    {
      if (!ParseRefsInExp(str, index))
        return false;

      if (1 == _wstrccount(str, '=')) //операция присваивания
        {
          wchar_t lvname[MAXNAMELEN], oname[MAXNAMELEN], prname[MAXNAMELEN];
          wchar_t rexpr[CHARSIZE];

          UINT eqpos = _wstrcpos(str, '=') + 1;
          wchar_t *eqptr = _wsetpstr(str, eqpos);

          _wstrcpywc(str, lvname, '='); //пишем имя объекта вместе со свойством

		  if (0 == _wstrccount(str, OBJPROPSEP))
			{
              AddInfoMsg(ILLGLOBJOPER, ERRMSG, index);

			  return false;
            }

		  if (_wstrccount(lvname, OBJPROPSEP) > 1) //больше одной точки - в строке свойство-объект
            {
			  if (!ParseIncObjects(lvname, index))
			  	return false;
			}

          wcscpy(rexpr, eqptr); //пишем правую часть выражения

          if (_wstrccount(lvname, '(') > 0) //в левой части находится метод - недопустимо
            {
              AddInfoMsg(ILLGLOBJOPER, ERRMSG, index);

              return false;
            }

//выделяем из левой части выражения имя объекта и имя свойства
		  UINT dpos = _wstrcpos(lvname, OBJPROPSEP) + 1;
          wchar_t *dptr = _wsetpstr(lvname, dpos);

          _wstrcpywc(lvname, oname, OBJPROPSEP); //пишем имя объекта
          wcscpy(prname, dptr); //пишем имя свойства

          if (wcslen(prname) == 0)
            {
              AddInfoMsg(OBJNOPROP, ERRMSG, index);

              return false;
            }

          if (!ParseVarInExp(oname, index))
            {
              AddInfoMsg(OBJNOCRTNAME, ERRMSG, index);

              return false;
            }

          if (!ParseVarInExp(prname, index))
            {
              AddInfoMsg(OBJNOCRTPROP, ERRMSG, index);

              return false;
            }

		  RESRECORDSET rs = objStack->Get(obj_id, std::wstring(oname));

          if (rs.size() == 0)
            {
              AddInfoMsg(OBJNONE, ERRMSG, index);

              return false;
            }

          rs.clear();

		  rs = objStack->Get(std::wstring(oname), std::wstring(prname));

          if (rs.size() > 0) //если объект найден
            {
              if (IsClassMember(rs[0]->ObjectCathegory.c_str(), prname))
                {
                  if (!IsAccessibleMember(oname, prname))
                    {
                      AddInfoMsg(OBJMEMNOTACC, ERRMSG, index);

                      return false;
                    }
                }

//пр. часть начинается с символа операции - это изменение значения свойства
//&v.pr = ++2 -> &v.pr = &v.pr + 2
              if (_wstrincl(rexpr, L"++", 0) ||
                  _wstrincl(rexpr, L"--", 0))
                {
				  wchar_t tmp[CHARSIZE];
				  std::wstring s = std::wstring(rexpr).erase(0, 1);
                  wcscpy(rexpr, s.c_str());
                  swprintf(tmp, L"%s.%s%s", oname, prname, rexpr);
                  wcscpy(rexpr, tmp);
                }
              else if (_wstrincl(rexpr, L"*", 0) ||
                       _wstrincl(rexpr, L"/", 0))
                {
                  wchar_t tmp[CHARSIZE];
                  swprintf(tmp, L"%s.%s%s", oname, prname, rexpr);
                  wcscpy(rexpr, tmp);
                }

              wchar_t tmp[NUMSIZE];
              float *nres = CalcExpNum(rexpr, index);

              if (nres)
                {
                  swprintf(tmp, FRMTNUM, *nres);
                  rs[0]->Value = tmp;

                  return true;
                }
              else
                {
                  const wchar_t *sres = CalcExpStr(rexpr, index);

                  if (0 == _wcsicmp(sres, ERROUT))
                    {
                      AddInfoMsg(STRERR, ERRMSG, index);

                      return false;
                    }

                  rs[0]->Value = sres;

                  return true;
                }
            }
          else
            {
              AddInfoMsg(OBJNOPROP, ERRMSG, index);

              return false;
            }
        }
      else if (_wstrccount(str, '=') > 1) //больше одного =, либо сравнение либо ошибка
        {
          AddInfoMsg(SYNTAXERR, ERRMSG, index);

          return false;
        }
      else //нет присваивание - исп. метод, например Create()
        {
          wchar_t result[2];

		  if (_wstrccount(str, OBJPROPSEP) > 1) //больше одной точки - в строке свойство-объект
			{
			  ParseIncObjects(str, index);
            }

          return WorkWithObject(str, result, index);
        }
    }
  else if (PROCSYM == str[0]) //в строке исп. запуск процедуры
    {
      wchar_t procname[MAXNAMELEN], procprm[CHARSIZE];

      if (!ParseRefsInExp(str, index))
        return false;

      if (!ParseFuncsInExp(str, index))
        return false;

      if (!ParseObjectsInExp(str, index))
        return false;

      if (!ParseVarInExp(str, index))
        return false;

      _wstrcpywc(str + 1, procname, '(');
      int pos = _wstrcpos(str, '(') + 1;
      _wstrcpywc(_wsetpstr(str, pos), procprm, ')');

      return RunProcedure(procname, procprm, index);
    }
  else if ('#' == str[0]) //у рядку міститься директива
	{
	  return ProcessDirective(str, index);
    }
  else if (_wstrincl(str, L"when", 0))
	{
	  return ExpWhen(str, index);
	}
  else if (_wstrincl(str, L"if", 0)) //условие if
	{
	  return ExpIf(str, index);
	}
  else if (_wstrincl(str, L"elseif", 0)) //условие else if
	{
	  return ExpElseIf(str, index);
    }
  else if (_wstrincl(str, L"else", 0)) //секция else
    {
      return ExpElse(str, index);
    }
  else if (_wstrincl(str, L"for", 0)) //цикл for
    {
      return ExpFor(str, index);
    }
  else if (_wstrincl(str, L"count", 0)) //цикл count
    {
      return ExpCount(str, index);
    }
  else if (_wstrincl(str, L"while", 0)) //цикл while
    {
      return ExpWhile(str, index);
    }
  else if (_wstrincl(str, L"select", 0)) //конструкция выбора select
    {
      return ExpSelect(str, index);
    }
  else //в строке находится ошибка
    {
      AddInfoMsg(SYNTAXERR, ERRMSG, index);

      return false;
    }

  AddInfoMsg(UNKERR, ERRMSG, index);

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::TranslateScriptLines()
{
  if (debug_eli)
	WriteELIDebug(L"TranslateScriptLines", L"[START]");

  try
	 {
	   wchar_t log[CHARSIZE];
	   st = vecVSt[0]; //ставим указатель на главный стек переменных
	   vecScList[0] = _wltrim(vecScList[0].c_str());

	   if (!_wstrincl(vecScList[0].c_str(), L"#begin", 0))
		 throw Exception(NOBEGIN);

	   if (0 != _wcsicmp(RemoveSpaces(vecScList[vecScList.size() - 1].c_str()), L"#end"))
		 throw Exception(NOEND);

	   if (vecScList[0].substr(6, vecScList[0].length() - 6) == L"")
		 throw Exception(NONAME);

	   wchar_t val[MAXNAMELEN];

	   wcscpy(val, RemoveSpaces(vecScList[0].substr(6, vecScList[0].length() - 6).c_str()));

	   if (!IsCorrectName(val))
		 throw Exception(ERRNAME);

	   pStack->Add(P_SCRNAME, val);

	   if (write_log)
		 {
		   String msg = L"Starting script [" + String(val) + L"]";
		   WriteLog(msg.c_str());
		 }

	   for (UINT index = 1; index < vecScList.size() - 1; index++) //скомпилим все строки кроме первой и последней
		  {
			if (!TranslateLine(vecScList[index].c_str(), index))
			  {
				if (write_log)
				  {
					swprintf(log, L"%s [%d] %s", TRFAIL, index, _wltrim(vecScList[index].c_str()));
					WriteLog(log);
					swprintf(log, L"%s [%d] %s", TRINFO, index, L"Stoping");
					WriteLog(log);
				  }

				throw Exception(SCSTP);
			  }
			else
			  {
				if (write_log)
				  {
					swprintf(log, L"%s [%d] %s", TROK, index, _wltrim(vecScList[index].c_str()));
					WriteLog(log);
				  }

				CheckTriggers();
			  }
          }
	 }
  catch (Exception &e)
	 {
       if (debug_eli)
		 {
		   WriteELIDebug(L"TranslateScriptLines", e.ToString().c_str());
		   WriteELIDebug(L"TranslateScriptLines", L"[FAIL]");
		 }

	   return false;
     }

  if (debug_eli)
	WriteELIDebug(L"TranslateScriptLines", L"[OK]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::TranslateFragment(SCRIPTLINES *vecFragment, UINT index)
{
  if (debug_eli)
	WriteELIDebug(L"TranslateFragment", L"[START]");

  try
	 {
	   if (!vecFragment)
		 throw Exception("Invalid fragment container");

	   if (vecFragment->empty())
		 throw Exception("Empty fragment container");

	   wchar_t log[CHARSIZE];

	   for (UINT ind = 0; ind < vecFragment->size(); ind++)
		  {
			if (!TranslateLine(vecFragment->at(ind).c_str(), index))
			  {
				if (write_log)
				  {
					AddInfoMsg(FRGMNTERR, ERRMSG, index);
					swprintf(log, L"%s [%d] %s", TRFAIL, index, _wltrim(vecFragment->at(ind).c_str()));
					WriteLog(log);
				  }

				throw Exception(FRGMNTERR);
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   if (debug_eli)
		 {
		   WriteELIDebug(L"TranslateFragment", e.ToString().c_str());
		   WriteELIDebug(L"TranslateFragment", L"[FAIL]");
		 }

	   return false;
	 }

  if (debug_eli)
    WriteELIDebug(L"TranslateFragment", L"[OK]");

  return true;
}
//-------------------------------------------------------------------------------

std::wstring ELI::MarkFragments(std::wstring &operstr)
{
  if (debug_eli)
    WriteELIDebug(L"MarkFragments", L"[START]");

  std::wstring fragmentstr, fm;
  wchar_t fmark[32];
  UINT oppos, clpos;

  do
    {
	  fragmentstr = operstr;
	  oppos = fragmentstr.find_last_of(L"{");
	  fragmentstr.erase(0, oppos);
	  clpos = fragmentstr.find_first_of(L"}");
	  fragmentstr.erase(clpos + 1, fragmentstr.length() - clpos - 1);
      swprintf(fmark, FRGMNTF, FrgmntNum);
	  operstr = ParseStringW(operstr, fragmentstr, std::wstring(fmark));

	  fragmentstr.erase(0, 1);
	  fragmentstr.erase(fragmentstr.length() - 1, 1);
	  fm = fmark;
	  fm.erase(fm.length() - 1, 1);
	  frgStack->Add(fragmentstr, fm, false);
	  FrgmntNum++;
	}
  while (operstr.find(L"{") != std::wstring::npos);

  if (debug_eli)
    WriteELIDebug(L"MarkFragments", L"[END]");

  return operstr;
}
//-------------------------------------------------------------------------------

void ELI::SearchAndMarkGlobalFragments()
{
  try
	 {
	   RESRECORDSET rs;

	   for (int i = 0; i < frgStack->Count; i++)
		  {
			if (frgStack->Fragments[i]->IsGlobal())
			  continue;
			else
			  {
				RESRECORDSET rs;

				std::wstring mark = frgStack->Fragments[i]->GetMark();
				rs = procStack->Get(val_v, mark);

				if (rs.size() > 0) //якщо мітка знайшлась
				  MarkGlobalFragment(frgStack->Fragments[i]);
			  }
		  }
	 }
  catch (Exception &e)
     {
	   if (debug_eli)
		 WriteELIDebug(L"SearchAndMarkGlobalFragments", String("Exception: " + e.ToString()).c_str());
	 }
}
//-------------------------------------------------------------------------------

void ELI::MarkGlobalFragment(FRAGMENTCODE *frg)
{
  try
	 {
//позначимо фрагмент як глобальний
	   frg->SetGlobal();

//пройдемося по всім міткам фрагментів, які прив'язані до неї
//і позначимо їх як глобальні
	   std::wstring frgtext = RemoveEndlines(frg->GetString());

	   while (frgtext.find(FRGMARK) != std::wstring::npos)
		 {
		   UINT pos = frgtext.find(FRGMARK);
		   std::wstring mark = frgtext.substr(pos, frgtext.length() - pos);

		   FRAGMENTCODE *child_frg = frgStack->Get(mark);

		   if (child_frg)
			 MarkGlobalFragment(child_frg);

		   frgtext = ParseStringW(frgtext, mark, L"~");
		 }
	 }
  catch (Exception &e)
     {
	   if (debug_eli)
		 WriteELIDebug(L"MarkGlobalFragment", String("Exception: " + e.ToString()).c_str());
	 }
}
//-------------------------------------------------------------------------------

int ELI::ExpTrue(std::wstring exp, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpTrue", L"[START]");
      WriteELIDebug(L"ExpTrue", exp.c_str());
    }

  std::wstring lval, rval;
  wchar_t tmp[CHARSIZE];
  float *ptlres, lres;
  float *ptrres, rres;
  UINT pos, cnt, term;

//проверим условие на истинность
  if (exp.find(L"==") != std::wstring::npos)
    {
	  pos = exp.find(L"==");
      term = 0;
      cnt = 2;
    }
  else if (exp.find(L"!=") != std::wstring::npos)
    {
	  pos = exp.find(L"!=");
      term = 1;
      cnt = 2;
    }
  else if (exp.find(L">=") != std::wstring::npos)
    {
	  pos = exp.find(L">=");
      term = 2;
      cnt = 2;
	}
  else if (exp.find(L"<=") != std::wstring::npos)
    {
	  pos = exp.find(L"<=");
      term = 3;
      cnt = 2;
    }
  else if (exp.find(L">") != std::wstring::npos)
    {
	  pos = exp.find(L">");
      term = 4;
      cnt = 1;
    }
  else if (exp.find(L"<") != std::wstring::npos)
    {
	  pos = exp.find(L"<");
	  term = 5;
      cnt = 1;
	}
  else if (_wstrincl(exp.c_str(), L"!", 0))
	{
	  exp.erase(0, 1);
      pos = exp.length();
	  term = 6;
	}
  else //нет символа сравнения, выражение проверяется на истинность
    {
	  pos = exp.length();
	  term = 7;
    }

//вычислим левую и правую часть выражения
  lval = exp.substr(0, pos);
  wcscpy(tmp, lval.c_str());
  ptlres = CalcExpNum(tmp, index);

  if (ptlres)
    lres = *ptlres;
  else
    return -1;

  if (term < 6)
	{
      rval = exp.substr(pos + cnt, exp.length() - pos - cnt);
	  wcscpy(tmp, rval.c_str());
	  ptrres = CalcExpNum(tmp, index);

      if (ptrres)
		rres = *ptrres;
	  else
		return -1;
	}

   switch (term)
      {
        case 0:
              {
                if (lres == rres)
                  return 1;
                else
				  return 0;
			  }

        case 1:
              {
                if (lres != rres)
                  return 1;
                else
				  return 0;
			  }

        case 2:
              {
                if (lres >= rres)
                  return 1;
                else
				  return 0;
			  }

        case 3:
              {
                if (lres <= rres)
                  return 1;
                else
				  return 0;
			  }

        case 4:
              {
                if (lres > rres)
                  return 1;
                else
                  return 0;
			  }

		case 5:
			  {
				if (lres < rres)
				  return 1;
				else
				  return 0;
			  }

		case 6:
			  {
				if (lres <= 0)
				  return 1;
				else
				  return 0;
			  }

		case 7:
			  {
				if (lres > 0)
				  return 1;
				else
				  return 0;
			  }

        default: return -1;
      }
}
//-------------------------------------------------------------------------------

bool ELI::ExpIf(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpIf", L"[START]");
      WriteELIDebug(L"ExpIf", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово if и обе скобки
  term.erase(0, 3);
  term.erase(term.length() - 1, 1);
  int res = ExpTrue(term, index);

  if (1 == res)
    {
      use_false = false;

      SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

      if (code)
        {
		  if (!TranslateFragment(code, index))
			return false;
        }
	  else
        {
          AddInfoMsg(FRGMNTERR, ERRMSG, index);

          return false;
        }

      return true;
    }
  else if (0 == res) //условие - ложно, секция else
    {
      use_false = true;

      return true;
    }
  else if (-1 == res)
    {
      AddInfoMsg(UNKERR, ERRMSG, index);

      return false;
    }

  AddInfoMsg(UNKERR, ERRMSG, index);

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::ExpElseIf(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpElseIf", L"[START]");
      WriteELIDebug(L"ExpElseIf", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово else if и обе скобки
  term.erase(0, 7);
  term.erase(term.length() - 1, 1);

  if (!use_false)
	return true;

  int res = ExpTrue(term, index);

  if (1 == res)
	{
      use_false = false;

      SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

	  if (code)
        {
		  if (!TranslateFragment(code, index))
            return false;
        }
      else
        {
          AddInfoMsg(FRGMNTERR, ERRMSG, index);

          return false;
        }

      return true;
    }
  else if (0 == res) //условие - ложно, секция else
    {
      use_false = true;

      return true;
    }
  else if (-1 == res)
    {
      AddInfoMsg(UNKERR, ERRMSG, index);

      return false;
    }

  AddInfoMsg(UNKERR, ERRMSG, index);

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::ExpElse(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpElse", L"[START]");
      WriteELIDebug(L"ExpElse", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово else
  term.erase(0, 4);

  if (use_false)
    {
      use_false = false;

      SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

      if (code)
        {
		  if (!TranslateFragment(code, index))
            return false;
          else
            return true;
        }
      else
        {
          AddInfoMsg(FRGMNTERR, ERRMSG, index);

          return false;
        }
    }
  else
    return true;
}
//-------------------------------------------------------------------------------

bool ELI::ExpFor(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpFor", L"[START]");
      WriteELIDebug(L"ExpFor", line);
    }

  std::wstring term = line, mark;
  wchar_t exp[CHARSIZE];
  float from, to;
  float *ptrfrom;
  float *ptrto;
  float *ptrtag;
  bool increment = true; //увеличивать или уменьшать счетчик
  UINT tag = 0; //шаг изменения счетчика
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово for и обе скобки
  term.erase(0, 4);
  term.erase(term.length() - 1, 1);

  SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

  if (!code)
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }

//разобьем выражение, чтобы определить условия выполнения цикла
  std::vector<std::wstring> vecTerms;
  StrToListW(&vecTerms, term, CYCLESEP, NODELIMEND);

  if (vecTerms.size() != 3)
    {
      AddInfoMsg(SYNTAXERR, ERRMSG, index);

      return false;
    }

//определим границы срабатывания цикла
  wcscpy(exp, vecTerms[0].c_str());
  ptrfrom = CalcExpNum(exp, index);

  if (!ptrfrom)
	return false;
  else
	from = *ptrfrom;

  if (vecTerms[2].c_str()[0] == '+')
	increment = true;
  else if (vecTerms[2].c_str()[0] == '-')
	increment = false;

  vecTerms[2].erase(0, 1);
  wcscpy(exp, vecTerms[2].c_str());
  ptrtag = CalcExpNum(exp, index);

  if (!ptrtag)
	return false;
  else
	tag = static_cast<int>(*ptrtag);

  if (vecTerms[1].find(L"==") != std::wstring::npos)
    {
	  pos = vecTerms[1].find(L"==");
	  wcscpy(exp, vecTerms[1].substr(pos + 2, vecTerms[1].length() - pos - 2).c_str());
      ptrto = CalcExpNum(exp, index);

      if (!ptrto)
        return false;
      else
        to = *ptrto;

      if (increment)
        {
          for (int ind = from; ind == to; ind = ind + tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"+" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }

        }
      else
        {
          for (int ind = from; ind == to; ind = ind - tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
				{
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"-" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
    }
  else if (vecTerms[1].find(L">=") != std::wstring::npos)
    {
	  pos = vecTerms[1].find(L">=");
	  wcscpy(exp, vecTerms[1].substr(pos + 2, vecTerms[1].length() - pos - 2).c_str());
      ptrto = CalcExpNum(exp, index);

      if (!ptrto)
        return false;
      else
        to = *ptrto;

      if (increment)
        {
          for (int ind = from; ind >= to; ind = ind + tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
				{
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"+" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
      else
        {
          for (int ind = from; ind >= to; ind = ind - tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"-" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
    }
  else if (vecTerms[1].find(L"<=") != std::wstring::npos)
    {
	  pos = vecTerms[1].find(L"<=");
	  wcscpy(exp, vecTerms[1].substr(pos + 2, vecTerms[1].length() - pos - 2).c_str());
      ptrto = CalcExpNum(exp, index);

      if (!ptrto)
        return false;
	  else
        to = *ptrto;

      if (increment)
        {
          for (int ind = from; ind <= to; ind = ind + tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"+" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
      else
        {
          for (int ind = from; ind <= to; ind = ind - tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
				  swprintf(strtag, L"%d", tag);
				  varline += L"-" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
			}
        }
    }
  else if (vecTerms[1].find(L">") != std::wstring::npos)
	{
	  pos = vecTerms[1].find(L">");
	  wcscpy(exp, vecTerms[1].substr(pos + 1, vecTerms[1].length() - pos - 1).c_str());
      ptrto = CalcExpNum(exp, index);

      if (!ptrto)
        return false;
      else
        to = *ptrto;

      if (increment)
        {
          for (int ind = from; ind > to; ind = ind + tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"+" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
      else
        {
          for (int ind = from; ind > to; ind = ind - tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"-" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
    }
  else if (vecTerms[1].find(L"<") != std::wstring::npos)
    {
	  pos = vecTerms[1].find(L"<");
	  wcscpy(exp, vecTerms[1].substr(pos + 1, vecTerms[1].length() - pos - 1).c_str());
      ptrto = CalcExpNum(exp, index);

      if (!ptrto)
        return false;
      else
        to = *ptrto;

      if (increment)
        {
          for (int ind = from; ind < to; ind = ind + tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"+" + std::wstring(strtag);

				  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
      else
        {
          for (int ind = from; ind < to; ind = ind - tag)
            {
			  if (!TranslateFragment(code, index))
                return false;

//если в качестве счетчика переменная - увеличим ее значение
			  if (vecTerms[0].find(VARSYM) != std::wstring::npos)
                {
				  std::wstring varline = vecTerms[0] + L"=" + vecTerms[0];
                  wchar_t strtag[NUMSIZE];
                  swprintf(strtag, L"%d", tag);
				  varline += L"-" + std::wstring(strtag);

                  if (!TranslateLine(varline.c_str(), index))
                    return false;
                }
            }
        }
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ExpCount(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpCount", L"[START]");
      WriteELIDebug(L"ExpCount", line);
    }

  std::wstring term = line, mark;
  wchar_t exp[CHARSIZE];
  float to;
  float *ptrto;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово count и обе скобки
  term.erase(0, 6);
  term.erase(term.length() - 1, 1);

  SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

  if (!code)
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }

  wcscpy(exp, term.c_str());
  ptrto = CalcExpNum(exp, index);

  if (!ptrto)
    return false;
  else
    to = *ptrto;

  if (to < 0)
    {
      AddInfoMsg(COUNTERR, ERRMSG, index);

      return false;
    }

  for (UINT ind = 0; ind < to; ind++)
    {
	  if (!TranslateFragment(code, index))
        return false;
    }

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::ExpWhile(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpWhile", L"[START]");
      WriteELIDebug(L"ExpWhile", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово while и обе скобки
  term.erase(0, 6);
  term.erase(term.length() - 1, 1);

  SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

  if (code)
    {
      while (1 == ExpTrue(term, index))
        {
		  if (!TranslateFragment(code, index))
            return false;
        }

      return true;
    }
  else
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }
}
//-------------------------------------------------------------------------------

bool ELI::ExpSelect(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpSelect", L"[START]");
      WriteELIDebug(L"ExpSelect", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово select и обе скобки
  term.erase(0, 7);
  term.erase(term.length() - 1, 1);

  SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

  if (code)
    {
      wchar_t str[CHARSIZE];

      wcscpy(str, term.c_str());

//определим тип параметра и приведем его к константному значению
      switch (str[0])
        {
          case VARSYM:
              {
                if (!ParseVarInExp(str, index))
                  return false;

                break;
              }
          case FUNCSYM:
              {
                if (!ParseFuncsInExp(str, index))
                  return false;

                break;
              }
          case OBJSYM:
              {
                if (!ParseObjectsInExp(str, index))
                  return false;

                break;
              }
          default:
              {
                AddInfoMsg(SELPRMERR, WRNMSG, index);
              }
        }

//создадим параметр в стеке для конструкции when
      pStack->Add(P_SELECT, str);

	  if (TranslateFragment(code, index))
        return true;
      else
        return false;
    }
  else
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }
}
//-------------------------------------------------------------------------------

bool ELI::ExpWhen(wchar_t *line, UINT index)
{
  if (debug_eli)
    {
      WriteELIDebug(L"ExpWhen", L"[START]");
      WriteELIDebug(L"ExpWhen", line);
    }

  std::wstring term = line, mark;
  UINT pos = term.find(FRGMARK);
  mark = term.substr(pos, term.length() - pos);
  term.erase(pos, term.length() - pos);
//уберем слово when и then, получим константное значение для сравнения
  term.erase(0, 4);

  pos = term.find(L"then");

  if (pos == std::wstring::npos)
    {
      AddInfoMsg(SYNTAXERR, ERRMSG, index);
      AddInfoMsg(NOTHENERR, ERRMSG, index);

      return false;
    }

  term.erase(term.length() - 4, 4);

  if (term == L"")
    {
      AddInfoMsg(SYNTAXERR, ERRMSG, index);
      AddInfoMsg(NOWHENPRM, ERRMSG, index);

      return false;
    }

  SCRIPTLINES *code = frgStack->GetFragmentCode(mark);

  if (code)
    {
      if (term == pStack->Get(P_SELECT)->ToStr())
        {
		  if (!TranslateFragment(code, index))
            return false;
        }

      return true;
    }
  else
    {
      AddInfoMsg(FRGMNTERR, ERRMSG, index);

      return false;
    }
}
//-------------------------------------------------------------------------------

bool ELI::VarInit(wchar_t *name, UINT type, wchar_t* defvalue, UINT index)
{
  if (debug_eli)
    {
	  WriteELIDebug(L"VarInit", L"[START]");
	  String str = String(name) + " = " + String(defvalue);
      WriteELIDebug(L"VarInit", str.c_str());
    }

  VARIABLE *pt;

  pt = st->Get(name);

  if (pt) //переменная есть в стеке - присвоим новое значение
    {
      if (SCNUM == type)
        {
          float nval = 0;

          try
             {
			   nval = _wtof(defvalue);
             }
		  catch(Exception &e)
             {
			   String msg = "VarInit(), exist var, nval = _wtof(defvalue): "
							+ e.ToString();
			   AddInfoMsg(NUMCONSLERR, ERRMSG, index);
			   AddInfoMsg(msg.c_str(), ERRMSG, index);

               return false;
             }

          st->SetNumElement(pt, nval);

          return true;
        }
      else if (SCSTR == type)
        {
          st->SetStrElement(pt, defvalue);

          return true;
        }

      return false;
    }

  if (!IsCorrectVarName(name))
    {
      AddInfoMsg(VARNAMEERR, ERRMSG, index);

      return false;
    }

  if (SCNUM == type)
    {
      float nval = 0;

      if (defvalue)
        {
          try
             {
               nval = _wtof(defvalue);
             }
		  catch(Exception &e)
             {
			   String msg = "VarInit(), new var, nval = _wtof(defvalue): "
							+ e.ToString();
			   AddInfoMsg(NUMCONSLERR, ERRMSG, index);
               AddInfoMsg(msg.c_str(), ERRMSG, index);

               return false;
             }
        }

      if (!st->Add(name, nval))
        {
          AddInfoMsg(INITERR, ERRMSG, index);

          return false;
        }
      else
        return true;
    }
  else if (SCSTR == type)
    {
	  std::wstring sval = L"";

      if (defvalue)
        {
          try
            {
              sval = defvalue;
            }
		  catch(Exception &e)
            {
              String msg = "VarInit(), new var, sval = defvalue: "
							+ e.ToString();
			  AddInfoMsg(STRCONSLERR, ERRMSG, index);
              AddInfoMsg(e.ToString().c_str(), ERRMSG, index);

              return false;
            }
        }

      if (!st->Add(name, sval))
        {
          AddInfoMsg(INITERR, ERRMSG, index);

          return false;
        }
      else
        return true;
    }

  AddInfoMsg(UNKERR, ERRMSG, index);

  if (debug_eli)
    WriteELIDebug(L"VarInit", L"[END]");

  return false;
}
//-------------------------------------------------------------------------------

void ELI::InitTranslatorFuncs()
{
  if (debug_eli)
    WriteELIDebug(L"InitTranslatorFuncs", L"[START]");

  fStack->Add(L"_random", L"num pArea", &scRandom);
  fStack->Add(L"_round", L"num pNumber,num pPrecision", &scRound);
  fStack->Add(L"_int", L"num pNumber", &scInt);
  fStack->Add(L"_strlen", L"sym pStr", &scStrLen);
  fStack->Add(L"_streq", L"sym pStr1,sym pStr2", &scStrEq);
  fStack->Add(L"_istreq", L"sym pStr1,sym pStr2", &scIStrEq);
  fStack->Add(L"_substr", L"sym pTargetStr,num pPos,num pCount", &scSubStr);
  fStack->Add(L"_return", L"sym pReturnVal", &scReturn);
  fStack->Add(L"_throw", L"sym pException", &scThrow);
  fStack->Add(L"_free", L"sym pVarName", &scFree);
  fStack->Add(L"_LoadObjStack", L"sym pFilePath,num pClear", &scLoadObjStack);
  fStack->Add(L"_SaveObjStack", L"sym pFilePath", &scSaveObjStack);
  fStack->Add(L"_SaveObjects", L"sym pFilePath,sym pCathegory", &scSaveObjects);
  fStack->Add(L"_CompactObjStack", L"", &scCompactObjStack);
  fStack->Add(L"_ClearObjStack", L"", &scClearObjStack);
  fStack->Add(L"_RemoveObjects", L"sym pCathegory", &scRemoveObjects);
  fStack->Add(L"_Run", L"sym pVarName", &scRun);
  fStack->Add(L"_GetParamAsNum", L"sym pParam", &scGetParamAsNum);
  fStack->Add(L"_GetParamAsStr", L"sym pParam", &scGetParamAsStr);
  fStack->Add(L"_SetParam", L"sym pParam,sym pValue", &scSetParam);
  fStack->Add(L"_LoadFileToVar", L"sym pFile,sym pTarget", &scLoadFileToVar);
  fStack->Add(L"_SaveVarToFile", L"sym pTarget,sym pFile", &scSaveVarToFile);
  fStack->Add(L"_SaveFragmentToFile", L"sym pTarget,sym pFile", &scSaveFragmentToFile);
  fStack->Add(L"_GetConfig", L"sym pFile,sym pLine", &scGetConfig);
  fStack->Add(L"_SaveState", L"", &scSaveState);
  fStack->Add(L"_SaveVarStack", L"num pLevel", &scSaveVarStack);
  fStack->Add(L"_WriteOut", L"sym pStr", &scWriteOut);
  fStack->Add(L"_ReadIn", L"sym pVar", &scReadIn);
  fStack->Add(L"_System", L"sym pCmd", &scSystem);
  fStack->Add(L"_LastError", L"", &scLastError);
  fStack->Add(L"_ConnectLib", L"sym pPath", &scConnectLib);
  fStack->Add(L"_FreeLib", L"num pHandle", &scFreeLib);
  fStack->Add(L"_ImportFunc", L"num pHandle,sym pExtName,sym pInName,sym pArgList", &scImportFunc);
  fStack->Add(L"_DebugIntoFile", L"sym pFile", &scDebugIntoFile);
  fStack->Add(L"_DebugIntoScreen", L"", &scDebugIntoScreen);
  fStack->Add(L"_StopDebug", L"", &scStopDebug);
  fStack->Add(L"_sleep", L"num pMsec", &scSleep);
  fStack->Add(L"_ShowMessage", L"sym pText", &scShowMessage);

//методы объектов
  fStack->Add(L"Create", L"sym pCathegory,sym pCtorParams", &objCreate);
  fStack->Add(L"Destroy", L"", &objDestroy);
  fStack->Add(L"Add", L"sym pNewPropName,sym pNewPropVal", &objAdd);
  fStack->Add(L"Remove", L"sym pPropName", &objRemove);
  fStack->Add(L"Exist", L"", &objExist);
  fStack->Add(L"Have", L"sym pPropName", &objHave);
  fStack->Add(L"Keep", L"sym pPropName,sym pBool", &objKeep);
  fStack->Add(L"Save", L"sym pPropName,sym pBool", &objSave);
  fStack->Add(L"Execute", L"sym pPropName", &objExecute);
  fStack->Add(L"Show", L"", &objShow);
  fStack->Add(L"Clone", L"sym pSource", &objClone);
  fStack->Add(L"GetName", L"", &objGetName);
  fStack->Add(L"Import", L"sym pSource,sym pPropName", &objImport);

  if (debug_eli)
	WriteELIDebug(L"InitTranslatorFuncs", L"[END]");
}
//-------------------------------------------------------------------------------

void ELI::InitRes(bool init)
{
  if (init)
	{
	  UINT len = wcslen(initdir);

//если стартовая директория - корень диска, удалим последний символ
	  if (initdir[len - 1] == '\\' && initdir[len - 2] == ':')
		wcscpy(initdir, std::wstring(initdir).erase(len - 1, 1).c_str());

//создаем стек объектов
	  objStack = new RESOURCESTACK();
//создаем стек классов
      clStack = new RESOURCESTACK();
//создаем стек процедур
      procStack = new RESOURCESTACK();
//создаем стек временных объектов
      tmpObj = new RESOURCESTACK();
//создаем стек функций
      fStack = new FUNCSTACK();
//создаем стек переменных
      vStack = new VARSTACK();
//создаем стек параметров
      pStack = new PARAMSTACK();
      frgStack = new FRAGMENTSTACK();
//заносим глобальный стек переменных в вектор стеков
      vecVSt.push_back(vStack);
//обнуляем нумерацию фрагментов кода
      FrgmntNum = 0;
	  TmpObjInd = 0;
	  InitTranslatorFuncs();
	  InterpreterSettings.ParseSymConst = true;
	  InterpreterSettings.ParseNumConst = true;
	  InterpreterSettings.KeepObjects = true;
	  InterpreterSettings.KeepClasses = true;
      pStack->Add(P_ELI_VER, GetVersion());
	  pStack->Add(P_ELI_PATH, path);
	  pStack->Add(P_ELI_DIR, initdir);
	  pStack->Add(P_ELI_HANDLE, IntToStr(reinterpret_cast<int>(this)).c_str());
	}
  else
   {
	 FreeRes();
     frgStack->ClearFragments(true);
     delete objStack;
     objStack = NULL;
     delete clStack;
     clStack = NULL;
     delete procStack;
     procStack = NULL;
     delete tmpObj;
     tmpObj = NULL;
     delete fStack;
     fStack = NULL;
     delete vStack;
     vStack = NULL;
     st = NULL;
     delete pStack;
     pStack = NULL;
     delete frgStack;
     frgStack = NULL;
     vecVSt.clear();

     for (UINT i = 0; i < vecLibs.size(); i++)
        FreeLibrary(vecLibs[i]);

     vecLibs.clear();
	 vecExtFuncs.clear();
	 vecRefs.clear();
   }
}
//-------------------------------------------------------------------------------

void ELI::FreeRes()
{
  if (debug_eli)
	WriteELIDebug(L"FreeRes", L"[START]");

  SearchAndMarkGlobalFragments();  //маркуємо всі глобальні фрагменти коду для
                                   //збереження після трансляції скрипту
  vecScList.clear();     		   //очищаем строки скрипта
  frgStack->ClearFragments(false); //очищаем стек фрагментов
  vStack->ClearStack();  		   //очищаем стеки переменных
  InfStack = L"";         		   //очищаем инфостек
  ScriptResult = L"";     		   //очищаем результат
  CstrInd = 0;           		   //обнуляем индекс конст. строк
  CnumInd = 0;                     //обнуляємо індекс числових констант
  scrtext = L"";       	 		   //очищаем текст скрипта
  vecTriggers.clear();
  InterpreterSettings.ParseSymConst = true;
  InterpreterSettings.ParseNumConst = true;
  InterpreterSettings.KeepObjects = true;
  InterpreterSettings.KeepClasses = true;
  use_return = false;
  LastErr = L"<none>";

  if (debug_eli)
    WriteELIDebug(L"FreeRes", L"[END]");
}
//-------------------------------------------------------------------------------

void ELI::SaveELIState()
{
  if (debug_eli)
    WriteELIDebug(L"SaveELIState", L"[START]");

  String path = LogPath + "\\state.log";
  wchar_t timestamp[64];
  swprintf(timestamp, L"%s %s", DateToStr(Date()).c_str(), TimeToStr(Time()).c_str());

  AddToFile(path, "[");
  AddToFile(path, timestamp);
  AddToFile(path, "] Current state of ELI stacks:\r\n\r\n");
  AddToFile(path, "*** Variable stacks ***\r\n");

  wchar_t str[32];

  for (UINT i = 0; i < vecVSt.size(); i++)
    {
      swprintf(str, L"Stack[%d]:\r\n", i);
	  AddToFile(path, str);
	  AddToFile(path, vecVSt[i]->GetString());
    }

  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Parameter stack ***\r\n");
  AddToFile(path, pStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Object stack ***\r\n");
  AddToFile(path, objStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Function stack ***\r\n");
  AddToFile(path, fStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Class stack ***\r\n");
  AddToFile(path, clStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Procedure stack ***\r\n");
  AddToFile(path, procStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n\r\n");
  AddToFile(path, "*** Pretranslated fragments stack ***\r\n");
  AddToFile(path, frgStack->GetString());
  AddToFile(path, "\r\n-----------------------------------------------------\r\n");

  if (debug_eli)
    WriteELIDebug(L"SaveELIState", L"[END]");
}
//-------------------------------------------------------------------------------

void ELI::SaveVStState(UINT level)
{
  String path = LogPath + "\\varstack.log";
  wchar_t timestamp[64];
  swprintf(timestamp, L"%s %s", DateToStr(Date()).c_str(), TimeToStr(Time()).c_str());

  AddToFile(path, "[");
  AddToFile(path, timestamp);
  AddToFile(path, " ]\r\n\r\n");

  if ((level == 0) && vStack)
	{
	  AddToFile(path, "*** Variable stack (global) ****\r\n");
	  AddToFile(path, vStack->GetString());
	  AddToFile(path, "-----------------------------------------------------\r\n\r\n");
	}
  else if ((level == 1) && st)
	{
	  AddToFile(path, "*** Variable stack (local) ***\r\n");
	  AddToFile(path, st->GetString());
	  AddToFile(path, "-----------------------------------------------------\r\n\r\n");
	}
  else
	{
	  AddToFile(path, "Error saving stack!\r\n");
	  AddToFile(path, "-----------------------------------------------------\r\n\r\n");
	}
}
//-------------------------------------------------------------------------------

void ELI::WriteLog(const wchar_t *rec)
{
  SaveLogToUserFolder("translate.log", "ELI", rec);
}
//-------------------------------------------------------------------------------

void ELI::WriteELIDebug(const wchar_t *event, const wchar_t *rec)
{
  String str = String(event) + ": " + String(rec);

  if (debug_in_file)
	SaveLog(debugfile.c_str(), str.c_str());
  else
    wprintf(L"%s\n", str.c_str());
}
//-------------------------------------------------------------------------------

HINSTANCE ELI::LoadExtLib(std::wstring &path)
{
  if (debug_eli)
	WriteELIDebug(L"LoadExtLib", L"[START]");

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(GetInitDir()) + path;
	}

  static HINSTANCE h = NULL;
  h = LoadLibraryW(path.c_str());

  if (h)
	{
	  for (UINT i = 0; i < vecLibs.size(); i++)
		 if (vecLibs[i] == h)
		   return NULL;

	  vecLibs.push_back(h);
	}

  if (debug_eli)
	WriteELIDebug(L"LoadExtLib", L"[END]");

  return h;
}
//-------------------------------------------------------------------------------

bool ELI::FreeExtLib(HINSTANCE hnd)
{
  if (debug_eli)
    WriteELIDebug(L"FreeExtLib", L"[START]");

  if (!hnd)
	return false;

    UINT i = 0;

  while (i < vecExtFuncs.size())
    {
      if (vecExtFuncs[i].exthinst == hnd)
		{
          fStack->Delete(vecExtFuncs[i].inname);
          vecExtFuncs.erase(vecExtFuncs.begin() + i);
        }
      else
        i++;
	}


  for (UINT i = 0; i < vecLibs.size(); i++)
    {
	  if (vecLibs[i] == hnd)
		{
		  vecLibs.erase(vecLibs.begin() + i);

		  return FreeLibrary(hnd);
		}
	}

  if (debug_eli)
	WriteELIDebug(L"FreeExtLib", L"[END]");

  return false;
}
//-------------------------------------------------------------------------------

bool ELI::AddClassProperty(std::wstring &cl_name, std::wstring &prop_str, bool is_public, UINT index)
{
  if (debug_eli)
	WriteELIDebug(L"AddClassProperty", L"[START]");

  UINT pos = prop_str.find(L"=");

  if (pos == std::wstring::npos)
    {
	  AddInfoMsg(SYNTAXERR, ERRMSG, index);

      return false;
	}

  std::wstring name = prop_str.substr(0, pos);
  std::wstring val = prop_str.substr(pos + 1, prop_str.length() - pos - 1);

  if (!IsCorrectName(name.c_str()))
	{
      AddInfoMsg(OBJNOCRTPROP, ERRMSG, index);

	  return false;
    }

  if (clStack->Get(cl_name, name).size() > 0)
	{
	  AddInfoMsg(CLPROPDUP, ERRMSG, index);

	  return false;
	}

//рассчитаем правую часть выражения, чтобы определить значение свойства
  wchar_t tmp[CHARSIZE];

  wcscpy(tmp, val.c_str());
  float *fval = CalcExpNum(tmp, index);

  if (fval)
    {
      swprintf(tmp, FRMTNUM, *fval);
      val = tmp;
    }
  else
    val = CalcExpStr(tmp, index);

  RESOURCE res;

  if (!is_public)
    res.ObjectCathegory = CLPROP;
  else
    res.ObjectCathegory = CLPUBPROP;

  res.ObjectID = cl_name;
  res.PropertyID = name;
  res.Value = val;

  if (clStack->Add(res) < 1)
    return false;

  if (debug_eli)
    WriteELIDebug(L"AddClassProperty", L"[END]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::AddClassMethod(std::wstring &cl_name, std::wstring &method_str, bool is_public, UINT index)
{
  if (debug_eli)
	WriteELIDebug(L"AddClassMethod", L"[START]");

  UINT op = method_str.find(L"("), cl = method_str.find(L")");

  if (!op || !cl)
	{
      AddInfoMsg(SYNTAXERR, ERRMSG, index);

	  return false;
	}

  std::wstring args = method_str.substr(op + 1, cl - op - 1);
  std::wstring name = method_str.substr(0, op);
  std::wstring mark = method_str.substr(cl + 1, method_str.length() - cl - 1);

  if (!IsCorrectName(name.c_str()))
	{
	  AddInfoMsg(OBJNOCRTPROP, ERRMSG, index);

      return false;
    }

  if (clStack->Get(cl_name, name).size() > 0)
    {
	  AddInfoMsg(CLMETHDUP, ERRMSG, index);

      return false;
    }

  wchar_t str[CHARSIZE];

  if (args != L"")
	swprintf(str, L"#procedure%s%s(%s,%s)%s", cl_name.c_str(), name.c_str(), OBJTHIS, args.c_str(), mark.c_str());
  else
    swprintf(str, L"#procedure%s%s(%s)%s", cl_name.c_str(), name.c_str(), OBJTHIS, mark.c_str());

  if (!CreateProcedure(str, index))
    return false;

  RESOURCE res;

  if (!is_public)
    res.ObjectCathegory = CLMETHOD;
  else
    res.ObjectCathegory = CLPUBMETHOD;

  res.ObjectID = cl_name;
  res.PropertyID = name;
  res.Value = args;

  if (clStack->Add(res) < 1)
    return false;

  if (debug_eli)
    WriteELIDebug(L"AddClassMethod", L"[END]");

  return true;
}
//-------------------------------------------------------------------------------

bool ELI::AddRef(const wchar_t *name, const wchar_t *val)
{
  for (UINT i = 0; i < vecRefs.size(); i++)
    {
	  if (0 == wcscmp(vecRefs[i].refname, name))
        {
          wcscpy(vecRefs[i].refobj, val);

          return true;
        }
    }

  REFERENCE rf;

  wcscpy(rf.refname, name);
  wcscpy(rf.refobj, val);

  vecRefs.push_back(rf);

  return true;
}
//-------------------------------------------------------------------------------

REFERENCE *ELI::GetRef(const wchar_t *name)
{
  for (UINT i = 0; i < vecRefs.size(); i++)
    {
      if (0 == wcscmp(vecRefs[i].refname, name))
		return &vecRefs[i];
    }

  return NULL;
}
//-------------------------------------------------------------------------------

bool ELI::ImportParentClass(std::wstring child, std::wstring parent, bool type)
{
  RESRECORDSET rs;
  CONDITION c;
  std::vector<CONDITION> cnds;
  std::vector<RESOURCE> records;

//ищем все публичные члены родительского класса
  c.type = obj_id;
  c.value = parent;
  cnds.push_back(c);

  c.type = obj_cath;

  if (type)
	c.value = CLPUBPROP;
  else
    c.value = CLPUBMETHOD;

  cnds.push_back(c);

  rs = clStack->Get(&cnds);

  if (rs.size() == 0)
	return false; //у родительского класса нет публичных членов

  for (auto imp : rs)
	records.push_back(*imp);

//добавляем их к дочернему классу
  for (auto rec : records)
    {
//если такое свойство есть у дочернего класса - не добавляем
	  if (clStack->Get(child, rec.PropertyID).size() == 0)
		{
		  rec.ObjectID = child;
		  clStack->Add(rec);

		  if (rec.ObjectCathegory == CLPUBMETHOD)
            {
			  RESRECORDSET pr = procStack->Get(obj_id, parent + rec.PropertyID);
			  RESOURCE ch_proc_prm, ch_proc_txt;

			  if (pr.size() < 2) //не вистачає записів у стеку процедур
				{
				  if (debug_eli)
					{
					  WriteELIDebug(L"ImportParentClass", std::wstring(parent + rec.PropertyID).c_str());
					  WriteELIDebug(L"ImportParentClass", L"[FAIL]");
					}

				  return false;
				}
			  else
				{
				  ch_proc_prm = *pr[0];
				  ch_proc_prm.ObjectID = child + rec.PropertyID;

				  ch_proc_txt = *pr[1];
				  ch_proc_txt.ObjectID = child + rec.PropertyID;

				  procStack->Add(ch_proc_prm);
				  procStack->Add(ch_proc_txt);
				}
			}
		}
	}

  return true;
}
//-------------------------------------------------------------------------------

const wchar_t *ELI::CreateTempObject(std::wstring ctor_str, std::wstring owner, UINT index)
{
  if (debug_eli)
	{
	  WriteELIDebug(L"CreateTempObject", L"[START]");
	  WriteELIDebug(L"CreateTempObject", ctor_str.c_str());
	}

  std::wstring clname, ctor_args;
  FStrBuffer = L"";

  UINT pos = ctor_str.find(L"(");

  if (pos != std::wstring::npos)
	{
	  ctor_args = ctor_str.substr(pos, ctor_str.length() - pos);

//додамо в аргументи символ строкового типу для більш коректної роботи конструктора
	  ctor_args.insert(1, 1, STRSYM);
	  ctor_args.insert(ctor_args.length() - 1, 1, STRSYM);

	  clname = ctor_str.erase(pos, ctor_str.length() - pos);
    }
  else
    clname = ctor_str;

  wchar_t tmp[MAXNAMELEN];
  swprintf(tmp, TMPOBJF, OBJSYM, TmpObjInd++);
  FStrBuffer = tmp;

  RESOURCE res;
  res.ObjectCathegory = clname;
  res.ObjectID = FStrBuffer;

  RESRECORDSET rs = clStack->Get(obj_id, clname);

  if (rs.size() == 0)
    {
      if (debug_eli)
		WriteELIDebug(L"CreateTempObject", L"[FAIL]");

      return L"0";
	}

  res.PropertyID = L"Owner";
  res.Value = owner;
  objStack->Add(res);

  for (UINT i = 0; i < rs.size(); i++)
    {
	  res.PropertyID = rs[i]->PropertyID;
      res.Value = rs[i]->Value;

	  if (res.Value.find(L"#class") != std::wstring::npos)
        {
		  res.Value.erase(0, 6);
		  res.Value = CreateTempObject(res.Value, FStrBuffer, index);
		}

	  objStack->Add(res);
	}

//и выполним конструктор временного класса, если он описан
  if (ctor_args != L"")
	{
	  std::wstring def_ctor = FStrBuffer + OBJPROPSEPSTR + clname + ctor_args;

	  if (!TranslateLine(def_ctor.c_str(), index))
		AddInfoMsg(OBJNOCTOR, WRNMSG, index);
    }

  if (debug_eli)
	{
	  WriteELIDebug(L"CreateTempObject", FStrBuffer.c_str());
	  WriteELIDebug(L"CreateTempObject", L"[OK]");
	}

  FStrBuffer = tmp; //повертаємо значення від початку функції, нівелюючи зміни рекурсії

  return FStrBuffer.c_str();
}
//-------------------------------------------------------------------------------

bool ELI::DestroyObject(std::wstring &obj_name, UINT index)
{
  bool res = false;

  if (debug_eli)
	{
	  WriteELIDebug(L"DestroyObject", L"[START]");
	  WriteELIDebug(L"DestroyObject", obj_name.c_str());
	}

  try
	 {
	   RESRECORDSET rs = objStack->Get(obj_id, obj_name);

	   if (rs.size() > 0)
		 {
		   for (auto rec : rs)
			  {
				rec->KeepInStack = NO;

//виконаємо деструктор якщо він є
				if (rec->PropertyID == DTORSYMB + rec->ObjectCathegory)
				  {
					std::wstring def_dtor = rec->ObjectID + OBJPROPSEPSTR + rec->PropertyID + L"()";
					TranslateLine(def_dtor.c_str(), index);
				  }

//знайдемо та видалимо всі об'єкти-властивості нашого об'єкта
				if ((rec->Value.find(OBJSYM) != std::wstring::npos) && (rec->PropertyID != L"Owner"))
				  DestroyObject(rec->Value, index);
			  }

		   res = true;
		 }
	   else
		 throw Exception(OBJNONE);

       if (debug_eli)
		 WriteELIDebug(L"DestroyObject", L"[OK]");
	 }
  catch (Exception &e)
	 {
	   if (debug_eli)
		 WriteELIDebug(L"DestroyObject", L"[FAIL]");

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::ImportMemberFromObject(std::wstring &obj_name, std::wstring &src_name, std::wstring &mb_name, UINT index)
{
  bool res = false;

  if (debug_eli)
	{
	  WriteELIDebug(L"ImportMemberFromObject", L"[START]");
	  WriteELIDebug(L"ImportMemberFromObject", obj_name.c_str());
	}

  try
	 {
	   wchar_t source[CHARSIZE];
	   swprintf(source, L"%c%s", OBJSYM, src_name.c_str());

	   std::wstring cath = objStack->Get(obj_id, obj_name)[0]->ObjectCathegory;

	   RESRECORDSET rs = objStack->Get(source, mb_name);

	   if (rs.size() == 0)
		 throw Exception(OBJNONE);
	   else
		 {
		   std::wstring src_cath = rs[0]->ObjectCathegory;
		   std::wstring val = rs[0]->Value;

		   if (IsClassMember(src_cath.c_str(), mb_name.c_str())) //якщо вказана властивість є членом класа
			 {
			   if (!ImportMemberFromClass(obj_name, src_cath, mb_name, index)) //імпортуємо дані з класу
				 throw Exception(L"");

//замінимо значення, що було імпортовано з класу на значення властивості об'єкта
			   rs.clear();

			   rs = objStack->Get(obj_name, mb_name);

			   rs[0]->Value = val;
			 }
		   else
			 objStack->Add({0, cath, obj_name, mb_name, val, YES, YES});

		   if (debug_eli)
			 WriteELIDebug(L"ImportMemberFromObject", L"[OK]");

           res = true;
		 }
	 }
  catch (Exception &e)
	 {
	   if (debug_eli)
		 {
           WriteELIDebug(L"ImportMemberFromObject", e.ToString().c_str());
		   WriteELIDebug(L"ImportMemberFromObject", L"[FAIL]");
		 }

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

bool ELI::ImportMemberFromClass(std::wstring &obj_name, std::wstring &cl_name, std::wstring &mb_name, UINT index)
{
  bool res = false;

  if (debug_eli)
	{
	  WriteELIDebug(L"ImportMemberFromClass", L"[START]");
	  WriteELIDebug(L"ImportMemberFromClass", obj_name.c_str());
	}

  try
	 {
	   if (clStack->Get(obj_id, cl_name).size() == 0)
		 throw Exception(CLNONE);

	   std::wstring cath = objStack->Get(obj_id, obj_name)[0]->ObjectCathegory;

	   RESRECORDSET rs = clStack->Get(cl_name, mb_name);

	   if (rs.size() == 0)
		 throw Exception(CLNOPROP);
	   else
		 {
		   if ((rs[0]->ObjectCathegory == CLPROP) || (rs[0]->ObjectCathegory == CLMETHOD)) //приватний член
			 throw Exception(CLMEMNOTACC);
		   else if (rs[0]->ObjectCathegory == CLPUBMETHOD) //публічний метод
			 {
//отримаємо зі стека процедур мітку фрагмента, який містить тіло процедури, що відповідає за метод
			   std::wstring src_frg = procStack->Get(cl_name + mb_name, OBJPROCTXT)[0]->Value;

			   RESOURCE res = *rs[0];

			   res.ObjectID = cath; //міняємо ім'я класа на категорію нашого об'єкта
			   clStack->Add(res); //додаємо в стек класів запис про метод

//створюємо записи в стеку процедур
			   procStack->Add({0, OBJPROC, cath + mb_name, OBJPROCPRM, OBJTHIS, YES, YES});
			   procStack->Add({0, OBJPROC, cath + mb_name, OBJPROCTXT, src_frg, YES, YES});
			 }

		   std::wstring val = rs[0]->Value;

//якщо значенням властивості є клас, створимо службовий об'єкт
		   if (val.find(L"#class") != std::wstring::npos)
			 {
			   val = val.erase(0, 6);
			   val = CreateTempObject(val, obj_name, index);
			 }

//додамо в стек об'єктів новий член класа
		   objStack->Add({0, cath, obj_name, mb_name, val, YES, YES});

           res = true;
		 }

       if (debug_eli)
		 WriteELIDebug(L"ImportMemberFromClass", L"[OK]");
	 }
  catch (Exception &e)
	 {
	   if (debug_eli)
		 {
		   WriteELIDebug(L"ImportMemberFromClass", e.ToString().c_str());
		   WriteELIDebug(L"ImportMemberFromClass", L"[FAIL]");
		 }

	   res = false;
	 }

  return res;
}
//-------------------------------------------------------------------------------

//экспортные функции
const wchar_t * __stdcall ELI::GetVersion()
{
  wchar_t path[1024];

  swprintf(path, L"%s\\ELI.dll", initdir);

  return GetVersionInString(path).c_str();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::RunScript(const wchar_t *imptext, const wchar_t *parameter, bool log)
{
  if (debug_eli)
    WriteELIDebug(L"RunScript", L"[START]");

  FreeRes();

  if (log)
    write_log = true;

//устанавливаем имя скрипта как неопределенное
  pStack->Add(P_SCRNAME, L"*unknown script*");

  if (wcslen(parameter) > 0)
    {
      wchar_t name[MAXNAMELEN];

	  UINT cnt = _wstrccount(parameter, EXTPRMSEP);

      if (cnt > 0)
		{
		  std::vector<std::wstring> tmp;

		  StrToListW(&tmp, std::wstring(parameter), EXTPRMSEPS, NODELIMEND);

          for (UINT i = 0; i < tmp.size(); i++)
            {
              swprintf(name, EXTPRMNM, i);
			  pStack->Add(name, tmp[i].c_str());
            }
        }
      else
        {
          swprintf(name, EXTPRMNM, 0);
          pStack->Add(name, parameter);
        }
    }

  scrtext = imptext; //получаем текст по указателю, полученному от приложения

  PrepareScript();

  if (TranslateScriptLines())
    AddInfoMsg(SCEND);
  else
    {
      AddInfoMsg(SCNOEND);

	  if (!use_return)
		ScriptResult = ERROUT;
    }

  AddInfoMsg(TRANSLATED);

  if (write_log)
    {
	  String msg = String(TRANSLATED) + L"\r\n-----------------------\r\n";
      WriteLog(msg.c_str());
    }

  if (debug_eli)
    {
	  WriteELIDebug(L"RunScript", TRANSLATED);
    }

  if (debug_eli)
    {
	  std::wstring msg = L"result = " + ScriptResult;
      WriteELIDebug(L"RunScript", msg.c_str());
      WriteELIDebug(L"RunScript", L"[OK]");
	}

  if (!InterpreterSettings.KeepObjects)
	objStack->Clear();

  if (!InterpreterSettings.KeepClasses)
	clStack->Clear();

  return ScriptResult.c_str();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::RunScriptFromFile(const wchar_t *filepath, const wchar_t *parameter, bool log)
{
  if (debug_eli)
    WriteELIDebug(L"RunScriptFromFile", L"[START]");

  FreeRes();

  if (log)
    write_log = true;

//устанавливаем имя скрипта как неопределенное
  pStack->Add(P_SCRNAME, L"*unknown script*");

  if (wcslen(parameter) > 0)
    {
      wchar_t name[MAXNAMELEN];

      UINT cnt = _wstrccount(parameter, EXTPRMSEP);

      if (cnt > 0)
        {
		  std::vector<std::wstring> tmp;

		  StrToListW(&tmp, std::wstring(parameter), EXTPRMSEPS, NODELIMEND);

          for (UINT i = 0; i < tmp.size(); i++)
            {
              swprintf(name, EXTPRMNM, i);
              pStack->Add(name, tmp[i].c_str());
            }
        }
      else
        {
          swprintf(name, EXTPRMNM, 0);
          pStack->Add(name, parameter);
        }
	}

  std::wstring path = filepath;

//использован путь типа ".\file.eli" - используется текущий каталог
  if (path[0] == '.')
	{
	  path.erase(0, 1);
	  path = std::wstring(GetInitDir()) + path;
    }

  scrtext = LoadTextFile(path.c_str()).c_str();

  if (scrtext == ERROUT)
    {
      if (debug_eli)
        WriteELIDebug(L"RunScriptFromFile", ERROUT);

      return ERROUT;
    }

  if (write_log)
    {
	  std::wstring msg = L"Open file: " + path;
      WriteLog(msg.c_str());
    }

  if (debug_eli)
    {
	  std::wstring msg = L"Open file: " + path;
      WriteELIDebug(L"RunScriptFromFile", msg.c_str());
    }

  PrepareScript();

  if (TranslateScriptLines())
    AddInfoMsg(SCEND);
  else
    {
      AddInfoMsg(SCNOEND);

      if (!use_return)
        ScriptResult = ERROUT;
    }

  AddInfoMsg(TRANSLATED);

  if (write_log)
    {
	  std::wstring msg = L"Closed file: " + path + L"\r\n-----------------------\r\n";
      WriteLog(msg.c_str());
    }

  if (debug_eli)
    {
	  std::wstring msg = L"Closed file: " + path;
      WriteELIDebug(L"RunScriptFromFile", msg.c_str());
    }

  if (debug_eli)
    {
	  std::wstring msg = L"result = " + ScriptResult;
      WriteELIDebug(L"RunScriptFromFile", msg.c_str());
      WriteELIDebug(L"RunScriptFromFile", L"[END]");
    }

  if (!InterpreterSettings.KeepObjects)
	objStack->Clear();

  if (!InterpreterSettings.KeepClasses)
	clStack->Clear();

  return ScriptResult.c_str();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowVarStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowVarStack", L"[START]");

  return st->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowObjStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowObjStack", L"[START]");

  return objStack->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowClassStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowClassStack", L"[START]");

  return clStack->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowProcStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowProcStack", L"[START]");

  return procStack->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowFragmentStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowFragmentStack", L"[START]");

  return frgStack->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowInfoMessages()
{
  if (debug_eli)
    WriteELIDebug(L"ShowInfoMessages", L"[START]");

  return InfStack.c_str();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowFuncStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowFuncStack", L"[START]");

  return fStack->GetString();
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::ShowParamStack()
{
  if (debug_eli)
    WriteELIDebug(L"ShowParamStack", L"[START]");

  return pStack->GetString();
}
//-------------------------------------------------------------------------------

void __stdcall ELI::SetDebug(bool enable_dbg, bool in_file)
{
  debug_eli = enable_dbg;
  debug_in_file = in_file;
}
//-------------------------------------------------------------------------------

void __stdcall ELI::AddFunction(const wchar_t *name, const wchar_t *params, func_ptr fptr)
{
  try
     {
       fStack->Add(name, params, fptr);
     }
  catch (Exception &e)
     {
	   String msg = "AddFunction(" + String(name) + ", " + String(params) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
	 }
}
//-------------------------------------------------------------------------------

void __stdcall ELI::DeleteFunction(const wchar_t *name)
{
  try
     {
       fStack->Delete(name);
     }
  catch (Exception &e)
     {
	   String msg = "DeleteFunction(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
     }
}
//-------------------------------------------------------------------------------

void __stdcall ELI::CallFunction(const wchar_t *name)
{
  try
     {
	   FUNC *fn = fStack->Get(name);

	   if (!fn)
		 throw Exception("Function not found");

	   fn->Call(this);
	 }
  catch (Exception &e)
	 {
	   String msg = "CallFunction(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
     }
}
//-------------------------------------------------------------------------------

wchar_t * __stdcall ELI::GetFunctionResult(const wchar_t *name)
{
  wchar_t *res;

  try
     {
       FUNC *fn = fStack->Get(name);

	   if (!fn)
		 throw Exception("Function not found");

	   res = fn->GetResult();
     }
  catch (Exception &e)
     {
	   String msg = "GetFunctionResult(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
     }

  return res;
}
//-------------------------------------------------------------------------------

void __stdcall ELI::SetFunctionResult(const wchar_t *name, const wchar_t* result)
{
  try
     {
       FUNC *fn = fStack->Get(name);

	   if (!fn)
		 throw Exception("Function not found");

	   fn->SetResult(result);
     }
  catch (Exception &e)
     {
	   String msg = "SetFunctionResult(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
     }
}
//-------------------------------------------------------------------------------

void __stdcall ELI::SetParam(const wchar_t *name, const wchar_t *new_val)
{
  try
     {
       pStack->Add(name, new_val);
     }
  catch (Exception &e)
     {
	   String msg = "SetParam(" + String(name) + String(new_val) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());
     }
}
//-------------------------------------------------------------------------------

int __stdcall ELI::GetParamToInt(const wchar_t *name)
{
  int res = 0;

  try
	 {
	   PARAM *p = pStack->Get(name);

	   if (!p)
		 throw Exception("Parameter not found");

	   res = p->ToInt();
     }
  catch (Exception &e)
	 {
	   String msg = "GetParamToInt(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

	   res = 0;
     }

  return res;
}
//-------------------------------------------------------------------------------

float __stdcall ELI::GetParamToFloat(const wchar_t *name)
{
  float res = 0.0f;

  try
	 {
	   PARAM *p = pStack->Get(name);

	   if (!p)
		 throw Exception("Parameter not found");

	   res = p->ToFloat();
	 }
  catch (Exception &e)
	 {
	   String msg = "GetParamToFloat(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

	   res = 0.0f;
	 }

  return res;
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::GetParamToStr(const wchar_t *name)
{
  const wchar_t * res = nullptr;

  try
	 {
	   PARAM *p = pStack->Get(name);

	   if (!p)
		 throw Exception("Parameter not found");

	   res = p->ToStr();
	 }
  catch (Exception &e)
	 {
	   String msg = "GetParamToStr(" + String(name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

	   res = nullptr;
	 }

  return res;
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::GetObjectProperty(const wchar_t *obj_name, const wchar_t *prop_name)
{
  try
     {
	   RESRECORDSET rs = objStack->Get(obj_id, std::wstring(obj_name));

	   if (rs.size() == 0)
		 throw Exception(OBJNONE);

	   rs.clear();

	   rs = objStack->Get(std::wstring(obj_name), std::wstring(prop_name));

	   if (rs.size() == 0)
		 throw Exception(OBJNOPROP);

	   if (IsClassMember(rs[0]->ObjectCathegory.c_str(), prop_name) && !IsAccessibleMember(obj_name, prop_name))
		 throw Exception(OBJMEMNOTACC);

	   return rs[0]->Value.c_str();
     }
  catch (Exception &e)
	 {
	   String msg = "GetObjectProperty(" + String(obj_name) + ", " + String(prop_name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

	   return L"";
     }

  return L"";
}
//-------------------------------------------------------------------------------

bool __stdcall ELI::SetObjectProperty(const wchar_t *obj_name, const wchar_t *prop_name, const wchar_t *val)
{
  try
     {
	   RESRECORDSET rs = objStack->Get(obj_id, std::wstring(obj_name));

	   if (rs.size() == 0)
		 throw Exception(OBJNONE);

	   rs.clear();

	   rs = objStack->Get(std::wstring(obj_name), std::wstring(prop_name));

	   if (rs.size() == 0)
		 throw Exception(OBJNOPROP);

	   if (IsClassMember(rs[0]->ObjectCathegory.c_str(), prop_name) && !IsAccessibleMember(obj_name, prop_name))
		 throw Exception(OBJMEMNOTACC);

	   rs[0]->Value = val;

	   return true;
     }
  catch (Exception &e)
	 {
	   String msg = "SetObjectProperty(" + String(obj_name) + ", " + String(prop_name) + ")";
	   SaveLogToUserFolder("ELI.log", "ELI", msg);
	   SaveLogToUserFolder("ELI.log", "ELI", e.ToString());

	   return false;
	 }

  return false;
}
//-------------------------------------------------------------------------------

const wchar_t * __stdcall ELI::GetCurrentFuncName()
{
  return current_func_name.c_str();
}
//-------------------------------------------------------------------------------

bool __stdcall ELI::DebugEnabled()
{
  return debug_eli;
}
//-------------------------------------------------------------------------------

const wchar_t* __stdcall ELI::GetInitDir()
{
  return initdir;
}
//-------------------------------------------------------------------------------

void __stdcall ELI::AddToLog(const wchar_t *msg)
{
  AddInfoMsg(msg);
}
//-------------------------------------------------------------------------------

///встроенные ф-ии для скриптового языка
void __stdcall scRandom(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scRandom", L"[START]");

  try
	 {
	   int area = e_ptr->GetParamToInt(L"pArea");
	   wchar_t res[NUMSIZE];

	   swprintf(res, L"%d", Random(area));

	   e_ptr->SetFunctionResult(L"_random", res);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_random", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scRandom: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRandom", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scRound(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scRound", L"[START]");

  try
	 {
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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_round", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scRound: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRound", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scInt(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scInt", L"[START]");

  try
	 {
	   wchar_t res[NUMSIZE];

	   swprintf(res, L"%d", e_ptr->GetParamToInt(L"pNumber"));
	   e_ptr->SetFunctionResult(L"_int", res);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_int", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scInt: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scInt", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scStrLen(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scStrLen", L"[START]");

  try
	 {
	   wchar_t res[NUMSIZE];
	   UINT len = wcslen(e_ptr->GetParamToStr(L"pStr"));

	   swprintf(res, L"%d", len);

	   e_ptr->SetFunctionResult(L"_strlen", res);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_strlen", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scStrLen: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrLen", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scStrEq(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scStrEq", L"[START]");

  try
	 {
	   if (wcscmp(e_ptr->GetParamToStr(L"pStr1"), e_ptr->GetParamToStr(L"pStr2")) == 0)
		 e_ptr->SetFunctionResult(L"_streq", L"1");
	   else
		 e_ptr->SetFunctionResult(L"_streq", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_streq", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scStrEq: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStrEq", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scIStrEq(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scIStrEq", L"[START]");

  try
	 {
	   if (_wcsicmp(e_ptr->GetParamToStr(L"pStr1"), e_ptr->GetParamToStr(L"pStr2")) == 0)
		 e_ptr->SetFunctionResult(L"_istreq", L"1");
	   else
		 e_ptr->SetFunctionResult(L"_istreq", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_istreq", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scIStrEq: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scIStrEq", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSubStr(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSubStr", L"[START]");

  try
	 {
	   std::wstring res = e_ptr->GetParamToStr(L"pTargetStr");

	   UINT pos = e_ptr->GetParamToInt(L"pPos");
	   UINT cnt = e_ptr->GetParamToInt(L"pCount");

	   res = res.substr(pos, cnt);

	   e_ptr->SetFunctionResult(L"_substr", res.c_str());
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_substr", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSubStr: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSubStr", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scReturn(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scReturn", L"[START]");

  try
	 {
	   e_ptr->Result = e_ptr->GetParamToStr(L"pReturnVal");
	   e_ptr->ReturnEnabled = true;
	   e_ptr->SetFunctionResult(L"_return", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_return", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scReturn: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReturn", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scThrow(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scThrow", L"[START]");

  try
	 {
	   e_ptr->AddInfoMsg(e_ptr->GetParamToStr(L"pException"), ERRMSG, e_ptr->GetParamToInt(P_IND));
	   e_ptr->SetFunctionResult(L"_throw", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_throw", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scThrow: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scThrow", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scFree(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scFree", L"[START]");

  try
	 {
	   UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

	   if (e_ptr->VarStack->Remove(e_ptr->GetParamToStr(L"pVarName")))
		 e_ptr->SetFunctionResult(L"_free", L"1");
	   else
		 {
		   e_ptr->AddInfoMsg(UNKVARNAME, WRNMSG, ind);

		   e_ptr->SetFunctionResult(L"_free", L"0");
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_free", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scFree: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFree", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scLoadObjStack(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scLoadObjStack", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFilePath");
	   wchar_t result[3];

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (path[0] == '.')
		 {
		   path.erase(0, 1);
		   path = std::wstring(e_ptr->GetInitDir()) + path;
		 }

	   if (e_ptr->GetParamToInt(L"pClear") > 0)
		 e_ptr->ObjStack->Clear();

	   int res = e_ptr->ObjStack->LoadResFile(path.c_str());

	   swprintf(result, L"%d", res);
	   e_ptr->SetFunctionResult(L"_LoadObjStack", result);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_LoadObjStack", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scLoadObjStack: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadObjStack", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveObjStack(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
   e_ptr->WriteELIDebug(L"scSaveObjStack", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFilePath");
	   wchar_t result[3];

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (path[0] == '.')
		 {
		   path.erase(0, 1);
		   path = std::wstring(e_ptr->GetInitDir()) + path;
		 }

	   int res = e_ptr->ObjStack->CreateResFile(path.c_str(), true);

	   swprintf(result, L"%d", res);
	   e_ptr->SetFunctionResult(L"_SaveObjStack", result);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveObjStack", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveObjStack: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveObjStack", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveObjects(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
   e_ptr->WriteELIDebug(L"scSaveObjects", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFilePath");
	   std::wstring cath = e_ptr->GetParamToStr(L"pCathegory");
	   wchar_t result[3];

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (path[0] == '.')
		 {
		   path.erase(0, 1);
		   path = std::wstring(e_ptr->GetInitDir()) + path;
		 }

	   int res = e_ptr->ObjStack->CreateResFile(path.c_str(), cath.c_str());

	   swprintf(result, L"%d", res);
	   e_ptr->SetFunctionResult(L"_SaveObjects", result);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveObjects", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveObjects: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveObjects", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scCompactObjStack(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scCompactObjStack", L"[START]");

  try
	 {
	   e_ptr->ObjStack->Compact();
	   e_ptr->SetFunctionResult(L"_CompactObjStack", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_CompactObjStack", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scCompactObjStack: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scCompactObjStack", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scClearObjStack(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scClearObjStack", L"[START]");

  try
	 {
	   e_ptr->ObjStack->Clear();
	   e_ptr->SetFunctionResult(L"_ClearObjStack", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_ClearObjStack", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scClearObjStack: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scClearObjStack", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scRemoveObjects(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRemoveObjects", L"[START]");

  try
	 {
	   std::wstring cath = e_ptr->GetParamToStr(L"pCathegory");

	   wchar_t result[3];

	   int res = e_ptr->ObjStack->Delete(cath, L"", L"");

	   swprintf(result, L"%d", res);
	   e_ptr->SetFunctionResult(L"_RemoveObjects", result);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_RemoveObjects", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scRemoveObjects: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRemoveObjects", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scRun(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRun", L"[START]");

  try
	 {
	   int index = e_ptr->GetParamToInt(P_IND);
	   std::wstring vname = e_ptr->GetParamToStr(L"pVarName");

	   VARIABLE *var = e_ptr->VarStack->Get(vname.c_str());

	   if (var)
		 {
		   std::wstring oldval = e_ptr->VarStack->GetStrElement(var);

		   std::wstring text = L"{" + oldval + L"}";
		   text = e_ptr->MarkFragments(text);
		   text.erase(text.length() - 1, 1);

		   e_ptr->VarStack->SetStrElement(var, text);

		   if (e_ptr->TranslateCodeFromVar(vname.c_str(), index))
			 e_ptr->SetFunctionResult(L"_Run", L"1");
		   else
			 e_ptr->SetFunctionResult(L"_Run", L"0");

//да, то, что ниже это костыль. Мне стыдно, но я так и не понял, почему указатель var
//превращается в тыкву, если ф-я _run() транслирует строку с ф-ей _importfunc()
		   var = e_ptr->VarStack->Get(vname.c_str());

		   e_ptr->VarStack->SetStrElement(var, oldval);
		 }
	   else
		 e_ptr->SetFunctionResult(L"_Run", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_Run", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scRun: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scRun", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scGetParamAsNum(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsNum", L"[START]");

  try
	 {
//ищем в стеке имя искомого параметра, которое хранит параметр pParam
	   PARAM *prm = e_ptr->ParamStack->Get(e_ptr->GetParamToStr(L"pParam"));

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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_GetParamAsNum", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scGetParamAsNum: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsNum", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scGetParamAsStr(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scGetParamAsStr", L"[START]");

  try
	 {
//ищем в стеке имя искомого параметра, которое хранит параметр pParam
	   PARAM *prm = e_ptr->ParamStack->Get(e_ptr->GetParamToStr(L"pParam"));

	   if (!prm)
		 {
		   e_ptr->SetFunctionResult(L"_GetParamAsStr", L"0");
		   e_ptr->AddInfoMsg(PARAMERR, WRNMSG, e_ptr->GetParamToInt(P_IND));
		 }
	   else
		 e_ptr->SetFunctionResult(L"_GetParamAsStr", prm->ToStr());
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_GetParamAsStr", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scGetParamAsStr: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetParamAsStr", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSetParam(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSetParam", L"[START]");

  try
	 {
//ищем в стеке имя искомого параметра, которое хранит параметр pParam
	   if (wcslen(e_ptr->GetParamToStr(L"pParam")) == 0)
		 e_ptr->SetFunctionResult(L"_SetParam", L"0");
	   else
		 {
		   e_ptr->ParamStack->Add(e_ptr->GetParamToStr(L"pParam"),
									   e_ptr->GetParamToStr(L"pValue"));
		   e_ptr->SetFunctionResult(L"_SetParam", L"1");
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SetParam", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSetParam: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSetParam", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scLoadFileToVar(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLoadFileToVar", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFile");
	   std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//використано шлях типу ".\file.eli" - використовується поточний каталог
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
			   VARIABLE *var = e_ptr->VarStack->Get(target.c_str());

			   if (var)
				 {
				   e_ptr->VarStack->SetStrElement(var, text);
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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_RemoveObjects", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scRemoveObjects: " + e.ToString());
	 }


  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scLoadFileToVar", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveVarToFile(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFile");
	   std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (path[0] == '.')
		 {
		   path.erase(0, 1);
		   path = std::wstring(e_ptr->GetInitDir()) + path;
		 }

	   if (target.find(L"$") != std::wstring::npos)
		 {
		   VARIABLE *var = e_ptr->VarStack->Get(target.c_str());

		   if (var)
			 {
			   std::wstring text = e_ptr->VarStack->GetStrElement(var);

			   SaveToFile(path.c_str(), text.c_str());
			   e_ptr->SetFunctionResult(L"_SaveVarToFile", L"1");
			 }
		   else
			 {
			   e_ptr->SetFunctionResult(L"_SaveVarToFile", L"0");

			   if (e_ptr->DebugEnabled())
				 e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[no variable!]");
			 }
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveVarToFile", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveVarToFile: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSaveVarToFile", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveFragmentToFile(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFile");
	   std::wstring target = e_ptr->GetParamToStr(L"pTarget");

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (path[0] == '.')
		 {
		   path.erase(0, 1);
		   path = std::wstring(e_ptr->GetInitDir()) + path;
		 }

	   if (target.find(L"$") != std::wstring::npos)
		 {
		   VARIABLE *var = e_ptr->VarStack->Get(target.c_str());

		   if (var)
			 {
			   std::wstring mark = e_ptr->VarStack->GetStrElement(var);

			   SCRIPTLINES *ptr = e_ptr->FragmentStack->GetFragmentCode(mark);

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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveFragmentToFile", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveFragmentToFile: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSaveFragmentToFile", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scGetConfig(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetConfig", L"[START]");

  try
	 {
	   std::wstring path = e_ptr->GetParamToStr(L"pFile");
	   std::wstring line = e_ptr->GetParamToStr(L"pLine");

//використано шлях типу ".\file.eli" - використовується поточний каталог
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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_GetConfig", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scGetConfig: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scGetConfig", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveState(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveState", L"[START]");

  try
	 {
	   e_ptr->SaveELIState();
	   e_ptr->SetFunctionResult(L"_SaveState", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveState", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveState: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveState", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSaveVarStack(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarStack", L"[START]");

  try
	 {
	   UINT level = e_ptr->GetParamToInt(L"pLevel");

	   e_ptr->SaveVStState(level);
	   e_ptr->SetFunctionResult(L"_SaveVarStack", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_SaveVarStack", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSaveVarStack: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSaveVarStack", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scWriteOut(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scWriteOut", L"[START]");

  try
	 {
	   const wchar_t *str = e_ptr->GetParamToStr(L"pStr");
	   std::wstring outstr = L"";

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
		 outstr = L"";
	   else
		 outstr = str;

	   outstr.append(L"\r\n");

	   wprintf(outstr.c_str());

	   e_ptr->SetFunctionResult(L"_WriteOut", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_WriteOut", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scWriteOut: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scWriteOut", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scReadIn(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReadIn", L"[START]");

  try
	 {
	   wchar_t str[CHARSIZE];

	   _getws(str);

	   VARIABLE *var = e_ptr->VarStack->Get(e_ptr->GetParamToStr(L"pVar"));

	   if (var && var->type == SCSTR)
		 {
		   e_ptr->VarStack->SetStrElement(var, std::wstring(str));
		   e_ptr->SetFunctionResult(L"_ReadIn", L"1");
		 }
	   else
		 e_ptr->SetFunctionResult(L"_ReadIn", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_ReadIn", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scReadIn: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scReadIn", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scSystem(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSystem", L"[START]");

  try
	 {
	   wchar_t res[NUMSIZE];

	   swprintf(res, L"%d", _wsystem(e_ptr->GetParamToStr(L"pCmd")));
	   e_ptr->SetFunctionResult(L"_System", res);
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_System", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSystem: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scSystem", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scLastError(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLastError", L"[START]");

  try
	 {
	   e_ptr->SetFunctionResult(L"_LastError", e_ptr->LastErr.c_str());
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_LastError", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scLastError: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scLastError", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scConnectLib(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scConnectLib", L"[START]");

  try
	 {
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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_ConnectLib", L"-1");
	   SaveLogToUserFolder("ELI.log", "ELI", "scConnectLib: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scConnectLib", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scFreeLib(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFreeLib", L"[START]");

  try
	 {
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
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_FreeLib", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scFreeLib: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scFreeLib", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scImportFunc(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scImportFunc", L"[START]");

  try
	 {
	   UINT hi = e_ptr->GetParamToInt(L"pHandle");
	   HINSTANCE h = (HINSTANCE)hi;
	   std::wstring ext_name = e_ptr->GetParamToStr(L"pExtName");
	   std::wstring in_name = e_ptr->GetParamToStr(L"pInName");
	   std::wstring args = e_ptr->GetParamToStr(L"pArgList");

	   if (h)
		 {
		   IMPORTFUNC fptr = (IMPORTFUNC)GetProcAddress(h, AnsiOf(ext_name.c_str()));

		   if (fptr)
			 {
			   e_ptr->AddFunction(in_name.c_str(), args.c_str(), fptr);

			   EXTFUNC ef;

			   ef.exthinst = h;
			   wcscpy(ef.inname, in_name.c_str());
			   e_ptr->ExtFnStack.push_back(ef);
			   e_ptr->SetFunctionResult(L"_ImportFunc", L"1");
			 }
		   else
			 e_ptr->SetFunctionResult(L"_ImportFunc", L"-1");
		 }
	   else
		 e_ptr->SetFunctionResult(L"_ImportFunc", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_ImportFunc", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scImportFunc: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scImportFunc", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scDebugIntoFile(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  try
	 {
	   e_ptr->SetDebug(true, true);

	   debugfile = e_ptr->GetParamToStr(L"pFile");

//використано шлях типу ".\file.eli" - використовується поточний каталог
	   if (debugfile[0] == '.')
		 {
		   debugfile.erase(0, 1);
		   debugfile = std::wstring(e_ptr->GetInitDir()) + debugfile;
		 }

	   e_ptr->SetFunctionResult(L"_DebugIntoFile", L"0");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_DebugIntoFile", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scDebugIntoFile: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scDebugIntoFile", L"[START]");
}
//-------------------------------------------------------------------------------

void __stdcall scDebugIntoScreen(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  try
	 {
	   e_ptr->SetDebug(true, false);

	   e_ptr->SetFunctionResult(L"_DebugIntoScreen", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_DebugIntoScreen", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scDebugIntoScreen: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scDebugIntoScreen", L"[START]");
}
//-------------------------------------------------------------------------------

void __stdcall scStopDebug(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"scStopDebug", L"[START]");

  e_ptr->SetDebug(false, false);

  debugfile = L"debug.log";

  e_ptr->SetFunctionResult(L"_StopDebug", L"0");
}
//-------------------------------------------------------------------------------

void __stdcall scSleep(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSleep", L"[START]");

  try
	 {
	   int msec = e_ptr->GetParamToInt(L"pMsec");

	   Sleep(msec);

	   e_ptr->SetFunctionResult(L"_Sleep", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_Sleep", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scSleep: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scSleep", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall scShowMessage(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scShowMessage", L"[START]");

  try
	 {
	   String msg = e_ptr->GetParamToStr(L"pText"),
			  hdr = e_ptr->GetParamToStr(P_SCRNAME);

	   int res = MessageBox(NULL, msg.c_str(), hdr.c_str(), MB_OK);

	   e_ptr->SetFunctionResult(L"_ShowMessage", IntToStr(res).c_str());
	 }
  catch (Exception &e)
	 {
	   e_ptr->SetFunctionResult(L"_ShowMessage", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "scShowMessage: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"scShowMessage", L"[END]");
}
//-------------------------------------------------------------------------------

///ф-ции, отвечающие за методы объектов
void __stdcall objCreate(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objCreate", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  try
	 {
	   std::wstring cath = e_ptr->GetParamToStr(L"pCathegory");
	   std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);
	   std::wstring ctor_args = e_ptr->GetParamToStr(L"pCtorParams");
	   RESOURCE res = {0, cath.c_str(), objname.c_str(), L"Owner", L"<none>", YES, YES};

	   std::wstring name = objname;
	   name.erase(0, 1);

	   if (!e_ptr->IsCorrectName(name.c_str()))
		 throw Exception(OBJNAMEERR);
	   else if (e_ptr->ObjStack->Get(obj_id, objname).size() > 0)
		 throw Exception(OBJNONE);
	   else if (e_ptr->ObjStack->Add(res) < 1)
		 throw Exception(STADDERR);
	   else
		 {
//проверим соответствует ли имя категории имени известного интерпретатору класса
		   RESRECORDSET rs = e_ptr->ClassStack->Get(obj_id, cath);

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
						res.Value = e_ptr->CreateTempObject(res.Value, objname, ind);
					  }

					e_ptr->ObjStack->Add(res);
				  }

//и выполним конструктор класса, если он описан
			   std::wstring def_ctor = objname + OBJPROPSEPSTR + cath + L"(";

			   if (ctor_args != L"")
				 def_ctor += ctor_args + L")";
			   else
				 def_ctor += L")";

			   if (!e_ptr->TranslateLine(def_ctor.c_str(), ind))
				 e_ptr->AddInfoMsg(OBJNOCTOR, WRNMSG, ind);
			 }

//добавим служебное свойство - имя объекта
		   res.PropertyID = L"ObjectName";
		   res.Value = name;

		   e_ptr->ObjStack->Add(res);

		   e_ptr->SetFunctionResult(L"Create", L"1");
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(OBJNOCRT, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Create", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objCreate: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objCreate", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objDestroy(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objDestroy", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);

  try
	 {
	   if (e_ptr->DestroyObject(objname, ind))
		 {
		   e_ptr->ObjStack->Compact();
		   e_ptr->SetFunctionResult(L"Destroy", L"1");
		 }
	   else
		 throw Exception(STREMERR);
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(OBJNODESTR, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Destroy", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objDestroy: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objDestroy", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objAdd(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objAdd", L"[START]");

  UINT ind = e_ptr->ParamStack->Get(P_IND)->ToInt(); //получаем индекс строки

  try
	 {
//проверим, нет ли у объекта такого свойства
	   RESRECORDSET rs = e_ptr->ObjStack->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
												   std::wstring(e_ptr->GetParamToStr(L"pNewPropName")));

	   if (rs.size() > 0)
		 throw Exception(OBJPROPDUP);
	   else if (!e_ptr->GetObjCathegory(e_ptr->ParamStack->Get(P_OBJNAME)->ToStr()))
		 throw Exception(OBJNONE);
	   else
		 {
		   RESOURCE res = {0,
						   e_ptr->GetObjCathegory(e_ptr->ParamStack->Get(P_OBJNAME)->ToStr()),
						   e_ptr->GetParamToStr(P_OBJNAME),
						   e_ptr->GetParamToStr(L"pNewPropName"),
						   e_ptr->GetParamToStr(L"pNewPropVal"),
						   YES, YES
						  };

//если значение свойства это класс, выполним создание временного объекта
		   if (res.Value.find(L"#class") != std::wstring::npos)
			 {
			   res.Value.erase(0, 6);
			   res.Value = e_ptr->CreateTempObject(res.Value, res.ObjectID, ind);
			 }

		   if (e_ptr->ObjStack->Add(res) < 1)
			 throw Exception(STADDERR);
		   else
		     e_ptr->SetFunctionResult(L"Add", L"1");
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(OBJPROPERR, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Add", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objAdd: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objAdd", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objRemove(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objRemove", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  try
	 {
	   int result = e_ptr->ObjStack->Delete(L"",
												 std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
												 std::wstring(e_ptr->GetParamToStr(L"pPropName")));

	   if (result == -1)
		 throw Exception(OBJINDERR);
	   else if (result == 0)
		 throw Exception(OBJNOPROP);
	   else
		 e_ptr->SetFunctionResult(L"Remove", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(STREMERR, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Remove", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objRemove: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objRemove", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objExist(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExist", L"[START]");

  if (e_ptr->ObjStack->Get(obj_id, std::wstring(e_ptr->GetParamToStr(P_OBJNAME))).size() > 0)
    e_ptr->SetFunctionResult(L"Exist", L"1");
  else
    e_ptr->SetFunctionResult(L"Exist", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExist", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objHave(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objHave", L"[START]");

  std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring prop = e_ptr->GetParamToStr(L"pPropName");

  if (e_ptr->ObjStack->Get(objname, prop).size() > 0)
    e_ptr->SetFunctionResult(L"Have", L"1");
  else
    e_ptr->SetFunctionResult(L"Have", L"0");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objHave", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objKeep(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objKeep", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  RESRECORDSET rs = e_ptr->ObjStack->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											  std::wstring(e_ptr->GetParamToStr(L"pPropName")));

  if (rs.size() == 1)
	{
	  rs[0]->KeepInStack = e_ptr->GetParamToStr(L"pBool");
	  e_ptr->SetFunctionResult(L"Keep", L"1");
	}
  else
	{
	  e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

	  e_ptr->SetFunctionResult(L"Keep", L"0");
	}

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objKeep", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objSave(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objSave", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  RESRECORDSET rs = e_ptr->ObjStack->Get(std::wstring(e_ptr->GetParamToStr(P_OBJNAME)),
											  std::wstring(e_ptr->GetParamToStr(L"pPropName")));

  if (rs.size() == 1)
    {
	  rs[0]->SaveInFile = e_ptr->GetParamToStr(L"pBool");
      e_ptr->SetFunctionResult(L"Save", L"1");
    }
  else
    {
      e_ptr->AddInfoMsg(OBJNOPROP, WRNMSG, ind);

      e_ptr->SetFunctionResult(L"Save", L"0");
    }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objSave", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objExecute(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExecute", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  try
	 {
	   std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);

	   RESRECORDSET rs = e_ptr->ObjStack->Get(objname,
												   std::wstring(e_ptr->GetParamToStr(L"pPropName")));

	   if (rs.size() == 1)
		 {
		   wchar_t cmp_line[128];

		   if (rs[0]->Value.find(FRGMARK) == std::wstring::npos)
			 throw Exception(NOPCCODE);
		   else
			 {
//уберем первый символ (&), потому что процедуре нужно передать имя объекта без спецсимвола
			   objname.erase(0, 1);
			   swprintf(cmp_line, L"#procedure proc%s(%s)%s", objname.c_str(), OBJTHIS, rs[0]->Value.c_str());

			   e_ptr->TranslateLine(cmp_line, ind);
			   swprintf(cmp_line, L":proc%s(%s)", objname.c_str(), objname.c_str());

			   if (e_ptr->TranslateLine(cmp_line, ind))
				 e_ptr->SetFunctionResult(L"Execute", L"1");
			   else
				 e_ptr->SetFunctionResult(L"Execute", L"0");

			   swprintf(cmp_line, L"#drop proc%s", objname.c_str());
			   e_ptr->TranslateLine(cmp_line, ind);
			 }
		 }
	   else
		 throw Exception(OBJNOPROP);
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(ILLGLOBJOPER, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Execute", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objExecute: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objExecute", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objShow(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objShow", L"[START]");

  std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);
  std::wstring outstr;

  RESRECORDSET rs = e_ptr->ObjStack->Get(obj_id, objname);

  if (rs.size() > 0)
	{
	  for (auto r : rs)
		outstr += r->PropertyID + L" = " + r->Value + L"\r\n";
	}

  wprintf(outstr.c_str());

  e_ptr->SetFunctionResult(L"Show", L"1");

  if (e_ptr->DebugEnabled())
    e_ptr->WriteELIDebug(L"objShow", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objClone(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objClone", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  try
	 {
	   RESOURCESTACK *st = e_ptr->ObjStack;
	   std::vector<RESOURCE> import_data; //буфер для імпортуємих даних
	   std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);

	   wchar_t source[CHARSIZE];
	   swprintf(source, L"%c%s", OBJSYM, e_ptr->GetParamToStr(L"pSource"));

	   RESRECORDSET rs = e_ptr->ObjStack->Get(obj_id, std::wstring(source));

	   if (rs.size() == 0)
		 throw Exception(OBJNONE);
	   else
		 {
//копіюємо всі дані донора зі стеку в буфер
		   for (auto src : rs)
			  import_data.push_back(*src);

//видалимо старі дані клона
		   RESRECORDSET old = e_ptr->ObjStack->Get(obj_id, objname);

		   for (auto o : old)
			  o->KeepInStack = NO;

		   st->Compact();

//потім передаємо дані донора в стек, замінивши тільки ім'я об'єкта
		   for (auto imp : import_data)
			  {
				if (imp.PropertyID == L"ObjectName")
				  imp.Value = objname.substr(1, objname.length() - 1);

				imp.ObjectID = objname;

				st->Add(imp);
			  }

		   e_ptr->SetFunctionResult(L"Clone", L"1");
		 }
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(OBJNOCRT, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Clone", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objClone: " + e.ToString());
	 }
  
  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objClone", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objGetName(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objGetName", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки
  std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);

  RESRECORDSET rs = e_ptr->ObjStack->Get(obj_id, objname);

  if (rs.size() == 0)
    {
	  e_ptr->AddInfoMsg(OBJNONE, WRNMSG, ind);

	  e_ptr->SetFunctionResult(L"GetName", L"0");
	}
  else
	{
	  objname = objname.erase(0, 1);
	  e_ptr->SetFunctionResult(L"GetName", objname.c_str());
	}

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objGetName", L"[END]");
}
//-------------------------------------------------------------------------------

void __stdcall objImport(void *p)
{
  ELI *e_ptr = static_cast<ELI*>(p);

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objImport", L"[START]");

  UINT ind = e_ptr->GetParamToInt(P_IND); //получаем индекс строки

  try
	 {
	   std::wstring objname = e_ptr->GetParamToStr(P_OBJNAME);
	   std::wstring src = e_ptr->GetParamToStr(L"pSource");
	   std::wstring prop = e_ptr->GetParamToStr(L"pPropName");

	   if (e_ptr->ObjStack->Get(obj_id, objname).size() == 0)
		 throw Exception(OBJNONE);

	   if (src.find(L"#class") != std::wstring::npos)
		 {
		   std::wstring cl = src.erase(0, 6);

		   if (!e_ptr->ImportMemberFromClass(objname, cl, prop, ind))
			 throw Exception(CLMEMIMPERR);
		 }
	   else
		 {
           if (!e_ptr->ImportMemberFromObject(objname, src, prop, ind))
			 throw Exception(OBJMEMIMPERR);
		 }

	   e_ptr->SetFunctionResult(L"Import", L"1");
	 }
  catch (Exception &e)
	 {
	   e_ptr->AddInfoMsg(e.ToString().c_str(), WRNMSG, ind);
	   e_ptr->AddInfoMsg(OBJPROPERR, WRNMSG, ind);
	   e_ptr->SetFunctionResult(L"Import", L"0");
	   SaveLogToUserFolder("ELI.log", "ELI", "objImport: " + e.ToString());
	 }

  if (e_ptr->DebugEnabled())
	e_ptr->WriteELIDebug(L"objImport", L"[END]");
}
//-------------------------------------------------------------------------------

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  LogPath = GetEnvironmentVariable("USERPROFILE") + "\\ELI";

  if (!DirectoryExists(LogPath))
	CreateDir(LogPath);

  GetModuleFileName(hinst, path, sizeof(path));

  wcscpy(initdir, GetDirPathFromFilePath(String(path)).c_str());

  return 1;
}
//-------------------------------------------------------------------------------
