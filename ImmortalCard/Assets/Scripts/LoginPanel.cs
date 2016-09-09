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
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			MsgCNEnter msg=new MsgCNEnter();
			msg.Mid=6001;
			msg.Version=100;
			msg.Service=Proto3.pb_enum.GameCard;
			msg.Uid="Unity";
			/*
			byte[] bmsg;
			using (MemoryStream ms = new MemoryStream()){
				msg.WriteTo(ms);
				bmsg = ms.ToArray();
			}

			MsgCNEnter msg2 = MsgCNEnter.Parser.ParseFrom(bmsg);
			*/
			MsgCNEnter.Parser.ToString();
			
			Destroy(gameObject);
		});
	}
}
