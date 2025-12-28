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

#ifndef definesH
#define definesH

#include <wtypes.h>
#include <vector>

/*! efstring.h copyright 2016-2018 Elsa Fenrich (elsa.fenrich@gmail.com) */
#include "..\work-functions\efstring.h"

#define BUILD_DLL

#include "eli_interface.h"

//константы, описывающие системные сообщения
extern const wchar_t *INFMSG;
extern const wchar_t *ERRMSG;
extern const wchar_t *WRNMSG;
extern const wchar_t *TRANSLATED;
extern const wchar_t *SCEXIT;
extern const wchar_t *SCEXCEPT;
extern const wchar_t *SCSTP;
extern const wchar_t *SCEND;
extern const wchar_t *SCNOEND;
extern const wchar_t *NOBEGIN;
extern const wchar_t *NOEND;
extern const wchar_t *NONAME;
extern const wchar_t *ERRNAME;
extern const wchar_t *INITERR;
extern const wchar_t *SYNTAXERR;
extern const wchar_t *UNKVARNAME;
extern const wchar_t *VARNAMEERR;
extern const wchar_t *UNKERR;
extern const wchar_t *NUMERR;
extern const wchar_t *STRERR;
extern const wchar_t *LREXPRERR;
extern const wchar_t *FNPTRERR;
extern const wchar_t *FNNAMERR;
extern const wchar_t *FNARGERR;
extern const wchar_t *FNARGCNTERR;
extern const wchar_t *FNEMPTYRES;
extern const wchar_t *PROCNAMERR;
extern const wchar_t *PROCARGCNTERR;
extern const wchar_t *PROCARGERR;
extern const wchar_t *PROCNOCRT;
extern const wchar_t *PROCNODEL;
extern const wchar_t *NUMCONSLERR;
extern const wchar_t *STRCONSLERR;
extern const wchar_t *ILLGLOPER;
extern const wchar_t *FRGMNTERR;
extern const wchar_t *COUNTERR;
extern const wchar_t *OBJINDERR;
extern const wchar_t *OBJNAMEERR;
extern const wchar_t *OBJNONE;
extern const wchar_t *OBJNOPROP;
extern const wchar_t *OBJNOCRT;
extern const wchar_t *OBJNOCRTNAME;
extern const wchar_t *OBJNOCRTPROP;
extern const wchar_t *OBJMEMNOTACC;
extern const wchar_t *OBJNODESTR;
extern const wchar_t *OBJPROPERR;
extern const wchar_t *ILLGLOBJOPER;
extern const wchar_t *OBJPROPDUP;
extern const wchar_t *OBJNOCTOR;
extern const wchar_t *OBJMEMIMPERR;
extern const wchar_t *CLNONE;
extern const wchar_t *CLNODESTR;
extern const wchar_t *CLNOPROP;
extern const wchar_t *CLNOPROPREM;
extern const wchar_t *CLDUP;
extern const wchar_t *CLPROPDUP;
extern const wchar_t *CLMETHDUP;
extern const wchar_t *CLNOPUBPROP;
extern const wchar_t *CLNOPUBMETH;
extern const wchar_t *CLMEMNOTACC;
extern const wchar_t *CLMEMIMPERR;
extern const wchar_t *PARAMERR;
extern const wchar_t *SELPRMERR;
extern const wchar_t *NOWHENPRM;
extern const wchar_t *NOTHENERR;
extern const wchar_t *NOPCCODE;
extern const wchar_t *UNKDECR;
extern const wchar_t *TRGCRTERR;
extern const wchar_t *TRGREMERR;
extern const wchar_t *TRGRUNERR;
extern const wchar_t *UNKTRG;
extern const wchar_t *TRGEXST;
extern const wchar_t *STADDERR;
extern const wchar_t *STREMERR;
//-------------------------------------------------------------------------------

//обозначения внутренних типов данных
enum {
       SCNUM = 1,
       SCSTR = 2
     };

