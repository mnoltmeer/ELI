$StringFuncsHinst = _ConnectLib(.\lib\eliStringFuncs.dll);

if ($StringFuncsHinst != -1)
  {
//show char code of all symbols in string;
	_importfunc($StringFuncsHinst, eShowSymbols@4, '_ShowSymbols', 'sym pStr');
//remove spaces from begin of string and return result;
	_importfunc($StringFuncsHinst, eLtrim@4, '_ltrim', 'sym pStr');
//insert character pInsSymb in position of char pMoveSymb;
	_importfunc($StringFuncsHinst, eInsertSymbInSymb@4, '_InsertSymbInSymb', 'sym pStr,sym pMoveSymb,sym pInsSymb');
	_importfunc($StringFuncsHinst, eSymbolCount@4, '_SymbolCount', 'sym pStr,sym pSymb');
	_importfunc($StringFuncsHinst, eInsertSymbInPos@4, '_InsertSymbInPos', 'sym pStr,sym pSymb,num pPos');
	_importfunc($StringFuncsHinst, eFindSymbPos@4, '_FindSymbPos', 'sym pStr,sym pSymb');
	_importfunc($StringFuncsHinst, eCopyWhileNotSymb@4, '_CopyWhileNotSymb', 'sym pStr,sym pSymb');
	_importfunc($StringFuncsHinst, eCopyWhileNotPos@4, '_CopyWhileNotPos', 'sym pStr,num pPos');	
//copy pCnt characters from pStr. Starting from position of pSymb;
	_importfunc($StringFuncsHinst, eCopyFromSymb@4, '_CopyFromSymb', 'sym pStr,sring pSymb,num pCnt');
	_importfunc($StringFuncsHinst, eCopyFromPos@4, '_CopyFromPos', 'sym pStr,num pCnt,num pPos');	
	_importfunc($StringFuncsHinst, eStrInclude@4, '_StrInclude', 'sym pInStr,sym pFndStr,num pPos');	
	_importfunc($StringFuncsHinst, eStrIgnInclude@4, '_StrIgnInclude', 'sym pInStr,sym pFndStr,num pPos');
  }