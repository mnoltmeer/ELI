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
#define MAXBUF 4096      //����. ����� ��� ������ ����� �� ����� ��������
#define FIELDSDELIM L"|"  //����������� �������� ����� � ����� ��������
#define SAVEFIELDCOUNT 4 //���-�� ����� ��������� RESOURCE ����������� � ����

#define YES L"1"
#define NO L"0"

//��������� ������������ �������� �������
enum {
       indx = 0,
       obj_cath = 1,
       obj_id = 2,
       prop_id = 3,
       val_v = 4,
       is_keep = 5,
       is_save = 6
     };

//��������� ������������ ��������� ��� ������� ��� �-� SelectRes() � Aquire()
struct CONDITION
{
  UINT type;
  std::wstring value;
};

struct RESOURCE
{
  UINT Index;                    //������
  std::wstring ObjectCathegory;  //��������� �������-���������
  std::wstring ObjectID;         //ID �������-���������
  std::wstring PropertyID;       //ID ��������
  std::wstring Value;            //��������
  std::wstring KeepInStack;      //������� ������ � ����� ����� ��������
  std::wstring SaveInFile;       //��������� �� �������� ������� � ���� �� �����
};

typedef std::vector<std::wstring> (StrList);
//����������� ��� ������ ���������� �� ������� �����
typedef std::vector<RESOURCE*> (RESRECORDSET);

class RESOURCESTACK
{
  private:
	UINT next_id;
	std::vector<RESOURCE> Stack;
	UINT GetNextID();
//������ ������� �� ����� �������� �� ������������� ��������, ���������� ������ ����������
//�� �� ��������, ������� ������������� ��������� value
	RESRECORDSET SelectRes(CONDITION cond);
//�������� ������� �� �-�� SelectRes �� �������� value, ���������� ������ ����������
	RESRECORDSET Aquire(RESRECORDSET source, CONDITION cond);
//��������� � ������ ������ �� �������, � ������� SaveInFile = 1
	StrList *ExportStrings();
	StrList *ExportStrings(std::wstring cath);

  public:
	RESOURCESTACK();
	RESOURCESTACK(const wchar_t *filename);
	virtual ~RESOURCESTACK();

	UINT StackSize();

	const wchar_t *StackInString();

//������ ������� �������� �� �������� ������ �������
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
