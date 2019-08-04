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

#ifndef SCDEFINES_H_INCLUDED
#define SCDEFINES_H_INCLUDED

#include <vector>

#define UINT unsigned int

typedef std::vector<std::wstring> (SCRIPTLINES);
typedef void (__stdcall *IMPORTFUNC)(void*);

//константы, описывающие системные сообщения
const wchar_t *INFMSG = L"INFO";
const wchar_t *ERRMSG = L"ERROR";
const wchar_t *WRNMSG = L"WARNING";
const wchar_t *COMPILED = L"End script";
const wchar_t *SCEXIT = L"Used _return()";
const wchar_t *SCEXCEPT = L"Throw exception";
const wchar_t *SCSTP = L"Force stop";
const wchar_t *SCEND = L"Finishing with #end";
const wchar_t *SCNOEND = L"Finishing without #end";
const wchar_t *NOBEGIN = L"Error initializing script - no #begin";
const wchar_t *NOEND = L"Error initializing script - no #end";
const wchar_t *NONAME = L"Script name is not defined";
const wchar_t *ERRNAME = L"Incorrect script name";
const wchar_t *INITERR = L"Variable initializing error";
const wchar_t *SYNTAXERR = L"Syntax error";
const wchar_t *UNKVARNAME = L"Unknown variable";
const wchar_t *VARNAMEERR = L"Incorrect variable name";
const wchar_t *UNKERR = L"Unknown error";
const wchar_t *NUMERR = L"Can't convert to number type";
const wchar_t *STRERR = L"Can't convert to wchar_tacter type";
const wchar_t *LREXPRERR = L"Left and right values of expression is not equal";
const wchar_t *FNNAMERR = L"Unknown function or method";
const wchar_t *FNARGERR = L"Error initializing argument(s) of function or method";
const wchar_t *FNARGCNTERR = L"Wrong count of function arguments";
const wchar_t *PROCNAMERR = L"Unknown procedure";
const wchar_t *PROCARGCNTERR = L"Wrong count of procedure arguments";
const wchar_t *PROCARGERR = L"Error initializing argument(s) of procedure";
const wchar_t *PROCNOCRT = L"Can't create procedure";
const wchar_t *PROCNODEL = L"Can't drop procedure";
const wchar_t *NUMCONSLERR = L"Incorrect operation with number type";
const wchar_t *STRCONSLERR = L"Incorrect operation with wchar_tacter type";
const wchar_t *ILLGLOPER = L"Illegal operation with data type";
const wchar_t *FRGMNTERR = L"Fragment compiling error";
const wchar_t *COUNTERR = L"Using negative value in counter";
const wchar_t *OBJINDERR = L"Incorrect index of object in stack";
const wchar_t *OBJNAMEERR = L"Incorrect object name";
const wchar_t *OBJNONE = L"Object not found";
const wchar_t *OBJNOPROP = L"Object doesn't contain selected property";
const wchar_t *OBJNOCRT = L"Can't create object";
const wchar_t *OBJNOCRTNAME = L"Can't compare object name";
const wchar_t *OBJNOCRTPROP = L"Can't compare property name";
const wchar_t *OBJMEMNOTACC = L"Member of object is not accessible";
const wchar_t *OBJNODESTR = L"Can't drop object";
const wchar_t *OBJPROPERR = L"Can't create object property";
const wchar_t *ILLGLOBJOPER = L"Illegal operation with object";
const wchar_t *OBJPROPDUP = L"Duplicate object property initialization";
const wchar_t *OBJNOCTOR = L"Constructor is not found in object class";
const wchar_t *CLNONE = L"Class not found";
const wchar_t *CLNODESTR = L"Can't drop class";
const wchar_t *CLNOPROP = L"Class doesn't contain selected property";
const wchar_t *CLNOPROPREM = L"Can't drop class property";
const wchar_t *CLDUP = L"Duplicate class initialization";
const wchar_t *CLPROPDUP = L"Duplicate class property initialization";
const wchar_t *CLMETHDUP = L"Duplicate class method initialization";
const wchar_t *CLNOPUBPROP = L"Parent class doesn't contain public properties";
const wchar_t *CLNOPUBMETH = L"Parent class doesn't contain public methods";
const wchar_t *PARAMERR = L"Error while getting parameter from stack";
const wchar_t *SELPRMERR = L"Error in 'select' - constant value";
const wchar_t *NOWHENPRM = L"Error in 'select' - no 'when'";
const wchar_t *NOTHENERR = L"Error in 'when' - no 'then'";
const wchar_t *NOPCCODE = L"Valuie isn't precompiled fragment";
const wchar_t *UNKDECR = L"Unknown decoration";
//----------------------------------------

