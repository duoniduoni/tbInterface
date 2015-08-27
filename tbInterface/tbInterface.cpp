// tbInterface.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "tbInterface.h"

#include <stdio.h>
#include <string>


// 这是导出变量的一个示例
TBINTERFACE_API int ntbInterface=0;

// 这是导出函数的一个示例。
TBINTERFACE_API int fntbInterface(void)
{
	
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 tbInterface.h
CtbInterface::CtbInterface()
{
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////
void string2wstring(const std::string& _str, std::wstring& _wstr )
{  
	if(_str == "") 
		return; 

	_wstr = L"";  

	int wlen = MultiByteToWideChar( CP_ACP, 0, _str.c_str(), -1, NULL, 0 ); 
	wchar_t * pwc = new wchar_t[wlen]; 
	memset((char*)pwc, 0, wlen*2);  
	MultiByteToWideChar( CP_ACP, 0, _str.c_str(), -1, pwc, wlen ); 
	_wstr = pwc;  

	/*unsigned char* p = (unsigned char*)pwc;
	for ( int i=0; i<wlen*2; ++i )
		cout<<uppercase<<hex<<int(p[i])<<" ";
	cout<<endl;*/

	delete []pwc; 
}  

std::string& wstring2string(const std::wstring& _wstr, std::string& _str ) 
{  
	if( _wstr == L"") 
		return _str; 

	_str = "";  
	int len = WideCharToMultiByte( CP_ACP, 0, _wstr.c_str(), -1, NULL, 0, NULL, NULL );  
	unsigned char * pc = new unsigned char[len]; 
	memset( pc, 0, len );  
	WideCharToMultiByte( CP_ACP, 0, _wstr.c_str(), -1, (char*)pc, len, NULL, NULL );  
	_str = (char*)pc;  

	/*for ( int i=0; i<len; ++i )
	cout<<uppercase<<hex<<int(pc[i])<<" ";
	cout<<endl;*/

	delete []pc; 

	return _str; 
}  

void UTF8string2wstring(const std::string& _str, std::wstring& _wstr)
{  
	if( _str == "") 
		return; 

	std::string str;  
	if( _str.size() > 3 && (unsigned char)_str[0] == 0xEF && (unsigned char)_str[1] == 0xBB && (unsigned char)_str[2] == 0xBF) 
		str = _str.substr( 3 ); 
	else
		str = _str; 

	_wstr = L"";  
	int len = _str.size() + 1;  
	int wlen = MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, NULL, 0 ); 
	wchar_t * pwc = new wchar_t[wlen]; 

	memset( (char*)pwc, 0, wlen*2 );  
	MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, pwc, wlen ); 
	_wstr = pwc;  

	/*unsigned char* p = (unsigned char*)pwc;
	for ( int i=0; i<wlen*2; ++i )
	cout<<uppercase<<hex<<int(p[i])<<" ";
	cout<<endl;
	*/

	delete []pwc;  
}  

std::string& wstring2utf8string( const std::wstring& _wstr, std::string& _str ) 
{  
	if( _wstr == L"") 
		return _str; 
	
	_str = "";  
	int len = WideCharToMultiByte( CP_UTF8, 0, _wstr.c_str(), -1, NULL, 0, NULL, NULL );  
	unsigned char * pc = new unsigned char [len]; 
	memset( pc, 0, len );  
	WideCharToMultiByte( CP_UTF8, 0, _wstr.c_str(), -1, (char *)pc, len, NULL, NULL );  
	_str = (char*)pc; 

	/*
	for ( int i=0; i<len; ++i )
	cout<<uppercase<<hex<<int(pc[i])<<" ";
	cout<<endl;*/

	delete []pc; 

	return _str; 
} 


////////////////////////////////////////////////////////////////////////////////////////////

