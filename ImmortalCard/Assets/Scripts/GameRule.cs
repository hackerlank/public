﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public abstract class GameRule {
	public uint[] Pile;
	public List<uint>[] Hands;

	public virtual int MaxCards{get{return 54;}}
	public virtual uint MaxPlayer{get{return 3;}}

	public List<uint[]> Hint(uint[] hands,uint[] ids){
		var hints=new List<uint[]>();
		if(hands!=null&&ids!=null&&hands.Length>0&&ids.Length>0){

		}
		return hints;
	}

	public bool Verify(bunch_t curr,bunch_t last){
		return pb_enum.BunchInvalid!=verifyBunch(curr)&&compareBunch(curr,last);
	}

	public MsgNCStart Deal(){
		Pile=new uint[MaxCards];

		var msg=new MsgNCStart();
		msg.Mid=pb_msg.MsgNcStart;
		msg.Banker=0;
		msg.Ante=10;
		msg.Multiple=1;

		deal(msg);
		return msg;
	}

	// pseudo-random number generator, using a time-dependent default seed value. 
	protected uint[] shuffle(uint[] list){
		System.Random random = new System.Random();
		for (uint i = 0; i < list.Length; ++i){
			uint var = (uint)random.Next(0, list.Length);
			uint temp = list[i];
			list[i] = list[var];
			list[var] = temp;
		}
		
		return list;
	}

	protected virtual void deal(MsgNCStart msg){}
	protected virtual pb_enum verifyBunch(bunch_t bunch){return pb_enum.BunchA;}
	protected virtual bool compareBunch(bunch_t bunch,bunch_t hist){return bunch.Type>hist.Type;}
	public abstract int comparision(uint x,uint y);
	public virtual uint transformValue(uint val){return val;}
	public virtual uint inverseTransformValue(uint val){return val;}
}
