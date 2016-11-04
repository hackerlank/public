using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Proto3;
using Google.Protobuf;

public class Player {
	//networking
	public delegate void	MessageHandler(pb_msg mid,byte[] bytes);
	public bool				Entered=false;
	public HttpProxy		http;
	WSProxy					ws;

	bool					connected=false;

	public List<PlayerController>	controllers=new List<PlayerController>();
	public int				gameId=0;
	public int				pos=0;
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

	public void Disconnect(){
		if(connected)
			ws.Close();
	}

	public void Send<T>(pb_msg mid,T msg) where T : IMessage<T>{
		ws.Send<T>(mid,msg);
	}
		
	public IEnumerator JoinGame(int id){
		gameId=id;
		Connect(gameId);
		while(!Entered)yield return null;
		
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
			msg.Mid=pb_msg.MsgCnEnter;
			msg.Version=100;
			Send<MsgCNEnter>(msg.Mid,msg);
		}
	}
	public void onClose(string error){
		connected=false;
		Debug.Log("OnClose "+error);
	}
	public void onError(string error){
		Debug.Log("OnError: "+error);
	}
	public void onMessage(pb_msg mid,byte[] bytes){
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
		case pb_msg.MsgNcEnter:
			MsgNCEnter msgEnter=MsgNCEnter.Parser.ParseFrom(bytes);
			//Debug.Log("entered");
			if(msgEnter.Result==pb_enum.Succeess){
				Entered=true;
			}else
				Debug.LogError("enter error: "+msgEnter.Result);
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate msgCreate=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+msgCreate.GameId);
			if(msgCreate.Result==pb_enum.Succeess){
				gameId=msgCreate.GameId;
				msgNCCreate=msgCreate;
			}else
				Debug.LogError("create error: "+msgCreate.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin msgJoin=MsgNCJoin.Parser.ParseFrom(bytes);
			//Debug.Log("joined game");
			if(msgJoin.Result==pb_enum.Succeess){
				msgNCJoin=msgJoin;
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
				pos=msgStart.Pos;
				playData.Hands.AddRange(msgStart.Hands);
				var str="deal "+pos+":";
				foreach(var hand in msgStart.Hands)str+=hand+",";
				Debug.Log(str);
			}else
				Debug.LogError("start error: "+msgStart.Result);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgStart(this,msgStart));
			break;
			
		case pb_msg.MsgNcDiscard:
			MsgNCDiscard msgDiscard=MsgNCDiscard.Parser.ParseFrom(bytes);
			foreach(var ctrl in controllers)Main.Instance.StartCoroutine(ctrl.OnMsgDiscard(this,msgDiscard));
			if(msgDiscard.Result==pb_enum.Succeess){
				//append historical
				var hist=Main.Instance.gameController.Rule.Historical;
				if(hist.Count<=0||hist[hist.Count-1].Pos!=msgDiscard.Bunch.Pos){
					Debug.Log("add historical for "+msgDiscard.Bunch.Pos+" "+Player.bunch2str(msgDiscard.Bunch));
					hist.Add(msgDiscard.Bunch);
				}
				//remove discards from hands
				if(pos==msgDiscard.Bunch.Pos){
					var rule=Main.Instance.gameController.Rule;
					rule.nHands[pos]-=msgDiscard.Bunch.Pawns.Count;
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
				var rule=Main.Instance.gameController.Rule;
				if(msgMeld.Bunch.Type==pb_enum.PhzBbbbdesk&&pos==rule.Token&&pos!=msgMeld.Bunch.Pos){
					if(rule.Pile.IndexOf(msgMeld.Bunch.Pawns[0])==-1){
						conflictMeld=true;
						Debug.Log(pos+" conflict "+msgMeld.Bunch.Pawns[0]);
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
			if(msgFinish.Result==pb_enum.Succeess){
			}else
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
