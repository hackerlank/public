//
// service_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License,Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_SERVICE_HANDLER_HPP
#define BAS_SERVICE_HANDLER_HPP
    
namespace bas {
//buffer aysnc read
//#define _READ_TIMER
#ifdef _READ_TIMER
static const std::size_t r_interval=1000;//ms
#endif

//buffer aysnc write
//#define _WRITE_TIMER
#ifdef _WRITE_TIMER
static const std::size_t wb_size=1460*2;
#endif

//buffer do read
//#define _DO_READ_TIMER
#ifdef _DO_READ_TIMER
static const std::size_t rb_size=1460*2;
#endif

#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
static const std::size_t w_interval=1000;//ms
struct upacket_t{
	unsigned short	cap,fin;	//buffer capacity,packet fin
	void*			buf;
};
#endif

template<typename,typename,typename> class service;
template<typename,typename,typename> class service_handler_pool;

/// Object for handle socket asynchronous operations.
template<typename Work_Handler,typename _Ax=std::allocator<char>,typename Socket_Service =boost::asio::ip::tcp::socket>
class service_handler
  : public boost::enable_shared_from_this<service_handler<Work_Handler,_Ax,Socket_Service> >,
    private boost::noncopyable{
public:
  using boost::enable_shared_from_this<service_handler<Work_Handler,_Ax,Socket_Service> >::shared_from_this;
  /// The type of the service_handler.
  typedef service_handler<Work_Handler,_Ax,Socket_Service> service_handler_type;
  /// The type of the work_handler.
  typedef Work_Handler work_handler_type;
  /// The type of the socket that will be used to provide asynchronous operations.
  typedef Socket_Service socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;
  typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

  /// Construct a service_handler object.
  explicit service_handler(work_handler_type* work_handler,
	  _Ax* allocator,
	  std::size_t r_size,
      std::size_t timeout_seconds,
      std::size_t closed_wait_delay)
    : work_handler_(work_handler),
	  allocator_(allocator),
      socket_(),
      timer_(),
      io_service_(0),
      work_service_(0),
      timer_count_(0),
	  rb_size_(r_size),
      stopped_(true),
      closed_(true),
	  read_ptr_(nullptr),
      timeout_seconds_(timeout_seconds),
      closed_wait_time_(boost::posix_time::seconds((long)closed_wait_delay)),
      restriction_time_(boost::posix_time::microsec_clock::universal_time()){
    BOOST_ASSERT(work_handler !=0);
    BOOST_ASSERT(closed_wait_delay !=0);
#ifdef _DO_READ_TIMER
	memset(&rpacket_,0,sizeof(rpacket_));
	if(rpacket_.buf=allocator_->allocate(rb_size))
		rpacket_.cap=rb_size;
	read_ptr_=(void*)allocator_->allocate(rb_size_);
#endif
#ifdef _WRITE_TIMER
	memset(&wpacket_,0,sizeof(wpacket_));
	if(wpacket_.buf=allocator_->allocate(wb_size))
		wpacket_.cap=wb_size;
	memset(&epacket_,0,sizeof(epacket_));
	if(epacket_.buf=allocator_->allocate(wb_size))
		epacket_.cap=wb_size;
#endif
  }

  ~service_handler(){
	  if(read_ptr_)
		allocator_->deallocate((char*)read_ptr_,1);
#ifdef _DO_READ_TIMER
	  if(rpacket_.buf)
		allocator_->deallocate((char*)rpacket_.buf,1);
#endif
#ifdef _WRITE_TIMER
	  if(wpacket_.buf)
		allocator_->deallocate((char*)wpacket_.buf,1);
	  if(epacket_.buf)
		allocator_->deallocate((char*)epacket_.buf,1);
#endif
  }

  /// Start the first operation.
  /// Usually the first asynchronous operation,can be call from any thread.
  void start(){
    BOOST_ASSERT(socket_.get() !=0);
    BOOST_ASSERT(timer_.get() !=0);
    BOOST_ASSERT(work_service_ !=0);
    // If start from connect,timer has been setted,don't set again.
    if(timer_count_==0)
      _set_expiry(timeout_seconds_);
    // Post to work_service for executing _do_open.
    work_service_->post(boost::bind(&service_handler_type::_do_open,
        shared_from_this()));
  }

  /// Start an asynchronous connect.
  /// Usually the first asynchronous operation,can be call from any thread.
  void connect(boost::asio::ip::tcp::endpoint& endpoint){
    BOOST_ASSERT(socket_.get() !=0);
    BOOST_ASSERT(timer_.get() !=0);
    BOOST_ASSERT(work_service_ !=0);

    // Set timer for connection timeout.
    // If reconnect,timer has been setted,don't set again.
    if(timer_count_==0)
      _set_expiry(timeout_seconds_);

    socket().lowest_layer().async_connect(endpoint,
        boost::bind(&service_handler_type::_handle_connect,
            shared_from_this(),
            boost::asio::placeholders::error));
  }

  /// Close the handler with error_code 0 from any thread.
  void close(){
    _close(boost::system::error_code(0,boost::system::get_system_category()));
  }

  /// Start an asynchronous operation from any thread to read any amount of data from the socket.
  void async_read_some(){
#ifdef _DO_READ_TIMER
	if(read_ptr_){
#else
	read_ptr_ = (void*)allocator_->allocate(rb_size_);
	if(nullptr!= read_ptr_){
#endif
		auto b=boost::asio::buffer(read_ptr_,rb_size_);
		io_service_->dispatch(boost::bind(&service_handler_type::_async_read_some_i<decltype(b)>,
			shared_from_this(),b));
	}
  }

  /// Start an asynchronous operation from any thread to write a certain amount of data to the socket.
  void async_write(void* buf,std::size_t length){
	  _async_real_write(buf,length);
  }

  /// Post event.
  void post_event(void* buf,std::size_t length){
	  _async_real_write(buf,length,true);
  }

  /// Setup a timer.
  void set_timer(std::size_t id,std::size_t milliseconds){
	  // Don't set timer while handler closed.
	  if(stopped_)
		  return;
	  std::map<std::size_t,timer_ptr>::iterator i=timers_.find(id);
	  timer_ptr timer;
	  if(timers_.end()==i){
		//using work io_service
		timer.reset(new boost::asio::deadline_timer(*work_service_));
		timers_.insert(std::make_pair(id,timer));
	  }else
		timer=i->second;
	  if(timer){
		timer->expires_from_now(boost::posix_time::milliseconds(milliseconds));
		timer->async_wait(boost::bind(&service_handler_type::_handle_timer,
			shared_from_this(),
			boost::asio::placeholders::error,id,milliseconds));
	  }
  }

  /// Unset timer.
  void unset_timer(std::size_t id){
	  std::map<std::size_t,timer_ptr>::iterator i=timers_.find(id);
	  if(timers_.end()!=i){
		timer_ptr timer=i->second;
        timer->cancel();
		timers_.erase(id);
	  }
  }

  /// Get the socket associated with the service_handler.
  socket_type& socket(){
	/* usage:
		boost::asio::ip::tcp::endpoint local_endpoint=socket().local_endpoint(),
			remote_endpoint=socket().remote_endpoint();
		boost::asio::ip::address address=remote_endpoint.address();
		unsigned short port=remote_endpoint.port();
		std::string str_addr=address.to_string();
	*/
    BOOST_ASSERT(socket_.get() !=0);
    return *socket_;
  }

	///the interface expsure
	std::shared_ptr<void>&	sptr(){
		return _s_ptr;
	}
private:
  template<typename,typename,typename> friend class service_handler_pool;
  template<typename,typename,typename> friend class service;

  /// Close the handler with error_code e from any thread.
  void _close(const boost::system::error_code& e)  {
    // The handler has been stopped,do nothing.
    if(stopped_)return;
    // Dispatch to io_service thread.
    io_service_->dispatch(boost::bind(&service_handler_type::_close_i,shared_from_this(),e));
  }

  /// Check handler is in busy or not.
  bool _is_busy(){
    return !closed_||(boost::posix_time::microsec_clock::universal_time() < restriction_time_);
  }

  /// Bind a service_handler with the given io_service and work_service.
  void _bind(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service){
    BOOST_ASSERT(timer_count_==0);
    closed_ =false;
    stopped_ =false;
    //socket_.reset(work_allocator.make_socket(io_service));
    socket_.reset(new socket_type(io_service));
    timer_.reset(new boost::asio::deadline_timer(io_service));
    io_service_ =&io_service;
    work_service_ =&work_service;
  }

  /// Start an asynchronous operation from io_service thread to read any amount of data to buffers from the socket.
  template<typename Buffers>
  void _async_read_some_i(Buffers& buffers){
    // The handler has been stopped,do nothing.
    if(stopped_)return;
	socket().async_read_some(buffers,
        boost::bind(&service_handler_type::_handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
			boost::asio::detail::buffer_cast_helper(buffers)));
  }

  /// Start an asynchronous operation from io_service thread to write buffers to the socket.
  template<typename Buffers>
  void _async_write_i(Buffers& buffers){
	auto wbuf=boost::asio::detail::buffer_cast_helper(buffers);
	// The handler has been stopped,break.
    if(stopped_)
		allocator_->deallocate((char*)wbuf,1);
	else
		boost::asio::async_write(socket(),
			buffers,
			boost::bind(&service_handler_type::_handle_write,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,wbuf));
  }

  void _async_real_write(void* buf,std::size_t length,bool event=false){
    if(buf&&length){
#ifdef _WRITE_TIMER
		auto& wpacket=event?epacket_:wpacket_;
		char* ptr=(char*)wpacket.buf;

		//overflow,flush the rest first
		if(length>wpacket.cap||wpacket.fin+length>wpacket.cap){
			_flush_write_packet(ptr,wpacket.fin,event);
			wpacket.fin=0;
		}

		if(length>wpacket.cap)
			//flush directly
			_flush_write_packet(buf,length,event);
		else{
			//copy to write buffer
			ptr+=wpacket.fin;
			memcpy(ptr,buf,length);
			wpacket.fin+=length;
			// check and set write timer for event
			if(!wtimer_){
				wtimer_.reset(new boost::asio::deadline_timer(*io_service_));
				_set_write_timer();
			}
		}
#else
		_flush_write_packet(buf,length,event);
#endif //_WRITE_TIMER
	}
  }

  void _flush_write_packet(void* buf,std::size_t length,bool event=false){
	//must make a copy,and deallocate after io service
	if(void* wbuf=(void*)allocator_->allocate(length)){
		memcpy(wbuf,buf,length);
		if(!event){
			auto b=boost::asio::buffer(wbuf,length);
			io_service_->dispatch(boost::bind(&service_handler_type::_async_write_i<decltype(b)>,
				shared_from_this(),b));
		}else{
			work_service_->post(boost::bind(&service_handler_type::_do_event,
				shared_from_this(),
				wbuf,length));
		}
	}
  }

  void _flush_read_packet(void* buf,std::size_t length,bool event=false){
#ifdef _DO_READ_TIMER
	//make a copy and post
	void* rbuf=(void*)allocator_->allocate(length);
	if(!rbuf)return;
	memcpy(rbuf,buf,length);
	buf=rbuf;
#endif
    // Post to work_service for executing _do_read.
    work_service_->post(boost::bind(&service_handler_type::_do_read,
        shared_from_this(),
        length,buf));
  }

  /// Set timer for connection.
  void _set_expiry(std::size_t timeout_seconds){
    if(timeout_seconds==0)return;
    BOOST_ASSERT(timer_.get() !=0);
    ++timer_count_;
    timer_->expires_from_now(boost::posix_time::seconds((long)timeout_seconds));
    timer_->async_wait(boost::bind(&service_handler_type::_handle_timeout,
        shared_from_this(),
        boost::asio::placeholders::error));
  }

  /// Handle completion of a connect operation in io_service thread.
  void _handle_connect(const boost::system::error_code& e){
    // The handler has been stopped,do nothing.
    if(stopped_)return;
    if(!e)
      start();
    else
      // Close with error_code e.
      _close_i(e);
  }

  /// Handle completion of a read operation in io_service thread.
  void _handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred,void* buf){
    // The handler has been stopped,do nothing.
    if(stopped_)return;

    if(!e){
#ifdef _DO_READ_TIMER
		char* ptr=(char*)rpacket_.buf;
		//overflow,flush the rest first
		if(bytes_transferred>rpacket_.cap||rpacket_.fin+bytes_transferred>rpacket_.cap){
			_flush_read_packet(ptr,rpacket_.fin);
			rpacket_.fin=0;
		}

		if(bytes_transferred>rpacket_.cap)
			//flush directly
			_flush_read_packet(buf,bytes_transferred);
		else{
			//copy to write buffer
			ptr+=rpacket_.fin;
			memcpy(ptr,buf,bytes_transferred);
			rpacket_.fin+=bytes_transferred;
		}
#else
	  _flush_read_packet(buf,bytes_transferred);
#endif
  	  // start the next read
#ifdef _READ_TIMER
	  _set_read_timer();
#else
	  async_read_some();
#endif
    }else
      // Close with error_code e.
      _close_i(e);
  }

  /// Handle completion of a write operation in io_service thread.
  void _handle_write(const boost::system::error_code& e,
      std::size_t bytes_transferred,void* wbuf){
    // The handler has been stopped,break.
	if(stopped_||e){
		allocator_->deallocate((char*)wbuf,1);
		// Close with error_code e.
		_close_i(e);
	}else{
		// Post to work_service for executing do_write.
		work_service_->post(boost::bind(&service_handler_type::_do_write,
			shared_from_this(),
			bytes_transferred,wbuf));
	}
  }

  /// Handle timeout of whole operation in io_service thread.
  void _handle_timer(const boost::system::error_code& e,std::size_t id,std::size_t milliseconds){
	if(!e){
		//break timer while false on_timer
		if(work_handler_->on_timer(*this,id,milliseconds))
			set_timer(id,milliseconds);
	}else{
//		unset_timer(id);
	}
  }

#ifdef _READ_TIMER
  /// Handle read timer
  void _handle_read_timer(const boost::system::error_code& e){
		if(stopped_||e)
			return;
		async_read_some();
  }
#endif
#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
  /// Handle write timer
  void _handle_write_timer(const boost::system::error_code& e){
		if(stopped_||e)
			return;
#ifdef _DO_READ_TIMER
		if(rpacket_.fin){
			//write
			char* ptr=(char*)rpacket_.buf;
			_flush_read_packet(ptr,rpacket_.fin);
			rpacket_.fin=0;
		}
#endif
#ifdef _WRITE_TIMER
		if(wpacket_.fin){
			//write
			char* ptr=(char*)wpacket_.buf;
			_flush_write_packet(ptr,wpacket_.fin);
			wpacket_.fin=0;
		}
		if(epacket_.fin){
			//event
			char* ptr=(char*)epacket_.buf;
			_flush_write_packet(ptr,epacket_.fin,true);
			epacket_.fin=0;
		}
#endif
		_set_write_timer();
  }
#endif

  /// Handle timeout of whole operation in io_service thread.
  void _handle_timeout(const boost::system::error_code& e){
    --timer_count_;

    // The handler has been stopped or timer has been cancelled,do nothing.
    if(stopped_||e==boost::asio::error::operation_aborted)
      return;

    if(!e)
      // Close with error_code boost::system::timed_out.
      _close_i(boost::system::error_code(boost::asio::error::timed_out,boost::system::get_system_category()));
    else
      // Close with error_code e.
      _close_i(e);
  }

  /// Close the handler in io_service thread.
  void _close_i(const boost::system::error_code& e){
    if(!stopped_){
      stopped_ =true;

      // Initiate graceful service_handler closure.
      boost::system::error_code ignored_ec;
      socket().lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both,ignored_ec);
      socket().lowest_layer().close();

	  // Stop timer first
	  for(std::map<std::size_t,timer_ptr>::iterator i=timers_.begin(),ii=timers_.end();i!=ii;++i){
		timer_ptr timer=i->second;
		timer->cancel();
	  }
#ifdef _READ_TIMER
	  if(rtimer_)
		  rtimer_->cancel();
#endif
#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
	  if(wtimer_)
		  wtimer_->cancel();
#endif

      // Timer has not been expired,or expired but not dispatched,
      // cancel it even if expired.
      if(timer_count_ !=0)
        timer_->cancel();

      // Post to work_service to executing do_close.
      work_service_->post(boost::bind(&service_handler_type::_do_close,
          shared_from_this(),
          e));
    }
  }

#ifdef _READ_TIMER
  void _set_read_timer(){
	rtimer_->expires_from_now(boost::posix_time::milliseconds(r_interval));
	rtimer_->async_wait(boost::bind(&service_handler_type::_handle_read_timer,
		shared_from_this(),
		boost::asio::placeholders::error));
  }
#endif
#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
  void _set_write_timer(){
	wtimer_->expires_from_now(boost::posix_time::milliseconds(w_interval));
	wtimer_->async_wait(boost::bind(&service_handler_type::_handle_write_timer,
		shared_from_this(),
		boost::asio::placeholders::error));
  }
#endif

