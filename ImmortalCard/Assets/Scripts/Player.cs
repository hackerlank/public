using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public class Player {
	//networking
	public delegate void	MessageHandler(pb_msg mid,byte[] bytes);
	public HttpProxy		http;
	WSProxy					ws;
	bool					connected=false;

	public bool				InGame=false;

	public List<PlayerController>	controllers=new List<PlayerController>();
	public StoreGame		storeGame;	//store game for reconnect
	public play_t			playData=new play_t();
	public pb_enum			category;

	//phz
	public List<int>		unpairedCards=new List<int>();
	public List<int>		dodgeCards=new List<int>();
	public List<bunch_t>	AAAAs=new List<bunch_t>();	//startup AAAA,does not copy to desk bunches
	public List<bunch_t>	AAAs=new List<bunch_t>();
	public bool				conflictMeld=false;

	public MsgNCCreate	msgNCCreate;
	public MsgNCJoin	msgNCJoin;

	public Player(){
		//networks
		http=new HttpProxy();
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
			ws.Connect(Configs.ws+"/"+key);
			Debug.Log("connecting by key "+key);
		}
	}

	public IEnumerator Reconnect(){
		//in game,send and wait for reconnect
		InGame=false;
		Connect(storeGame.gameId);
		while(!InGame)yield return null;
		
		MsgCNReconnect msg=new MsgCNReconnect();
		msg.Mid=pb_msg.MsgCnRevive;
		msg.Game=storeGame.gameId;
		Main.Instance.MainPlayer.Send<MsgCNReconnect>(msg.Mid,msg);
		Debug.Log("reconnect game by key "+storeGame.gameId);

		Debug.Log("reconnect game controller="+Main.Instance.gameController);
		if(Main.Instance.gameController==null)
			yield return Main.Instance.StartCoroutine(CreateGame(
				(pb_enum)storeGame.gameType,storeGame.gameId,storeGame.robots));
	}

	public void Disconnect(){
		InGame=false;
		if(connected)
			ws.Close();
	}

	public IEnumerator CreateGame(pb_enum game,int gameId,int nRobots=0){
		GamePanel panel=null;
		System.Action<Component> handler=delegate(Component obj){
			panel=obj as GamePanel;
		};

		switch(game){
		case pb_enum.GameMj:
			MahJongPanel.Create(handler);
			break;
		case pb_enum.GamePhz:
			PaohuziPanel.Create(handler);
			break;
		case pb_enum.GameDdz:
		default:
			DoudeZhuPanel.Create(handler);
			break;
		}
		while (panel==null)
			yield return null;

		if(nRobots>0){
			//add robots demand
			var MP=panel.Rule.MaxPlayer;
			if(nRobots>=MP)nRobots=MP-1;
			for(uint i=0;i<nRobots;++i){
				var robot=new Player();
				robot.playData=new Proto3.play_t();
				robot.playData.Player=new Proto3.player_t();
				robot.playData.Player.Uid="robot_"+i;
				robot.controllers.Add(panel.Rule.AIController);
				Main.Instance.robots.Add(robot);
				panel.StartCoroutine(robot.JoinGame(gameId));
			}
		}

		Main.Instance.Wait=false;
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

	public void onOpen(string error){
		if(!connected){
			connected=true;
			MsgCNEnter msg=new MsgCNEnter();
			msg.Mid=pb_msg.MsgCnConnect;
			msg.Version=100;
			msg.Uid=playData.Player.Uid;
			Send<MsgCNEnter>(msg.Mid,msg);
		}
		Loom.QueueOnMainThread(delegate{
			//dispatch to main thread
			Main.Instance.Wait=false;
		});
	}
	public void onClose(string error){
		Loom.QueueOnMainThread(delegate{
			if(InGame){
				if(this==Main.Instance.MainPlayer){
					Debug.Log("----disconnect and reconnect game");
				}else{
					Debug.Log("----disconnect robot");
				}
				Main.Instance.StartCoroutine(Reconnect());
			}
			
			var reconnect=false;
			if(reconnect){
				Main.Instance.Wait=true;
			}else{
				InGame=false;
				Main.Instance.Wait=false;
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

	public void onMessage(pb_msg mid,byte[] bytes){
		/*
		Loom.QueueOnMainThread(delegate{
			//dispatch to main thread
			Main.Instance.StopWait();
		});
		*/

		//receive and handle logic which PlayerController indenpendent
		//Debug.Log("OnMessage "+mid);
		switch(mid){
		case pb_msg.MsgScLogin:
			MsgSCLogin msgLogin=MsgSCLogin.Parser.ParseFrom(bytes);
			Debug.Log("response mid="+mid+",uid="+msgLogin.Uid+",ip="+msgLogin.Ip+",port="+msgLogin.Port);
			if(msgLogin.Result==pb_enum.Succeess){
				if(LoginPanel.Instance!=null)LoginPanel.Instance.DoLogin();
			}else
				Debug.LogError("login error: "+msgLogin.Result);
			break;
		case pb_msg.MsgNcConnect:
			MsgNCEnter msgEnter=MsgNCEnter.Parser.ParseFrom(bytes);
			Debug.Log("entered node");
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
					storeGame.gameId=msgCreate.GameId;
					var str=storeGame.ToString();
					PlayerPrefs.SetString(Configs.PrefsKey_StoreGame,str);
					Debug.Log("----create game and cache gameid "+storeGame.gameId);
				}
			}else
				Debug.LogError("create error: "+msgCreate.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin msgJoin=MsgNCJoin.Parser.ParseFrom(bytes);
			//Debug.Log("joined game");
			if(msgJoin.Result==pb_enum.Succeess){
				msgNCJoin=msgJoin;
				if(this==Main.Instance.MainPlayer)
				{
					Debug.Log("----create game and cache");
					storeGame.gameType=(int)msgJoin.Game;
					var str=storeGame.ToString();
					PlayerPrefs.SetString(Configs.PrefsKey_StoreGame,str);
				}
			}else
				Debug.LogError("join error: "+msgJoin.Result);
			break;
		case pb_msg.MsgNcEngage:
			MsgNCEngage msgEngage=MsgNCEngage.Parser.ParseFrom(bytes);
			if(msgEngage.Result==pb_enum.Succeess){
			}else
				Debug.LogError("engage error: "+msgEngage.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgEngage(this,msgEngage));
			break;

		case pb_msg.MsgNcRevive:
			MsgNCReconnect msgReconn=MsgNCReconnect.Parser.ParseFrom(bytes);
			Debug.Log("reconnected game");
			if(msgReconn.Result==pb_enum.Succeess){
				foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgRevive(this,msgReconn));
			}
			break;

		case pb_msg.MsgNcStart:
			MsgNCStart msgStart=MsgNCStart.Parser.ParseFrom(bytes);
			if(msgStart.Result==pb_enum.Succeess){
				var rule=Main.Instance.gameController.Rule;
				var M=rule.MaxPlayer;

				//clear rule data
				rule.nHands=new int[M];
				for(int i=0;i<M;++i)rule.nHands[i]=msgStart.Count[i];

				//clear player data
				playData.Hands.Clear();
				playData.Discards.Clear();
				playData.Bunch.Clear();
				playData.SelectedCard=-1;
				AAAs.Clear();
				AAAAs.Clear();
				unpairedCards.Clear();
				dodgeCards.Clear();
				conflictMeld=false;

				//copy data
				playData.Seat=msgStart.Pos;
				playData.Hands.AddRange(msgStart.Hands);
				var str="deal "+playData.Seat+":";
				foreach(var hand in msgStart.Hands)str+=hand+",";
				Debug.Log(str);
			}else
				Debug.LogError("start error: "+msgStart.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgStart(this,msgStart));
			break;
			
		case pb_msg.MsgNcDiscard:
			if(null==Main.Instance.gameController)break;
			MsgNCDiscard msgDiscard=MsgNCDiscard.Parser.ParseFrom(bytes);
			if(msgDiscard.Result==pb_enum.Succeess){
				//count hands before handle message
				if(Main.Instance.MainPlayer==this){
					Debug.Log("----MsgNcDiscard player");
					//how can i do? we can only decreament once
					var rule=Main.Instance.gameController.Rule;
					rule.nHands[msgDiscard.Bunch.Pos]-=msgDiscard.Bunch.Pawns.Count;
				}else
					Debug.Log("----MsgNcDiscard robot");

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
					Debug.Log("----finish game and clear cache");
					PlayerPrefs.DeleteKey(Configs.PrefsKey_StoreGame);
				}
			}
			else
				Debug.LogError("finish error: "+msgFinish.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgFinish(this,msgFinish));
			break;
		case pb_msg.MsgNcDismissSync:
			MsgNCDismissSync msgDismissSync=MsgNCDismissSync.Parser.ParseFrom(bytes);
			if(msgDismissSync.Result==pb_enum.Succeess){
			}else
				Debug.LogError("dismiss sync error: "+msgDismissSync.Result);
			//foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMessage(this,msg));
			break;
		case pb_msg.MsgNcDismissAck:
			MsgNCDismissAck msgDismissAck=MsgNCDismissAck.Parser.ParseFrom(bytes);
			if(msgDismissAck.Result==pb_enum.Succeess){
			}else
				Debug.LogError("dismiss ack error: "+msgDismissAck.Result);
			//foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMessage(this,msg));
			break;
		default:
			break;
		}
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
