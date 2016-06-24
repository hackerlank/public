#include "stdafx.h"
#include "protocal.h"
#include "robot.h"
#include "robot_ai.h"

using namespace keye;
void AiMove::action(){
	const int f=3;
	if(++ipos>3)ipos=0;
	switch(ipos){
	case 0:
		_y-=f;
		break;
	case 1:
		_x+=f;
		break;
	case 2:
		_y+=f;
		break;
	case 3:
		_x-=f;
		break;
	}
	auto len=sizeof(packet_crc_t)+sizeof(robot.guid)+2*sizeof(int);
	std::shared_ptr<char> spo;
	spo.reset(new char[len],std::default_delete<char[]>());
	auto po=(packet_crc_t*)spo.get();
	auto obuf=(char*)po->data;
	buf_set(&obuf,robot.guid);
	buf_set(&obuf,_x);
	buf_set(&obuf,_y);
	po->length=len-sizeof(packet_t);
	po->crc=CMSG_PLAYER_MOVE;
	robot.send(*(packet_t*)po);
}

void AiChangeScene::push(size_t i){
	if(curr==-1)curr=0;
	scenes.push_back(i);
}

void AiChangeScene::action(){
	if(last!=curr){
		//ChangeScene
		auto scene_id=scenes[curr];
		auto len=sizeof(packet_crc_t)+sizeof(scene_id);
		std::shared_ptr<char> spo;
		spo.reset(new char[len],std::default_delete<char[]>());
		auto po=(packet_crc_t*)spo.get();
		auto obuf=(char*)po->data;
		buf_set(&obuf,scene_id);
		po->length=len-sizeof(packet_t);
		po->crc=CMSG_TELE;
		robot.send(*(packet_t*)po);

		//update pos
		auto sz=scenes.size();
		if(++curr>=sz)curr=0;
		if(++last>=sz)last=0;
	}
}