//обозначения внутренних типов данных
enum {
       SCNUM = 1,
       SCSTR = 2
     };

const wchar_t *FRMTNUM = L"%.3f";     //представление числа при выводе в строку (точность числа)
const wchar_t *ENDLNSTR = L";";       //признак конца строки
const wchar_t *CYCLESEP = L",";       //разделитель выражений в условии цикла
const wchar_t *STRCONS = L"^";        //знак конкатенации строк (заменяет собой + в выражениях)
const wchar_t *NUMTYPE = L"num";      //признак числового типа
const wchar_t *STRTYPE = L"sym";      //признак строкового типа
const wchar_t *ERROUT = L"-err-";     //признак ошибки (возвращается из строковых ф-й)
const wchar_t *CSTRF = L"$CSTR[%d]";  //шаблон имени конст. строки
const wchar_t *TMPOBJF = L"%c___%d";  //шаблон имени временного объекта
const wchar_t *EXTPRMF = L"$GET[%d]"; //шаблон имени для переменной, содержащей вх. параметр
const wchar_t *EXTPRMNM = L"INPRM%d"; //шаблон имени параметра стека для хранения вх. параметра
const wchar_t *FRGMARK = L"@";        //символ метки фрагмента
const wchar_t *FRGMNTF = L"@%d;";     //шаблон имени для маркера фрагмента кода
const wchar_t *EXTPRMSEPS = L"|";     //разделитель строки входящих параметров для скрипта
const wchar_t *OBJPROPSEPSTR = L".";  //символ-признак свойства объекта
const wchar_t *OBJPROC = L"procedure";//имя категории объекта для хранения процедур
const wchar_t *OBJPROCPRM = L"params";//имя свойства объекта для хранения параметров процедур
const wchar_t *OBJPROCTXT = L"text";  //имя свойства объекта для хранения тела процедур
const wchar_t *CLMETHOD = L"method-private";  //имя категории объекта для хранения метода класса
const wchar_t *CLPROP = L"property-private";  //имя категории объекта для хранения свойства класса
const wchar_t *CLPUBMETHOD = L"method-public";  //то же, но для публичного метода класса
const wchar_t *CLPUBPROP = L"property-public";  //то же, но для публичного свойства класса
bool use_proc = false;

#define STRBUFSTACK 4096   //макс. размер строкового буфера для экспорта содержимого стеков
#define CHARSIZE 4096      //максимальная длина строкового типа в скрипте
#define NUMSIZE 24         //максимальное кол-во символов, описывающее число
#define MAXNAMELEN 32      //максимальная длина имени переменной
#define VARSYM '$'         //признак переменной
#define REFSYM '?'         //признак декорации
#define FUNCSYM '_'        //признак имени функции
#define OBJSYM '&'         //признак имени объекта
#define PROCSYM ':'        //признак имени процедуры
#define STRSYM wchar_t(39) //признак строкового типа (')
#define OBJPROPSEP '.'     //символ-признак свойства объекта
#define NEGNUM '!'         //признак отрицательного числа
#define ENDLN ';'          //признак конца строки
#define STRCCONS '^'       //знак конкатенации строк (заменяет собой + в выражениях)
#define COMSYM '/'         //символ комментария
#define EXTPRMSEP '|'      //разделитель строки входящих параметров для скрипта
//----------------------------------------

//имена служебных параметров
const wchar_t *P_IND = L"sCurrentLine";   //индекс строки скрипта
const wchar_t *P_OBJNAME = L"sObjName";   //имя объекта с которым произв. операция
const wchar_t *P_PROPNAME = L"sPropName"; //имя свойства объекта с которым произв. операция
const wchar_t *P_SCRNAME = L"sScrName";   //имя скрипта
const wchar_t *P_SELECT = L"sSelectVal";  //значение полученное конструкцией select

