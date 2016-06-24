// --------------------------------------------------------
/*Copyright KeyeLeo.All rights reserved.
 *
 *File		: packet.h
 *Desc		: 
 *Version	: 1.0
 *Program	: KeyeLeo
 *Date		: 2012-10-1
 */
// --------------------------------------------------------
#ifndef _packet_h_
#define _packet_h_

namespace keye{
// --------------------------------------------------------
#pragma warning(disable:4200)
//a common protocal: each packet has the packet_t head,it provide clean cut packet.
struct KEYE_API packet_t{
	unsigned short	length;
	unsigned char	data[0];
};

class KEYE_API packet_handler{
public:
	virtual void	handle(svc_handler& sh,const packet_t& p)=0;
	virtual size_t	packet_length(const packet_t& p){
		return p.length;//+sizeof(packet_t)
	}
};

/*a packet_t unpacker
	_Hx-the real handler with on_read and send function
	_Ax-the allocator */
class KEYE_API packet_reader{
	typedef packet_handler _Hx;
	typedef htio_alloc _Ax;
	typedef _Ax::value_type alloc_value_type;
public:
					packet_reader(_Hx& hx,_Ax& ax);
	virtual			~packet_reader();
	virtual void	on_read(svc_handler& sh,void* buf,size_t trans);
protected:
	void*			_prepare(size_t len);
	void			_append(const void* buf,size_t len);
	struct upacket_t{
		size_t		cap,fin;	//buffer capacity,packet fin
		packet_t*	packet;
	};
	_Hx&			_hx;
	_Ax&			_ax;
	upacket_t		_unfin_packet;
	static size_t	_default_length;
};
// --------------------------------------------------------
};
#endif // _svc_handler_h_