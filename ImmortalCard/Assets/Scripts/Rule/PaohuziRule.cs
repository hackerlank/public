using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziRule: GameRule {

	public override int MaxCards{get{return 80;}}
	public override int MaxPlayer{
		get{
			switch(Main.Instance.MainPlayer.category){
			case pb_enum.PhzHy:
			case pb_enum.PhzPeghz:
				return 4;
			default:
				return 3;
			}
		}
	}

	protected override void deal(MsgNCDeal msg){
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
	}

	public override List<bunch_t> Hint(Player player,bunch_t src_bunch){
		var output=new List<bunch_t>();
		var copy=new List<int>(player.playData.Hands);
		if(player==null||src_bunch==null||copy.Count<=0)
			return output;
		
		int pos=player.playData.Seat;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return output;
		}
		var card=src_bunch.Pawns[0];

		if(card==Configs.invalidCard){
			//check natural win at startup
			var win=isWin(player,copy,card);
			if(win!=null)
				output.Add(win);
		}else{
			//normal hints
			var bDraw=(Pile.IndexOf(card)!=-1);
			//handle past card
			var past=false;
			var dodge=false;
			foreach(var c in player.dodgeCards)if(c/1000==card/1000 && c%100==card%100){
				dodge=true;
				break;
			}
			foreach(var c in player.unpairedCards)if(c/1000==card/1000 && c%100==card%100){
				past=true;
				break;
			}

			//backup
			var AAABack=new List<bunch_t>(player.AAAs);
			var BunchesBack=new List<bunch_t>(player.playData.Bunch);

			//AAA & AAAA first
			bunch_t AAAAs=null;
			var needAA=false;
			if(bDraw && pos==src_bunch.Pos){
				//check AAAs
				foreach(var aaas in player.AAAs){
					var h=aaas.Pawns[0];
					if(h/1000==card/1000 && h%100==card%100){
						AAAAs=new bunch_t();
						AAAAs.Type=pb_enum.PhzAaaa;
						AAAAs.Pawns.Add(card);
						AAAAs.Pawns.AddRange(aaas.Pawns);
						
						player.AAAs.Remove(aaas);
						break;
					}
				}
				//check desk
				if(AAAAs==null){
					foreach(var desk in player.playData.Bunch){
						if(desk.Type==pb_enum.PhzAaawei){
							var h=desk.Pawns[0];
							if(h/1000==card/1000 && h%100==card%100){
								AAAAs=new bunch_t();
								AAAAs.Type=pb_enum.PhzAaaadesk;
								AAAAs.Pawns.Add(card);
								AAAAs.Pawns.Add(desk.Pawns);
								
								player.playData.Bunch.Remove(desk);
								break;
							}
						}
					}
				}
				//check hands
				if(AAAAs!=null)
					needAA=true;
				else if(pos==Token){
					List<int> aa=new List<int>();
					foreach(var h in copy)if(h/1000==card/1000 && h%100==card%100)aa.Add(h);
					if(aa.Count==2){
						AAAAs=new bunch_t();
						AAAAs.Type=pb_enum.PhzAaawei;
						AAAAs.Pawns.Add(card);
						AAAAs.Pawns.Add(aa);
						if(chouWei(player,AAAAs))
							AAAAs.Type=pb_enum.PhzAaachou;
						//force remove from copied hands
						foreach(var a in aa)copy.Remove(a);
					}
				}
				//also set card to invalid
				if(AAAAs!=null){
					card=Configs.invalidCard;
					output.Add(AAAAs);
				}
			}
			
			//then win
			if(AAAAs!=null)
				player.playData.Bunch.Add(AAAAs);
			var win=isWin(player,copy,card,needAA);
			if(win!=null){
				if(AAAAs!=null)
					//AAAs win,clear AAAs
					output.Clear();
				//restore win card
				win.Pawns[0]=src_bunch.Pawns[0];
				output.Add(win);
			}

			//restore all
			player.AAAs=AAABack;
			player.playData.Bunch.Clear();
			foreach(var b in BunchesBack)player.playData.Bunch.Add(b);

			bunch_t BBBB=null;
			if(AAAAs==null){
				//then BBBB
				//check AAAs
				foreach(var aaas in player.AAAs){
					var h=aaas.Pawns[0];
					if(h/1000==card/1000 && h%100==card%100){
						BBBB=new bunch_t();
						BBBB.Type=pb_enum.PhzBbbB;
						BBBB.Pawns.Add(card);
						BBBB.Pawns.Add(aaas.Pawns);
						break;
					}
				}
				//check desk
				if(BBBB==null){
					foreach(var desk in player.playData.Bunch){
						if(desk.Type==pb_enum.PhzBbb&&bDraw
						   ||desk.Type==pb_enum.PhzAaawei){
							var h=desk.Pawns[0];
							if(h/1000==card/1000 && h%100==card%100){
								BBBB=new bunch_t();
								BBBB.Type=(desk.Type==pb_enum.PhzAaawei?
								           pb_enum.PhzBbbbdesk:pb_enum.PhzB4B3);
								BBBB.Pawns.Add(card);
								BBBB.Pawns.Add(desk.Pawns);
								break;
							}
						}
					}
				}
				if(BBBB!=null)
					output.Add(BBBB);
				else if(!player.conflictMeld && !past){
					//then BBB,ABC
					var abcs=hintABC(copy,card);
					var tmp=new List<bunch_t>();
					bool meltable=((Token==pos&&bDraw) || (pos==(Token+1)%MaxPlayer));
					foreach(var abc in abcs){
						if(abc.Type==pb_enum.PhzBbb && !dodge
						   || abc.Type==pb_enum.PhzAbc && meltable){
							tmp.Add(abc);
						}
					}
					
					//baihuo
					abcs=new List<bunch_t>(tmp);
					if(abcs.Count>0){
						var same=new List<int>();
						foreach(var B in copy)
							if(card!=B && B%100==card%100 && B/1000==card/1000)
								same.Add(B);
						same.Add(card);
						
						abcs.Clear();
						foreach(bunch_t j in tmp)
							if(checkBaihuo(copy,same,j))
								abcs.Add(j);
						output.AddRange(abcs);
					}
				}
			}
		}
		foreach(var o in output)o.Pos=player.playData.Seat;
		
		var count=output.Count;
		if(count>0){
			string str=count+" hints for "+pos+": ";
			foreach(var bunch in output){
				str+=Player.bunch2str(bunch);
				if(bunch.Child.Count>0){
					str+="{";
					foreach(var ch in bunch.Child)str+=Player.bunch2str(ch);
					str+="}\n";
				}
			}
			Debug.Log(str);
		}
		return output;
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
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
		case pb_enum.PhzAaaadesk:
			foreach(var bun in player.playData.Bunch){
				var A=bun.Pawns[0];
				if(A/1000==B/1000 && A%100==B%100){
					Debug.Log(player.playData.Seat+" meld bbbb desk remove "+Player.bunch2str(bun));
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
				Debug.Log(player.playData.Seat+" meld "+Player.bunch2str(bun));
			}
		}else if(bunch.Type>pb_enum.PhzAbc){
			Debug.Log(player.playData.Seat+" meld "+Player.bunch2str(bunch));
			player.playData.Bunch.Add(bunch);
		}
	}

	bunch_t isWin(Player player,List<int> hands,int card,bool needAA=false){
		//already cullout AAA & AAAA outside in Hint
		bunch_t output=null;
		var bunches=new List<bunch_t>();
		var AAAs=new List<bunch_t>(player.AAAs);
		needAA=(needAA||player.AAAAs.Count>0);
		var desks=new List<bunch_t>(player.playData.Bunch);
		if(!needAA){
			foreach(var bbbb in desks){
				if(bbbb.Pawns.Count>=4){
					needAA=true;
					break;
				}
			}
		}

		if(card==Configs.invalidCard){
			//natural win: AAAs
			if(player.AAAAs.Count>=3 || player.AAAs.Count>=5)
				bunches=buildFrees(hands,player.AAAs.Count+player.AAAAs.Count);
			else
				//BBB,ABC
				bunches=buildABC(hands,needAA);
		}else do{
			var category=Main.Instance.MainPlayer.category;
			var draw=Pile.IndexOf(card)!=-1;
			var fire=(player.playData.Seat!=Token && !draw
			           &&   (category==pb_enum.PhzLd||category==pb_enum.PhzHy||
			      category==pb_enum.PhzXxGhz||category==pb_enum.PhzCz||
			      category==pb_enum.PhzHy||category==pb_enum.PhzGx));
			if(!fire && !draw){
				Debug.Log(player.playData.Seat+" win failed,draw="+draw+",fire="+fire+",category="+(int)category+",token="+Token);
				break;
			}

			//could BBBB
			bunch_t BBBB=null;
			//check AAAs
			bunch_t paaa=null;
			foreach(var aaa in AAAs){
				var h=aaa.Pawns[0];
				if(h/1000==card/1000 && h%100==card%100){
					BBBB=new bunch_t();
					BBBB.Type=pb_enum.PhzBbbB;
					BBBB.Pawns.Add(card);
					BBBB.Pawns.Add(aaa.Pawns);

					paaa=aaa;
					AAAs.Remove(aaa);
					break;
				}
			}
			//check desk
			bunch_t pdesk=null;
			if(BBBB==null){
				foreach(var desk in desks){
					if(desk.Type==pb_enum.PhzAaawei||
					   desk.Type==pb_enum.PhzBbb&&draw){
						var h=desk.Pawns[0];
						if(h/1000==card/1000 && h%100==card%100){
							BBBB=new bunch_t();
							BBBB.Type=desk.Type==pb_enum.PhzAaawei?
								pb_enum.PhzBbbbdesk:pb_enum.PhzB4B3;
							BBBB.Pawns.Add(card);
							BBBB.Pawns.Add(desk.Pawns);

							pdesk=desk;
							desks.Remove(desk);
							break;
						}
					}
				}
			}

			//build with BBBB
			var out0=new List<bunch_t>();
			if(BBBB!=null){
				out0=buildABC(hands,true);
				if(out0.Count>0)
					out0.Add(BBBB);
			}

			//build without BBBB
			hands.Add(card);
			var out1=buildABC(hands,needAA);
			if(out1.Count>0){
				if(pdesk!=null)out1.Add(pdesk);
				if(paaa!=null)out1.Add(paaa);
			}
			hands.Remove(card);

			var p0=calcPoints(out0);
			var p1=calcPoints(out1);
			bunches=(p0>p1?out0:out1);
		}while(false);

		if(bunches.Count>0){
			bunches.AddRange(player.AAAAs);
			bunches.AddRange(AAAs);
			bunches.AddRange(desks);

			var pt=calcPoints(bunches);
			if(pt>=winPoint(Main.Instance.MainPlayer.category)){
				//could add bunch type to win here
				output=new bunch_t();
				output.Type=pb_enum.BunchWin;
				output.Pos=player.playData.Seat;
				output.Pawns.Add(card);
				output.Child.Add(bunches);
			}
		}

		return output;
	}

	List<bunch_t> buildABC(List<int> cards,bool needAA){
		List<bunch_t> output=new List<bunch_t>();
		if(needAA){
			var copy=new List<int>(cards);
			var AAs=new List<int>();
			copy.Sort(comparision);
			for(int i=0;i<copy.Count-1;++i){
				var A=copy[i];
				var B=copy[i+1];
				if(A/1000==B/1000&&A%100==B%100)
					AAs.Add(i++);
			}

			int PT=-1;
			int j=-1;
			foreach(var i in AAs){
				//make a temp hand cards
				List<int> tmpHand=new List<int>(copy);
				tmpHand.RemoveRange(i,2);
				
				List<bunch_t> tmpSuites=new List<bunch_t>();
				if(buildABCWithoutAA(tmpHand,tmpSuites)){
					var pt=calcPoints(tmpSuites);
					if(pt>PT){
						//find a max one
						PT=pt;
						j=i;
						output=tmpSuites;
					}
				}
			}
			if(j>=0){
				var jiang=new bunch_t();
				jiang.Type=pb_enum.PhzAa;
				jiang.Pawns.Add(copy[j]);
				jiang.Pawns.Add(copy[j+1]);
				output.Add(jiang);
			}
		}else{
			buildABCWithoutAA(cards,output);
		}
		return output;
	}

	bool buildABCWithoutAA(List<int> cards,List<bunch_t> output){
		//recursive check if is game over
		var outSuites=new List<bunch_t>();
		var multiSuites=new List<bunch_t>();
		//copy hand
		var copy=new List<int>(cards);
		
		//logCards(_cards,"-----isGameOverr:");
		//先找每张牌的组合，如果没有则返回
		var checkSet=new Dictionary<int,int>();	//skip duplicated
		foreach(var card in copy)checkSet[(card/1000)*100+card%100]=card;
		foreach(var kv in checkSet){
			var card=kv.Value;
			copy.Remove(card);	//avoid duplicated
			var hints=hintABC(copy,card);
			copy.Add(card);
			if(hints.Count<=0){
				//此牌无组合
				//Debug.Log("isGameOver no suite for card "+card);
				return false;
			} else if(hints.Count==1){
				//此牌唯一组合,剔除
				//log("isGameOver single suite for card=%d:%d", card, allCards[card]%100);
				var ihint=hints[0];
				outSuites.Add(ihint);
				multiSuites.Clear();

				foreach(var j in ihint.Pawns){
					foreach(var k in copy)
					if(k==j){
						copy.Remove(k);
						break;
					}
				}

				//递归
				if(buildABCWithoutAA(copy,outSuites)){
					output.AddRange(outSuites);
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
				List<int> mcards=new List<int>(copy);
				//此组合的牌从临时手牌移除，并加入临时suites
				foreach(var l in m.Pawns)
				foreach(var k in mcards){
					if(k==l){
						mcards.Remove(k);
						break;
					}
				}
				vm.Clear();
				if(buildABCWithoutAA(mcards,vm)){
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
					output.AddRange(outSuites);
					output.AddRange(iwin);
					return true;
				}
			}
			return false;
		}
		
		//all cards pass
		return true;
	}

	public static List<bunch_t> buildFrees(List<int> hands,int nBunches){
		var bunches=new List<bunch_t>();

		//add other cards [2,7,10] [1,2,3]
		var all=new List<int>[2]{new List<int>(),new List<int>()};
		var two=new List<int>[2]{new List<int>(),new List<int>()};
		foreach(var C in hands){
			int x=(C/1000==2?1:0);
			if(C%100==1||C%100==3||C%100==7||C%100==10){
				all[x].Add(C);
			} else if(C%100==2){
				two[x].Add(C);
			}
		}
		
		for(int i=0;i<2;++i){
			//按大小分
			foreach(var i2 in two[i]){
				//找一张二的组合
				var suites=hintABC(all[i],i2);
				if(suites.Count<=0)
					//没有，完事儿
					break;
				else{
					//加入到算分，剔除
					var bunch=suites[0];
					bunches.Add(bunch);
					foreach(var ih in bunch.Pawns)
						foreach(var a in all[i])
						if(ih==a){
							all[i].Remove(a);
							break;
						}
					//下一张二
				}
			}
		}
		
		// 从手牌中删除成
		foreach(var s in bunches)
		foreach(var j in s.Pawns){
			foreach(var k in hands)
			if(j==k){
				hands.Remove(k);
				break;
			}
		}
		//剩下的随便组吧
		var M=Main.Instance.gameController.Rule.MaxPlayer;
		int MIN_SUITES=(M==4?5:7);
		int J=MIN_SUITES-bunches.Count-nBunches;
		var sz=hands.Count;
		if(sz>0){
			var I=(sz+2)/3;
			for(int i=0;i<I;++i){
				var suite=new bunch_t();
				suite.Type=pb_enum.OpPass;	//must be OpPass
				for(int j=0;j<3;++j)if(i*3+j<sz)suite.Pawns.Add(hands[i*3+j]);
				bunches.Add(suite);
			}
			for(int i=I;i<J;++i){
				var suite=new bunch_t();
				suite.Type=pb_enum.OpPass;	//must be OpPass
				bunches.Add(suite);
			}
		}
		return bunches;
	}
	
	static List<bunch_t> hintABC(List<int> cards,int A){
		//all BBB,ABC for card against cards
		var output=new List<bunch_t>();

		//这张牌可能的所有组合：句,绞
		var diff=new List<int>();
		var same=new List<int>();

		var copy=new List<int>(cards);
		//按颜色和点数排序
		foreach(var C in copy){
			if(A==C)continue;	//跳过自己
			//找相同点数牌
			if(C%100==A%100){
				if(C/1000!=A/1000)diff.Add(C);
				else same.Add(C);
			}
		}
		
		if(same.Count==2){
			//BBB
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzBbb;
			suite.Pawns.Add(A);
			foreach(var c in same)suite.Pawns.Add(c);
			output.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		if(diff.Count==2){
			//绞，append to hints: Bbb
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzAbc;
			suite.Pawns.Add(A);
			foreach(var c in diff)suite.Pawns.Add(c);
			output.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		if(same.Count>0&&diff.Count>0){
			//绞，append to hints: BBb
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzAbc;
			suite.Pawns.Add(A);
			foreach(var it in same)if(A!=it){
				//别把自己放进去
				suite.Pawns.Add(it);
				break;
			}
			suite.Pawns.Add(diff[0]);
			output.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		
		//句 首先查找与A相临 前面两张牌和后面两张牌的id 和数量 -1表示存在
		List<int>[] FontABackID=new List<int>[4];	// CBBC
		for(int f=0;f<4;++f)FontABackID[f]=new List<int>();
		foreach(var it in copy){
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
				suite.Pawns.Add(A);
				suite.Pawns.Add(FontABackID[i][0]);
				suite.Pawns.Add(FontABackID[i+1][0]);
				output.Add(suite);
				if(k==0)break;
			}
		}
		
		//find same color and sort
		var color=new List<int>();
		foreach(var it in copy){
			if(A==it)continue;	//跳过自己
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
					suite.Pawns.Add(A);
					suite.Pawns.Add(E[0]);
					suite.Pawns.Add(F[0]);
					output.Add(suite);
					//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
					if(e==0)break;
				}
			}
		}

		return output;
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
			var tmpHint=hintABC(tmpHand,tmpSame[tmpSame.Count-1]);
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
		case pb_enum.PhzB4B3:
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

		if(cx==cy)return (int)x%100-(int)y%100;
		else return cx-cy;
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
			var pt=CalcPoints(i);
			point+=pt;
			//Debug.Log("bunch point="+pt+",ops="+Player.bunch2str(i));
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
			case pb_enum.PhzB4B3:
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
				if(A/1000==B/1000 && (A%100==1 || (A%100==2&&B%100==7)))
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
		var vp=player.dodgeCards;
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
						var rule=Main.Instance.gameController.Rule;
						rule.nHands[player.playData.Seat]-=4;
					}else if(iv.Count==3){
						//add to AAA
						bunch.Type=pb_enum.PhzAaa;
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

	// ------------------------------------------------------
	// resources related
	// ------------------------------------------------------
	override public string CardPrefab{get{return "Zipai";}}
	override public string Id2File(int color,int value){
		color-=1;
		string[] Colors={"s","b"};
		value=inverseTransformValue(value);
		if(color<Colors.Length)
			return CardPrefab+"/"+string.Format("{0}{1:00}",Colors[color],value);
		return "";
	}
	
	override public void PrepareCache(){
		var files=new List<string>();
		files.Add(CardPrefab+"/"+"back");
		for(int k=1;k<=2;++k)for(int i=1;i<=10;++i)
			files.Add(Id2File(k,i));
		Main.Instance.StartCoroutine(CardCache.Load(files.ToArray(),"Zipai"));
	}
	
	override public float DiscardScalar{get{return 1f;}}
}
