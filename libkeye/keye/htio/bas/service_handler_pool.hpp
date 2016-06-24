//
// service_handler_pool.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License,Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_SERVICE_HANDLER_POOL_HPP
#define BAS_SERVICE_HANDLER_POOL_HPP

namespace bas {
#define __READ_BUFFER_SIZE 510
#define __TIMEOUT_SECONDS 0
#define __BAS_GRACEFUL_CLOSED_WAIT_DELAY 5
#define __ERROR_CONNECTION_LIMIT_ONLY4_TEST 99

/// A pool of service_handler objects.
template<typename Work_Handler,typename _Ax=std::allocator<char>,typename Socket_Service =boost::asio::ip::tcp::socket>
class service_handler_pool
  : private boost::noncopyable
{
public:
  /// The type of the service_handler.
  typedef service_handler<Work_Handler,_Ax,Socket_Service> service_handler_type;
  typedef boost::shared_ptr<service_handler_type> service_handler_ptr;

  /// Construct the service_handler pool.
  explicit service_handler_pool(Work_Handler* work_handler,
      std::size_t initial_pool_size=1,
	  _Ax* allocator=nullptr,
	  size_t rb_size=__READ_BUFFER_SIZE,
      std::size_t timeout_seconds =__TIMEOUT_SECONDS,
      std::size_t closed_wait_delay =__BAS_GRACEFUL_CLOSED_WAIT_DELAY)
    : service_handlers_(),
	  work_handler_(work_handler),
	  allocator_(allocator),
	  rb_size_(rb_size),
      initial_pool_size_(initial_pool_size),
      timeout_seconds_(timeout_seconds),
      closed_wait_delay_(closed_wait_delay),
      next_service_handler_(0)
  {
    BOOST_ASSERT(initial_pool_size !=0);

    // Create preallocated service_handler pool.
    for(std::size_t i =0; i < initial_pool_size_; ++i)
    {
      service_handler_ptr service_handler(make_handler());
      service_handlers_.push_back(service_handler);
    }
  }

  /// Destruct the pool object.
  ~service_handler_pool()
  {
	close();
  }

  /// Close all service_handler.
  void close()
  {
    for(std::size_t i =0,ii = service_handlers_.size();i<ii; ++i)
      service_handlers_[i]->close();
  }

  /// Get an service_handler to use.
  service_handler_ptr get_service_handler(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service)
  {
    service_handler_ptr service_handler;

    // Check the next handler is busy or not.
    if(!service_handlers_[next_service_handler_]->_is_busy())
    {
      service_handler =service_handlers_[next_service_handler_];
      if(++next_service_handler_==service_handlers_.size())
        next_service_handler_ =0;
    }
    else
      next_service_handler_ =0;

    // If the next handler is busy,create new handler.
    if(service_handler.get()==0)
    {
	  if(service_handlers_.size()<=__ERROR_CONNECTION_LIMIT_ONLY4_TEST)
	  service_handler.reset(make_handler());
      service_handlers_.push_back(service_handler);
    }

    // Bind the service handler with given io_service and work_service.
    service_handler->_bind(io_service,work_service);
    return service_handler;
  }

  /// Get an service_handler with the given mutex.
  service_handler_ptr get_service_handler(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service,
      boost::asio::detail::mutex& mutex)
  {
    // For client handler,need lock in multiple thread model.
    boost::asio::detail::mutex::scoped_lock lock(mutex);
    return get_service_handler(io_service,work_service);
  }

private:
  /// Make one service_handler.
  service_handler_type* make_handler()
  {
    return new service_handler_type(work_handler_,
		allocator_,
		rb_size_,
        timeout_seconds_,
        closed_wait_delay_);
  }

private:
  /// The pool of preallocated service_handler.
  std::vector<service_handler_ptr> service_handlers_;

  Work_Handler* work_handler_;
  /// Buffer allocator
  _Ax* allocator_;

  size_t rb_size_;
  /// Preallocated service_handler number.
  std::size_t initial_pool_size_;

  /// The expiry seconds of connection.
  std::size_t timeout_seconds_;

  /// The delay seconds before reuse again.
  std::size_t closed_wait_delay_;

  /// The next service_handler to use for a connection.
  std::size_t next_service_handler_;
};

} // namespace bas

#endif // BAS_SERVICE_HANDLER_POOL_HPP
