using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main	Instance=null;
	public HttpProxy	http;
	public WSProxy		ws;

	public bool			skipLogin=true;

	void Awake(){
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

	}
	
	void Update () {
	
	}
}
