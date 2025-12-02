/*!
Copyright 2014-2025 Maxim Noltmeer (m.noltmeer@gmail.com)

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//---------------------------------------------------------------------------
//import from somefunc.h
//LEGACY CODE :)
//---------------------------------------------------------------------------

#ifndef oldfuncsH
#define oldfuncsH

inline std::wstring ParseStringW(const std::wstring main_str,
								 std::wstring target_str,
								 const std::wstring insert_str)
{
  std::wstring result = main_str;
  UINT pos, scount;

  while (result.find(target_str) != std::wstring::npos)
    {
      pos = result.find(target_str);
      scount = target_str.length();
      result.erase(pos, scount);
      result = result.insert(pos, insert_str);
    }

  return result;
};
//---------------------------------------------------------------------------

inline std::wstring &LoadFileToStringW(std::wstring path)
{
  wchar_t str[128];
  static std::wstring outstr;

  outstr = L"";

//открываем файл
  try
     {
	   wchar_t tmp[8192];
	   std::wifstream fin;

	   fin.open(AnsiOf(path.c_str()), std::ios_base::in);

       if (!fin.is_open())
		 outstr = L"-err-";
       else
		 {
           fin.seekg(0);

		   while (!fin.eof())
             {
               fin.getline(tmp, sizeof(tmp));

               if (fin.fail())
				 return outstr;

               outstr = outstr.append(tmp);
			 }

		   fin.close();
         }
     }
  catch(Exception &e)
     {
	   outstr = L"-err-";
	 }

  return outstr;
};
//---------------------------------------------------------------------------

inline bool LoadFileToVectorW(std::vector<std::wstring> *vecStr, std::wstring path)
{
  wchar_t str[128];
  bool res;

//открываем файл
  try
     {
       wchar_t tmp[4096];
       std::wifstream fin;

       fin.open(AnsiOf(path.c_str()), std::ios_base::in);

       if (!fin.is_open())
		 res = false;
	   else
         {
           fin.seekg(0);

           while (!fin.eof())
             {
               fin.getline(tmp, sizeof(tmp));

               if (fin.fail())
				 res = false;
			   else
				res = true;

               vecStr->push_back(std::wstring(tmp));
             }

		   fin.close();
         }
     }
  catch(Exception &e)
	 {
	   res = false;
	 }

  return res;
};
//---------------------------------------------------------------------------

inline const wchar_t *ReadFileToStrW(const wchar_t *path)
{
  wchar_t str[128];
  static std::wstring outstr;
  std::wstringstream in;
  outstr.clear();

//открываем файл
  try
     {
       std::wifstream fin;

       fin.open(AnsiOf(path), std::ios_base::in);

       if (!fin.is_open())
		 outstr = L"-err-";
       else
         {
           wchar_t ch;

           while (!fin.eof())
             {
               fin.get(ch);
               in << ch;
             }

           fin.close();
           outstr = in.str();
		   outstr.erase(outstr.length() - 1, 1);
         }
     }
  catch(Exception &e)
	 {
	   outstr = L"-err-";
	 }

  return outstr.c_str();
};
//---------------------------------------------------------------------------

inline bool SaveStrToFileW(const wchar_t *msg, const wchar_t *path)
{
  wchar_t str[128];
  bool res;

//открываем файл
  try
     {
       std::wofstream fout(AnsiOf(path), std::ios_base::app | std::ios_base::ate);

       if (fout.fail())
		 res = false;
       else
         {
           fout << msg << std::endl;

           if (fout.fail())
			 res = false;
		   else
			 res = true;

		   fout.close();
         }
     }
  catch(Exception &e)
	 {
	   res = false;
     }

  return res;
};
//---------------------------------------------------------------------------

inline bool SaveVectorToFileW(std::vector<std::wstring> *vecStr, std::wstring path)
{
  wchar_t str[128];

//открываем файл
  try
     {
       std::wofstream fout;
	   fout.open(AnsiOf(path.c_str()), std::ios_base::trunc | std::ios_base::out);

       if (!fout.is_open())
		 return false;
       else
         {
           fout.seekp(0);

           for (UINT i = 0; i < vecStr->size(); i++)
             {
			   fout << vecStr->at(i) << std::endl;

               if (fout.fail())
				 return false;
             }

           fout.close();
           return true;
         }
     }
  catch(Exception &e)
     {
	   return false;
	 }

  return false;
};
//---------------------------------------------------------------------------

inline void StrToListW(std::vector <std::wstring> *vecList, std::wstring text, std::wstring delim, bool isdelimend)
{
 UINT pos;
 std::wstring str;

 vecList->clear();

//если разделитель не задан, по умолчанию используется признак конца строки
 if (delim == L"")
   delim = L"\n";

//разбиваем строку выражения на фрагменты используя символ-разделитель
//и заносим в вектор
  while (text.find(delim) != std::wstring::npos)
    {
	  pos = text.find(delim);
      str = text.substr(0, pos);
      text.erase(0, pos + 1);
      vecList->push_back(str);
    }

 if (!isdelimend)
   {
//заносим в вектор оставшуюся часть исходной строки
     vecList->push_back(text);
   }
};
//---------------------------------------------------------------------------

inline std::wstring GetConfigLineW(std::wstring conf_file, UINT index)
{
 std::wstring param;

//создадим вектор и загрузим в него содержимое конфиг-файла
 std::vector<std::wstring> vecList;

 if (!LoadFileToVectorW(&vecList, conf_file))
    return L"^err_open_file";

 if (vecList.size() != 0)
   {
//если список не пустой и в нем есть строка с указанным индексом
     if (vecList.size() > index)
       {
         param = vecList.at(index);
         if (param.find(L"=") != std::wstring::npos)
           param = param.erase(1, param.find(L"="));
       }
     else
       param = L"^no_line";
   }
 else
   param = L"^no_data";

 return param;
};
//---------------------------------------------------------------------------

inline int GetConfigLineIndW(std::wstring conf_file, std::wstring param)
{
  int index = -1;
  int length, pos = 0;

//создадим вектор и загрузим в него содержимое конфиг-файла
 std::vector<std::wstring> vecList;

 if (!LoadFileToVectorW(&vecList, conf_file))
    return -1;

 if (vecList.size() > 0)
   {
//если список не пуст, построчно сверим имена параметров с исходным
     UINT i = 0;

     while (i < vecList.size())
        {
//вырежем из строки значение параметра, оставив только имя
          length = vecList.at(i).length();

          if (param.find(L"=") != std::wstring::npos)
            pos = vecList.at(i).find(L"=");

          vecList.at(i).erase(pos, length - pos);

          if (vecList.at(i).find(param) != std::wstring::npos)
            {
              index = i;
            }

          i++;
        }
   }

  return index;
};
//---------------------------------------------------------------------------

inline std::wstring GetConfigLineW(std::wstring conf_file, std::wstring param_name)
{
  int length, pos = 0;

//создадим вектор и загрузим в него содержимое конфиг-файла
 std::vector<std::wstring> vecList;

 if (!LoadFileToVectorW(&vecList, conf_file))
    return L"^err_open_file";

 if (vecList.size() > 0)
   {
//если список не пуст, построчно сверим имена параметров с исходным
     UINT i = 0;

     while (i < vecList.size())
        {
//вырежем из строки значение параметра, оставив только имя
          length = vecList.at(i).length();
          pos = vecList.at(i).find(L"=");

          if (vecList.at(i).find(param_name) != std::wstring::npos)
            {
              return vecList.at(i).substr(pos + 1, length - pos);
            }

          i++;
        }
   }

  return L"^no_line";
};
//---------------------------------------------------------------------------

inline bool SetConfigLineW(std::wstring conf_file, UINT index, std::wstring value)
{
//создадим лист и загрузим в него содержимое конфиг-файла
 std::vector<std::wstring> vecList;
 LoadFileToVectorW(&vecList, conf_file);

 if (vecList.size() > 0)
   {
//если список не пустой и в нем есть строка с указанным индексом
     if (vecList.size() > index)
       {
         std::wstring str = vecList.at(index);
         int pos = str.find(L"=") + 1;
//уберем старый параметр
         str = str.erase(pos, str.length() - pos + 1);
//внесем новый
         vecList[index] = str + value;

         if (SaveVectorToFileW(&vecList, conf_file))
           return true;
         else
           return false;
       }
     else
       return false;
   }
 else
   return false;
};
//---------------------------------------------------------------------------

inline bool SetConfigLineW(std::wstring conf_file, std::wstring param, std::wstring value)
{
  return SetConfigLineW(conf_file,
                       GetConfigLineIndW(conf_file, param),
                       value);
};
//---------------------------------------------------------------------------

#endif