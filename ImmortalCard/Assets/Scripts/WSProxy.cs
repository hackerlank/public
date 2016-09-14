using UnityEngine;
using System.Collections;
using System.IO;
using Proto3;
using Google.Protobuf;

public class WSProxy {

	public delegate void Handler(string error=null);

	public event Handler		onOpen=delegate(string error){};
	public event Handler		onClose=delegate(string error){};
	public event Handler		onError=delegate(string error){};
	public event MsgHandler.MessageHandler onMessage=delegate(pb_msg mid,byte[] bytes){};

	WebSocketSharp.WebSocket socket;

	public void Connect(string uri){
		//ws://127.0.0.1:8820/100
		socket=new WebSocketSharp.WebSocket(uri);

		socket.OnOpen+=delegate(object sender, System.EventArgs e){
			onOpen.Invoke(e.ToString());
		};
		socket.OnClose+=delegate(object sender, WebSocketSharp.CloseEventArgs e){
			onClose.Invoke(e.Reason);
		};
		socket.OnError+=delegate(object sender, WebSocketSharp.ErrorEventArgs e) {
			onOpen.Invoke(e.Message);
		};
		socket.OnMessage+=delegate(object sender, WebSocketSharp.MessageEventArgs e) {
			var raw=e.RawData;
			
			byte[] body=new byte[raw.Length-2];
			System.Buffer.BlockCopy(raw,2,body,0,body.Length);

			MsgBase baseMsg=MsgBase.Parser.ParseFrom(body);
			var mid=baseMsg.Mid;
			
			onMessage.Invoke(mid,body);
		};
		socket.Connect();
	}

	public void Send<T>(pb_msg mid,T msg) where T : IMessage<T>{
		var body=MsgIntepreter.EncodeBytes<T>(msg);
		var len=body.Length;

		byte[] bytes=new byte[len+2];
		bytes[0]=(byte)(len&0xff);
		bytes[1]=(byte)(len>>8);
		System.Buffer.BlockCopy(body,0,bytes,2,len);

		socket.SendAsync(bytes,delegate(bool result){
			Debug.Log("sent "+mid+" bytes="+bytes.Length+" "+result);
		});
	}
}
