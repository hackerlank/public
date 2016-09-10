using UnityEngine;
using System.Collections;
using System.IO;
using Proto3;

public class LoginPanel : MonoBehaviour {

	//public SocketIO.SocketIOComponent socket;
	public WebSocketSharp.WebSocket socket=new WebSocketSharp.WebSocket("ws://127.0.0.1:8820/100");

	// Use this for initialization
	void Start () {
		//socket=gameObject.AddComponent<SocketIO.SocketIOComponent>();
	}
	
	// Update is called once per frame
	void Update () {
	}
	
	public void OnLogin(){
		Debug.Log("----OnLogin");

		socket.OnOpen+=delegate(object sender, System.EventArgs e) {
			Debug.Log("----OnOpen");
		};
		socket.OnError+=delegate(object sender, WebSocketSharp.ErrorEventArgs e) {
			Debug.Log("----OnError: "+e.Message);
		};
		socket.OnMessage+=delegate(object sender, WebSocketSharp.MessageEventArgs e) {
			Debug.Log("----OnMessage");
			var data=e.Data;
			var raw=e.RawData;

			byte[] body=new byte[raw.Length-2];
			System.Buffer.BlockCopy(raw,2,body,0,body.Length);

			MsgBase baseMsg=MsgBase.Parser.ParseFrom(body);
			var mid=baseMsg.Mid;

			switch(mid){
			case 6002:
				MsgNCEnter imsg=MsgNCEnter.Parser.ParseFrom(raw);
				Debug.Log("entered game "+imsg.GameInfo.Gid+",uid="+imsg.GameInfo.Uid);
				break;
			default:
				break;
			}
		};
		socket.Connect();

		MsgCNEnter msg=new MsgCNEnter();
		msg.Mid=6001;
		msg.Version=100;
		msg.Key=66;

		var bytes=MsgIntepreter.EncodeBytes<MsgCNEnter>(msg);
		socket.SendAsync(bytes,delegate(bool obj) {
			Debug.Log("----SendAsync "+obj);
		});

		//socket.url="ws://127.0.0.1/100";
		//socket.Emit("hello");
		/*
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj){
			MsgCSLogin msg=new MsgCSLogin();
			msg.Mid=2001;
			msg.Version=100;
			msg.User=new user_t();
			msg.User.Account="Unity";
			msg.User.DevType=pb_enum.DevPc;
			msg.User.Name="vic";
			msg.User.Udid=SystemInfo.deviceUniqueIdentifier;

			Debug.Log("----DoLogin account="+msg.User.Account);

			Main.Instance.http.SetUri("http://127.0.0.1:8800");
			Main.Instance.http.Request<MsgCSLogin>((int)msg.Mid,msg);
			Destroy(gameObject);
		});
		*/
	}
}