TBINTERFACE_API int run(char * cmd, char * argv, std::string * op)
{
	char Buffer[4096];  
    STARTUPINFO sInfo;  
    PROCESS_INFORMATION pInfo;  
    SECURITY_ATTRIBUTES sa;  
    HANDLE hRead,hWrite;  
    DWORD bytesRead;  

	int retval = 0;

	std::string & output = *op;

	output = "";
  
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);  
    sa.lpSecurityDescriptor = NULL;  
    sa.bInheritHandle = TRUE;  
  
    if (!CreatePipe(&hRead,&hWrite,&sa,0)) //创建匿名管道  
    {  
		char tmp[256] = {0};
        sprintf_s(tmp, 256, "CreatePipe failed (%d)!\n", GetLastError());  
		output = std::string(tmp);
		retval = -1;
        goto out;  
    }  
  
    GetStartupInfo(&sInfo);  
    sInfo.cb = sizeof(sInfo);  
    sInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  
    sInfo.wShowWindow = SW_HIDE;  
    sInfo.hStdError = hWrite;   //将管道的写端交给子进程  
    sInfo.hStdOutput = hWrite;  
    memset(&pInfo, 0, sizeof(pInfo));  
  
    if(!CreateProcess(cmd, argv, NULL, NULL, TRUE, 0, NULL, NULL, &sInfo, &pInfo)) //创建子进程  
    {  
        char tmp[256] = {0};
        sprintf_s(tmp, 256, "CreateProcess failed (%d)!\n", GetLastError());  
		output = std::string(tmp);

        CloseHandle(hWrite);  
        CloseHandle(hRead);  

		retval = -2;
        goto out;  
    }  
    CloseHandle(hWrite); //关闭父进程的写端  
  
    while (1)  
    {  
        if (!ReadFile(hRead,Buffer,sizeof(Buffer)-1,&bytesRead,NULL)) //读取内容  
        {  
            break;  
        }  
        Buffer[bytesRead] = 0;  
		output += std::string(Buffer, bytesRead);
    }  
  
    WaitForSingleObject(pInfo.hProcess, INFINITE);  
    CloseHandle(hRead);  
out:  
	return retval;  
}

char curpath[MAX_PATH] = {0};
char adbpath[MAX_PATH] = {0};
char jarpath[MAX_PATH] = {0};

int push_jar_to_phone(std::string *output)
{
	GetCurrentDirectory(MAX_PATH, curpath);

	//构造adb全路径
	sprintf_s(adbpath, MAX_PATH, "%s\\adb.exe", curpath);

	//构造jar包地址
	sprintf_s(jarpath, MAX_PATH, "%s\\tbrun.jar", curpath);

	//构造push命令
	char pushcmd[256] = {0};
	sprintf_s(pushcmd, 256, "adb push %s /data/local/tmp", jarpath);

	return run(adbpath, pushcmd, output);
}

int run_taobao_process(char * search, char* matchs, bool ioec, unsigned int sct, unsigned int set, std::string * output)
{
	//构造命令参数
	std::string old_cmd = search;

	int templen = old_cmd.size();
	if(old_cmd.at(templen - 1) == '#' &&
		old_cmd.at(templen - 2) == '\\' &&
		old_cmd.at(templen - 3) == '\\'
		)
	{
	}
	else
		old_cmd += "\\#";

	old_cmd += "*";
	old_cmd += matchs;

	templen = old_cmd.size();
	if(old_cmd.at(templen - 1) == '#' &&
		old_cmd.at(templen - 2) == '\\' &&
		old_cmd.at(templen - 3) == '\\'
		)
	{
	}
	else
		old_cmd += "\\#";

	char tmp[256] = {0};
	sprintf_s(tmp, 256, "*%s*%d*%d", ioec ? "TRUE":"FALSE", sct, set);

	old_cmd += tmp;
	
	std::string param_string = old_cmd;
	std::wstring param_wstring;
	string2wstring(param_string, param_wstring);
	std::string param_utf8;
	wstring2utf8string(param_wstring,param_utf8);

	//构造运行命令
	char runcmd[512] = {0};
	sprintf_s(runcmd, 512, "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#testDemo2 -e args %s", 
		param_utf8.c_str());
	/*
	//将运行命令转换为UTF8编码
	char * runcmd_utf8 = GB18030ToUTF_8(runcmd, strlen(runcmd));
	int retval = run(adbpath, runcmd_utf8, output);
	delete []runcmd_utf8;
	*/
	int retval = run(adbpath, runcmd, output);
	
	return retval;
}