  /// Do on_open in work_service thread.
  void _do_open(){
    // The handler has been stopped,do nothing.
    if(stopped_)
      return;
    // Call on_open function of the work handler.
    work_handler_->on_open(*this);
	// start the first read
#ifdef _READ_TIMER
	// read timer
	rtimer_.reset(new boost::asio::deadline_timer(*io_service_));
	_set_read_timer();
#else
	async_read_some();
#endif
#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
	// write timer
	wtimer_.reset(new boost::asio::deadline_timer(*io_service_));
	_set_write_timer();
#endif
 }

  /// Do on_read in work_service thread.
  void _do_read(std::size_t bytes_transferred,void* buf){
	  _do_real_read(bytes_transferred,buf);
  }

  /// Do on_read in work_service thread.
  void _do_real_read(std::size_t bytes_transferred,void* rbuf,bool event=false){
    // The handler has been stopped,do nothing.
    if(!stopped_&&rbuf&&bytes_transferred){
		// Call on_read function of the work handler.
		if(!event)
			work_handler_->on_read(*this,rbuf,bytes_transferred);
		else
			work_handler_->on_event(*this,bytes_transferred,rbuf);
	}
	if(rbuf){
		allocator_->deallocate((char*)rbuf,1);
//		read_ptr_=nullptr;
	}
  }

