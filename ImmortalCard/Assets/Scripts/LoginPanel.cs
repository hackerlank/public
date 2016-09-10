using UnityEngine;
using System.Collections;
using System.IO;
using Proto3;

public class LoginPanel : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	}

	public void OnLogin(){
		Debug.Log("----OnLogin");
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj){
			TheMsg tmsg = new TheMsg();
			tmsg.Name = "am the name";
			tmsg.Num = 32;

			MsgCNEnter msg=new MsgCNEnter();
			MsgCSLogin ml=new MsgCSLogin();
			/*
			msg.Mid=2001;
			msg.Version=100;
			msg.Service=Proto3.pb_enum.GameCard;
			msg.Uid="Unity";
			*/

			Main.Instance.http.SetUri("http://127.0.0.1:8800");
			Main.Instance.http.Request<MsgCNEnter>((int)msg.Mid,msg);

			byte[] bmsg;
			MemoryStream ms=new MemoryStream();
			Google.Protobuf.CodedOutputStream co=new Google.Protobuf.CodedOutputStream(ms);
			msg.WriteTo(co);
			bmsg=ms.ToArray();

			MsgCNEnter msg2 = MsgCNEnter.Parser.ParseFrom(bmsg);
			MsgCNEnter.Parser.ToString();
			
			//Destroy(gameObject);
		});
	}
}