//структура описывающая ассоциации скриптовых переменных
//и элементов стеков интерпретатора
struct VARIABLE
{
  wchar_t varname[MAXNAMELEN]; //имя переменной из скрипта
  UINT type;                //тип стека: 1 - num, 2 - String
  UINT ind;                 //индекс в стеке
  bool isfree;              //признак того действующая переменная или нет
};
//--------------------------------------------------------------------------------------------

//структура описывающая ссылку
struct REFERENCE
{
  wchar_t refname[MAXNAMELEN];
  wchar_t refobj[CHARSIZE];
};
//--------------------------------------------------------------------------------------------

class VARSTACK
{
  public:
    inline VARSTACK(){};
    inline virtual ~VARSTACK(){ClearStack();}

inline VARIABLE *GetFirstFree(UINT type)
{
  for (UINT i = 0; i < stMain.size(); i++)
    {
      if (stMain[i].isfree && stMain[i].type == type)
        return &stMain[i];
    }

  return NULL;
}
//--------------------------------------------------------------------------------------------

inline VARIABLE *Get(const wchar_t *varname)
{
  for (UINT i = 0; i < stMain.size(); i++)
    {
      if ((0 == wcscmp(stMain[i].varname, varname)) && !stMain[i].isfree)
        return &stMain[i];
    }

  return NULL;
}
//-------------------------------------------------------------------------------------------------

inline VARIABLE *GetByValue(std::wstring val)
{
  for (UINT i = 0; i < stStr.size(); i++)
    {
      if (stStr.at(i) == val)
        {
          for (UINT j = 0; j < stMain.size(); j++)
            {
              if ((stMain[j].ind == i) && (2 == stMain[j].type))
                return &stMain[j];
            }
        }
    }

  return NULL;
}
//-------------------------------------------------------------------------------------------------

inline VARIABLE *GetByValue(float val)
{
  for (UINT i = 0; i < stNum.size(); i++)
    {
      if (stNum.at(i) == val)
        {
          for (UINT j = 0; j < stMain.size(); j++)
            {
              if (stMain.at(j).ind == i)
                return &stMain.at(i);
            }
        }
    }

  return NULL;
}
//-------------------------------------------------------------------------------------------------

inline bool Add(wchar_t *name, std::wstring val)
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
//-------------------------------------------------------------------------------------------------

inline bool Add(wchar_t *name, float val)
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
//-------------------------------------------------------------------------------------------------

inline bool Remove(const wchar_t *name)
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
//-------------------------------------------------------------------------------------------------

inline void ClearStack()
{
  stMain.clear();
  stNum.clear();
  stStr.clear();
}
//-------------------------------------------------------------------------------------------------
inline float GetNumElement(VARIABLE *var){return stNum[var->ind];}

inline void SetNumElement(VARIABLE *var, float val){stNum[var->ind] = val;}

inline std::wstring GetStrElement(VARIABLE *var){return stStr[var->ind];}

inline void SetStrElement(VARIABLE *var, std::wstring val){stStr[var->ind] = val;}

//-------------------------------------------------------------------------------------------------

inline const wchar_t *ShowInString()
{
  wchar_t frmt[512];
  static std::wstring res;
  res = L"";
  wchar_t out[4096];

  wcscpy(frmt, L"[%d] Stack[%d] %s %s = ");
  wcscat(frmt, FRMTNUM);
  wcscat(frmt, L"\r\n");

  for (UINT i = 0; i < stMain.size(); i++)
     {
       if (!stMain[i].isfree)
         {
           if (SCNUM == stMain[i].type)
             swprintf(out, frmt,
                                i,
                                stMain[i].ind,
                                NUMTYPE,
                                stMain[i].varname,
                                stNum[stMain[i].ind]);
           else if (SCSTR == stMain[i].type && !stMain[i].isfree)
			   swprintf(out, L"[%d] Stack[%d] %s %s = %s\r\n",
                                                        i,
                                                        stMain[i].ind,
                                                        STRTYPE,
                                                        stMain[i].varname,
                                                        stStr[stMain[i].ind].c_str());

		   res += out;
         }
     }

  return res.c_str();
}
//-------------------------------------------------------------------------------------------------

