using UnityEngine;
using System.Collections;

public class WSProxy {

	public SocketIO.SocketIOComponent socket;

	public void Connect(string uri){
		socket.url="ws://127.0.0.1/100";
		socket.Emit("hello");
	}

	public void Send(){

	}

	public void OnOpen(){

	}

	public void OnRead(){

	}
}
