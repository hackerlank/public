// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: char_string.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#ifndef _char_string_h_
#define _char_string_h_

namespace keye{
// --------------------------------------------------------
// str_util: string utilities
// --------------------------------------------------------
class KEYE_API str_util{
public:
	static void				split_lines(std::vector<std::string>&,const std::string&);
	static void				wsplit_lines(std::vector<std::wstring>&,const std::wstring&);
	static bool				wstr2str(std::string&,const wchar_t*);
	static bool				str2wstr(std::wstring&,const char*);
	static std::string		bytes2hex(const unsigned char*,size_t);
};
// --------------------------------------------------------
};// namespace
#endif
