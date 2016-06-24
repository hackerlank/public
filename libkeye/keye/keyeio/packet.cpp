// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: packet.cpp
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-09-31
 */
// --------------------------------------------------------
#include "stdafx.h"
#include "keyeio_fwd.h"

using namespace boost::asio;
using namespace keye;
// --------------------------------------------------------
// Packet_reader
// --------------------------------------------------------
size_t Packet_reader::_default_length=2020;

Packet_reader::Packet_reader(_Hx& hx,_Ax& ax):_hx(hx),_ax(ax){
	memset(&_unfin_packet,0,sizeof(_unfin_packet));
}

Packet_reader::~Packet_reader(){
	if(Packet_t* p=_unfin_packet.packet)
		_ax.deallocate((alloc_value_type*)p,1);
};

void Packet_reader::on_read(Svc_handler& sh,void* buf,size_t trans){
	const size_t sz_head=sizeof(Packet_t);
	char* data=(char*)buf;
	// process old packet first
	do if(_unfin_packet.fin){
		//need head size at least
		if(_unfin_packet.fin<sz_head){
			const size_t sz_head_need=sz_head-_unfin_packet.fin;
			if(trans>=sz_head_need){
				_append(data,sz_head_need);
				trans-=sz_head_need;
				data+=sz_head_need;
			}else
				break;
		}
		//copy data
		std::size_t total=_hx.packet_length(*_unfin_packet.packet);
		std::size_t rest=total-_unfin_packet.fin;
		bool complete=trans>=rest;
		_append(data,complete?rest:trans);
		if(complete){
			//complete
			_hx.handle(sh,*_unfin_packet.packet);
			trans-=rest;
			data+=rest;
			//reset _unfin_packet
			_unfin_packet.fin=0;
		}else{
			//update packet and do nothing
			return;
		}
	}while(false);
	//process the rest
	while(trans>=sz_head){
		const Packet_t* p=(Packet_t*)data;
		std::size_t total=_hx.packet_length(*p);
		if(total==(std::size_t)0||total>(std::size_t)0x40000000){
			//wrong packet!
			//reset _unfin_packet
			_unfin_packet.fin=0;
			break;
		}else if(trans>=total){
			//has complete packet
			_hx.handle(sh,*p);
			trans-=total;
			data+=total;
		}else{
			//only imcomplete,copy to read buffer
			_prepare(total);
			_append(data,trans);
			trans=0;
			break;
		}
	}
	//residue
	if(trans)_append(data,trans);
}

void* Packet_reader::_prepare(size_t len){
	//need reallocate
	if(!_unfin_packet.packet||_unfin_packet.cap-_unfin_packet.fin<len){
		auto old=_unfin_packet.packet;
		//allocate
		_unfin_packet.cap=_unfin_packet.fin+len;
		//we donot like fragment
		if(_unfin_packet.cap<=sizeof(Packet_t))
			_unfin_packet.cap=_default_length;
		_unfin_packet.packet=(Packet_t*)_ax.allocate(_unfin_packet.cap);
		if(old){
			//cp old
			memcpy(_unfin_packet.packet,old,_unfin_packet.fin);
			//deallocate
			_ax.deallocate((alloc_value_type*)old,1);
		}else
			_unfin_packet.fin=0;
	}
	return _unfin_packet.packet;
}

void Packet_reader::_append(const void* buf,size_t len){
	if(_prepare(len)){
		char* ptr=(char*)_unfin_packet.packet+_unfin_packet.fin;
		memcpy(ptr,buf,len);
		_unfin_packet.fin+=len;
	}
}
