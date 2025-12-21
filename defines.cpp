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

#include "defines.h"

const wchar_t *INFMSG = L"INFO";
const wchar_t *ERRMSG = L"ERROR";
const wchar_t *WRNMSG = L"WARNING";
const wchar_t *TRANSLATED = L"End script";
const wchar_t *SCEXIT = L"Used _return()";
const wchar_t *SCEXCEPT = L"Throw exception";
const wchar_t *SCSTP = L"Force stop";
const wchar_t *SCEND = L"Finishing with #end";
const wchar_t *SCNOEND = L"Finishing without #end";
const wchar_t *NOBEGIN = L"Error initialising script - no #begin";
const wchar_t *NOEND = L"Error initialising script - no #end";
const wchar_t *NONAME = L"Script name is not defined";
const wchar_t *ERRNAME = L"Incorrect script name";
const wchar_t *INITERR = L"Variable initialising error";
const wchar_t *SYNTAXERR = L"Syntax error";
const wchar_t *UNKVARNAME = L"Unknown variable";
const wchar_t *VARNAMEERR = L"Incorrect variable name";
const wchar_t *UNKERR = L"Unknown error";
const wchar_t *NUMERR = L"Can't convert to number type";
const wchar_t *STRERR = L"Can't convert to character type";
const wchar_t *LREXPRERR = L"Left and right values of expression is not equal";
const wchar_t *FNPTRERR = L"Invalid pointer to function";
const wchar_t *FNNAMERR = L"Unknown function or method";
const wchar_t *FNARGERR = L"Error initialising argument(s) of function or method";
const wchar_t *FNARGCNTERR = L"Wrong count of function arguments";
const wchar_t *FNEMPTYRES = L"Function returns empty value!";
const wchar_t *PROCNAMERR = L"Unknown procedure";
const wchar_t *PROCARGCNTERR = L"Wrong count of procedure arguments";
const wchar_t *PROCARGERR = L"Error initialising argument(s) of procedure";
const wchar_t *PROCNOCRT = L"Can't create procedure";
const wchar_t *PROCNODEL = L"Can't drop procedure";
const wchar_t *NUMCONSLERR = L"Incorrect operation with number type";
const wchar_t *STRCONSLERR = L"Incorrect operation with character type";
const wchar_t *ILLGLOPER = L"Illegal operation with data type";
const wchar_t *FRGMNTERR = L"Fragment translating error";
const wchar_t *COUNTERR = L"Using negative value in counter";
const wchar_t *OBJINDERR = L"Incorrect index of object in stack";
const wchar_t *OBJNAMEERR = L"Incorrect object name";
const wchar_t *OBJNONE = L"Object not found";
const wchar_t *OBJNOPROP = L"Object doesn't contain selected property";
const wchar_t *OBJNOCRT = L"Can't create object";
const wchar_t *OBJNOCRTNAME = L"Can't compare object name";
const wchar_t *OBJNOCRTPROP = L"Can't compare property name";
const wchar_t *OBJMEMNOTACC = L"Object member is not accessible";
const wchar_t *OBJNODESTR = L"Can't drop object";
const wchar_t *OBJPROPERR = L"Can't create object property";
const wchar_t *ILLGLOBJOPER = L"Illegal operation with object";
const wchar_t *OBJPROPDUP = L"Duplicate object property initialisation";
const wchar_t *OBJNOCTOR = L"Constructor is not found in object class";
const wchar_t *OBJMEMIMPERR = L"Error importing object member";
const wchar_t *CLNONE = L"Class not found";
const wchar_t *CLNODESTR = L"Can't drop class";
const wchar_t *CLNOPROP = L"Class doesn't contain selected property";
const wchar_t *CLNOPROPREM = L"Can't drop class property";
const wchar_t *CLDUP = L"Duplicate class initialisation";
const wchar_t *CLPROPDUP = L"Duplicate class property initialisation";
const wchar_t *CLMETHDUP = L"Duplicate class method initialisation";
const wchar_t *CLNOPUBPROP = L"Parent class doesn't contain public properties";
const wchar_t *CLNOPUBMETH = L"Parent class doesn't contain public methods";
const wchar_t *CLMEMNOTACC = L"Class member is not accessible";
const wchar_t *CLMEMIMPERR = L"Error importing class member";
const wchar_t *PARAMERR = L"Error while getting parameter from stack";
const wchar_t *SELPRMERR = L"Error in 'select' - constant value";
const wchar_t *NOWHENPRM = L"Error in 'select' - no 'when'";
const wchar_t *NOTHENERR = L"Error in 'when' - no 'then'";
const wchar_t *NOPCCODE = L"Value isn't pretranslated fragment";
const wchar_t *UNKDECR = L"Unknown decoration";
const wchar_t *TRGCRTERR = L"Error while creating trigger";
const wchar_t *TRGREMERR = L"Error while removing trigger";
const wchar_t *TRGRUNERR = L"Error while running trigger";
const wchar_t *UNKTRG = L"Unknown trigger";
const wchar_t *TRGEXST = L"Trigger already exist";
const wchar_t *STADDERR = L"Error adding data to stack";
const wchar_t *STREMERR = L"Error removing data from stack";
const wchar_t *CHSETERR = L"Error changing interpreter settings";

const wchar_t *FRMTNUM = L"%.3f";
const wchar_t *ENDLNSTR = L";";
const wchar_t *CYCLESEP = L",";
const wchar_t *STRCONS = L"^";
const wchar_t *NUMTYPE = L"num";
const wchar_t *STRTYPE = L"sym";
const wchar_t *ERROUT = L"-err-";
const wchar_t *CSTRF = L"$CSTR[%d]";
const wchar_t *CNUMF = L"$CNUM[%d]";
const wchar_t *TMPOBJF = L"%c_%d";
const wchar_t *EXTPRMF = L"$GET[%d]";
const wchar_t *EXTPRMNM = L"INPRM%d";
const wchar_t *FRGMARK = L"@";
const wchar_t *FRGMNTF = L"@%d;";
const wchar_t *EXTPRMSEPS = L"|";
const wchar_t *OBJPROPSEPSTR = L".";
const wchar_t *OBJPROC = L"procedure";
const wchar_t *OBJPROCPRM = L"params";
const wchar_t *OBJPROCTXT = L"text";
const wchar_t *CLMETHOD = L"private-method";
const wchar_t *CLPROP = L"private-property";
const wchar_t *CLPUBMETHOD = L"public-method";
const wchar_t *CLPUBPROP = L"public-property";
const wchar_t *DTORSYMB = L"~";
const wchar_t *S_NUM = L"num";
const wchar_t *S_SYM = L"sym";
const wchar_t *FARGSSEP = L",";
const wchar_t *OBJTHIS = L"$this";

wchar_t STRSYM = wchar_t(34);

const wchar_t *TROK = L"+";
const wchar_t *TRFAIL = L"x";
const wchar_t *TRINFO = L"!";

const wchar_t *P_IND = L"sCurrentLine";
const wchar_t *P_OBJNAME = L"sObjName";
const wchar_t *P_PROPNAME = L"sPropName";
const wchar_t *P_SCRNAME = L"sScrName";
const wchar_t *P_SELECT = L"sSelectVal";
const wchar_t *P_ELI_VER = L"sELIVersion";
const wchar_t *P_ELI_PATH = L"sELIPath";
const wchar_t *P_ELI_DIR = L"sELIWorkDir";
const wchar_t *P_ELI_HANDLE = L"sELIHandle";
