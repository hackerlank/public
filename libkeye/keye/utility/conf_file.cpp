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
#include <fstream>
#include <regex>
#include "utility_fwd.h"

using namespace std;
using namespace keye;

static const std::string LF("\n"),CR("\r"),CL("\r\n"),TAB("\t"),SPACE(" ");
// --------------------------------------------------------
txt_file::txt_file():_endl(CL),_endlen(CL.length()){}

bool txt_file::load(const char* file){
	if(!file){
		KEYE_LOG("no file specified.\n");
		return false;
	}

	std::ifstream fp;
	fp.open(file,std::ios::in|std::ios::binary);
	if(!fp.is_open()){
		KEYE_LOG("file %s does not exist\n",file);
		return false;
	}

	fp.seekg(0,std::ios_base::end);
	auto sz=(size_t)fp.tellg();
	if(sz){
		fp.seekg(0, std::ios_base::beg);
		std::string buf;
		buf.resize(sz);
		auto ptr=(char*)buf.data();
		fp.read(ptr,static_cast<std::streamsize>(sz));
		fp.close();
		if(_check_endl(buf,CL)||_check_endl(buf,LF)||_check_endl(buf,CR))
			return _parse(buf);
		else{
			KEYE_LOG("file without endl\n");
			return false;
		}
	}else{
		fp.close();
		KEYE_LOG("file empty\n");
		return false;
	}
}

bool txt_file::save(const char* file){
	if(!file){
		KEYE_LOG("no file specified.\n");
		return false;
	}

	std::ofstream fp;
	fp.open(file,std::ios::out|std::ios::binary);
	if(!fp.is_open()){
		KEYE_LOG("file %s open failed.\n",file);
		return false;
	}

	std::string buf;
	auto ret=_make_buffer(buf);
	if(ret)
		fp.write(buf.c_str(),buf.size());
	else
		KEYE_LOG("file empty\n");
	fp.close();
	return true;
}

bool txt_file::_check_endl(const std::string& buf,const std::string& endl){
	if(std::string::npos!=buf.find(endl)){
		_endl=endl;
		_endlen=endl.length();
		return true;
	}else
		return false;
}
// --------------------------------------------------------
csv_file::csv_file(bool use_comma):_rows(0),_cols(0),_use_comma(use_comma){}

