// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: op_packer.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _op_packer_h_
#define _op_packer_h_

namespace keye{
// --------------------------------------------------------
// OpPacket process
// --------------------------------------------------------
inline int pack_opcode(unsigned short len,unsigned short opcode){
	//length doesn't include sizeof opcode
	return (0xffff0000&(int)len<<16)|(0xffff&(int)opcode);
}

inline unsigned short unpack_opcode(int value,unsigned short& len){
	len=(unsigned short)(value>>16);
	return (unsigned short)value;
}
	
class OpPacker:public Packer{
public:
	//insert packet size and opcode as header
	virtual Packer& operator<<(const PacketWrapper& i){
		auto used=_tail-_data;
		header_type length=0;
		auto opcode=unpack_opcode((int)i.length,length);
		if(length+2*sizeof(header_type)<=PACKET_SIZE-used){
			*(header_type*)_tail=(header_type)length;
			_tail+=sizeof(header_type);
			*(header_type*)_tail=(header_type)opcode;
			_tail+=sizeof(header_type);
			memcpy(_tail,i.data,length);
			_tail+=length;
		}
		return *this;
	}

	friend const PacketWrapper& operator>>(const PacketWrapper& i,OpPacker& o){
		o<<i;
		return i;
	}
};

class OpUnpacker:public Packer{
	char*	_ptr;
public:
	OpUnpacker():_ptr(_data){}

	virtual Packer& operator>>(PacketWrapper& o){
		//move forward
		if(_ptr>=_tail){
			_tail=_data;
		}else if(_ptr>_data){
			memmove(_data,_ptr,_tail-_ptr);
			_tail-=_ptr-_data;
		}
		_ptr=_data;

		size_t used=_tail-_data;
		auto length=*(header_type*)_ptr;
		if(used==0||length==(std::size_t)0||length>(std::size_t)0x40000000){
			//wrong packet!
			o.length=0;
			_tail=_data; 
			return *this;
		}

		if(used<length+sizeof(header_type)){
			//imcomplete
			o.length=0;
			return *this;
		}

		//pack length with opcode
		_ptr+=sizeof(header_type);
		auto opcode=*(header_type*)_ptr;
		_ptr+=sizeof(header_type);
		o.length=pack_opcode(length,opcode);
		o.data=_ptr;
		_ptr+=length;

		//don't move forward this time, or will override output
		return *this;
	}

	friend const PacketWrapper& operator<<(PacketWrapper& o,OpUnpacker& i){
		i>>o;
		return o;
	}
};
// --------------------------------------------------------
// OpPackHelper
// --------------------------------------------------------
template <typename SH,typename Handler>
class OpPackHelper{
public:
	void	send(SH& sh,void* buf,size_t len,size_t opcode,bool immediately=true){
		//pack opcode and push
		auto header=pack_opcode((unsigned short)len,(unsigned short)opcode);
		PacketWrapper pw(buf,header);
		packer<<pw;
		if(immediately){
			//send raw packet
			packer>>pw;
			sh.send(pw.data,pw.length);
		}
	}

	void	flush(SH& sh){
		if(packer.Capacity()>0){
			PacketWrapper pw;
			//send raw packet
			packer>>pw;
			sh.send(pw.data,pw.length);
		}
	}

	void	on_read(SH& sh,Handler& h,void* buf,size_t len){
		//raw data
		PacketWrapper pw(buf,len);
		unpacker<<pw;
		while(true){
			//unpacked length and opcode
			unpacker>>pw;
			if(pw.length>0)
				h.on_message(sh,pw);
			else break;
		}
	}
private:
	OpPacker	packer;
	OpUnpacker	unpacker;
};
// --------------------------------------------------------
};
#endif // _op_packer_h_