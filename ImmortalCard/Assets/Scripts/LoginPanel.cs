using UnityEngine;
using System.Collections;
using System.IO;
using Proto3;

public class LoginPanel : MonoBehaviour {

	public static LoginPanel Instance=null;
	void Awake(){Instance=this;}
	void OnDestroy(){Instance=null;}
	
	public void OnLogin(){
		if(Main.Instance.GameMode==Main.Mode.STANDALONE){
			//only for testing
			DoudeZhuPanel.Create(delegate(Component obj){
				var gp=obj as DoudeZhuPanel;
				var msg=gp.Rule.Deal();

				gp.StartCoroutine(gp.Deal(msg));
				Destroy(gameObject);
			});
		}else if(Main.Instance.GameMode==Main.Mode.NODE)
			DoLogin();
		else{
			MsgCSLogin msg=new MsgCSLogin();
			msg.Mid=pb_msg.MsgCsLogin;
			msg.Version=100;
			msg.User=new user_t();
			msg.User.Account="Unity";
			msg.User.DevType=pb_enum.DevPc;
			msg.User.Name="vic";
			msg.User.Udid=SystemInfo.deviceUniqueIdentifier;
			
			//Debug.Log("----DoLogin account="+msg.User.Account);
			Main.Instance.http.SetUri(Configs.uri);
			Main.Instance.http.Request<MsgCSLogin>(msg.Mid,msg);
		}
	}

	public void DoLogin(){
		Destroy(gameObject);
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj){
			//Destroy(gameObject);
		});
	}
}
