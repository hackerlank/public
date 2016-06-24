//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License,Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*
	1.Bind work_handler and allocator to service;
	2.Service provide 3 service(socket,timer and event) by io_service;
	3.Service dispatch(immediately) service to io_service(thread),
		io_service post(deferred) service to work_service(thread);
*/

#ifndef BAS_SERVER_HPP
#define BAS_SERVER_HPP

namespace bas {
/// The top-level class of the server.
template<typename Work_Handler,typename _Ax=std::allocator<char>,typename Socket_Service =boost::asio::ip::tcp::socket>
class service
  : private boost::noncopyable
{
public:
  /// The type of the service_handler.
  typedef service_handler<Work_Handler,_Ax,Socket_Service> service_handler_type;
  typedef boost::shared_ptr<service_handler_type> service_handler_ptr;
  typedef service_handler_pool<Work_Handler,_Ax,Socket_Service> service_handler_pool_type;

  /// Construct the service to listen on the specified TCP address and port.
  explicit service(Work_Handler* work_handler,
	  _Ax* alloator,
      std::size_t io_service_pool_size,
      std::size_t work_service_pool_size,
	  size_t rb_size=__READ_BUFFER_SIZE)
    : accept_service_pool_(1),
      io_service_pool_(io_service_pool_size),
      work_service_pool_(work_service_pool_size),
      service_handler_pool_(work_handler,1,alloator,rb_size),	//preallocated_handler_number
      acceptor_(accept_service_pool_.get_io_service()),
      started_(false)
  {
    // Start work_service_pool with nonblock to perform synchronous works.
    work_service_pool_.start();
    // Start io_service_pool with nonblock to perform asynchronous i/o operations.
    io_service_pool_.start();
	connected_=true;
  }

  /// Destruct the service object.
  ~service()
  {
    // Stop the service's io_service loop.
    stop();
  }

  /// Run the service's io_service loop.
  void run(unsigned short port=0,const char* address=nullptr){
    if(started_)
      return;
	//we do not open accept while port was 0
	if(port){
		if(address&&0<strlen(address))
			//auto address
			endpoint_.address(boost::asio::ip::address::from_string(address));
		endpoint_.port(port);

		// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
		acceptor_.open(endpoint_.protocol());
		acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		try{
			acceptor_.bind(endpoint_);
			acceptor_.listen();
		}catch(boost::system::system_error se){
			KEYE_LOG(se.what());
		}
		  // Accept new connection.
		accept_one();
	}

    started_ =true;
	KEYE_LOG("server started on %d, with %d io and %d work threads.\n",
		(int)port,(int)io_service_pool_.size(),(int)work_service_pool_.size());
    // Start accept_service_pool with block to perform asynchronous accept operations.
    accept_service_pool_.run();
  }

  /// Stop the service.
  void stop()
  {
    if(!started_&&!connected_)
      return;

	KEYE_LOG("start stop server.\n");
    // Close the acceptor in the same thread.
    acceptor_.get_io_service().dispatch(boost::bind(&boost::asio::ip::tcp::acceptor::close,&acceptor_));
    // Stop accept_service_pool from block.
    accept_service_pool_.stop();

    // Close service_handler.
    service_handler_pool_.close();

//	boost::this_thread::sleep(boost::posix_time::seconds(1));

    // Stop io_service_pool.
    io_service_pool_.stop();
    // Stop work_service_pool.
    work_service_pool_.stop();

    started_=connected_=false;
	KEYE_LOG("server stopped.\n");
  }

  /// Make an connection with given io_service and work_service.
  void connect(const std::string& address,unsigned short port,unsigned short conns=1)
  {
    // Connect with the internal endpoint.
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(address),port);
	for(unsigned short i=0;i<conns;++i)
	    connect(io_service_pool_.get_io_service(),work_service_pool_.get_io_service(),ep);
	KEYE_LOG("start %d connects to %s:%d.\n",conns,address.c_str(),port);
  }

  /// Setup a timer.
  void set_timer(size_t id,size_t milliseconds){
	  if(!timer_servic_handler_)
		  timer_servic_handler_=service_handler_pool_.get_service_handler(
			io_service_pool_.get_io_service(),work_service_pool_.get_io_service());
	  if(timer_servic_handler_)
		  timer_servic_handler_->set_timer(id,milliseconds);
  }

  /// Unset timer.
  void unset_timer(size_t id){
	  if(timer_servic_handler_)
		  timer_servic_handler_->unset_timer(id);
  }

  /// Post event.
  void post_event(void* buf,size_t length){
	  if(!event_service_handler_)
		  event_service_handler_=service_handler_pool_.get_service_handler(
			io_service_pool_.get_io_service(),work_service_pool_.get_io_service());
	  if(event_service_handler_)
		  event_service_handler_->post_event(buf,length);
  }
private:
  /// Start to accept one connection.
  void accept_one()
  {
    // Get new handler for accept,and bind with acceptor's io_service.
    service_handler_ptr handler =service_handler_pool_.get_service_handler(io_service_pool_.get_io_service(),
        work_service_pool_.get_io_service());
    // Use new handler to accept.
    acceptor_.async_accept(handler->socket().lowest_layer(),
        boost::bind(&service::handle_accept,
            this,
            boost::asio::placeholders::error,
            handler));
  }

  /// Make an connection with given io_service and work_service.
  void connect(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service,
      boost::asio::ip::tcp::endpoint& endpoint)
  {
    // Get new handler for connect.
    service_handler_ptr new_handler =service_handler_pool_.get_service_handler(io_service,work_service);
    // Use new handler to connect.
    new_handler->connect(endpoint);
  }

  /// Handle completion of an asynchronous accept operation.
  void handle_accept(const boost::system::error_code& e,
      service_handler_ptr handler)
  {
    if(!e)
    {
      // Start the first operation of the current handler.
      handler->start();
      // Accept new connection.
      accept_one();
    }
    else
      handler->_close(e);
  }

private:
  /// The pool of io_service objects used to perform asynchronous accept operations.
  io_service_pool accept_service_pool_;

  /// The pool of io_service objects used to perform asynchronous i/o operations.
  io_service_pool io_service_pool_;

  /// The pool of io_service objects used to perform synchronous works.
  io_service_pool work_service_pool_;

  /// The pool of service_handler objects.
  service_handler_pool_type service_handler_pool_;

  /// The timer and event service handler
  service_handler_ptr
	  timer_servic_handler_,event_service_handler_;	

  /// The acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The server endpoint.
  boost::asio::ip::tcp::endpoint endpoint_;

  // Flag to indicate that the server is started or not.
  bool started_,connected_;
};

} // namespace bas

#endif // BAS_SERVER_HPP
