#include "fn_prm.h"

PARAM::PARAM(const char* p_name, const char *p_value)
{
  name = p_name;
  val = p_value;

  int pos = val.find(".");

  if (val.substr(pos + 1, 2) == "00")
    val.erase(pos, val.length() - pos);
}
//---------------------------------------------------------------------------

void PARAM::Set(const char *new_val)
{
  val = new_val;

  int pos = val.find(".");

  if (val.substr(pos + 1, 2) == "00")
    val.erase(pos, val.length() - pos);
}
//---------------------------------------------------------------------------

int PARAM::ToInt(){return atoi(val.c_str());}
float PARAM::ToFloat(){return atof(val.c_str());}
const char *PARAM::ToStr(){return val.c_str();}

void PARAMSTACK::Add(const char *name, const char *val)
{
  int ind = GetParamInd(name);

  if (ind < 0)
    {
      PARAM *new_p = new PARAM(name, val);

      Stack.push_back(new_p);
    }
  else
    {
      Stack[ind]->Set(val);
    }
}
//--------------------------------------------------------------------------------------------

UINT PARAMSTACK::Count(){return Stack.size();}
void PARAMSTACK::Clear(){Stack.clear();}

char *PARAMSTACK::ShowInStrings()
{
  static char output[STRBUFSTACK];
  UINT cnt = 0;

  for (UINT i = 0; i < Stack.size(); i++)
    {
      cnt += sprintf(output + cnt, "[%d] %s = %s\n",
                    i,
                    Stack[i]->name.c_str(),
                    Stack[i]->ToStr());
    }

  return output;
}
//--------------------------------------------------------------------------------------------

PARAM *PARAMSTACK::Get(const char *name)
{
  int ind = GetParamInd(name);

  if (ind < 0)
    return NULL;
  else
    return Stack[ind];
}
//--------------------------------------------------------------------------------------------

inline int PARAMSTACK::GetParamInd(const char *name)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == stricmp(Stack[i]->name.c_str(), name))
        return i;
    }

  return -1;
}
//--------------------------------------------------------------------------------------------

void PARAMSTACK::Delete()
{
  for (UINT i = 0; i < Stack.size(); i++)
     {
        delete Stack[i];
        Stack[i] = NULL;
     }
}
//--------------------------------------------------------------------------------------------

FUNC::FUNC(const char *name, const char *params, func_ptr fptr)
{
  strcpy(this->name, name);
  this->ptr = fptr;
  strcpy(this->prms_format, params);
}
//--------------------------------------------------------------------------------------------

char *FUNC::GetName(){return this->name;}
char *FUNC::GetParams(){return this->prms_format;}
func_ptr FUNC::GetPointer(){return this->ptr;}
void FUNC::Call(PARAMSTACK *pStack){this->ptr(pStack);}
char *FUNC::GetResult(){return this->return_val;}
void FUNC::SetResult(const char* result){strcpy(return_val, result);}

FUNCSTACK::FUNCSTACK()
{

}
//--------------------------------------------------------------------------------------------

FUNCSTACK::~FUNCSTACK()
{
  for (UINT i = 0; i < Stack.size(); i++)
     {
        delete Stack[i];
        Stack[i] = NULL;
     }

  Stack.clear();
}
//--------------------------------------------------------------------------------------------

UINT FUNCSTACK::CountFuncs(){return Stack.size();}

FUNC *FUNCSTACK::Get(UINT st_ind)
{
  if (st_ind >= Stack.size())
    return NULL;
  else
    return Stack[st_ind];
};
//--------------------------------------------------------------------------------------------

FUNC *FUNCSTACK::Get(const char *fname)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == stricmp(Stack[i]->GetName(), fname))
        return Stack[i];
    }

  return NULL;
}
//--------------------------------------------------------------------------------------------

void FUNCSTACK::Add(const char *name, const char *params, func_ptr fptr)
{
  FUNC *fn = new FUNC(name, params, fptr);

  Stack.push_back(fn);
}
//--------------------------------------------------------------------------------------------

void FUNCSTACK::DeleteFunc(const char *name)
{
  int del_ind = GetInd(name);

  if (del_ind > -1)
    {
      delete Stack[del_ind];
      Stack[del_ind] = NULL;
      Stack.erase(Stack.begin() + del_ind);
    }
}
//--------------------------------------------------------------------------------------------

char *FUNCSTACK::ShowInString()
{
  static char output[STRBUFSTACK];
  UINT cnt = 0;

  for (UINT i = 0; i < Stack.size(); i++)
    {
      cnt += sprintf(output + cnt, "[%d] %s(%s), ptr = %d\n",
                    i,
                    Stack[i]->GetName(),
                    Stack[i]->GetParams(),
                    (int)Stack[i]->GetPointer());
    }

  return output;
}
//--------------------------------------------------------------------------------------------

int FUNCSTACK::GetInd(const char *fname)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == stricmp(Stack[i]->GetName(), fname))
        return i;
    }

  return -1;
}
//--------------------------------------------------------------------------------------------
