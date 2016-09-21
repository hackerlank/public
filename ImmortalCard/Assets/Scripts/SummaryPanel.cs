using UnityEngine;
using System.Collections;

public class SummaryPanel : MonoBehaviour {

	void Start () {
	
	}
	
	public void OnClose(){
		Destroy(gameObject);
		if(GamePanel.Instance!=null)GamePanel.Instance.OnExit();
	}
}
