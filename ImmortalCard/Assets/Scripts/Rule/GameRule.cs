using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public abstract class GameRule {
	public List<int> Pile;
	public List<int>[] Hands;
	public List<bunch_t> Historical=new List<bunch_t>(); //historical game data
	public int Token,Banker;
	protected PlayerController aiController;
	public abstract PlayerController AIController{get;}

	public virtual int MaxCards{get{return 54;}}
	public virtual int MaxPlayer{get{return 3;}}

	public virtual List<bunch_t> Hint(Player player,int[] hands,bunch_t src_bunch){return new List<bunch_t>();}

	public MsgNCStart Deal(){
		Pile=new List<int>();

		var msg=new MsgNCStart();
		msg.Mid=pb_msg.MsgNcStart;
		msg.Banker=0;
		msg.Ante=10;
		msg.Multiple=1;

		deal(msg);
		return msg;
	}

	// pseudo-random number generator, using a time-dependent default seed value. 
	protected List<int> shuffle(List<int> list){
		System.Random random = new System.Random();
		for (int i = 0; i < list.Count; ++i){
			int var = random.Next(0, list.Count);
			int temp = list[i];
			list[i] = list[var];
			list[var] = temp;
		}
		
		return list;
	}

	protected virtual void deal(MsgNCStart msg){}
	protected virtual pb_enum verifyBunch(bunch_t bunch){return pb_enum.BunchA;}
	protected virtual bool compareBunch(bunch_t bunch,bunch_t hist){return true;}
	public virtual bool verifyDiscard(Player player,bunch_t bunch){return true;}
	public abstract int comparision(int x,int y);
	public virtual int transformValue(int val){return val;}
	public virtual int inverseTransformValue(int val){return val;}
}