  private:
    std::vector<VARIABLE> stMain;
    std::vector<float> stNum;
	std::vector<std::wstring> stStr;
};
//-------------------------------------------------------------------------------------------------

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
  public:
    FRAGMENTCODE(const wchar_t *mark, SCRIPTLINES *code, bool global)
    {
      fMark = mark;
      fCode = *code;
      fGlobal = global;
    }
    FRAGMENTCODE(){};
    inline virtual ~FRAGMENTCODE(){fCode.clear();}

	inline void SetMark(const wchar_t *new_mark){fMark += new_mark;}
    inline const wchar_t *GetMark(){return fMark.c_str();}
    inline void SetGlobal(bool val){fGlobal = val;}
    inline bool IsGlobal(){return fGlobal;}
    inline SCRIPTLINES *GetCode(){return &fCode;};
    inline const wchar_t *GetCodeStrings()
	{
	  static String res;
	  res = "";

      for (UINT i = 0; i < fCode.size(); i++)
		 {res += _wltrim(fCode[i].c_str()); res += L"\r\n";}

	  return res.c_str();
    }

  private:
    SCRIPTLINES fCode;
	std::wstring fMark;
    bool fGlobal;
};


class FRAGMENTSTACK
{
  public:
      FRAGMENTSTACK(){};
      inline virtual ~FRAGMENTSTACK(){};

inline void Add(std::wstring frg_str, std::wstring mark, bool global)
{
  FRAGMENTCODE *new_frg = new FRAGMENTCODE();

  new_frg->SetMark(mark.c_str());
  new_frg->SetGlobal(global);
  StrToListW(new_frg->GetCode(), frg_str, ENDLNSTR, DELIMEND);

  Stack.push_back(new_frg);
}
//-------------------------------------------------------------------------------------------------

inline void Remove(std::wstring mark)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (wcscmp(mark.c_str(), Stack[i]->GetMark()) == 0)
        {
          delete Stack[i];
          Stack[i] = NULL;
          Stack.erase(Stack.begin() + i);
        }
    }
}
//-------------------------------------------------------------------------------------------------

inline SCRIPTLINES *GetFragmentCode(std::wstring mark)
{
  for (UINT i = 0; i < Stack.size(); i++)
	{
      if (wcscmp(mark.c_str(), Stack[i]->GetMark()) == 0)
        {
          if (!use_proc)
            Stack[i]->SetGlobal(false);

          return Stack[i]->GetCode();
        }
    }

  return NULL;
}
//-------------------------------------------------------------------------------------------------

inline FRAGMENTCODE *Get(std::wstring mark)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (wcscmp(mark.c_str(), Stack[i]->GetMark()) == 0)
        return Stack[i];
    }

  return NULL;
}
//-------------------------------------------------------------------------------------------------

void ClearFragments(bool all)
{
  if (all)
    {
      for (UINT i = 0; i < Stack.size(); i++)
        {
          delete Stack[i];
          Stack[i] = NULL;
        }

      Stack.clear();
    }
  else
    {
	  for (UINT i = 0; i < Stack.size(); i++)
        {
          if (!Stack[i]->IsGlobal())
            {
              delete Stack[i];
              Stack[i] = NULL;
              Stack.erase(Stack.begin() + i);
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------

inline const wchar_t *ShowInString()
{
  wchar_t frmt[512];
  static std::wstring res;
  res = L"";

  for (UINT i = 0; i < Stack.size(); i++)
     {
       swprintf(frmt, L"[%d] {%s} GLOBAL = %d TEXT = \r\n%s\r\n",
               i,
               Stack[i]->GetMark(),
               (UINT)Stack[i]->IsGlobal(),
               Stack[i]->GetCodeStrings());
       res += frmt;
     }

  return res.c_str();
}
//-------------------------------------------------------------------------------------------------

  private:
	std::vector<FRAGMENTCODE*> Stack;
    UINT fSize;
};

#endif // DEFINES_H_INCLUDED
