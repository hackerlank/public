using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class GameIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	GameId;

	public void OnGame(){
		var canvas=gameObject.GetComponentInParent<Canvas>();
		Utils.Load<CreatePanel>(canvas.transform,delegate(Component obj){
			var panel=obj as CreatePanel;
			panel.CurrentGame=this;
			if(LobbyPanel.Instance)Destroy(LobbyPanel.Instance.gameObject);
		});
	}
}
