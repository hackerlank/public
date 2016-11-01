using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziRule: GameRule {

	public override int MaxCards{get{return 80;}}
	public override int MaxPlayer{get{return 3;}}

	protected override void deal(MsgNCStart msg){
		int id=0;
		for(int k=1;k<=3;++k){
			for(int i=1;i<=9;++i){
				for(int j=0;j<4;++j){
					id=k*1000+j*100+i;
					Pile.Add(id);
				}
			}
		}
		//shuffle
		Pile=shuffle(Pile);
		/*
		//deal
		for(int i=0;i<14;++i)
			msg.Hands.Add(Pile[i]);
		//other hands
		Hands=new List<int>[2]{new List<int>(),new List<int>()};
		for(int i=14;i<14+13;++i)
			Hands[0].Add(Pile[i]);
		for(int i=14+13;i<14+13*2;++i)
			Hands[1].Add(Pile[i]);
		*/
	}

	public override List<bunch_t> Hint(Player player,bunch_t src_bunch,bool filtered=false){
		//for meld: BUNCH_AAA,BUNCH_AAAA,BUNCH_WIN; no BUNCH_ABC no BUNCH_WIN
		var hints=new List<bunch_t>();
		var hands=new List<int>(player.playData.Hands);
		if(player==null||src_bunch==null||hands.Count<=0)
			return hints;

		int pos=player.pos;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return hints;
		}

		var card=src_bunch.Pawns[0];
		var bDraw=(src_bunch.Type==pb_enum.BunchA||card==Configs.invalidCard);

		//hint3
		var output3=new bunch_t();
		if(hint3(player,card,output3,bDraw)){
			output3.Pos=pos;
		}

		//hint
		bool meltable=(Token==pos&&bDraw)||pos==(Token+1)%MaxPlayer;
		var output1=new List<bunch_t>();
		if(meltable)
			hint(player,card,output1);

		var all=new List<bunch_t>();
		var win=IsWin(player,card,all,bDraw);
		if(win){
			//game over
			var output=new bunch_t();
			output.Pos=pos;
			output.Type=pb_enum.BunchWin;
			output.Pawns.Add(card);
			hints.Add(output);
		}

		//handle past card
		var past=false;
		foreach(var c in player.unpairedCards)if(c/1000==card/1000 && c%100==card%100){
			past=true;
			break;
		}
		if(past){
			output1.Clear();
			if(output3.Pawns.Count==3)
				output3.Pawns.Clear();
		}
		if(output3.Pawns.Count>0)hints.Add(output3);
		if(output1.Count>0)hints.AddRange(output1);

		var count=hints.Count;
		if(count>0){
			string str=count+" hints from pos="+pos+": ";
			foreach(var bunch in hints)
				str+=Player.bunch2str(bunch);
			Debug.Log(str);
		}
		return hints;
	}

	public override void Meld(Player player,bunch_t bunch){
		//remove from hand
		foreach(var card in bunch.Pawns)player.playData.Hands.Remove(card);

		var B=bunch.Pawns[0];
		//remove from desk or AAAA
		switch (bunch.Type) {
		case pb_enum.PhzBbbB:
		case pb_enum.PhzAaaa:
			foreach(var AAA in player.AAAs){
				var A=AAA.Pawns[0];
				if(A/1000==B/1000 && A%100==B%100){
					player.AAAs.Remove(AAA);
					break;
				}
			}
			break;
		case pb_enum.PhzBbbbdesk:
		case pb_enum.PhzAaaadesk:
			foreach(var bun in player.playData.Bunch){
				var A=bun.Pawns[0];
				if(A/1000==B/1000 && A%100==B%100){
					player.playData.Bunch.Remove(bun);
					break;
				}
			}
			break;
		default:
			break;
		}

		//add to bunches
		if(bunch.Type==pb_enum.PhzAbc){
			//baihuo bunches
			for(var i=0;i<bunch.Pawns.Count/3;++i){
				var bun=new bunch_t();
				bun.Pos=bunch.Pos;
				bun.Type=bunch.Type;
				bun.Pawns.Add(bunch.Pawns[i*3+0]);
				bun.Pawns.Add(bunch.Pawns[i*3+1]);
				bun.Pawns.Add(bunch.Pawns[i*3+2]);
				player.playData.Bunch.Add(bun);
			}
		}else
			player.playData.Bunch.Add(bunch);
	}
	
	bool IsWin(Player player,int card,List<bunch_t> output,bool bDraw){
		var pos=player.pos;
		var playdata=player.playData;
		var suite=playdata.Bunch;
		var hands=player.playData.Hands;
		if(hands.Count<2){
			Debug.Log("isGameOver failed: len="+hands.Count);
			return false;
		}

		if(player.AAAAs.Count>=3 || player.AAAs.Count>=5){
			output.AddRange(player.AAAs);
			output.AddRange(player.AAAAs);
			//no need other cards
			return true;
		}

		//can't win hand card if not fire
		var category=Main.Instance.MainPlayer.category;
		var fire=(pos!=Token && !bDraw
		          && (category==pb_enum.PhzLd||category==pb_enum.PhzHy||
		      category==pb_enum.PhzXxGhz||category==pb_enum.PhzCz||
		      category==pb_enum.PhzGx));
		if(!bDraw && !fire){
			Debug.Log("isWin failed: not fire and not from pile");
			return false;
		}

		//check desk
		var myself=(pos==Token);
		
		List<bunch_t> allSuites=new List<bunch_t>(suite);
		List<int> hand=new List<int>(hands);

		if(card>0){
			var inhand=false;
			foreach(var i in hand)if(i==card){inhand=true;break;}
			if(!inhand)hand.Add(card);
		}

		hand.Sort(comparision);

		//logHands(game,pos);
		//是否需要将
		bool needJiang=false;
		foreach(var iv in suite){
			var ops=fixOps(iv.Type);
			if(ops==pb_enum.PhzAaaa||ops==pb_enum.PhzAaaastart||ops==pb_enum.PhzBbbB||ops==pb_enum.PhzBbbbdesk||ops==pb_enum.PhzAaaadesk){
				needJiang=true;
				break;
			}
		}
		
		//按颜色和点数排序
		List<List<int>>[] mc=new List<List<int>>[2];
		mc[0]=new List<List<int>>();
		mc[1]=new List<List<int>>();
		for(int l=0;l<10;++l){
			mc[0].Add(new List<int>());
			mc[1].Add(new List<int>());
		}
		foreach(var it in hand){
			var C=it;
			int x=C/1000-1;
			var v=mc[x][C%100-1];
			v.Add(C);
			//可能刚跑，仍检查手牌
			if(v.Count>=4)needJiang=true;
		}
		
		//坎牌剔除，记录所有将
		List<bunch_t> jiangs=new List<bunch_t>();
		for(int j=0; j<2; ++j){
			var v=mc[j];
			foreach(var iv in v){
				if(needJiang&&iv.Count==2){
					//add to suite
					var su=new bunch_t();
					su.Type=pb_enum.PhzAa;
					foreach(var it in iv)su.Pawns.Add(it);
					jiangs.Add(su);
				} else if(iv.Count>=3){
					//处理3cards中不是坎的情况
					bool btmp=false;
					foreach(var x in iv){
						if((bDraw||!myself)&&card==x&&iv.Count==3){
							btmp=true;
							break;
						}
					}
					if(btmp){
						//allSuites.pop_back();
						List<int> tmpCards=new List<int>(iv);
						var su=new bunch_t();
						su.Type=pb_enum.PhzAa;
						tmpCards.RemoveAt(tmpCards.Count-1);
						foreach(var it in tmpCards)su.Pawns.Add(it);
						jiangs.Add(su);
						break;
					}
					//处理坎牌
					//add to stratagy suite
					var kan=new bunch_t();
					var bAAA=true;
					foreach(var x in iv){
						if(card==x)bAAA=false;
						kan.Pawns.Add(x);
						//remove from hands
						foreach(var it in hand)
						if(x==it){
							hand.Remove(it);
							break;
						}
					}
					if(iv.Count==3)
						kan.Type=(bAAA?pb_enum.PhzAaa:(bDraw&&myself?pb_enum.PhzAaawei:pb_enum.PhzBbb));
					else
						kan.Type=(bDraw&&myself?pb_enum.PhzAaaa:pb_enum.PhzBbbB);
					allSuites.Add(kan);
				}
			}
		}
		//提坎已剔除，只剩绞，句
		bool over=false;
		if(needJiang){
			//优先剔除将
			if(jiangs.Count<1)return false;
			int PT=0;
			bool btmpOver=false;
			List<bunch_t> resSuites=new List<bunch_t>();
			var tmpJiang=jiangs[jiangs.Count-1];
			foreach(var ij in jiangs){
				List<bunch_t> tmpSuites=new List<bunch_t>(allSuites);
				var jiang=ij;
				//make a temp hand cards
				List<int> tmpHand=new List<int>();
				foreach(var it in hand)
					if(jiang.Pawns[0]!=it&&jiang.Pawns[1]!=it)
						tmpHand.Add(it);
				//recursived
				over=isWin(tmpHand,tmpSuites);
				if(over){
					var pt=calcPoints(tmpSuites);
					if(pt>=PT){
						resSuites.Clear();
						PT=pt;
						btmpOver=over;
						tmpJiang=ij;
						resSuites.AddRange(tmpSuites);
					}
				}
			}
			over=btmpOver;
			if(btmpOver){
				resSuites.Add(tmpJiang);
				allSuites.Clear();
				allSuites.AddRange(resSuites);
			}else{
				return false;
			}
		} else{
			//recursived
			over=isWin(hand,allSuites);
		}
		if(over){
			var M=MaxPlayer;
			int MIN_SUITES=7;
			if(M==4)MIN_SUITES=5;//衡阳，碰胡子玩法
			if(allSuites.Count+player.AAAs.Count>=MIN_SUITES){
				var win=false;
				if(Main.Instance.MainPlayer.category==pb_enum.PhzPeghz){
					//碰胡子判胡
					win=true;
				}else{
					var point=calcPoints(allSuites);
					point+=calcPoints(player.AAAs);
					if(point>=winPoint(Main.Instance.MainPlayer.category)){
						win=true;
					}
				}
				if(win){
					output.AddRange(allSuites);
					output.AddRange(player.AAAs);
				}
				return win;
			}
		}
		
		return false;
	}
	
	bool isWin(List<int> cards,List<bunch_t> allSuites){
		//recursive check if is game over
		List<bunch_t> outSuites=new List<bunch_t>();
		List<bunch_t> multiSuites=new List<bunch_t>();
		//copy hand
		List<int> _cards=new List<int>(cards);
		
		//logCards(_cards,"-----isGameOverr:");
		//先找每张牌的组合，如果没有则返回
		foreach(var i in cards){
			var card=i;
			List<bunch_t> hints=new List<bunch_t>();
			buildBunch(card,_cards,hints);
			if(hints.Count<=0){
				//此牌无组合
				//log("isGameOver no suite for card=%d:%d",card,allCards[card]%100);
				return false;
			} else if(hints.Count==1){
				//此牌唯一组合,剔除
				//log("isGameOver single suite for card=%d:%d", card, allCards[card]%100);
				var ihint=hints[0];
				outSuites.Add(ihint);
				multiSuites.Clear();
				var toRm=new List<int>();
				foreach(var k in _cards){
					foreach(var j in ihint.Pawns)
					if(k==j){
						toRm.Add(j);
						break;
					}
				}
				foreach(var rm in toRm)_cards.Remove(rm);
				//递归
				if(isWin(_cards,outSuites)){
					allSuites.AddRange(outSuites);
					return true;
				} else
					return false;
			} else{
				multiSuites.AddRange(hints);
			}
		}
		
		//存在多张组合的牌
		List<List<bunch_t>> vvm=new List<List<bunch_t>>();
		if(multiSuites.Count>0){
			//log("isGameOver cards=%d, multiSuites=%d", _cards.Count, multiSuites.Count);
			//copy temp hand
			//List<bunch_t> tmphints=new List<bunch_t>();
			
			//遍历每个组合
			List<bunch_t> vm=new List<bunch_t>();
			foreach(var m in multiSuites){
				List<int> mcards=new List<int>(_cards);
				//此组合的牌从临时手牌移除，并加入临时suites
				foreach(var l in m.Pawns)
				foreach(var k in mcards){
					if(k==l){
						mcards.Remove(k);
						break;
					}
				}
				vm.Clear();
				if(isWin(mcards,vm)){
					vm.Add(m);
					vvm.Add(vm);
				}
			}
			if(vvm.Count>0){
				int PT=0;
				List<bunch_t> iwin=null;
				foreach(var ivvm in vvm){
					var pt=calcPoints(ivvm);
					if(pt>=PT){
						PT=pt;
						iwin=ivvm;
					}
				}
				if(iwin!=null){
					//最优胡牌组合
					allSuites.AddRange(outSuites);
					allSuites.AddRange(iwin);
					return true;
				}
			}
			return false;
		}
		
		//all cards past
		return true;
	}
	
	bool hint3(Player player,int card,bunch_t hints,bool bDraw){
		//all possible AAA,AAAA,BBB,BBBB like
		var pos=player.pos;
		var playdata=player.playData;
		var hand=playdata.Hands;
		var suite=playdata.Bunch;
		
		var A=card;
		List<int> tmp=new List<int>();
		
		//check desk
		var myself=(pos==Token);
		
		//skip self
		if(myself&&!bDraw)return false;

		//find same cards in AAA
		foreach(var it in player.AAAs){
			var B=it.Pawns[0];
			if(A/1000==B/1000 && A%100==B%100){
				foreach(var c in it.Pawns)
					tmp.Add(c);
			}
		}

		//find same cards in hands
		if(tmp.Count<=0)foreach(var it in hand){
			var B=it;
			if(A/1000==B/1000 && A%100==B%100)
				tmp.Add(B);
		}

		if(tmp.Count>=2){
			foreach(var it in tmp)hints.Pawns.Add(it);
			hints.Pawns.Add(card);
			if(tmp.Count==2)
				hints.Type=bDraw&&myself?pb_enum.PhzAaawei:pb_enum.PhzBbb;
			else
				hints.Type=(bDraw&&myself?pb_enum.PhzAaaa:pb_enum.PhzBbbB);
			//臭偎
			if(hints.Type==pb_enum.PhzAaawei && chouWei(player,hints))
				hints.Type=pb_enum.PhzAaachou;
			return true;
		}

		foreach(var i in suite){
			//检测桌面牌组：偎子直接跑，碰子须抓的才能跑
			if(i.Type==pb_enum.PhzAaawei||i.Type==pb_enum.PhzAaachou||(i.Type==pb_enum.PhzBbb&&bDraw)){
				var B=i.Pawns[0];
				if(A%100==B%100 && A/1000==B/1000 && A!=B){
					hints.Pawns.AddRange(i.Pawns);
					//for(var c in i.Pawns)hints.Pawns.Add(c);
					hints.Pawns.Add(card);
					hints.Type=(i.Type==pb_enum.PhzAaawei && bDraw && myself?pb_enum.PhzAaaadesk:pb_enum.PhzBbbbdesk);
					return true;
				}
			}
		}
		return false;
	}

	bool hint(Player player,int card,List<bunch_t> hints){
		var hands=new List<int>(player.playData.Hands);
		if(buildBunch(card,hands,hints)){
			//baihuo
			List<bunch_t> tmp=new List<bunch_t>(hints);
			List<int> sameCard=new List<int>();
			foreach(var C in hands)
				if(card/1000==C/1000&&C%100==card%100&&card!=C)
					sameCard.Add(C);
			
			sameCard.Add(card);
			
			hints.Clear();
			foreach(bunch_t j in tmp)
				if(checkBaihuo(hands,sameCard,j))
					hints.Add(j);

			foreach(var o in hints)o.Pos=player.pos;
		}
		return hints.Count>0;
	}

	bool buildBunch(int card,List<int> _hand,List<bunch_t> hints){
		//这张牌可能的所有组合：句,绞
		var jiao=new List<int>();
		var same=new List<int>();
		var A=card;
		
		List<int> hand=new List<int>(_hand);
		//按颜色和点数排序
		List<List<int>>[] mc=new List<List<int>>[2];
		mc[0]=new List<List<int>>();
		mc[1]=new List<List<int>>();
		for(int l=0;l<10;++l){
			mc[0].Add(new List<int>());
			mc[1].Add(new List<int>());
		}
		foreach(var it in hand){
			if(card==it)continue;	//跳过自己
			var C=it;
			int x=C/1000-1;
			mc[x][C%100-1].Add(C);
		}
		
		//坎牌剔除
		for(int j=0; j<2; ++j){
			var v=mc[j];
			foreach(var iv in v){
				if(iv.Count==3){
					foreach(var x in iv){
						foreach(var it in hand)
						if(x==it){
							hand.Remove(it);
							break;
						}
					}
				}
			}
		}
		
		//找相同点数牌
		foreach(var it in hand){
			if(card==it)continue;	//跳过自己
			var B=it;
			if(B%100==A%100){
				if(B/1000!=A/1000)jiao.Add(it);
				else same.Add(it);
			}
		}
		if(jiao.Count==2){
			//绞，append to hints: Bbb
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzAbc;
			foreach(var c in jiao)suite.Pawns.Add(c);
			suite.Pawns.Add(card);
			hints.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		if(same.Count>0&&jiao.Count>0){
			//绞，append to hints: BBb
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzAbc;
			foreach(var it in same)
			if(card!=it){
				//别把自己放进去
				suite.Pawns.Add(it);
				break;
			}
			suite.Pawns.Add(jiao[0]);
			suite.Pawns.Add(card);
			hints.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		
		//句 首先查找与A相临 前面两张牌和后面两张牌的id 和数量 -1表示存在
		List<int>[] FontABackID=new List<int>[4];	// CBBC
		for(int f=0;f<4;++f)FontABackID[f]=new List<int>();
		foreach(var it in hand){
			var B=it;
			if( B/1000 != A/1000 ) {
				continue;	// 颜色不一样 直接跳过
			}
			int off = B%100 - A%100;
			if ( off == 0 ) {
				continue;
			}
			off += (off > 0 ? 1 : 2);		// 计算这个牌的坐标
			if ( off >= 0 && off <= 3 ) {
				FontABackID[off].Add( it );
			}
		}
		//
		for ( int i = 0; i < 3; ++i ) {
			int count = Mathf.Min( FontABackID[i].Count, FontABackID[i+1].Count);
			for ( int k = 0; k < count; ++k ) {
				var suite=new bunch_t();
				suite.Type=pb_enum.PhzAbc;
				suite.Pawns.Add(FontABackID[i][0]);
				suite.Pawns.Add(FontABackID[i+1][0]);
				suite.Pawns.Add(card);
				hints.Add(suite);
				if(k==0)break;
			}
		}
		
		//find same color and sort
		var color=new List<int>();
		foreach(var it in hand){
			if(card==it)continue;	//跳过自己
			var B=it;
			if(B/1000==A/1000)color.Add(it);
		}
		//2,7,10
		var mm=new Dictionary<int,List<int>>();	//[value,ids]
		if(A%100==2||A%100==7||A%100==10){
			foreach(var it in color){
				var B=it;
				if(B%100!=A%100&&(B%100==2||B%100==7||B%100==10)){
					if(!mm.ContainsKey(B%100))mm[B%100]=new List<int>();
					mm[B%100].Add(B);
				}
			}
			if(mm.Count==2){
				var ll=new List<List<int>>();
				ll.AddRange(mm.Values);
				var E=ll[0];
				var F=ll[1];
				var sz=Mathf.Min(E.Count,F.Count);
				for(int e=0;e<sz;++e){
					var suite=new bunch_t();
					suite.Type=pb_enum.PhzAbc;
					suite.Pawns.Add(E[0]);
					suite.Pawns.Add(F[0]);
					suite.Pawns.Add(card);
					hints.Add(suite);
					//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
					if(e==0)break;
				}
			}
		}
		return hints.Count>0;
	}
	
	bool checkBaihuo(List<int> hand,List<int> sameCard,bunch_t suite){
		if(sameCard.Count<=0)return true;
		//检查是否够将sameCard从hand全部摆出去
		var tmpHand=new List<int>(hand);
		var tmpSame=new List<int>(sameCard);
		//用掉这张
		tmpSame.RemoveAt(tmpSame.Count-1);
		if(tmpSame.Count<=0)return true;
		//拷贝手牌并删除这个组合的牌
		var tmp=suite.Pawns;
		foreach(var it in tmp){
			foreach(var u in tmpHand){
				if(it==u){
					tmpHand.Remove(u);
					break;
				}
			}
			//用掉这张
			foreach(var u in tmpSame){
				if(it==u){
					tmpSame.Remove(u);
					break;
				}
			}
		}
		if(tmpSame.Count<=0)
			return true;
		else{
			List<bunch_t> tmpHint=new List<bunch_t>();
			buildBunch(tmpSame[tmpSame.Count-1],tmpHand,tmpHint);
			var ok=false;
			foreach(var j in tmpHint){
				if(checkBaihuo(tmpHand,tmpSame,j)){
					suite.Child.Add(j);
					ok=true;
				}
			}
			return ok;
		}
	}

	protected override pb_enum verifyBunch(bunch_t bunch){
		var bt=pb_enum.BunchInvalid;
		var type=fixOps(bunch.Type);
		switch(type) {
		case pb_enum.PhzAaa:
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
		case pb_enum.PhzBbb:
			if(bunch.Pawns.Count==3){
				var A=bunch.Pawns[0];
				var B=bunch.Pawns[1];
				var C=bunch.Pawns[2];
				if(A/1000==B/1000 && A/1000==C/1000 &&
				   A%100==B%100 && A%100==C%100)
					bt=bunch.Type;
			}
			break;
		case pb_enum.PhzAaaa:
		case pb_enum.PhzAaaastart:
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzBbbB:
		case pb_enum.PhzBbbbdesk:
			if(bunch.Pawns.Count==4){
				var A=bunch.Pawns[0];
				var B=bunch.Pawns[1];
				var C=bunch.Pawns[2];
				var D=bunch.Pawns[3];
				if(A/1000==B/1000 && A/1000==C/1000 && A/1000==D/1000 &&
				   A%100==B%100 && A%100==C%100 && A%100==D%100)
					bt=bunch.Type;
			}
			break;
		case pb_enum.PhzAbc:
			if(bunch.Pawns.Count%3==0){
				var ok=true;
				for(var i=0;i<bunch.Pawns.Count/3;++i){
					var j=i*3;
					List<int> cards=new List<int>();
					cards.Add(bunch.Pawns[j+0]);
					cards.Add(bunch.Pawns[j+1]);
					cards.Add(bunch.Pawns[j+2]);
					cards.Sort(comparision);
					
					var A=cards[0];
					var B=cards[1];
					var C=cards[2];
					ok=(
						(A/1000==B/1000 && A/1000==C/1000 &&
					 ((A%100+1==B%100 && A%100+2==C%100) || (A%100==2 && B%100==7 && C%100==10))
					 ) ||
						
						(A%100==B%100 && A%100==C%100 && !(A/1000==B/1000 && A/1000==C/1000))
						);
					if(!ok)break;
				}
				if(ok)bt=bunch.Type;
			}
			break;
		case pb_enum.PhzAa:
			if(bunch.Pawns.Count==2){
				var A=bunch.Pawns[0];
				var B=bunch.Pawns[1];
				if(A/1000==B/1000 && A%100==B%100)
					bt=bunch.Type;
			}
			break;
		case pb_enum.OpPass:
			bt=bunch.Type;
			break;
		default:
			//invalid
			break;
		}
		bunch.Type=bt;
		return bt;
	}

	public override int comparision(int x,int y){
		var cx=(int)x/1000;
		var cy=(int)y/1000;
		if(cx<cy)return 1;
		else if(cx==cy)return (int)y%100-(int)x%100;
		else return -1;
	}

	public override int transformValue(int val){
		return val;
	}

	public override int inverseTransformValue(int val){
		return val;
	}

	static pb_enum fixOps(pb_enum ops){
		if(ops>=pb_enum.BunchWin)
			ops=(pb_enum)((int)ops%(int)pb_enum.BunchWin);
		return ops;
	}

	int winPoint(pb_enum category){
		int point=10;
		switch(category){
		case pb_enum.PhzLd:		//娄底放炮
		case pb_enum.PhzHh:		//怀化红拐弯
		case pb_enum.PhzCdQmt:	//常德全名堂
		case pb_enum.PhzCdHhd:	//常德红黑点
		case pb_enum.PhzCs:		//长沙跑胡子
		case pb_enum.PhzXxGhz:	//湘乡告胡子
			point=15;	break;
		case pb_enum.PhzCz:		//郴州毛胡子
			point=9;	break;
		case pb_enum.PhzHy:		//衡阳六条枪
			point=6;	break;
		case pb_enum.PhzGx:		//广西
			point=10;	break;
		case pb_enum.PhzSy:		//邵阳字牌
		case pb_enum.PhzSybp:     //邵阳剥皮
		default:
			break;
		}
		return point;
	}

	int calcScore(pb_enum category,int points){
		int score=0;
		switch(category){
		case pb_enum.PhzHh:		//怀化红拐弯
		case pb_enum.PhzCdQmt:	//常德全名堂
		case pb_enum.PhzCdHhd:	//常德红黑点
		case pb_enum.PhzCs:		//长沙跑胡子
			score=(points-12)/3;	//(x-15)/3+1
			break;
		case pb_enum.PhzSy:		//邵阳字牌
			score=(points-5)/5;		//(x-10)/5+1
			if(score>0)
				++score;
			break;
		case pb_enum.PhzCz: //郴州毛胡子
			score=(points-6)/3;
			break;
		case pb_enum.PhzHy:		//衡阳六条枪
			score=(points-3)/3;
			break;
		case pb_enum.PhzGx:		//广西
			score=(points-5)/5;
			break;
		case pb_enum.PhzLd:		//娄底放炮
		case pb_enum.PhzSybp:	//邵阳剥皮
		case pb_enum.PhzXxGhz: //湘乡告胡子
		default:
			score=points;
			break;
		}
		if(score<0)score=0;
		return score;
	}

	int calcPoints(List<bunch_t> allSuites){
		int point=0;
		foreach(var i in allSuites){
			var suite=i;
			point+=CalcPoints(suite);
			//log("settle point=%d, small=%d, ops=%s", pt, small, ops2String(suite.ops).c_str());
		}
		return point;
	}

	public static int CalcPoints(bunch_t suite){
		int pt=0;
		var rule=Main.Instance.gameController.Rule;
		if(suite.Pawns.Count>0){
			var small=(1==suite.Pawns[0]/1000);
			switch(fixOps(suite.Type)){
			case pb_enum.PhzAaaa:
			case pb_enum.PhzAaaastart:
			case pb_enum.PhzAaaadesk:
				pt+=(small?9:12);
				break;
			case pb_enum.PhzBbbbdesk:
			case pb_enum.PhzBbbB:
				pt+=(small?6:9);
				break;
			case pb_enum.PhzAaawei:
			case pb_enum.PhzAaa:
			case pb_enum.PhzAaachou:
				pt+=(small?3:6);
				break;
			case pb_enum.PhzBbb:
				pt+=(small?1:3);
				break;
			case pb_enum.PhzAbc:{
				List<int> sl=new List<int>(suite.Pawns);
				sl.Sort(rule.comparision);
				var A=sl[0];
				var B=sl[1];
				if(A%100==1 || (A%100==2&&B%100==7))
					pt+=(small?3:6);
				break;
			}
			default:
				break;
			}
		}
		return pt;
	}

	bool chouWei(Player player,bunch_t bunch){
		//臭偎
		var vp=player.unpairedCards;
		var n=bunch.Pawns[0];
		foreach(var i in vp){
			var m=i;
			if(m%100==n%100&&m/1000==n/1000){
				//vp.erase(i);
				return true;
			}
		}
		return false;
	}

	public static void prepareAAAA(Player player){
		List<List<int>>[] mc=new List<List<int>>[2];
		mc[0]=new List<List<int>>();
		mc[1]=new List<List<int>>();
		for(int l=0;l<10;++l){
			mc[0].Add(new List<int>());
			mc[1].Add(new List<int>());
		}
		foreach(var it in player.playData.Hands){
			var C=it;
			int x=C/1000-1;
			var v=mc[x][C%100-1];
			v.Add(C);
		}
		
		for(int j=0; j<2; ++j){
			var v=mc[j];
			foreach(var iv in v){
				if(iv.Count>=3){
					var bunch=new bunch_t();
					//the cards
					foreach(var x in iv){
						bunch.Pawns.Add(x);
						//remove from hands
						player.playData.Hands.Remove(x);
					}//for
					if(iv.Count==4){
						//add to AAAA
						bunch.Type=pb_enum.PhzAaaastart;
						player.AAAAs.Add(bunch);
					}else if(iv.Count==3){
						//add to AAA
						bunch.Type=pb_enum.PhzAaaastart;
						player.AAAs.Add(bunch);
					}
				}//>3
			}//for iv
		}//for j
	}

	public override bool checkDiscard(Player player,int drawCard){
		var sz=player.playData.Hands.Count;
		if(sz<=0)
			return false;
		
		var aaaa=(player.AAAAs.Count>0);
		if(!aaaa){
			var bunches=player.playData.Bunch;
			foreach(var bunch in bunches){
				if(bunch.Pawns.Count>3){
					aaaa=true;
					break;
				}
			}
		}
		if(drawCard!=-1)++sz;
		return (sz%3==(aaaa?2:0));
	}

	public override PlayerController AIController{
		get{
			if(aiController==null)aiController=new PaohuziAIController();
			return aiController;
		}
	}
}
