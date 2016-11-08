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

	public List<bunch_t> newHint(Player player,bunch_t src_bunch){
		var output=new List<bunch_t>();
		var copy=new List<int>(player.playData.Hands);
		if(player==null||src_bunch==null||copy.Count<=0)
			return output;
		
		int pos=player.pos;
		if(src_bunch.Pawns.Count!=1){
			Debug.Log("hint wrong cards len="+src_bunch.Pawns.Count+",pos="+pos);
			return output;
		}
		var card=src_bunch.Pawns[0];

		if(card==Configs.invalidCard){
			//check natural win at startup
			var win=newIsWin(player,copy,card);
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

			//AAA & AAAA first
			bunch_t AAAAs=null;
			bunch_t paaa=null;
			bunch_t pdesk=null;
			var needAA=false;
			if(bDraw){
				//check AAAs
				foreach(var aaas in player.AAAs){
					var h=aaas.Pawns[0];
					if(h/1000==card/1000 && h%100==card%100){
						AAAAs=new bunch_t();
						AAAAs.Type=pb_enum.PhzAaaa;
						AAAAs.Pawns.Add(card);
						AAAAs.Pawns.AddRange(aaas.Pawns);

						paaa=aaas;
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

								pdesk=desk;
								break;
							}
						}
					}
				}
				//check hands
				if(AAAAs==null){
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
				}else
					needAA=true;
				//also set card to invalid
				if(AAAAs!=null){
					card=Configs.invalidCard;
					output.Add(AAAAs);
				}
			}

			//then win
			var win=newIsWin(player,copy,card,needAA);
			if(win!=null){
				if(AAAAs!=null){
					//AAAs win
					output.Clear();
					//add or remove
					win.Child.Add(AAAAs);
					if(paaa!=null)win.Child.Remove(paaa);
					if(pdesk!=null)win.Child.Remove(pdesk);
				}
				//restore win card
				win.Pawns[0]=src_bunch.Pawns[0];
				output.Add(win);
			}

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
						if(desk.Type==pb_enum.PhzAaawei){
							var h=desk.Pawns[0];
							if(h/1000==card/1000 && h%100==card%100){
								BBBB=new bunch_t();
								BBBB.Type=pb_enum.PhzBbbbdesk;
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
							if(card!=B && B%100==card%100 && B/1000!=card/1000)
								same.Add(B);
						same.Add(card);
						
						abcs.Clear();
						foreach(bunch_t j in tmp)
							if(checkBaihuo(copy,same,j))
								abcs.Add(j);
						output.AddRange(abcs);

						//add pass option
						var pass=new bunch_t();
						pass.Type=pb_enum.OpPass;
						pass.Pawns.Add(card);
						output.Add(pass);
					}
				}
			}
		}
		foreach(var o in output)o.Pos=player.pos;

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
		
	public override List<bunch_t> Hint(Player player,bunch_t src_bunch){
		return newHint(player,src_bunch);
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
		var bDraw=(Pile.IndexOf(card)!=-1/*src_bunch.Type==pb_enum.BunchA*/||card==Configs.invalidCard);
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

		//hint3
		var output3=new bunch_t();
		if(hint3(player,card,output3,bDraw)){
			if(output3.Type==pb_enum.PhzBbb){
				if(past||dodge||player.conflictMeld)
					output3.Pawns.Clear();
			}else
				output3.Pos=pos;
		}

		//hint
		bool meltable=(!player.conflictMeld) && (!past)
			&& ((Token==pos&&bDraw) || (pos==(Token+1)%MaxPlayer));
		var output1=new List<bunch_t>();
		if(meltable)
			hint(player,card,output1);
		Debug.Log("----Hint for "+pos+",draw="+bDraw+",past="+past+",dodge="+dodge
		          +",conflict="+player.conflictMeld+",melt="+meltable);

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

		if(output3.Pawns.Count>0)hints.Add(output3);
		if(output1.Count>0)hints.AddRange(output1);

		var count=hints.Count;
		if(count>0){
			string str=count+" hints for "+pos+": ";
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
					Debug.Log(player.pos+" meld bbbb desk remove "+Player.bunch2str(bun));
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
				Debug.Log(player.pos+" meld "+Player.bunch2str(bun));
			}
		}else if(bunch.Type>pb_enum.PhzAbc){
			Debug.Log(player.pos+" meld "+Player.bunch2str(bunch));
			player.playData.Bunch.Add(bunch);
		}
	}
	
	bool IsWin(Player player,int card,List<bunch_t> output,bool bDraw){
		var pos=player.pos;
		var suite=player.playData.Bunch;
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
			Debug.Log("isWin for "+player.pos+" failed: not fire and not from pile");
			return false;
		}

		//check desk
		var myself=(pos==Token);
		
		List<bunch_t> allSuites=new List<bunch_t>(suite);
		List<int> hand=new List<int>(hands);

		//handle AAA first
		bunch_t AAAs=null;
		foreach(var aaa in player.AAAs){
			var a=aaa.Pawns[0];
			if(a/1000==card/1000 && a%100==card%100){
				hand.AddRange(aaa.Pawns);
				AAAs=aaa;
				player.AAAs.Remove(aaa);
				break;
			}
		}
		
		if(card>0){
			var inhand=false;
			foreach(var i in hand)if(i==card){inhand=true;break;}
			if(!inhand)hand.Add(card);
		}

		hand.Sort(comparision);

		//logHands(game,pos);
		//是否需要将
		bool needJiang=(player.AAAAs.Count>0);
		if(!needJiang)
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
		if(player.pos==0)Debug.Log("bunches 0("+allSuites.Count+"): "+Player.bunches2str(allSuites));
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
			if(over){
				resSuites.Add(tmpJiang);
				allSuites.Clear();
				allSuites.AddRange(resSuites);
				if(player.pos==0)Debug.Log("bunches 1("+allSuites.Count+"): "+Player.bunches2str(allSuites));
			}
		} else{
			//recursived
			over=isWin(hand,allSuites);
			if(player.pos==0)Debug.Log("bunches 2("+allSuites.Count+"): "+Player.bunches2str(allSuites));
		}
		if(over){
			over=false;
			var M=MaxPlayer;
			int MIN_SUITES=7;
			if(M==4)MIN_SUITES=5;//衡阳，碰胡子玩法
			if(allSuites.Count+player.AAAs.Count>=MIN_SUITES){
				if(Main.Instance.MainPlayer.category==pb_enum.PhzPeghz){
					//碰胡子判胡
					over=true;
				}else{
					var point=calcPoints(allSuites);
					point+=calcPoints(player.AAAs);
					if(point>=winPoint(Main.Instance.MainPlayer.category))
						over=true;
				}
				if(over){
					output.AddRange(allSuites);
					output.AddRange(player.AAAs);
				}
			}
		}
		if(!over && AAAs!=null)player.AAAs.Add(AAAs);

		return over;
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
				Debug.Log("isGameOver no suite for card "+card);
				buildBunch(card,_cards,hints);	//debug
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
		var hand=player.playData.Hands;
		var suite=player.playData.Bunch;
		
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
			hints.Pawns.Add(card);
			foreach(var it in tmp)hints.Pawns.Add(it);
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
					hints.Pawns.Add(card);
					hints.Pawns.AddRange(i.Pawns);
					//for(var c in i.Pawns)hints.Pawns.Add(c);
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

	bunch_t newIsWin(Player player,List<int> hands,int card,bool needAA=false){
		//already cullout AAA & AAAA outside in Hint
		bunch_t output=null;
		var bunches=new List<bunch_t>();
		var AAAs=new List<bunch_t>(player.AAAs);
		needAA=(needAA||player.AAAAs.Count>0);
		if(card==Configs.invalidCard){
			//natural win: AAAs
			if(player.AAAAs.Count>=3 || player.AAAs.Count>=5){
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
				var sz=hands.Count;
				if(sz>0){
					var I=(sz+2)/3;
					var suites=new List<bunch_t>(I);
					for(int i=0;i<sz;++i){
						var x=i/3;
						var suite=suites[x];
						suite.Type=pb_enum.Unknown;
						suite.Pawns.Add(hands[i]);
					}
					bunches.AddRange(suites);
				}
			}else
				//BBB,ABC
				bunches=buildABC(hands,needAA);
		}else{
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
			var desks=new List<bunch_t>(player.playData.Bunch);
			if(BBBB==null){
				foreach(var desk in desks){
					if(desk.Type==pb_enum.PhzAaawei){
						var h=desk.Pawns[0];
						if(h/1000==card/1000 && h%100==card%100){
							BBBB=new bunch_t();
							BBBB.Type=pb_enum.PhzBbbbdesk;
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
			if(!needAA){
				foreach(var bbbb in player.playData.Bunch){
					if(bbbb.Pawns.Count>=4){
						needAA=true;
						break;
					}
				}
			}
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
			if(bunches.Count>0)
				bunches.AddRange(desks);
		}

		if(bunches.Count>0){
			bunches.AddRange(player.AAAAs);
			bunches.AddRange(AAAs);

			output=new bunch_t();
			output.Type=pb_enum.BunchWin;
			output.Pos=player.pos;
			output.Pawns.Add(card==Configs.invalidCard?player.playData.Hands[0]:card);
			output.Child.Add(bunches);
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
				Debug.Log("isGameOver no suite for card "+card);
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
	
	List<bunch_t> hintABC(List<int> cards,int A){
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
/*
		//baihuo
		if(output.Count>0){
			var tmp=new List<bunch_t>(output);
			same.Add(A);
			
			output.Clear();
			foreach(bunch_t j in tmp)
				if(checkBaihuo(cards,same,j))
					output.Add(j);
		}
*/
		return output;
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
			suite.Pawns.Add(card);
			foreach(var c in jiao)suite.Pawns.Add(c);
			hints.Add(suite);
			//log("hint ops=%s, cards=%d, %d, %d", ops2String(suite.ops).c_str(), suite.Pawns[0], suite.Pawns[1], suite.Pawns[2]);
		}
		if(same.Count>0&&jiao.Count>0){
			//绞，append to hints: BBb
			var suite=new bunch_t();
			suite.Type=pb_enum.PhzAbc;
			suite.Pawns.Add(card);
			foreach(var it in same)if(card!=it){
				//别把自己放进去
				suite.Pawns.Add(it);
				break;
			}
			suite.Pawns.Add(jiao[0]);
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
				suite.Pawns.Add(card);
				suite.Pawns.Add(FontABackID[i][0]);
				suite.Pawns.Add(FontABackID[i+1][0]);
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
					suite.Pawns.Add(card);
					suite.Pawns.Add(E[0]);
					suite.Pawns.Add(F[0]);
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
						rule.nHands[player.pos]-=4;
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
}
