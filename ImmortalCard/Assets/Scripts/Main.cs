using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main		Instance=null;
	public HttpProxy		http;
	public WSProxy			ws;

	public uint				Round=4;
	public GameRule			gameRule=new GameRule();
	public GameController	gameController=null;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	void Awake(){
		//Loom
		gameObject.AddComponent<Loom>();

		//networks
		http=new HttpProxy();
		http.onResponse+=MsgHandler.onMessage;

		ws=new WSProxy();
		ws.onOpen+=MsgHandler.onOpen;
		ws.onClose+=MsgHandler.onClose;
		ws.onError+=MsgHandler.onError;
		ws.onMessage+=MsgHandler.onMessage;

		Instance=this;
	}

	void Start () {
		StartCoroutine(CardCache.Load());
	}
	
	void Update () {
	
	}
}
