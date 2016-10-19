using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class RuleIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	GameId;
	public GameObject	OutLineEfx;

	public void OnGame(){
		OutLineEfx.SetActive(true);
		if(CreatePanel.Instance.Icon!=null)
			CreatePanel.Instance.Icon.OutLineEfx.SetActive(false);
		CreatePanel.Instance.Icon=this;
	}
}
