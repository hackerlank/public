// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: ICore.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "htio_fwd.h"
#include <mutex>

namespace keye{
class flow_metric_impl{
public:
				flow_metric_impl(flow_metric&);
	void		on_open();
	void		on_close();
	void		on_read(std::size_t bytes_transferred);
	void		on_write(std::size_t bytes_transferred);
	bool		on_timer(size_t elapsed_milliseconds);
private:
	flow_metric&			_base;
	flow_metric::metric_t	_current,_last;
	std::mutex				_mutex_flow,_mutex_connect;
};

flow_metric_impl::flow_metric_impl(flow_metric& base):_base(base){
	memset(&_current,0,sizeof(_current));
	memset(&_last,0,sizeof(_last));
}

void flow_metric_impl::on_open(){
//	boost::mutex::scoped_lock lock(_mutex_connect);
	std::lock_guard<std::mutex> lock(_mutex_connect);
	++_base.connects;
}

void flow_metric_impl::on_close(){
//	boost::mutex::scoped_lock lock(_mutex_connect);
	std::lock_guard<std::mutex> lock(_mutex_connect);
	--_base.connects;
}

void flow_metric_impl::on_read(std::size_t bytes_transferred){
//	boost::mutex::scoped_lock lock(_mutex_flow);
	std::lock_guard<std::mutex> lock(_mutex_flow);
	++_current.read_count;
	_current.read_bytes+=bytes_transferred;
}

void flow_metric_impl::on_write(std::size_t bytes_transferred){
//	boost::mutex::scoped_lock lock(_mutex_flow);
	std::lock_guard<std::mutex> lock(_mutex_flow);
	++_current.write_count;
	_current.write_bytes+=bytes_transferred;
}

bool flow_metric_impl::on_timer(size_t elapsed_milliseconds){
	if(elapsed_milliseconds){
		_base.metric.read_bytes=1000*(_current.read_bytes-_last.read_bytes)/elapsed_milliseconds;
		_base.metric.write_bytes=1000*(_current.write_bytes-_last.write_bytes)/elapsed_milliseconds;
		_base.metric.read_count=1000*(_current.read_count-_last.read_count)/elapsed_milliseconds;
		_base.metric.write_count=1000*(_current.write_count-_last.write_count)/elapsed_milliseconds;
		memcpy(&_last,&_current,sizeof(_current));
	}
	return true;
}
// --------------------------------------------------------
flow_metric::flow_metric():connects(0){
	memset(&metric,0,sizeof(metric));
	_impl.reset(new flow_metric_impl(*this));
}

void flow_metric::on_open(){
	if(_impl)_impl->on_open();
}

void flow_metric::on_close(){
	if(_impl)_impl->on_close();
}

void flow_metric::on_read(std::size_t bytes_transferred){
	if(_impl)_impl->on_read(bytes_transferred);
}

void flow_metric::on_write(std::size_t bytes_transferred){
	if(_impl)_impl->on_write(bytes_transferred);
}

bool flow_metric::on_timer(size_t elapsed_milliseconds){
	return _impl?_impl->on_timer(elapsed_milliseconds):true;
}
};