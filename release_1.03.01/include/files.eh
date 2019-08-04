$FilesHinst = _ConnectLib(.\lib\eliFiles.dll);

if ($FilesHinst != -1)
  {
//return: 1 - if exist, 0 - if not, -1 - handled exception;
	_importfunc($FilesHinst, eIsFileExist, '_FileExists', 'sym pFile');

//return: 1 - success, 0 - fail, -1 - handled exception;
//pFailExist indicates whether to interrupt the operation if target file exists. Can take values 0 or 1;	
	_importfunc($FilesHinst, eCopyFile, '_CopyFile', 'sym pFrom,sym pTo,sym pFailExist');

//return: 1 - success, 0 - fail, -1 - handled exception;	
	_importfunc($FilesHinst, eDeleteFile, '_DeleteFile', 'sym pFile');
	
	_importfunc($FilesHinst, eReadTextFile, '_ReadTextFile', 'sym pFile');
	
//return: 1 - success, 0 - fail, -1 - handled exception;
//pMode indicates write mode. Can take values 'ap' (append data to file) or 'ow' (overwrite file);	
	_importfunc($FilesHinst, eWriteTextFile, '_WriteTextFile', 'sym pFile,sym pText,sym pMode');
	
//return -1 if error;	
	_importfunc($FilesHinst, eFileSize, '_FileSize', 'sym pFile');
	
//return -1 if handled exception, otherwise return result of WinAPI function SHFileOperation();	
	_importfunc($FilesHinst, eCopyDir, '_CopyDir', 'sym pDirSrc,sym pDirDest');
	
//return -1 if handled exception, otherwise return 1;	
	_importfunc($FilesHinst, eClearDir, '_ClearDir', 'sym pDir');
  }