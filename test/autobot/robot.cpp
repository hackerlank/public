#include "stdafx.h"
#include "protocal.h"
#include "robot.h"
#include "robot_ai.h"

using namespace keye;
Robot::Robot(keye::svc_handler& sh,myalloc& ax)
:SessionBase(sh,ax){
}

void Robot::login(){
	//auth
	char _buf[64];
	auto& p=*(packet_t*)_buf;
	auto& cp=*(packet_crc_t*)_buf;
	cp.length=2*_name.size()+4;
	cp.crc=CMSG_AUTH_SESSION;
	char* ptr=(char*)cp.data;
	buf_set(&ptr,_name);
	buf_set(&ptr,_name);
	send(p);
}

void Robot::_handle(packet_crc_t& p){
	std::shared_ptr<char> spo;
	auto buf=(const char*)p.data;
	switch(p.crc){
	case SMSG_AUTH_RESPONSE:{
		//char enum
		packet_crc_t msg;
		msg.length=2;
		msg.crc=CMSG_CHAR_ENUM;
		send(*(packet_t*)&msg);
		break;
	}
	case SMSG_CHAR_ENUM:{
		unsigned char num=0;
		buf_get(num,&buf);
		for(auto i=0;i<num;++i){
			int id,len,race,clazz,gender;
			std::string name;
			buf_get(guid,&buf);
			buf_get(id,&buf);
			buf_get(len,&buf);
			buf_get(name,&buf);
			buf_get(race,&buf);
			buf_get(clazz,&buf);
			buf_get(gender,&buf);
			break;
		}
		if(0==num){
			//create role
			unsigned char sex=0,career=1,gen=1;
			unsigned short name_len=_name.length();
			auto len=sizeof(packet_crc_t)+sizeof(name_len)+_name.length()+3*sizeof(char);
			spo.reset(new char[len],std::default_delete<char[]>());
			auto po=(packet_crc_t*)spo.get();
			auto obuf=(char*)po->data;
			buf_set(&obuf,name_len);
			buf_set(&obuf,_name);--obuf;
			buf_set(&obuf,sex);
			buf_set(&obuf,career);
			buf_set(&obuf,gen);
			po->length=len-sizeof(packet_t);
			po->crc=CMSG_CREAT_ROLE;
			send(*(packet_t*)po);
		}else{
			//enter scene
			auto len=sizeof(packet_crc_t)+sizeof(guid);
			spo.reset(new char[len],std::default_delete<char[]>());
			auto po=(packet_crc_t*)spo.get();
			auto obuf=(char*)po->data;
			buf_set(&obuf,guid);
			po->length=len-sizeof(packet_t);
			po->crc=CMSG_PLAYER_ENTERGAME;
			send(*(packet_t*)po);
/*
			//change scene
			Sleep(1000);
			AiChangeScene ai(*this);
			ai.push(1);	//196616É­ÁÖÈë¿Ú
			ai.action();
*/
		}
		break;
	}
	case SMSG_LOGIN_VERIFY_WORLD:{
		int sceneId;
		float x,y;
		int bag,bank,len;
		std::string name;
		buf_get(sceneId,&buf);
		buf_get(x,&buf);
		buf_get(y,&buf);
		buf_get(bag,&buf);
		buf_get(bank,&buf);
		buf_get(len,&buf);
		buf_get(name,&buf);
		//tele.txt
		x=290,y=390;	//scene 196612
		//random position
		const size_t area=50;
		float fx,fy;
//		int ix,iy;
		srand(_sh->id());
		fx=(float)rand();
		srand(_sh->id()&0x103d);
		fy=(float)rand();
		fx/=(float)RAND_MAX;
		fy/=(float)RAND_MAX;
		x+=area*fx,y+=area*fy;
		
		//create moving ai
		_ai.reset(new AiMove(*this,(int)x,(int)y));
		break;
	}
	case SMSG_PLAYER_MOVE:
		break;
	case CMSG_ROBOT:
		break;
	}
}

void Robot::action(e_action a){
	_action=a;
	switch(_action){
	case AI_WRITE:
		break;
	case AI_MOVE:
		break;
	}
	if(_ai)_ai->action();
}
