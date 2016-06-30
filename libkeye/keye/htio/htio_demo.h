// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: htio_demo.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _htio_demo_h_
#define _htio_demo_h_

#include <assert.h>
// --------------------------------------------------------
// forward declaration
// --------------------------------------------------------
namespace keye{
int KEYE_API myclient(const char* host,unsigned short port,size_t ios=1,size_t works=1);
int KEYE_API myserver(unsigned short port,size_t ios=1,size_t works=1);
};
// --------------------------------------------------------
// implementation
// --------------------------------------------------------
#define FLOW_TIMER 999
#define WRITE_TIMER 998
#define WRITE_MIN 64
#define WRITE_MAX 4096
#define WRITE_FREQ 250

using namespace keye;
#pragma warning(disable:4200)

// --------------------------------------------------------
// RawService
// --------------------------------------------------------
class RawService:public service{
public:
	RawService(): client(false)
		,show_status(true)
		,response(false)
		,send(false)
		,echo(false)
		,crc(true)
		,interval(WRITE_FREQ)
		,pack(WRITE_MIN)
		,conns(0){}
	virtual	~RawService(){}
	bool	client,show_status,response,send,echo,crc;
	size_t	conns,interval,pack;
protected:
	flow_metric	_metric;
};

class RawServer:public RawService{
public:
	virtual void	on_open(svc_handler&){
		_metric.on_open();
	}
	virtual void	on_close(svc_handler&){
		_metric.on_close();
	}
	virtual void	on_read(svc_handler& sh,void* buf,size_t sz){
		_metric.on_read(sz);

		if(crc){
			//				auto sc=crc(cp.data,p.length-sizeof(packet_crc_t),16);
		}
		if(echo)
			KEYE_LOG("read %d:%s\n",(int)sz,(char*)buf);
		if(response)
			sh.send(buf,sz);
	}
	virtual void	on_write(svc_handler&,void*,size_t sz){
		_metric.on_write(sz);
	}
	virtual bool	on_timer(svc_handler&,size_t id,size_t milliseconds){
		if(FLOW_TIMER==id){
			_metric.on_timer(milliseconds);
			if(show_status)
				KEYE_LOG("connects:%d, rb:%dk/s, wb:%dk/s, rc:%d/s, wc:%d/s\n",(int)_metric.connects,
					(int)_metric.metric.read_bytes>>10,(int)_metric.metric.write_bytes>>10,(int)_metric.metric.read_count,(int)_metric.metric.write_count);
		}
		return true;
	}
};

class RawClient:public RawService{
public:
	virtual void	on_open(svc_handler& sh){
		sh.set_timer(WRITE_TIMER,WRITE_FREQ);
	}
	virtual void	on_read(svc_handler& sh,void* buf,size_t sz){
		if(echo)
			KEYE_LOG("read %d:%s\n",(int)sz,(char*)buf);
	}
	virtual bool	on_timer(svc_handler& sh,size_t id,size_t milliseconds){
		bool ret=true;
		if(WRITE_TIMER==id){
			if(interval!=milliseconds){
				sh.set_timer(WRITE_TIMER,interval);
				ret=false;
			}
			if(send){
				_buf[pack-1]='\0';
				sh.send(_buf,pack);
			}
		}
		return ret;
	}
private:
	char _buf[WRITE_MAX];
};

// --------------------------------------------------------
// SessionServer
// --------------------------------------------------------
template<typename SessionType>
class SessionServer:public service{
public:
	virtual ~SessionServer(){}
	virtual void	on_open(svc_handler& sh){
		sessions.insert(std::make_pair(sh.id(),std::shared_ptr<SessionType>(new SessionType)));
	}
	virtual void	on_read(svc_handler& sh,void* buf,size_t sz){
		PacketWrapper pw(buf,sz);
		packer<<pw;
		packer>>pw;
		if(pw.length){
			//do sth.
		}
	}
protected:
	std::map<size_t,std::shared_ptr<SessionType>> sessions;
	HeadUnpacker packer;
};
// --------------------------------------------------------
// helpers
// --------------------------------------------------------
void wait_for(RawService& s);
static std::string _host;
static unsigned _port=0;
namespace keye{
inline int myclient(const char* host,unsigned short port,size_t ios,size_t works){
	RawClient rc;
	rc.client=true;
	size_t rb_size=1460;
	_host=host;
	_port=port;
	wait_for(rc);
	return 0;
}

inline int myserver(unsigned short port,size_t ios,size_t works){
	RawServer rs;
	size_t rb_size=1460;
	_port=port;
	rs.run(port);
	rs.set_timer(FLOW_TIMER,5000);
	wait_for(rs);
	return 0;
}};

inline void prompt(){
	KEYE_LOG("\ncommand list:\n \
	?:\t\tshow this\n \
	e:\t\techo messages\n \
	h[ip:port]:\tset host ip and port\n \
	i[ms]:\t\tsend interval\n \
	l[n]:\t\tsend packet length\n \
	n[n]:\t\tincreament connects\n \
	p:\t\tprint status\n \
	r:\t\tresponse messages\n \
	s:\t\tsend messages\n \
	t:\t\tshow status\n \
	x:\t\tclose\n \
");
}

inline void wait_for(RawService& s){
	prompt();
	bool exit=false;
	while(!exit||!s.closed())
	switch(std::getchar()){
	case 'e':
		s.echo=!s.echo;
		break;
	case 'h':
		if(s.client){
			char host[32];
			scanf("%s",&host);
			std::string str(host),s;
			size_t n=str.find(':');
			_host=str.substr(0,n);
			if(_host.length()<=2)_host="127.0.0.1";
			s=str.substr(n+1);
			_port=atoi(s.c_str());
			printf("set host %s:%d\n",_host.c_str(),_port);
		}
		break;
	case '?':
		prompt();
		break;
	case 'i':
		if(s.client){
			int interval=WRITE_FREQ;
			scanf("%d",&interval);
			s.interval=interval;
		}
		break;
	case 'l':
		if(s.client){
			int pack=WRITE_MIN;
			scanf("%d",&pack);
			s.pack=pack;
		}
		break;
	case 'n':
		if(s.client){
			int conns=1;
			scanf("%d",&conns);
			s.connect(_host.c_str(),_port,conns);
		}
		break;
	case 'p':
		KEYE_LOG("\ncommand list:\n \
echo:\t\t%s\n \
response:\t%s\n \
send:\t\t%s\n \
status:\t%s\n \
interval:\t%d\n \
length:\t%d\n \
host:\t\t%s:%d\n \
conns:\t\t%d\n \
",s.echo?"on":"off",s.response?"on":"off",s.send?"on":"off",s.show_status?"on":"off"
,(int)s.interval,(int)s.pack,_host.c_str(),_port,(int)s.conns);
		break;
	case 'r':
		s.response=!s.response;
		break;
	case 's':
		s.send=!s.send;
		break;
	case 't':
		s.show_status=!s.show_status;
		break;
	case 'w':
		s.crc=!s.crc;
		break;
	case 'x':
		{
		s.close();
		exit=true;
		break;
		}
	}
}
// --------------------------------------------------------
#endif