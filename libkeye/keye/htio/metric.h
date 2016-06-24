// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: metric.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _metric_h_
#define _metric_h_

#pragma warning(disable:4251)	//avoid export implementation class

namespace keye{
// --------------------------------------------------------
/* flow metric:helper to record flow from service,
	it always embeds in work_handler */
// --------------------------------------------------------
class flow_metric_impl;
class KEYE_API flow_metric{
public:
	//update connects
	void		on_open();
	void		on_close();
	//update read/write bytes and count
	void		on_read(std::size_t bytes_transferred);
	void		on_write(std::size_t bytes_transferred);
	//update average read/write bytes from last call
	bool		on_timer(size_t elapsed_milliseconds);
				flow_metric();
	struct metric_t{
		//read/write bytes and count per second
		size_t	read_bytes,write_bytes,read_count,write_count;
	};
	metric_t	metric;
	//current connect count
	size_t		connects;
private:
	std::shared_ptr<flow_metric_impl>	_impl;
};
// --------------------------------------------------------
};
#endif // _metric_h_