  /// Do on_write in work_service thread.
  void _do_write(std::size_t bytes_transferred,void* wbuf){
    if(!stopped_)
		// Call on_write function of the work handler.
		work_handler_->on_write(*this,bytes_transferred,wbuf);
	if(wbuf)
		allocator_->deallocate((char*)wbuf,1);
  }

  /// Do on_event in work_service thread.
  void _do_event(void* buf,std::size_t length){
	// Call on_event function of the work handler.
	_do_real_read(length,buf,true);
  }

  /// Do on_close and reset handler for next connaction in work_service thread.
  void _do_close(const boost::system::error_code& e){
    // Call on_close function of the work handler.
    work_handler_->on_close(*this,e);
    timer_.reset();
	timers_.clear();
    // Set restriction time before reuse,wait uncompleted operations to be finished.
    restriction_time_ =boost::posix_time::microsec_clock::universal_time()+closed_wait_time_;
    closed_ =true;
    // Leave socket to destroy delay for finishing uncompleted SSL operations.
    // Leave io_service_/work_service_ for finishing uncompleted operations.
  }

private:
  /// Work handler of the service_handler.
  work_handler_type* work_handler_;
  /// Buffer allocator
  _Ax* allocator_;
  /// Record the read pointer for deallocate
  void*	read_ptr_;