bool csv_file::_parse(const std::string& buf){
	//reset
	_rows=_cols=0;
	_grids.clear();
	//load
	wstring wbuf;
	str_util::str2wstr(wbuf,buf.c_str());
	vector<wstring> lines;
	str_util::wsplit_lines(lines,wbuf);
	auto nl=lines.size();
	if(nl>0){
		vector<wstring> fields;
		if(parse_csv(fields,lines[0],!_use_comma)){
			//header
			_cols=fields.size();
			string f;
			for(auto& wf:fields){
				str_util::wstr2str(f,wf.c_str());
				_grids.push_back(cast_t(f.c_str()));
			}
			//contents
			for(decltype(nl) i=1;i<nl;++i){
				fields.clear();
				auto& line=lines[i];
				if(parse_csv(fields,line,!_use_comma)){
					auto jj=fields.size();
					if(jj>0){
						//skip empty lines
						for(size_t j=0;j<_cols;++j){
							if(j<jj){
								auto& wf=fields[j];
								if(wf.empty())wf=L"0";
								str_util::wstr2str(f,wf.c_str());
							} else{
								//empty grids
								f="0";
							}
							_grids.push_back(cast_t(f.c_str()));
						}
						++_rows;
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool csv_file::parse_csv(vector<wstring>& fields,const wstring& line,bool tab){
	const wchar_t* csv_pattern=
		L"(?!\\s*$)"                                 // Don't match empty last value.
		"\\s*"                                       // Strip whitespace before value.
		"(?:"                                        // Group for value alternatives.
		"'([^']*(?:[\\S\\s][^']*)*?)'"               // Either $1: Single quoted string,
		"|\"([^\"]*(?:[\\S\\s][^\"]*)*?)\""          // or $2: Double quoted string,
		"|([^,'\"]*\\w[^,'\"]*?)"                    // or $3: Non-comma, non-quote stuff.
		")"                                          // End group of value alternatives.
		"\\s*"                                       // Strip whitespace after value.
		"(?:,|$)";                                   // Field ends on comma or EOS.

	const wchar_t* tab_pattern=
		L"(?!\\s*$)"                                 // Don't match empty last value.
		"\\s*"                                       // Strip whitespace before value.
		"(?:"                                        // Group for value alternatives.
		"'([^']*(?:[\\S\\s][^']*)*?)'"               // Either $1: Single quoted string,
		"|\"([^\"]*(?:[\\S\\s][^\"]*)*?)\""          // or $2: Double quoted string,
		"|([^\t'\"]*\\w[^\t'\"]*?)"                  // or $3: Non-comma, non-quote stuff.
		")"                                          // End group of value alternatives.
		"\\s*"                                       // Strip whitespace after value.
		"(?:\t|$)";                                  // Field ends on comma or EOS.
	try{
		wregex pattern(tab?tab_pattern:csv_pattern);
		auto beg = std::wsregex_iterator(line.begin(),line.end(),pattern);
		auto end = std::wsregex_iterator();
		for(auto i=beg;i!=end; ++i){
			auto& match=*i;
			auto empty=true;
			for(auto it=++match.begin(),iend=match.end();it!=iend;++it)
				if(it->length()>0){
					fields.push_back(*it);
					empty=false;
					break;
				}
			if(empty)fields.push_back(L"");
		}
	} catch(regex_error e){
		KEYE_LOG("%s\n",e.what());
		return false;
	}
	return true;
}

size_t csv_file::rows()const{
	return _rows;
}

size_t csv_file::colulmns()const{
	return _cols;
}

const cast_t& csv_file::value(size_t row,size_t col)const{
	if(row<_rows&&col<_cols)
		return _grids[(row+1)*_cols+col];
	return _default;
}

bool csv_file::_make_buffer(std::string& buf){
	if(0==_rows||0==_cols)return false;

	buf.clear();
	//title
	for(size_t j=0;j<_cols;++j){
		auto s=(const char*)_grids[j];
		buf+=s;
		buf+=(j==_cols-1)?CL:TAB;
	}
	//content
	for(size_t i=0;i<_rows;++i){
		for(size_t j=0;j<_cols;++j){
			auto s=(const char*)value(i,j);
			buf+=s;
			buf+=(j==_cols-1)?CL:TAB;
		}
	}
	return true;
}

void csv_file::print()const{
	if(0==_rows||0==_cols)return;

	const char* LINE="|---------------";
	KEYE_LOG("rows=%d,columns=%d\n",(int)_rows,(int)_cols);
	//title
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
	for(size_t j=0;j<_cols;++j){
		auto s=(const char*)_grids[j];
		KEYE_LOG("%s",s);
		if(strlen(s)<8)
			KEYE_LOG("\t\t");
		else if(strlen(s)<16)
			KEYE_LOG("\t");
	}
	KEYE_LOG("\n");
	//content
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
	for(size_t i=0;i<_rows;++i){
		for(size_t j=0;j<_cols;++j){
			auto s=(const char*)value(i,j);
			KEYE_LOG("%s",s);
			if(strlen(s)<8)
				KEYE_LOG("\t\t");
			else if(strlen(s)<16)
				KEYE_LOG("\t");
		}
		KEYE_LOG("\n");
	}
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
}
// --------------------------------------------------------
bool ini_cfg_file::_parse(const std::string& buf){
	//reset
	_map.clear();
	//load
	wstring wbuf;
	str_util::str2wstr(wbuf,buf.c_str());
	vector<wstring> lines;
	str_util::wsplit_lines(lines,wbuf);
	auto nl=lines.size();
	map<wstring,wstring> dict;
	for(auto& line:lines)
		parse_ini(dict,line);
	for(auto& kv:dict){
		auto wkey=kv.first,wval=kv.second;
		string key,val;
		str_util::wstr2str(key,wkey.c_str());
		str_util::wstr2str(val,wval.c_str());
		if(val.empty())val="0";
		_clean(key),_clean(key,' ');
		_clean(val),_clean(val,' ');
		_map.insert(std::make_pair(key,cast_t(val.c_str())));
	}
	return nl>0;
}

bool ini_cfg_file::parse_ini(map<wstring,wstring>& dict,const wstring& line){
	const wchar_t* ini_pattern=
		L"(?!\\s*$)"                                     // Don't match empty last value.
		"\\s*"                                          // Strip whitespace before value.
		"([^=]+)=([\\S\\s]*)"                           // Group key and value.
		"\\s*"                                          // Strip whitespace after value.
		"(?:$)";                                     // Field ends on line end or EOS.

	try{
		wregex pattern(ini_pattern);
		wsmatch match;
		if(regex_match(line,match,pattern)){
			dict[match[1]]=match[2];
			return true;
		}
	} catch(regex_error e){
		KEYE_LOG("%s\n",e.what());
	}
	return false;
}

const cast_t& ini_cfg_file::value(const char* key)const{
	if(key&&!_map.empty()){
		auto i=_map.find(key);
		if(i!=_map.end())
			return i->second;
	}
	return _default;
}

bool ini_cfg_file::_make_buffer(std::string& buf){
	if(_map.empty())return false;

	buf.clear();
	for(auto i=_map.begin(),ii=_map.end();i!=ii;++i){
		auto& key=i->first;
		const char* val=i->second;
		buf+=key+"="+val+CL;
	}
	return true;
}

void ini_cfg_file::_clean(std::string& ss,char c){
	while(0==ss.find(c))ss=ss.substr(1);
	while(true){
		size_t i=ss.rfind(c),ii=ss.length()-1;
		if(i==ii&&i!=std::string::npos)ss=ss.substr(0,ii);
		else break;
	}
}

void ini_cfg_file::print()const{
	for(auto i=_map.begin(),ii=_map.end();i!=ii;++i){
		auto& key=i->first;
		const char* val=i->second;
		KEYE_LOG("%s",key.c_str());
		auto len=key.length();
		if(len<8)
			KEYE_LOG("\t\t");
		else if(len<16)
			KEYE_LOG("\t");
		KEYE_LOG("=%s\n",val);
	}
}
// --------------------------------------------------------
tab_file::tab_file():_rows(0),_cols(0){}

bool tab_file::_parse(const std::string& buf){
	//reset
	_rows=_cols=0;
	_grids.clear();
	//load
	size_t beg=0,end=0;
	end=(size_t)buf.find(_endl,beg);
	auto title=buf.substr(beg,end-beg);
	if(_parse_title(title)){
		while(true){
			beg=end+_endlen;
			end=buf.find(_endl,beg);
			if(std::string::npos==end)
				break;

			auto content=buf.substr(beg,end-beg);
			if(_parse_content(content))
				++_rows;
		}
		return true;
	}else
		return false;
}

bool tab_file::_parse_title(const std::string& buf){
	size_t beg=0,cur=0,end=buf.size();
	bool over=false;
	while(!over){
		cur=buf.find(TAB,beg);
		if(std::string::npos==cur){
			cur=end;
			over=true;
		}
		auto str=buf.substr(beg,cur-beg);
		cast_t grid(str.c_str());
		_grids.push_back(grid);
		if(!over)
			beg=cur+TAB.length();
		//update
		++_cols;
	}
	return true;
}

bool tab_file::_parse_content(const std::string& buf){
	size_t beg=0,cur=0,end=buf.size();
	bool over=false;
	while(!over){
		cur=buf.find(TAB,beg);
		if(std::string::npos==cur){
			cur=end;
			over=true;
		}
		std::string str(buf.substr(beg,cur-beg));
		if(str.empty())str="0";
		cast_t grid(str.c_str());
		_grids.push_back(grid);
		if(!over)
			beg=cur+TAB.length();
	}
	return true;
}

size_t tab_file::rows()const{
	return _rows;
}

size_t tab_file::colulmns()const{
	return _cols;
}

const cast_t& tab_file::value(size_t row,size_t col)const{
	if(row<_rows&&col<_cols)
		return _grids[(row+1)*_cols+col];
	return _default;
}

bool tab_file::_make_buffer(std::string& buf){
	if(0==_rows||0==_cols)return false;

	buf.clear();
	//title
	for(size_t j=0;j<_cols;++j){
		auto s=(const char*)_grids[j];
		buf+=s;
		buf+=(j==_cols-1)?CL:TAB;
	}
	//content
	for(size_t i=0;i<_rows;++i){
		for(size_t j=0;j<_cols;++j){
			auto s=(const char*)value(i,j);
			buf+=s;
			buf+=(j==_cols-1)?CL:TAB;
		}
	}
	return true;
}

void tab_file::print()const{
	if(0==_rows||0==_cols)return;

	const char* LINE="|---------------";
	KEYE_LOG("rows=%d,columns=%d\n",(int)_rows,(int)_cols);
	//title
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
	for(size_t j=0;j<_cols;++j){
		auto s=(const char*)_grids[j];
		KEYE_LOG("%s",s);
		if(strlen(s)<8)
			KEYE_LOG("\t\t");
		else if(strlen(s)<16)
			KEYE_LOG("\t");
	}
	KEYE_LOG("\n");
	//content
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
	for(size_t i=0;i<_rows;++i){
		for(size_t j=0;j<_cols;++j){
			auto s=(const char*)value(i,j);
			KEYE_LOG("%s",s);
			if(strlen(s)<8)
				KEYE_LOG("\t\t");
			else if(strlen(s)<16)
				KEYE_LOG("\t");
		}
		KEYE_LOG("\n");
	}
	for(size_t j=0;j<_cols;++j)
		KEYE_LOG("%s",LINE);
	KEYE_LOG("|\n");
}
// --------------------------------------------------------
bool ini_file::_parse(const std::string& buf){
	//reset
	_map.clear();
	//load
	size_t beg=0,end=0,cur=0;
	bool over=false;
	while(!over){
		cur=buf.find(_endl,beg);
		if(std::string::npos==cur){
			cur=end;
			over=true;
		}
		auto str=buf.substr(beg,cur-beg);
		_parse_content(str);
		if(!over)
			beg=cur+_endlen;
	}
	return true;
}

bool ini_file::_parse_content(const std::string& buf){
	size_t beg=0,cur=0;
	cur=buf.find("=",beg);
	if(std::string::npos!=cur){
		std::string key(buf.substr(0,cur)),
			val(buf.substr(cur+1));
		_clean(key),_clean(key,' ');
		_clean(val),_clean(val,' ');
		if(val.empty())val="0";
		cast_t grid(val.c_str());
		_map.insert(std::make_pair(key,grid));
	}
	return true;
}

const cast_t& ini_file::value(const char* key)const{
	if(key&&!_map.empty()){
		auto i=_map.find(key);
		if(i!=_map.end())
			return i->second;
	}
	return _default;
}

void ini_file::_clean(std::string& ss,char c){
	while(0==ss.find(c))ss=ss.substr(1);
	while(true){
		size_t i=ss.rfind(c),ii=ss.length()-1;
		if(i==ii&&i!=std::string::npos)ss=ss.substr(0,ii);
		else break;
	}
}

bool ini_file::_make_buffer(std::string& buf){
	if(_map.empty())return false;

	buf.clear();
	for(auto i=_map.begin(),ii=_map.end();i!=ii;++i){
		auto& key=i->first;
		const char* val=i->second;
		buf+=key+"="+val+CL;
	}
	return true;
}

void ini_file::print()const{
	for(auto i=_map.begin(),ii=_map.end();i!=ii;++i){
		auto& key=i->first;
		const char* val=i->second;
		KEYE_LOG("%s",key.c_str());
		auto len=key.length();
		if(len<8)
			KEYE_LOG("\t\t");
		else if(len<16)
			KEYE_LOG("\t");
		KEYE_LOG("=%s\n",val);
	}
}
// --------------------------------------------------------
cast_t::cast_t(const char* raw){
	_raw=raw?raw:"0";
}

cast_t::operator int()const{
	return atoi(_raw.c_str());
}

cast_t::operator float()const{
	return (float)atof(_raw.c_str());
}

cast_t::operator const char*()const{
	return _raw.c_str();
}
