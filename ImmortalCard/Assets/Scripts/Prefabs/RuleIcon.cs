using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class RuleIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	GameId;

	public void OnGame(){
		CreatePanel.Instance.Icon=this;
	}
}
