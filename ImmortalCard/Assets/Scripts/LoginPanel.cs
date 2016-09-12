using UnityEngine;
using System.Collections;
using System.IO;
using Proto3;

public class LoginPanel : MonoBehaviour {

	//public SocketIO.SocketIOComponent socket;

	// Use this for initialization
	void Start () {
		//socket=gameObject.AddComponent<SocketIO.SocketIOComponent>();
	}
	
	// Update is called once per frame
	void Update () {
	}
	
	public void OnLogin(){
		Debug.Log("----OnLogin");

		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj){
			MsgCSLogin msg=new MsgCSLogin();
			msg.Mid=pb_msg.MsgCsLogin;
			msg.Version=100;
			msg.User=new user_t();
			msg.User.Account="Unity";
			msg.User.DevType=pb_enum.DevPc;
			msg.User.Name="vic";
			msg.User.Udid=SystemInfo.deviceUniqueIdentifier;

			Debug.Log("----DoLogin account="+msg.User.Account);

			Main.Instance.http.SetUri("http://127.0.0.1:8800");
			Main.Instance.http.Request<MsgCSLogin>(msg.Mid,msg);

			Destroy(gameObject);
		});
	}
}
