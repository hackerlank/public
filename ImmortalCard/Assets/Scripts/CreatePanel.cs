using UnityEngine;
using System.Collections;
using Proto3;

public class CreatePanel : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}
	
	public void OnCreate(){
		Utils.Load<GamePanel>(gameObject.transform.parent,delegate(Component obj) {
			Main.Instance.ws.Connect("ws://127.0.0.1:8820/100");

			MsgCNEnter msg=new MsgCNEnter();
			msg.Mid=pb_msg.MsgCnEnter;
			msg.Version=100;
			msg.Key=66;

			Main.Instance.ws.Send<MsgCNEnter>(msg.Mid,msg);

			Destroy(gameObject);
		});
	}

	public void OnJoin(){
		Utils.Load<GameKeyPopup>(gameObject.transform.parent,delegate(Component obj) {
			GameKeyPopup pop=obj as GameKeyPopup;
			if(pop)
				pop.goParent=gameObject;
		});
	}
}
