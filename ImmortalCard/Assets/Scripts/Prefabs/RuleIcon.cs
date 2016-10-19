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
		if(EnterPanel.Instance.Icon!=null)
			EnterPanel.Instance.Icon.OutLineEfx.SetActive(false);
		EnterPanel.Instance.Icon=this;
	}
}