TBINTERFACE_API int make_monkey(char * search, char* matchs, bool ioec, unsigned int sct, unsigned int set, std::string * output)
{
	if(push_jar_to_phone(output) < 0)
		return -1;

	if(run_taobao_process(search, matchs, ioec, sct, set, output) < 0)
		return -2;

	return 0;
}

/*
	-----------------------------------------------------------------------------------------------------------------------------------------
*/
char result[128] = {0};
char * getReturnString(std::string & str)
{
	char * src = (char *)str.c_str();
	const char * key = "resultKey=";

	do
	{
		memset(result, 0, 128);
		char * p = strstr(src, key);
		if(p == NULL)
		{
			sprintf_s(result, 128, "not find result key");
			break;
		}

		char * p2 = strstr(p, "\r\n");
		if(p2 == NULL)
		{
			sprintf_s(result, 128, "not find \r\n");
			break;
		}

		strncpy_s(result, 128, p + strlen(key), p2 - p - strlen(key));
	}while(false);

	return result;
}

TBINTERFACE_API int initial()
{
	std::string output;
	return push_jar_to_phone(&output);
}

TBINTERFACE_API char * startTaobao()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#startTaobao";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * stopTaobao()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#stopTaobao";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entryMainActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryMainActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entrySearchConditionActivity(char * arg)
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entrySearchConditionActivity -e args ";

	char tmp[256] = {0};
	sprintf_s(tmp, 256, "%s", arg);
	
	std::string param_string = tmp;
	int templen = param_string.size();
	if(param_string.at(templen - 1) == '#' &&
		param_string.at(templen - 2) == '\\' &&
		param_string.at(templen - 3) == '\\'
		)
	{
	}
	else
		param_string += "\\#";

	std::wstring param_wstring;
	string2wstring(param_string, param_wstring);
	std::string param_utf8;
	wstring2utf8string(param_wstring,param_utf8);

	cmd += param_utf8;

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entrySearchResultActivity(char * arg)
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entrySearchResultActivity -e args ";

	char tmp[256] = {0};
	sprintf_s(tmp, 256, "%s", arg);
	
	std::string param_string = tmp;
	int templen = param_string.size();
	if(param_string.at(templen - 1) == '#' &&
		param_string.at(templen - 2) == '\\' &&
		param_string.at(templen - 3) == '\\'
		)
	{
	}
	else
		param_string += "\\#";

	std::wstring param_wstring;
	string2wstring(param_string, param_wstring);
	std::string param_utf8;
	wstring2utf8string(param_wstring,param_utf8);

	cmd += param_utf8;

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entryCommodityActivity(int arg)
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryCommodityActivity -e args ";

	char tmp[256] = {0};
	sprintf_s(tmp, 256, "%d", arg);
	
	std::string param_string = tmp;
	std::wstring param_wstring;
	string2wstring(param_string, param_wstring);
	std::string param_utf8;
	wstring2utf8string(param_wstring,param_utf8);

	cmd += param_utf8;

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entryEvaluationActivity(int arg)
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryEvaluationActivity -e args ";

	char tmp[256] = {0};
	sprintf_s(tmp, 256, "%d", arg);
	
	std::string param_string = tmp;
	std::wstring param_wstring;
	string2wstring(param_string, param_wstring);
	std::string param_utf8;
	wstring2utf8string(param_wstring,param_utf8);

	cmd += param_utf8;

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entryCommodityActivityRandomly()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryCommodityActivityRandomly";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitCommodityActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitCommodityActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitShopActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitShopActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitSearchResultActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitSearchResultActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitSearchConditionActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitSearchConditionActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitMainActivity()
{
	std::string cmd = "adb shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitMainActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}
