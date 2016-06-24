// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: conf_file.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include <regex>
#include <locale.h>
#include "utility_fwd.h"

using namespace std;
using namespace keye;

void str_util::split_lines(std::vector<std::string>& lines,const std::string& buf){
	const char* _pattern="[^\n]+(?:$)";
	try{
		regex pattern(_pattern);
		auto beg = std::sregex_iterator(buf.begin(),buf.end(),pattern);
		auto end = std::sregex_iterator();
		for(auto i=beg;i!=end; ++i)
			//for(auto s:*i)lines.push_back(s);
			for(auto s:*i){
				auto str=s.str();
				if(str.back()==L'\r')str.pop_back();
				lines.push_back(str);
			}
	} catch(regex_error e){
		KEYE_LOG("%s\n",e.what());
	}
}

void str_util::wsplit_lines(std::vector<std::wstring>& lines,const std::wstring& buf){
	const wchar_t* _pattern=L"[^\n]+(?:$)";
	try{
		wregex pattern(_pattern);
		auto beg = std::wsregex_iterator(buf.begin(),buf.end(),pattern);
		auto end = std::wsregex_iterator();
		for(auto i=beg;i!=end; ++i)
			for(auto s:*i){
				auto str=s.str();
				if(str.back()==L'\r')str.pop_back();
				lines.push_back(str);
			}
	} catch(regex_error e){
		KEYE_LOG("%s\n",e.what());
	}
}

bool str_util::wstr2str(std::string& str,const wchar_t* wstr){
	if(wstr){
		//char* old_locale=_strdup(setlocale(LC_CTYPE,nullptr));	//store locale
		setlocale(LC_CTYPE,setlocale(LC_ALL,""));
		auto len=wcstombs(nullptr,wstr,0);	//no need '\0' for string
		if(len==-1)return false;
		str.resize(len);
		len=wcstombs((char*)str.data(),wstr,len);
		//if(old_locale){ setlocale(LC_CTYPE,old_locale);free(old_locale); }	//restore locale
		return len>0;
	}
	return false;
}

bool str_util::str2wstr(std::wstring& wstr,const char* str){
	if(str){
		setlocale(LC_CTYPE,setlocale(LC_ALL,""));
		auto len=mbstowcs(nullptr,str,0);	//no need '\0' for string
		if(len==-1)return false;
		wstr.resize(len);
		auto wbuf=wstr.data();
		len=mbstowcs((wchar_t*)wbuf,str,len);
		return len>0;
	}
	return false;
}

std::string str_util::bytes2hex(const unsigned char* buf,size_t len){
	std::string str;
	char tmp[3];
	for(uint32 i=0; i<len; i++){
		sprintf(tmp,"%02x",buf[i]);
		str += tmp;
	}
	return str;
}