extern const wchar_t *FRMTNUM; //представление числа при выводе в строку (точность числа)
extern const wchar_t *ENDLNSTR; //признак конца строки
extern const wchar_t *CYCLESEP; //разделитель выражений в условии цикла
extern const wchar_t *STRCONS; //знак конкатенации строк (заменяет собой + в выражениях)
extern const wchar_t *NUMTYPE; //признак числового типа
extern const wchar_t *STRTYPE; //признак строкового типа
extern const wchar_t *ERROUT; //признак ошибки (возвращается из строковых ф-й)
extern const wchar_t *CSTRF; //шаблон имени конст. строки
extern const wchar_t *CNUMF; //шаблон імени числової константи
extern const wchar_t *TMPOBJF; //шаблон имени временного объекта
extern const wchar_t *EXTPRMF; //шаблон имени для переменной, содержащей вх. параметр
extern const wchar_t *EXTPRMNM; //шаблон имени параметра стека для хранения вх. параметра
extern const wchar_t *FRGMARK; //символ метки фрагмента
extern const wchar_t *FRGMNTF; //шаблон имени для маркера фрагмента кода
extern const wchar_t *EXTPRMSEPS; //разделитель строки входящих параметров для скрипта
extern const wchar_t *OBJPROPSEPSTR; //символ-признак свойства объекта
extern const wchar_t *OBJPROC; //имя категории объекта для хранения процедур
extern const wchar_t *OBJPROCPRM; //имя свойства объекта для хранения параметров процедур
extern const wchar_t *OBJPROCTXT; //имя свойства объекта для хранения тела процедур
extern const wchar_t *CLMETHOD; //имя категории объекта для хранения метода класса
extern const wchar_t *CLPROP; //имя категории объекта для хранения свойства класса
extern const wchar_t *CLPUBMETHOD; //то же, но для публичного метода класса
extern const wchar_t *CLPUBPROP; //то же, но для публичного свойства класса
extern const wchar_t *DTORSYMB; //символ що позначає деструктор
extern const wchar_t *S_NUM;
extern const wchar_t *S_SYM;
extern const wchar_t *FARGSSEP; //разделитель в описании параметров ф-ии
extern const wchar_t *OBJTHIS; //шаблон імені змінної ідентифікатора this для об'єктів

extern wchar_t STRSYM; //признак строкового типа (")

//маркування для логувань трансляції рядків
extern const wchar_t *TROK;    //успішно
extern const wchar_t *TRFAIL;  //помилка трансляції
extern const wchar_t *TRINFO;  //повідомлення про помилку/подію

#define STRBUFSTACK 4096   //макс. размер строкового буфера для экспорта содержимого стеков
#define CHARSIZE 8192      //максимальная длина строкового типа в скрипте
#define NUMSIZE 24         //максимальное кол-во символов, описывающее число
#define MAXNAMELEN 32      //максимальная длина имени переменной
#define VARSYM '$'         //признак переменной
#define REFSYM '?'         //признак декорации
#define FUNCSYM '_'        //признак имени функции
#define OBJSYM '&'         //признак имени объекта
#define PROCSYM ':'        //признак имени процедуры
#define OBJPROPSEP '.'     //символ-признак свойства объекта
#define NEGNUM '!'         //признак отрицательного числа
#define ENDLN ';'          //признак конца строки
#define STRCCONS '^'       //знак конкатенации строк (заменяет собой + в выражениях)
#define COMSYM '/'         //символ комментария
#define EXTPRMSEP '|'      //разделитель строки входящих параметров для скрипта
#define FNAMESIZE 32       //максимальная длина имени ф-ии
#define PNAMESIZE 24       //максимальная длина имени параметра
#define MAXBUF 4096        //макс. буфер для чтения строк из файла ресурсов
#define FIELDSDELIM L"|"   //разделитель значений полей в файле ресурсов
#define SAVEFIELDCOUNT 4   //кол-во полей структуры RESOURCE сохраняемых в файл
#define YES L"1"
#define NO L"0"
#define UINT unsigned int
//-------------------------------------------------------------------------------

//имена служебных параметров
extern const wchar_t *P_IND;   //индекс строки скрипта
extern const wchar_t *P_OBJNAME;   //имя объекта с которым произв. операция
extern const wchar_t *P_PROPNAME; //имя свойства объекта с которым произв. операция
extern const wchar_t *P_SCRNAME;   //имя скрипта
extern const wchar_t *P_SELECT;  //значение полученное конструкцией select
extern const wchar_t *P_ELI_VER; //версія інтерпретатора
extern const wchar_t *P_ELI_PATH; //шлях до бібліотеки
extern const wchar_t *P_ELI_DIR; //робочий каталог
extern const wchar_t *P_ELI_HANDLE; //дескриптор об'єкта ELI

typedef std::vector<std::wstring> (SCRIPTLINES);
typedef void (__stdcall *IMPORTFUNC)(void*);
typedef std::vector<std::wstring> (StrList);

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
  bool KeepObjects;   //зберігати вміст стеку об'єктів до кінця роботи ELI
  bool KeepClasses;   //зберігати вміст стеку класів до кінця роботи ELI
};
//-------------------------------------------------------------------------------

struct EXTFUNC
{
  wchar_t inname[MAXNAMELEN]; //внутреннее имя
  HMODULE exthinst;      //дескриптор вн. либы
};

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

#endif
