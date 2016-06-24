#include "stdafx.h"
#include "utility_test.h"
#include <keye/utility/utility_fwd.h>

using namespace keye;
// --------------------------------------------------------
class PrintNode:public joint_observer{
public:
	virtual void	update(joint* n){
		if(n)
		printf("%d updated\n",n->id());
	}
};
// --------------------------------------------------------
class Player{
public:
	void			walk(){
		printf("I'm walking.\n");
	}
};

class Walk:public Behavior<>{
public:
					Walk(Player* p,joint_scheduler& sx)
						:Behavior(sx)
						,_player(p){}
	//derived from joint_observer,do real action
	virtual void	update(joint*){if(_player)_player->walk();};
private:
	Player*			_player;
};

void test_bahavior(){
	joint_scheduler jsch;
	Player player;
	s_ptr<behavior> walk(new Walk(&player,jsch));
	bh_signal sig(&walk);
}
// --------------------------------------------------------
void utility_test(){
	joint_scheduler g;
	PrintNode observer;
	const size_t N=100;
	joint* joints[N];
	for(int i=1;i<N;++i){
		joints[i]=joint::create(i);
		joints[i]->link(&observer);
	}
	for(int i=1;i<4;++i)
		g.root().insert(joints[i]);
	joints[1]->insert(joints[4]);
	joints[1]->insert(joints[5]);
	joints[2]->insert(joints[6]);
	joints[4]->insert(joints[7]);
	joints[4]->insert(joints[8]);
	joints[7]->insert(joints[9]);
	joints[8]->insert(joints[9]);
	joints[9]->insert(joints[5]);
	joints[5]->insert(joints[2]);
//	joints[2]->insert(joints[9]);

	g.start_update();

	pause();
}
