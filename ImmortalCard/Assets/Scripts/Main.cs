using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main		Instance=null;
	public HttpProxy		http;
	public WSProxy			ws;
	public Player			player=new Player();

	public uint				Round=4;
	public GameController	gameController=null;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	public MsgHandler msgHandler=new MsgHandler();
	void Awake(){
		//Loom
		gameObject.AddComponent<Loom>();

		//networks
		http=new HttpProxy();
		http.onResponse+=msgHandler.onMessage;

		ws=new WSProxy();
		ws.onOpen+=msgHandler.onOpen;
		ws.onClose+=msgHandler.onClose;
		ws.onError+=msgHandler.onError;
		ws.onMessage+=msgHandler.onMessage;

		Instance=this;
	}

	void Start () {
	}
	
	void Update () {
	}
}
