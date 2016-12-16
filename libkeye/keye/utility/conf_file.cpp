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
static const std::wstring WLF(L"\n"),WCR(L"\r"),WCL(L"\r\n"),WTAB(L"\t"),WSPACE(L" ");
// --------------------------------------------------------
txt_file::txt_file():_endl(CL),_wendl(WCL),_endlen(CL.length()){}

bool txt_file::load(const char* file,bool ansi){
	if(!file){
		KEYE_LOG("no file specified.\n");
		return false;
	}

    if(ansi){
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
            }
        }else{
            KEYE_LOG("file empty\n");
        }
        fp.close();
        return false;
    }else{
        std::wifstream fp;
        fp.open(file,std::ios::in|std::ios::binary);
        if(!fp.is_open()){
            KEYE_LOG("file %s does not exist\n",file);
            return false;
        }
        
        fp.seekg(0,std::ios_base::end);
        auto sz=(size_t)fp.tellg();
        if(sz){
            fp.seekg(0, std::ios_base::beg);
            std::wstring buf;
            buf.resize(sz);
            auto ptr=(wchar_t*)buf.data();
            fp.read(ptr,static_cast<std::streamsize>(sz));
            fp.close();
            if(_check_endl(buf,WCL)||_check_endl(buf,WLF)||_check_endl(buf,WCR))
                return _parse(buf);
            else{
                KEYE_LOG("file without endl\n");
            }
        }else{
            KEYE_LOG("file empty\n");
        }
        fp.close();
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

bool txt_file::_check_endl(const std::wstring& buf,const std::wstring& endl){
	if(std::wstring::npos!=buf.find(endl)){
		_wendl=endl;
		_endlen=endl.length();
		return true;
	}else
		return false;
}
// --------------------------------------------------------
csv_file::csv_file(bool use_comma):_rows(0),_cols(0),_use_comma(use_comma){}

bool csv_file::_parse(const std::string& buf){
    //load
    wstring wbuf;
    str_util::str2wstr(wbuf,buf.c_str());
    return _parse(wbuf);
}

bool csv_file::_parse(const std::wstring& wbuf){
	//reset
	_rows=_cols=0;
	_grids.clear();
	//load
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
    
    std::locale old;
    std::locale::global(std::locale("en_US.UTF-8"));

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
        std::locale::global(old);
		KEYE_LOG("%s\n",e.what());
		return false;
	}
    std::locale::global(old);
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

bool csv_file::_make_buffer(std::wstring& buf){
	if(0==_rows||0==_cols)return false;

	buf.clear();
	//title
	for(size_t j=0;j<_cols;++j){
		auto s=(const wchar_t*)_grids[j];
		buf+=s;
		buf+=(j==_cols-1)?WCL:WTAB;
	}
	//content
	for(size_t i=0;i<_rows;++i){
		for(size_t j=0;j<_cols;++j){
			auto s=(const wchar_t*)value(i,j);
			buf+=s;
			buf+=(j==_cols-1)?WCL:WTAB;
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
bool ini_file::_parse(const std::string& buf){
    wstring wbuf;
    str_util::str2wstr(wbuf,buf.c_str());
    //reset
    _map.clear();
    //load
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

bool ini_file::_parse(const std::wstring& wbuf){
	//reset
	_wmap.clear();
	//load
	vector<wstring> lines;
	str_util::wsplit_lines(lines,wbuf);
	auto nl=lines.size();
	map<wstring,wstring> dict;
	for(auto& line:lines)
		parse_ini(dict,line);
	for(auto& kv:dict){
		auto wkey=kv.first,wval=kv.second;
		if(wval.empty())wval=L"0";
		_clean(wkey),_clean(wkey,' ');
		_clean(wval),_clean(wval,' ');
		_wmap.insert(std::make_pair(wkey,cast_t(wval.c_str())));
	}
	return nl>0;
}

bool ini_file::parse_ini(map<wstring,wstring>& dict,const wstring& line){
	const wchar_t* ini_pattern=
		L"(?!\\s*$)"                                     // Don't match empty last value.
		"\\s*"                                          // Strip whitespace before value.
		"([^=]+)=([\\S\\s]*)"                           // Group key and value.
		"\\s*"                                          // Strip whitespace after value.
		"(?:$)";                                     // Field ends on line end or EOS.

    std::locale old;
    std::locale::global(std::locale("en_US.UTF-8"));

	try{
		wregex pattern(ini_pattern);
		wsmatch match;
		if(regex_match(line,match,pattern)){
			dict[match[1]]=match[2];
            std::locale::global(std::locale(old));
			return true;
		}
	} catch(regex_error e){
		KEYE_LOG("%s\n",e.what());
	}
    std::locale::global(std::locale(old));
	return false;
}

const cast_t& ini_file::value(const char* key)const{
    if(key&&!_map.empty()){
        auto i=_map.find(key);
        if(i!=_map.end())
            return i->second;
    }
    return _default;
}

const cast_t& ini_file::value(const wchar_t* key)const{
	if(key&&!_wmap.empty()){
		auto i=_wmap.find(key);
		if(i!=_wmap.end())
			return i->second;
	}
	return _default;
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

bool ini_file::_make_buffer(std::wstring& buf){
	if(_wmap.empty())return false;

	buf.clear();
	for(auto i=_wmap.begin(),ii=_wmap.end();i!=ii;++i){
		auto& key=i->first;
		const wchar_t* val=i->second;
		buf+=key+L"="+val+WCL;
	}
	return true;
}

void ini_file::_clean(std::string& ss,char c){
    while(0==ss.find(c))ss=ss.substr(1);
    while(true){
        size_t i=ss.rfind(c),ii=ss.length()-1;
        if(i==ii&&i!=std::string::npos)ss=ss.substr(0,ii);
        else break;
    }
}

void ini_file::_clean(std::wstring& ss,wchar_t c){
	while(0==ss.find(c))ss=ss.substr(1);
	while(true){
		size_t i=ss.rfind(c),ii=ss.length()-1;
		if(i==ii&&i!=std::wstring::npos)ss=ss.substr(0,ii);
		else break;
	}
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

cast_t::cast_t(const wchar_t* raw){
	_wraw=raw?raw:L"0";
}

cast_t::operator int()const{
    if(!_raw.empty())
        return atoi(_raw.c_str());
    else{
        std::string a("0");
        str_util::wstr2str(a,_wraw.c_str());
        return atoi(a.c_str());
    }
}

cast_t::operator float()const{
    if(!_raw.empty())
        return (float)atof(_raw.c_str());
    else{
        std::string a("0");
        str_util::wstr2str(a,_wraw.c_str());
        return (float)atof(a.c_str());
    }
}

cast_t::operator const char*()const{
    return _raw.c_str();
}

cast_t::operator const wchar_t*()const{
	return _wraw.c_str();
}