  std::size_t rb_size_;
#ifdef _READ_TIMER
  timer_ptr rtimer_;
#endif
#ifdef _DO_READ_TIMER
  upacket_t	rpacket_;
#endif
#ifdef _WRITE_TIMER
  upacket_t	wpacket_,epacket_;
#endif
#if (defined(_WRITE_TIMER)||defined(_DO_READ_TIMER))
  timer_ptr wtimer_;
#endif
  /// Socket for the service_handler.
  socket_ptr socket_;
  ///the interface expsure
  std::shared_ptr<void>	_s_ptr;

  /// The io_service for for asynchronous operations.
  boost::asio::io_service* io_service_;

  /// The io_service for for executing synchronous works.
  boost::asio::io_service* work_service_;

  /// Events timers
  std::map<std::size_t,timer_ptr> timers_;

  // Flag to indicate that the handler has been stopped and can not do synchronous operations.
  bool stopped_;
  // Flag to indicate that the handler has been closed and can be reuse again after some seconds.
  bool closed_;

  /// Timeout for accept or connect
  /// Timer for timeout operation.
  timer_ptr timer_;

  /// Count of waiting timer.
  std::size_t timer_count_;
  /// The expiry seconds of connection.
  std::size_t timeout_seconds_;
  /// For graceful close,if preallocated service_handler number is not huge enough,
  /// should delay some seconds before any handler reuse again.
  boost::posix_time::time_duration closed_wait_time_;
  /// The time of hander can use again.
  boost::posix_time::ptime restriction_time_;
};
} // namespace bas

#endif // BAS_SERVICE_HANDLER_HPP
