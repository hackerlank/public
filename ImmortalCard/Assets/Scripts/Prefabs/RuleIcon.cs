using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class RuleIcon : MonoBehaviour {

	public Image	Icon;
	public Text		Name;
	public pb_enum	Category;
	public GameObject	OutLineEfx;

	public void OnGame(){
		if(EnterPanel.Instance.GameCategory!=null && EnterPanel.Instance.GameCategory!=this)
			EnterPanel.Instance.GameCategory.OutLineEfx.SetActive(false);
		OutLineEfx.SetActive(true);
		EnterPanel.Instance.GameCategory=this;
	}
}
