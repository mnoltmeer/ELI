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

#ifndef FN_PRM_H_INCLUDED
#define FN_PRM_H_INCLUDED

#include <stdio.h>
#include <vector>
#include <iterator>
#include <string.h>
#include <cstdlib>

#define CHARSIZE 4096    //������������ ����� ���������� ���� � �������
#define STRBUFSTACK 4096 //����. ������ ���������� ������ ��� �������� ����������� ������
#define FNAMESIZE 32     //������������ ����� ����� �-��
#define PNAMESIZE 16     //������������ ����� ����� ���������
#define UINT unsigned int

const wchar_t *S_NUM = L"num";
const wchar_t *S_SYM = L"sym";
const wchar_t *FARGSSEP = L",";     //����������� � �������� ���������� �-��

struct EXTFUNC
{
  wchar_t inname[MAXNAMELEN]; //���������� ���
  HINSTANCE exthinst;      //���������� ��. ����
};

class PARAM
{
  public:
    inline PARAM(const wchar_t* p_name, const wchar_t *p_value)
      {
        name = p_name;
        Set(p_value);
      }

    inline virtual ~PARAM(){};

	std::wstring name; //��� ���������

void Set(const wchar_t *new_val){val = new_val;}

//����������� �������� � integer � ���������� ���
int ToInt()
{
  UINT pos = val.find(L".");

  if (pos != std::wstring::npos)
	val.erase(pos, val.length() - pos);

  return _wtoi(val.c_str());
}
//--------------------------------------------------------------------------------------------

//����������� �������� � float � ���������� ���
float ToFloat(){return _wtof(val.c_str());}
//--------------------------------------------------------------------------------------------

//����������� �������� � ������ � ���������� ��������� �� ���
//� ������ ������ ���������� NULL
const wchar_t *ToStr(){return val.c_str();}

  private:
    std::wstring val; //��������
};
//--------------------------------------------------------------------------------------------

class PARAMSTACK
{
  public:
    inline PARAMSTACK(){};
    inline virtual ~PARAMSTACK(){Clear();}

//��������� �������� � ����� �����
void Add(const wchar_t *name, const wchar_t *val)
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

//���������� ���������� ���������� � �����
UINT Count(){return Stack.size();}

//���������� ��������������� ������ �� ������� ���� �-�
wchar_t *ShowInString()
{
  static wchar_t output[STRBUFSTACK];
  UINT cnt = 0;

  for (UINT i = 0; i < Stack.size(); i++)
    {
      cnt += swprintf(output + cnt, L"[%d] %s = %s\r\n",
                    i,
                    Stack[i]->name.c_str(),
                    Stack[i]->ToStr());
    }

  return output;
}
//--------------------------------------------------------------------------------------------

inline PARAM *Get(const wchar_t *name)
{
  int ind = GetParamInd(name);

  if (ind < 0)
    return NULL;
  else
    return Stack[ind];
}
//--------------------------------------------------------------------------------------------

//������� ���� ����������
inline void Clear()
{
  for (UINT i = 0; i < Stack.size(); i++)
     {
        delete Stack[i];
        Stack[i] = NULL;
     }

  Stack.clear();
}
//--------------------------------------------------------------------------------------------

  private:
    std::vector<PARAM*> Stack;

//���� �������� � ����� �� ����� � ���������� ������
//-1 - �� �������
inline int GetParamInd(const wchar_t *name)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == _wcsicmp(Stack[i]->name.c_str(), name))
        return i;
    }

  return -1;
}
//--------------------------------------------------------------------------------------------
};
//--------------------------------------------------------------------------------------------

class FUNC
{
  public:
    inline FUNC(const wchar_t *name, const wchar_t *params, func_ptr fptr)
    {
	  this->name = new wchar_t[FNAMESIZE];
      this->prms_format = new wchar_t[CHARSIZE];
      return_val = new wchar_t[CHARSIZE];
      wcscpy(this->name, name);
      this->ptr = fptr;
      wcscpy(this->prms_format, params);
    }

    inline virtual ~FUNC()
    {
      delete [] name;
      name = NULL;
      delete [] prms_format;
      prms_format = NULL;
      delete [] return_val;
      return_val = NULL;
      ptr = NULL;
    }

    inline wchar_t *GetName(){return name;}

    inline wchar_t *GetParams(){return prms_format;}

    inline func_ptr GetPointer(){return ptr;}

//�������� �������, ���������� 1 � ������ ������
	inline void Call(void *e_ptr){ptr(e_ptr);}

//����������� ������������ �������� �-�� name � ������ � ���������� ��������� �� ���
//� ������ ������ ���������� NULL
    inline wchar_t *GetResult(){return return_val;}

//������������� ������������ �������� �-��
    inline void SetResult(const wchar_t* result){wcscpy(return_val, result);}

  private:
    wchar_t *name;        //��� �-��
    func_ptr ptr;      //��������� �� �-� �� ������
    wchar_t *prms_format; //����� ������������ ����� � ��� ���������� �-�
                       //num val,sym val
    wchar_t *return_val;  //������������ ��������

};
//--------------------------------------------------------------------------------------------

class FUNCSTACK
{
  public:
      inline FUNCSTACK(){};
      inline virtual ~FUNCSTACK()
      {
        for (UINT i = 0; i < Stack.size(); i++)
           {
             delete Stack[i];
             Stack[i] = NULL;
           }

        Stack.clear();
      }

//���������� ���������� ������� � �����
inline UINT CountFuncs(){return Stack.size();};

//���������� ��������� �� �������� ������� � �������� st_ind �� �����
//� ������ ������ ���������� NULL
inline FUNC *Get(UINT st_ind)
{
  if (st_ind >= Stack.size())
    return NULL;
  else
    return Stack[st_ind];
};

//���������� ��������� �� �������� ������� name �� �����
//� ������ ������ ���������� NULL
inline FUNC *Get(const wchar_t *fname)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == _wcsicmp(Stack[i]->GetName(), fname))
        return Stack[i];
    }

  return NULL;
};

//��������� �-� � ����
void Add(const wchar_t *name, const wchar_t *params, func_ptr fptr)
{
  if (!Get(name))
    {
      FUNC *fn = new FUNC(name, params, fptr);
      Stack.push_back(fn);
    }
}

//������� �-� �� �����
void Delete(const wchar_t *name)
{
  int del_ind = GetInd(name);

  if (del_ind > -1)
    {
      delete Stack[del_ind];
      Stack[del_ind] = NULL;
      Stack.erase(Stack.begin() + del_ind);
    }
}

//���������� ��������������� ������ �� ������� ���� �-�
wchar_t *ShowInString()
{
  static wchar_t output[STRBUFSTACK];
  UINT cnt = 0;

  for (UINT i = 0; i < Stack.size(); i++)
    {
      cnt += swprintf(output + cnt, L"[%d] %s(%s), ptr = %d\r\n",
									i,
									Stack[i]->GetName(),
									Stack[i]->GetParams(),
                    				(int)Stack[i]->GetPointer());
    }

  return output;
}

  private:
      std::vector<FUNC*> Stack;

//���� �-� � �����, ���������� �� ������
//���������� -1 � ������ �������
inline int GetInd(const wchar_t *fname)
{
  for (UINT i = 0; i < Stack.size(); i++)
    {
      if (0 == _wcsicmp(Stack[i]->GetName(), fname))
        return i;
    }

  return -1;
}

};
//--------------------------------------------------------------------------------------------

#endif // FN_PRM_H_INCLUDED
