using UnityEngine;
using System.Collections;

public class SummaryPanel : MonoBehaviour {

	void Start () {
	
	}
	
	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController!=null)Main.Instance.gameController.OnExit();
	}
}
