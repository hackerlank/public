using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class GameIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	game;

	public void OnGame(){
		var path="Prefabs/EnterPanel";
		StartCoroutine(Main.Instance.resourceUpdater.Load<EnterPanel>(path,delegate(Object arg1, Hashtable arg2) {
			var lobby=arg1 as EnterPanel;
			lobby.transform.SetParent(Main.Instance.RootPanel,false);
			lobby.CurrentGame=this;
			if(LobbyPanel.Instance)Destroy(LobbyPanel.Instance.gameObject);
		}));
		/*
		Utils.Load<EnterPanel>(Main.Instance.RootPanel,delegate(Component obj){
			var panel=obj as EnterPanel;
			panel.CurrentGame=this;
			if(LobbyPanel.Instance)Destroy(LobbyPanel.Instance.gameObject);
		});
		*/
	}
}
