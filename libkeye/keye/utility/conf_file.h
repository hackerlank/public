// --------------------------------------------------------
/*Copyright Keye Knew.All rights reserved.
 *
 *File		: conf_file.h
 *Desc		: 
 *Version	: 1.0
 *Program	: Keye Knew
 *Date		: 2012-10-31
 */
// --------------------------------------------------------
#ifndef _conf_file_h_
#define _conf_file_h_

#pragma warning(disable:4251)	//avoid export implementation class

namespace keye{
// --------------------------------------------------------
// cast_t:a help type casting class
// --------------------------------------------------------
class KEYE_API cast_t{
	std::string		_raw;
public:
					cast_t(const char* =nullptr);
					operator int()const;
					operator float()const;
					operator const char*()const;
};
// --------------------------------------------------------
/* txt_file:file lines split by "\n","\r" or "\r\n".
	store by string and auto cast value returned */
// --------------------------------------------------------
class KEYE_API txt_file{
public:
					txt_file();
	virtual			~txt_file(){}
	bool			load(const char*);
	bool			save(const char*);

	//test print in console
	virtual void	print()const=0;
protected:
	virtual bool	_parse(const std::string&)=0;
	virtual bool	_make_buffer(std::string&)=0;
	bool			_check_endl(const std::string& buf,const std::string& endl);

	cast_t			_default;
	std::string		_endl;	//"\n","\r" or "\r\n"
	size_t			_endlen;
};
// --------------------------------------------------------
// csv_file:csv file splits by "\t" or ","
// --------------------------------------------------------
class KEYE_API csv_file:public txt_file{
public:
	csv_file(bool use_comma=true);
	size_t			rows()const;
	size_t			colulmns()const;
	/* retrieve values,usage:auto val=value(x,y)
	cast_t will auto cast types */
	const cast_t&	value(size_t row,size_t col)const;
	//test print in console
	virtual void	print()const;
private:
	virtual bool	_parse(const std::string&);
	bool			_parse_title(const std::string& buf);
	bool			_parse_content(const std::string& buf);

	bool			parse_csv(std::vector<std::wstring>&,const std::wstring&,bool tab=false);

	virtual bool	_make_buffer(std::string&);

	bool				_use_comma;
	size_t				_rows,_cols;
	std::vector<cast_t>	_grids;
};
// --------------------------------------------------------
// ini_cfg_file:elements inline split by "=".
// --------------------------------------------------------
class KEYE_API ini_cfg_file:public txt_file{
public:
	/* retrieve values,usage:auto val=value(name)
	cast_t will auto cast types */
	const cast_t&	value(const char*)const;
	//test print in console
	virtual void	print()const;
private:
	virtual bool	_parse(const std::string&);
	bool			_parse_content(const std::string& buf);

	virtual bool	_make_buffer(std::string&);

	//clean '\t' and space before and after string
	void			_clean(std::string&,char='\t');

	bool			parse_ini(std::map<std::wstring,std::wstring>& dict,const std::wstring& line);
		
	std::map<std::string,cast_t>	_map;
};
// --------------------------------------------------------
// tab_file:elements inline split by "\t"
// --------------------------------------------------------
class KEYE_API tab_file:public txt_file{
public:
					tab_file();
	size_t			rows()const;
	size_t			colulmns()const;
	/* retrieve values,usage:auto val=value(x,y)
		cast_t will auto cast types */
	const cast_t&	value(size_t row,size_t col)const;
	//test print in console
	virtual void	print()const;
private:
	virtual bool	_parse(const std::string&);
	bool			_parse_title(const std::string& buf);
	bool			_parse_content(const std::string& buf);

	virtual bool	_make_buffer(std::string&);

	size_t				_rows,_cols;
	std::vector<cast_t>	_grids;
};
// --------------------------------------------------------
// ini_file:elements inline split by "=".
// --------------------------------------------------------
class KEYE_API ini_file:public txt_file{
public:
	/* retrieve values,usage:auto val=value(name)
		cast_t will auto cast types */
	const cast_t&	value(const char*)const;
	//test print in console
	virtual void	print()const;
private:
	virtual bool	_parse(const std::string&);
	bool			_parse_content(const std::string& buf);

	virtual bool	_make_buffer(std::string&);

	//clean '\t' and space before and after string
	void			_clean(std::string&,char='\t');

	std::map<std::string,cast_t>	_map;
};
// --------------------------------------------------------
};// namespace
#endif
