using UnityEngine;
using System.Collections;

public class Player {
	public HttpProxy	http;
	public WSProxy		ws;
	public MsgHandler	msgHandler=new MsgHandler();

	public Player(){
		//networks
		http=new HttpProxy();
		http.onResponse+=msgHandler.onMessage;
		
		ws=new WSProxy();
		ws.onOpen+=msgHandler.onOpen;
		ws.onClose+=msgHandler.onClose;
		ws.onError+=msgHandler.onError;
		ws.onMessage+=msgHandler.onMessage;
	}
}
