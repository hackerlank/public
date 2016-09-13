﻿using UnityEngine;
using System.Collections;

public class Main : MonoBehaviour {

	public static Main	Instance=null;
	public HttpProxy	http;
	public WSProxy		ws;
	
	public enum Mode{
		NORMAL,
		STANDALONE,
		LOGIN,
		LOBBY,
	}
	public Mode GameMode{get{return Mode.STANDALONE;}}

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
	}
	
	void Update () {
	
	}
}
