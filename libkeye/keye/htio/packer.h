// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: packer.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _packer_h_
#define _packer_h_

namespace keye{
// --------------------------------------------------------
// Packet process
// --------------------------------------------------------
static const size_t	PACKET_SIZE=8*1024;

struct PacketWrapper{
			PacketWrapper(void* b=nullptr,size_t l=0):data(b),length(l){}
	size_t	length;
	//warning: volatile pointer
	void*	data;
};
	
class Packer{
public:
	typedef unsigned short	header_type;
	//simple stash
	virtual Packer& operator<<(const PacketWrapper& i){
		auto used=_tail-_data;
		if(i.length<=PACKET_SIZE-used){
			memcpy(_tail,i.data,i.length);
			_tail+=i.length;
		}
		return *this;
	}

	//simple unstash
	virtual Packer& operator>>(PacketWrapper& o){
		auto used=_tail-_data;
		o.data=_data;
		o.length=used;
		_tail=_data;
		return *this;
	}

	friend const PacketWrapper& operator>>(const PacketWrapper& i,Packer& o){
		o<<i;
		return i;
	}

	friend const PacketWrapper& operator<<(PacketWrapper& o,Packer& i){
		i>>o;
		return o;
	}

	size_t Capacity(){
		return _tail-_data;
	}
protected:
	Packer()	{ _tail=_data; }
	char*		_tail;
	char		_data[PACKET_SIZE];
};

class HeadPacker:public Packer{
public:
	//insert packet size as header
	virtual Packer& operator<<(const PacketWrapper& i){
		auto used=_tail-_data;
		if(i.length+sizeof(header_type)<=PACKET_SIZE-used){
			*(header_type*)_tail=(header_type)i.length;
			_tail+=sizeof(header_type);
			memcpy(_tail,i.data,i.length);
			_tail+=i.length;
		}
		return *this;
	}

	friend const PacketWrapper& operator>>(const PacketWrapper& i,HeadPacker& o){
		o<<i;
		return i;
	}
};

class HeadUnpacker:public Packer{
	char*	_ptr;
public:
	HeadUnpacker():_ptr(_data){}

	virtual Packer& operator>>(PacketWrapper& o){
		//move forward
		if(_ptr>=_tail){
			_tail=_data;
		}else if(_ptr>_data){
			memmove(_data,_ptr,_tail-_ptr);
			_tail-=_ptr-_data;
		}
		_ptr=_data;

		auto length=*(header_type*)_ptr;
		if(length==(header_type)0||length>(header_type)0x7fff){ //[0,32k]
			//wrong packet!
			o.length=0;
			return *this;
		}

		size_t used=_tail-_data;
		if(used<length+sizeof(header_type)){
			//imcomplete
			o.length=0;
			return *this;
		}

		_ptr+=sizeof(header_type);
		o.data=_ptr;
		o.length=length;
		_ptr+=length;

		//don't move forward this time, or will override output
		return *this;
	}

	friend const PacketWrapper& operator<<(PacketWrapper& o,HeadUnpacker& i){
		i>>o;
		return o;
	}
};
// --------------------------------------------------------
// PackHelper
// --------------------------------------------------------
template <typename SH,typename Handler>
class PackHelper{
public:
	void	send(SH& sh,PacketWrapper& pw,bool immediately=true){
		packer<<pw;
		if(immediately){
			packer>>pw;
			sh.send(pw.data,pw.length);
		}
	}

	void	flush(SH& sh){
		if(packer.Capacity()>0){
			PacketWrapper pw;
			packer>>pw;
			sh.send(pw.data,pw.length);
		}
	}

	void	on_read(SH& sh,Handler& h,PacketWrapper& pw){
		unpacker<<pw;
		while(true){
			unpacker>>pw;
			if(pw.length>0)
				h.on_message(sh,pw);
			else break;
		}
	}
private:
	HeadPacker		packer;
	HeadUnpacker	unpacker;
};
// --------------------------------------------------------
};
#endif // _packer_h_
