using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GameHud : MonoBehaviour {

	public void OnSettings(){
		var panel=Main.Instance.gameController as GamePanel;
		if(null!=panel)
			panel.OnSettings();
	}

	public void OnBack(){
		var panel=Main.Instance.gameController as GamePanel;
		if(null!=panel)
			panel.OnBack();
	}

	public void OnExit(){
		var panel=Main.Instance.gameController as GamePanel;
		if(null!=panel)
			panel.OnExit();
	}
}
