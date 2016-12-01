using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class GameIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	game;

	public void OnGame(){
		StartCoroutine(Main.Instance.updater.Load<EnterPanel>(
			"Prefabs/EnterPanel",Main.Instance.RootPanel,delegate(Object arg1, Hashtable arg2) {
			var lobby=arg1 as EnterPanel;
			lobby.CurrentGame=this;
			if(LobbyPanel.Instance)Destroy(LobbyPanel.Instance.gameObject);
		}));
	}
}
