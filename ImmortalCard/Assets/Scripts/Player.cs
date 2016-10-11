using UnityEngine;
using System.Collections;
using Proto3;
using Google.Protobuf;

public class Player {
	//networking
	public delegate void	MessageHandler(Player player,IMessage msg);
	public delegate void	ProtobufHandler(pb_msg mid,byte[] bytes);
	public bool				Entered=false;
	public HttpProxy		http;
	WSProxy					ws;

	bool					connected=false;
	//ui controller,keep null for robot
	//public PlayerController	controller;
	public MessageHandler	msgHandler=delegate(Player player,IMessage msg){};

	public uint				gameId=0;
	public uint				pos=0;
	public game_data_t		gameData=new game_data_t();

	bool					bRobot=false;

	public MsgNCCreate	msgNCCreate;
	public MsgNCJoin	msgNCJoin;

	public Player(bool robot=false){
		bRobot=robot;

		//networks
		http=new HttpProxy();
		http.onResponse+=onMessage;
		
		ws=new WSProxy();
		ws.onOpen+=onOpen;
		ws.onClose+=onClose;
		ws.onError+=onError;
		ws.onMessage+=onMessage;
	}

	public void Connect(uint id=0){
		if(!connected){
			var M=(uint)pb_enum.DefMaxNodes;
			if(id==0){
				//re-generate key
				Random.seed=(int)Utils.time;
				id=(uint)(Random.value*M);
			}
			gameId=id;

			//connect by key
			var key=id%M;
			ws.Connect(Configs.ws+"/"+key);
			Debug.Log("connecting by key "+key);
		}
	}

	public void Send<T>(pb_msg mid,T msg) where T : IMessage<T>{
		ws.Send<T>(mid,msg);
	}
		
	public IEnumerator JoinGame(uint id){
		Connect(id);
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
			/*
			Loom.QueueOnMainThread(delegate{
				if(CreatePanel.Instance!=null)
					CreatePanel.Instance.OnConnected();
			});
			*/
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
		//Debug.Log("OnMessage "+mid);
		IMessage msg=null;
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
			Debug.Log("entered");
			if(msgEnter.Result==pb_enum.Succeess){
				Entered=true;
				/*
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnMsgCNEnter();
				});
				*/
			}else
				Debug.LogError("enter error: "+msgEnter.Result);
			break;
		case pb_msg.MsgNcCreate:
			MsgNCCreate msgCreate=MsgNCCreate.Parser.ParseFrom(bytes);
			Debug.Log("created game "+msgCreate.GameId);
			if(msgCreate.Result==pb_enum.Succeess){
				msgNCCreate=msgCreate;
				/*
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnCreated(msgCreate);
				});
				*/
			}else
				Debug.LogError("create error: "+msgCreate.Result);
			break;
		case pb_msg.MsgNcJoin:
			MsgNCJoin msgJoin=MsgNCJoin.Parser.ParseFrom(bytes);
			Debug.Log("joined game");
			if(msgJoin.Result==pb_enum.Succeess){
				msgNCJoin=msgJoin;
				/*
				Loom.QueueOnMainThread(delegate{
					if(CreatePanel.Instance!=null)CreatePanel.Instance.OnJoined(msgJoin);
				});
				*/
			}else
				Debug.LogError("join error: "+msgJoin.Result);
			break;
		case pb_msg.MsgNcStart:
			MsgNCStart msgStart=MsgNCStart.Parser.ParseFrom(bytes);
			Debug.Log("start game");
			if(msgStart.Result==pb_enum.Succeess){
				msg=msgStart;
			}else
				Debug.LogError("start error: "+msgStart.Result);
			break;
			
		case pb_msg.MsgNcDiscard:
			MsgNCDiscard msgDiscard=MsgNCDiscard.Parser.ParseFrom(bytes);
			if(msgDiscard.Result==pb_enum.Succeess){
				msg=msgDiscard;
			}else
				Debug.LogError("discard error: "+msgDiscard.Result);
			break;
		case pb_msg.MsgNcMeld:
			MsgNCMeld msgMeld=MsgNCMeld.Parser.ParseFrom(bytes);
			if(msgMeld.Result==pb_enum.Succeess){
				msg=msgMeld;
			}else
				Debug.LogError("meld error: "+msgMeld.Result);
			break;
		case pb_msg.MsgNcDraw:
			MsgNCDraw msgDraw=MsgNCDraw.Parser.ParseFrom(bytes);
			msg=msgDraw;
			break;
		case pb_msg.MsgNcSettle:
			MsgNCSettle msgSettle=MsgNCSettle.Parser.ParseFrom(bytes);
			if(msgSettle.Result==pb_enum.Succeess){
				msg=msgSettle;
			}else
				Debug.LogError("settle error: "+msgSettle.Result);
			break;
		case pb_msg.MsgNcFinish:
			MsgNCFinish msgFinish=MsgNCFinish.Parser.ParseFrom(bytes);
			if(msgFinish.Result==pb_enum.Succeess){
				msg=msgFinish;
			}else
				Debug.LogError("finish error: "+msgFinish.Result);
			break;
		case pb_msg.MsgNcDismissSync:
			MsgNCDismissSync msgDismissSync=MsgNCDismissSync.Parser.ParseFrom(bytes);
			if(msgDismissSync.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss sync error: "+msgDismissSync.Result);
			break;
		case pb_msg.MsgNcDismissAck:
			MsgNCDismissAck msgDismissAck=MsgNCDismissAck.Parser.ParseFrom(bytes);
			if(msgDismissAck.Result==pb_enum.Succeess){
				Loom.QueueOnMainThread(delegate{
				});
			}else
				Debug.LogError("dismiss ack error: "+msgDismissAck.Result);
			break;
		default:
			break;
		}
		if(msg!=null)msgHandler.Invoke(this,msg);
	}
}
