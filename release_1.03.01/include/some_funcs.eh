$SomeFuncHinst = _ConnectLib(.\lib\eliSomeFuncs.dll);

if ($SomeFuncHinst != -1)
  {
//проверяет, что в строке содержится число;
//возвращает 0 - если integer;
//1 - float;
//-1 - строка не является float;
//-2 - недопустимые символы в строке;
//признаком float считается как точка, так и запятая ;
    _importfunc($SomeFuncHinst, eIsStrNum, '_IsStrNum', 'sym pStr');
	
//возвращат текущую дату в формате %Y-%m-%d;  
    _importfunc($SomeFuncHinst, eGetCurrDateToStr, '_CurrentDate', '');
	
//возвращает текущее время в формате %H:%M:%S;	
	_importfunc($SomeFuncHinst, eGetCurrTimeToStr, '_CurrentTime', '');

//парсит строку pMainStr заменяя в ней подстроку pTargetStr на строку pInsertStr;	
	_importfunc($SomeFuncHinst, eParseString, '_ParseString', 'sym pMainStr,sym pTargetStr,sym pInsertStr');
  }