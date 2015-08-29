// tbInterface.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "tbInterface.h"

#include <stdio.h>
#include <string>


// ���ǵ���������һ��ʾ��
TBINTERFACE_API int ntbInterface=0;

// ���ǵ���������һ��ʾ����
TBINTERFACE_API int fntbInterface(void)
{
	
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� tbInterface.h
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
  
    if (!CreatePipe(&hRead,&hWrite,&sa,0)) //���������ܵ�  
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
    sInfo.hStdError = hWrite;   //���ܵ���д�˽����ӽ���  
    sInfo.hStdOutput = hWrite;  
    memset(&pInfo, 0, sizeof(pInfo));  
  
    if(!CreateProcess(cmd, argv, NULL, NULL, TRUE, 0, NULL, NULL, &sInfo, &pInfo)) //�����ӽ���  
    {  
        char tmp[256] = {0};
        sprintf_s(tmp, 256, "CreateProcess failed (%d)!\n", GetLastError());  
		output = std::string(tmp);

        CloseHandle(hWrite);  
        CloseHandle(hRead);  

		retval = -2;
        goto out;  
    }  
    CloseHandle(hWrite); //�رո����̵�д��  
  
    while (1)  
    {  
        if (!ReadFile(hRead,Buffer,sizeof(Buffer)-1,&bytesRead,NULL)) //��ȡ����  
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

int push_jar_to_phone(char * device, std::string *output)
{
	GetCurrentDirectory(MAX_PATH, curpath);

	//����adbȫ·��
	sprintf_s(adbpath, MAX_PATH, "%s\\adb.exe", curpath);

	//����jar����ַ
	sprintf_s(jarpath, MAX_PATH, "%s\\tbrun.jar", curpath);

	//����push����
	char pushcmd[1024] = {0};
	if(device == NULL)
		sprintf_s(pushcmd, 1024, "adb push %s /data/local/tmp", jarpath);
	else
		sprintf_s(pushcmd, 1024, "adb -s %s push %s /data/local/tmp", device, jarpath);

	return run(adbpath, pushcmd, output);
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

TBINTERFACE_API int initial(char * device)
{
	std::string output;
	return push_jar_to_phone(device, &output);
}

TBINTERFACE_API char * startTaobao(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#startTaobao";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * stopTaobao(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#stopTaobao";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entryMainActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryMainActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * entrySearchConditionActivity(char * device, char * arg)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entrySearchConditionActivity -e args ";

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

TBINTERFACE_API char * entrySearchResultActivity(char * device, char * arg)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entrySearchResultActivity -e args ";

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

TBINTERFACE_API char * entryCommodityActivity(char * device, int arg)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryCommodityActivity -e args ";

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

TBINTERFACE_API char * entryEvaluationActivity(char * device, int arg)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryEvaluationActivity -e args ";

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

TBINTERFACE_API char * entryCommodityActivityRandomly(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#entryCommodityActivityRandomly";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitCommodityActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitCommodityActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitShopActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitShopActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitSearchResultActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitSearchResultActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitSearchConditionActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitSearchConditionActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}

TBINTERFACE_API char * exitMainActivity(char * device)
{
	std::string cmd = "adb";
	if(device)
	{
		cmd += " -s ";
		cmd += device;
	}
	cmd += " shell uiautomator runtest tbrun.jar -c com.uiautomatortest.Test#exitMainActivity";

	std::string output;
	int retval = run(adbpath, (char *)cmd.c_str(), &output);
	if(retval < 0)
		return "run fail";

	return getReturnString(output);
}