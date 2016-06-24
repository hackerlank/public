#include "stdafx.h"
#include <assert.h>
#include "protocal.h"
#include "robot.h"
#include "autobot.h"

using namespace keye;
// --------------------------------------------------------
class MegaSvc:public ServiceBase{
public:
			MegaSvc(work_handler& w,myalloc& a,size_t ios=1,size_t works=1,size_t rb_size=510);
	void	add(size_t,Robot*);
	Robot*	find(size_t);
	void	remove(size_t);
	//current robot index
	size_t	index;
private:
	std::map<size_t,Robot*>	robots;
};
// --------------------------------------------------------
template<typename _Ax=std_ax_t>
class RobotWx:public SessionHandlerBase<Robot,_Ax>{
	typedef SessionHandlerBase<Robot,_Ax> _MyBase;
public:
					RobotWx(_Ax& ax):_MyBase(ax),_auto_client(true){}
	virtual void	on_open(svc_handler& sh){
		_MyBase::on_open(sh);
		if(_auto_client){
			MegaSvc* svc=(MegaSvc*)_MyBase::_s;
			char cidx[32];
			sprintf(cidx,"test%d",svc->index++);
			auto id=sh.id();

			auto r=(Robot*)sh.sptr().get();
			r->_name=cidx;
			svc->add(id,r);
			r->login();
		}
	}
	virtual void	on_close(svc_handler& sh){
		_MyBase::on_close(sh);
		LOG("Closed\n");
	}
	virtual bool	on_timer(svc_handler& sh,size_t id,size_t milliseconds){
		bool ret=_MyBase::on_timer(sh,id,milliseconds);
		if(WRITE_TIMER==id&&_MyBase::_s){
			if(_auto_client){
				if(auto r=(Robot*)sh.sptr().get())
					r->action();
			}else if(_MyBase::_s->send){
				auto& p=*(packet_t*)_buf;
				auto& cp=*(packet_crc_t*)_buf;
				_buf[_MyBase::_s->pack-1]='\0';
				cp.length=_MyBase::_s->pack-sizeof(packet_t);
				cp.crc=CMSG_ROBOT;//crc(cp.data,_s->pack-sizeof(packet_crc_t),16);

				sh.send(p.data,p.length);
			}
		}
		return ret;
	}
private:
	bool		_auto_client;
	char		_buf[WRITE_MAX];
};
// --------------------------------------------------------
MegaSvc::MegaSvc(work_handler& w,myalloc& a,size_t ios,size_t works,size_t rb_size)
	:ServiceBase(w,a,ios,works,rb_size)
	,index(5000)
{}

void MegaSvc::add(size_t id,Robot* r){
	if(!find(id)){
		robots.insert(std::make_pair(id,r));
	}
}

void MegaSvc::remove(size_t id){
	robots.erase(id);
}

Robot* MegaSvc::find(size_t id){
	auto i=robots.find(id);
	return i==robots.end()?nullptr:i->second;
}

static int _myclient(const char* host,unsigned short port,size_t ios=1,size_t works=1){
	myalloc a;
	RobotWx<myalloc> w(a);
	size_t rb_size=1460;
	MegaSvc s(w,a,ios,works,rb_size);
	w.set_svc(s);
	s.client=true;
	_host=host;
	_port=port;
	wait_for(s);
	return 0;
}
int main(int argc,char* argv[]){
//	auto f=freopen("log.txt","w+",stdout);
	const char* host[]={
		"127.0.0.1",
		"192.168.28.231",
		"121.10.118.77"};
	unsigned short port=8891;
	_myclient(host[0],port,4);

	keye::pause();
	return 0;
}
// --------------------------------------------------------
class autobot_impl{
public:
	int		run(int argc,char* argv[]);
	void	stop();
	bool	stopped();
private:
	myalloc								_ax;
	std::shared_ptr<RobotWx<myalloc>>	_hx;
	std::shared_ptr<MegaSvc>			_sx;
};

int autobot_impl::run(int argc,char* argv[]){
	size_t io_thread=4,wk_thread=1;
	size_t rb_size=1460;

//	auto f=freopen("log.txt","w+",stdout);
	const char* host[]={
		"127.0.0.1",
		"192.168.28.231",
		"121.10.118.77"};
	unsigned short port=8891;

	auto& a=_ax;
	_hx.reset(new RobotWx<myalloc>(a));
	_sx.reset(new MegaSvc(*_hx,a,io_thread,wk_thread,rb_size));
	_hx->set_svc(*_sx);
	_sx->client=true;
	_host=host[0];
	_port=port;
	wait_for(*_sx);

	return 0;
}

void autobot_impl::stop(){
	if(_sx)_sx->svc.close();
}

bool autobot_impl::stopped(){
	return _sx?_sx->svc.closed():true;
}
// --------------------------------------------------------
int Autobot::run(int argc,char* argv[]){
	_impl.reset(new autobot_impl);
	return _impl?_impl->run(argc,argv):0;
}

void Autobot::stop(){
	if(_impl)_impl->stop();
}

bool Autobot::stopped(){
	return _impl?_impl->stopped():true;
}
