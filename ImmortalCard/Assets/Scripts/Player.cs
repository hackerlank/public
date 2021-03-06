using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public class Player {
	//networking
	public HttpProxy		http;
	WSProxy					ws;
	bool					connected=false;

	public bool				InGame=false;

	public List<PlayerController>	controllers=new List<PlayerController>();
	public play_t			playData=new play_t();
	public ulong			session;
	public pb_enum			category;

	//phz
	public List<int>		unpairedCards=new List<int>();
	public List<int>		dodgeCards=new List<int>();
	public List<bunch_t>	AAAAs=new List<bunch_t>();	//startup AAAA,does not copy to desk bunches
	public List<bunch_t>	AAAs=new List<bunch_t>();
	public bool				conflictMeld=false;

	public MsgLCLogin		msgLCLogin;
	public MsgNCCreate		msgNCCreate;
	public MsgNCJoin		msgNCJoin;
	public MsgCNRevive		msgRevive;

	public Player(){
		//networks
		http=new HttpProxy();
		http.onError+=onError;
		http.onResponse+=onMessage;
		
		ws=new WSProxy();
		ws.onOpen+=onOpen;
		ws.onClose+=onClose;
		ws.onError+=onError;
		ws.onMessage+=onMessage;
	}

	public void Connect(int gid=0){
		if(!connected){
			var M=(int)pb_enum.DefMaxNodes;
			if(gid==0){
				//re-generate key
				Random.seed=(int)Utils.time;
				gid=(int)(Random.value*M)*(int)pb_enum.DefMaxGamesPerNode;
			}

			//connect by key
			var key=gid/(int)pb_enum.DefMaxGamesPerNode;
			ws.Connect(Config.ws+"/"+key);
			//Debug.Log("connecting by key "+key);
		}
	}

	public IEnumerator Reconnect(){
		//reconnect
		InGame=false;
		var storeGame=Cache.storeGame;
		Connect(storeGame.gameId);

		while(!InGame)yield return null;

		//reloaded
		if(this==Main.Instance.MainPlayer && Main.Instance.gameController==null){
			//create game panel
			yield return Main.Instance.StartCoroutine(
				CreateGame((pb_enum)storeGame.gameType,storeGame.gameId)
				);

			//add robots and reconnect
			addRobots(storeGame.robots);
			foreach(var robot in Main.Instance.robots){
				Main.Instance.StartCoroutine(robot.Reconnect());
			}
		}

		var msg=new MsgCNRevive();
		msg.Mid=pb_msg.MsgCnRevive;
		msg.Game=storeGame.gameId;
		Send<MsgCNRevive>(msg.Mid,msg);
		Debug.Log("revive game "+storeGame.gameId);
	}

	public void Disconnect(){
		InGame=false;
		if(connected)
			ws.Close();
	}

	public IEnumerator CreateGame(pb_enum game,int gameId){
		GamePanel panel=null;
		System.Action<Component> handler=delegate(Component obj){
			panel=obj as GamePanel;
		};

		switch(game){
		case pb_enum.GamePhz:
			PaohuziPanel.Create(handler);
			break;
		case pb_enum.GameDdz:
			DoudeZhuPanel.Create(handler);
			break;
		case pb_enum.GameMj:
		default:
			MahJongPanel.Create(handler);
			break;
		}
		while (panel==null)
			yield return null;

		//prepare cache before panel shown,to ensure revive
		panel.Rule.PrepareCache();
		while(!CardCache.Ready)yield return null;

		BlockView.Instance.Blocking=false;
	}

	public static void addRobots(int nRobots){
		var ctrl=Main.Instance.gameController;
		if(ctrl==null){
			Debug.Log("no game controller when add robot");
			return;
		}

		if(nRobots>0){
			//add robots demand
			Main.Instance.robots.Clear();
			var MP=ctrl.Rule.MaxPlayer;
			if(nRobots>=MP)nRobots=MP-1;
			for(uint i=0;i<nRobots;++i){
				var robot=new Player();
				robot.playData=new Proto3.play_t();
				robot.playData.Player=new Proto3.player_t();
				robot.playData.Player.Uid="robot_"+i;
				robot.controllers.Add(ctrl.Rule.AIController);
				Main.Instance.robots.Add(robot);
			}
		}
	}

	public IEnumerator JoinGame(int gameId){
		Connect(gameId);
		while(!InGame)yield return null;
		
		MsgCNJoin msgJ=new MsgCNJoin();
		msgJ.Mid=pb_msg.MsgCnJoin;
		msgJ.GameId=gameId;
		
		Send<MsgCNJoin>(msgJ.Mid,msgJ);
		Debug.Log("join game by id "+gameId);
		
		while(msgNCJoin==null)yield return null;
	}

	public void DismissAck(pb_enum ops){
		var msg=new MsgCNDismissAck();
		msg.Mid=pb_msg.MsgCnDismissAck;
		msg.Ops=ops;
		Send<MsgCNDismissAck>(msg.Mid,msg);
	}

	public void onOpen(string error){
		if(!connected){
			connected=true;
			var msg=new MsgCNConnect();
			msg.Mid=pb_msg.MsgCnConnect;
			msg.Version=100;
			msg.Uid=playData.Player.Uid;
			Send<MsgCNConnect>(msg.Mid,msg);
		}
		Loom.QueueOnMainThread(delegate{
			//dispatch to main thread
			BlockView.Instance.Blocking=false;
		});
	}

	public void onClose(string error){
		Loom.QueueOnMainThread(delegate{
			if(InGame){
				Main.Instance.StartCoroutine(Reconnect());
			}
			
			var reconnect=false;
			if(reconnect){
				BlockView.Instance.Blocking=true;
			}else{
				InGame=false;
				BlockView.Instance.Blocking=false;
			}
		});
		connected=false;
		Debug.Log("OnClose "+error);
	}
	public void onError(string error){
		Debug.Log("OnError: "+error);
	}

	public void Send<T>(pb_msg mid,T msg) where T : IMessage<T>{
		/*
		Loom.QueueOnMainThread(delegate{
			//dispatch to main thread
			Main.Instance.StartWait();
		});
		*/
		ws.Send<T>(mid,msg);
	}

	public void onError(pb_msg mid,string error){
		switch(mid){
		case pb_msg.MsgClLogin:
			BlockView.Instance.ShowDialog(error,"登录错误!");
			break;
		default:
			break;
		}
	}

	public void onMessage(pb_msg mid,byte[] bytes){
		//receive and handle logic which PlayerController indenpendent
		//Debug.Log("OnMessage "+mid);
		foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.PreMessage(mid,bytes));
		switch(mid){
			// -----------------------------------------------------------
			// Lobby
			// -----------------------------------------------------------
		//Login
		case pb_msg.MsgLcLogin:
			MsgLCLogin msgLogin=MsgLCLogin.Parser.ParseFrom(bytes);
			if(msgLogin.Result==pb_enum.Succeess){
				Debug.Log(msgLogin.Player.Uid+" login");
				playData.Player=msgLogin.Player;
				session=msgLogin.Session;
				msgLCLogin=msgLogin;
			}else
				Debug.LogError("login error: "+msgLogin.Result);
			break;

		//Lobby
		case pb_msg.MsgLcLobby:
			MsgLCLobby msgLobby=MsgLCLobby.Parser.ParseFrom(bytes);
			if(msgLobby.Result==pb_enum.Succeess){
				LobbyPanel.lobby=msgLobby.Lobby;
				Config.parseLobby(msgLobby);
			}else
				Debug.LogError("lobby error: "+msgLobby.Result);
			break;

		case pb_msg.MsgLcReplays:
			MsgLCReplays msgReplays=MsgLCReplays.Parser.ParseFrom(bytes);
			if(msgReplays.Result==pb_enum.Succeess){
				ReplayView.msgReplays=msgReplays;
			}else
				Debug.LogError("lobby error: "+msgReplays.Result);
			break;

		case pb_msg.MsgLcReplay:
			MsgLCReplay msgReplay=MsgLCReplay.Parser.ParseFrom(bytes);
			if(msgReplay.Result==pb_enum.Succeess){
				ReplayView.msgReplayData=msgReplay;
			}else
				Debug.LogError("lobby error: "+msgReplay.Result);
			break;
			// -----------------------------------------------------------
			// Node
			// -----------------------------------------------------------
		//Node
		case pb_msg.MsgNcConnect:
			MsgNCConnect msgEnter=MsgNCConnect.Parser.ParseFrom(bytes);
			//Debug.Log("connected node");
			if(msgEnter.Result==pb_enum.Succeess){
				InGame=true;
			}else{
				Debug.LogError("enter error: "+msgEnter.Result);
				Disconnect();
			}
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate msgCreate=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+msgCreate.GameId);
			if(msgCreate.Result==pb_enum.Succeess){
				msgNCCreate=msgCreate;
				if(this==Main.Instance.MainPlayer)
				{
					var storeGame=Cache.storeGame;
					storeGame.gameId=msgCreate.GameId;
					var str=storeGame.ToString();
					PlayerPrefs.SetString(Cache.PrefsKey_StoreGame,str);
				}
			}else
				Debug.LogError("create error: "+msgCreate.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin msgJoin=MsgNCJoin.Parser.ParseFrom(bytes);
			//Debug.Log("joined game");
			if(msgJoin.Result==pb_enum.Succeess){
				msgNCJoin=msgJoin;
				Main.Instance.Round=msgJoin.MaxRound;
				if(this==Main.Instance.MainPlayer)
				{
					var storeGame=Cache.storeGame;
					storeGame.gameType=(int)msgJoin.Game;
					var str=storeGame.ToString();
					PlayerPrefs.SetString(Cache.PrefsKey_StoreGame,str);
				}
			}else
				Debug.LogError("join error: "+msgJoin.Result);
			break;

		case pb_msg.MsgNcDeal:
			if(Main.Instance.gameController==null)break;
			
			MsgNCDeal msgDeal=MsgNCDeal.Parser.ParseFrom(bytes);
			if(msgDeal.Result==pb_enum.Succeess){
				var rule=Main.Instance.gameController.Rule;
				var M=rule.MaxPlayer;
				
				//clear rule data
				rule.nHands=new int[M];
				for(int i=0;i<M;++i)rule.nHands[i]=msgDeal.Count[i];

				//Debug.Log("---- deal "+playData.Seat+" score="+playData.Score+",total="+playData.Total);

				//clear player data
				playData.Hands.Clear();
				playData.Discards.Clear();
				playData.Bunch.Clear();
				playData.Engagement=-1;
				AAAs.Clear();
				AAAAs.Clear();
				unpairedCards.Clear();
				dodgeCards.Clear();
				conflictMeld=false;
				
				//copy data
				playData.Seat=msgDeal.Pos;
				playData.Hands.AddRange(msgDeal.Hands);
				var str="deal "+playData.Seat+":";
				foreach(var hand in msgDeal.Hands)str+=hand+",";
				Debug.Log(str);
			}else
				Debug.LogError("start error: "+msgDeal.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgDeal(this,msgDeal));
			break;

		case pb_msg.MsgNcEngage:
			MsgNCEngage msgEngage=MsgNCEngage.Parser.ParseFrom(bytes);
			if(msgEngage.Result==pb_enum.Succeess){
			}else
				Debug.LogError("engage error: "+msgEngage.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgEngage(this,msgEngage));
			break;

		case pb_msg.MsgNcRevive:
			MsgNCRevive msgRevive=MsgNCRevive.Parser.ParseFrom(bytes);
			if(msgRevive.Result==pb_enum.Succeess){
				var rule=Main.Instance.gameController.Rule;
				var M=rule.MaxPlayer;
				
				//clear rule data
				rule.nHands=new int[M];
				for(int i=0;i<M;++i)rule.nHands[i]=msgRevive.Deal.Count[i];
				
				//clear player data
				playData.Hands.Clear();
				playData.Discards.Clear();
				playData.Bunch.Clear();
				playData.Engagement=-1;
				AAAs.Clear();
				AAAAs.Clear();
				unpairedCards.Clear();
				dodgeCards.Clear();
				conflictMeld=false;
				
				//copy data
				var playFrom=msgRevive.Play[msgRevive.Deal.Pos];
				playData.MergeFrom(playFrom);
				playData.Seat=msgRevive.Deal.Pos;
				playData.Hands.AddRange(msgRevive.Deal.Hands);

				Main.Instance.gameController.Round=msgRevive.Round;
				Main.Instance.Round=msgRevive.MaxRound;
				var str="revive deal "+playData.Seat+":";
				foreach(var hand in playData.Hands)str+=hand+",";
				Debug.Log(str);
			}else
				Debug.LogError("revive error: "+msgRevive.Result);

			foreach(var ctrl in controllers){
				Main.Instance.StartCoroutine(ctrl.OnMsgRevive(this,msgRevive));
			}
			break;

		case pb_msg.MsgNcDiscard:
			if(null==Main.Instance.gameController)break;
			MsgNCDiscard msgDiscard=MsgNCDiscard.Parser.ParseFrom(bytes);
			if(msgDiscard.Result==pb_enum.Succeess){
				//count hands before handle message
				if(Main.Instance.MainPlayer==this){
					//how can i do? we can only decreament once
					var rule=Main.Instance.gameController.Rule;
					rule.nHands[msgDiscard.Bunch.Pos]-=msgDiscard.Bunch.Pawns.Count;
				}

				//handle only succeess to avoid crash
				foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgDiscard(this,msgDiscard));
				//append historical
				var hist=Main.Instance.gameController.Rule.Historical;
				if(hist.Count<=0||hist[hist.Count-1].Pos!=msgDiscard.Bunch.Pos){
					Debug.Log("add historical for "+msgDiscard.Bunch.Pos+" "+Player.bunch2str(msgDiscard.Bunch));
					hist.Add(msgDiscard.Bunch);
				}
				//remove discards from hands after handle message
				if(playData.Seat==msgDiscard.Bunch.Pos){
					foreach(var card in msgDiscard.Bunch.Pawns)
						playData.Hands.Remove(card);
				}
			}else
				Debug.LogError("discard error: "+msgDiscard.Result);
			break;
		case pb_msg.MsgNcMeld:
			MsgNCMeld msgMeld=MsgNCMeld.Parser.ParseFrom(bytes);
			if(msgMeld.Result==pb_enum.Succeess){
				//remember conflict meld
				var from=msgMeld.From;
				var rule=Main.Instance.gameController.Rule;
				if(msgMeld.Bunch.Type==pb_enum.PhzBbbbdesk
				   && playData.Seat==from&&playData.Seat!=msgMeld.Bunch.Pos){
					if(rule.Pile.IndexOf(msgMeld.Bunch.Pawns[0])==-1){
						conflictMeld=true;
						Debug.Log(playData.Seat+" conflict "+msgMeld.Bunch.Pawns[0]);
					}
				}
			}else
				Debug.LogError("meld error: "+msgMeld.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgMeld(this,msgMeld));
			break;
		case pb_msg.MsgNcDraw:
			MsgNCDraw msgDraw=MsgNCDraw.Parser.ParseFrom(bytes);
			if(this==Main.Instance.MainPlayer)
				//should modify before all controllers
				Main.Instance.gameController.Rule.Pile.Add(msgDraw.Card);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgDraw(this,msgDraw));
			break;
		case pb_msg.MsgNcSettle:
			MsgNCSettle msgSettle=MsgNCSettle.Parser.ParseFrom(bytes);
			if(msgSettle.Result==pb_enum.Succeess){
				if(this!=Main.Instance.MainPlayer){
					//robots
					var omsgReady=new MsgCNReady();
					omsgReady.Mid=pb_msg.MsgCnReady;
					Send<MsgCNReady>(omsgReady.Mid,omsgReady);
				}
			}else
				Debug.LogError("settle error: "+msgSettle.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgSettle(this,msgSettle));
			break;
		case pb_msg.MsgNcFinish:
			MsgNCFinish msgFinish=MsgNCFinish.Parser.ParseFrom(bytes);
			if(msgFinish.Result==pb_enum.Succeess)
			{
				if(this==Main.Instance.MainPlayer)
				{
					//Debug.Log("----finish game and clear cache");
					PlayerPrefs.DeleteKey(Cache.PrefsKey_StoreGame);
				}
			}
			else
				Debug.LogError("finish error: "+msgFinish.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgFinish(this,msgFinish));
			break;
		case pb_msg.MsgNcDismissSync:
			MsgNCDismissSync msgDismissSync=MsgNCDismissSync.Parser.ParseFrom(bytes);
			if(msgDismissSync.Result==pb_enum.Succeess){
				var panel=Main.Instance.gameController as GamePanel;
				if(this==Main.Instance.MainPlayer && null!=panel){
					var info=string.Format("玩家 {0} 申请解散房间，是否同意？",msgDismissSync.Pos);
					BlockView.Instance.ShowDialog(info,"",
					                              delegate {
						DismissAck(pb_enum.Succeess);
						panel.StartCoroutine(panel.DismissWaiting());
					},delegate {
						DismissAck(pb_enum.ErrCancelled);
					});
				}else
					DismissAck(pb_enum.Succeess);
			}else
				Debug.LogError("dismiss sync error: "+msgDismissSync.Result);
			//foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMessage(this,msg));
			break;
		case pb_msg.MsgNcDismissAck:
			MsgNCDismissAck msgDismissAck=MsgNCDismissAck.Parser.ParseFrom(bytes);
			if(msgDismissAck.Result==pb_enum.Succeess){
				var panel=Main.Instance.gameController as GamePanel;
				if(null!=panel && this==Main.Instance.MainPlayer)
					panel.msgDismissAck=msgDismissAck;
			}else
				Debug.LogError("dismiss ack error: "+msgDismissAck.Result);
			//foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMessage(this,msg));
			break;
		default:
			break;
		}
		foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.PostMessage(mid,bytes));
	}

	public static string bunch2str(bunch_t bunch){
		string str="ops="+(int)bunch.Type;
		if(bunch.Pawns.Count<=0)
			str+=",empty";
		else{
			str+=",cards(";
			foreach(var card in bunch.Pawns)
				str+=card+",";
			str+=")";
		}
		return str;
	}
	
	public static string bunches2str(List<bunch_t> bunches){
		var str="";
		foreach(var bunch in bunches)str+=bunch2str(bunch);
		return str;
	}

	public static string bunches2str(global::Google.Protobuf.Collections.RepeatedField<bunch_t> bunches){
		var str="";
		foreach(var bunch in bunches)str+=bunch2str(bunch);
		return str;
	}
}
