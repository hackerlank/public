using UnityEngine;
using System.Collections;

public class SettlePopup : MonoBehaviour {

	void Start () {
	
	}
	
	public void OnClose(){
		Destroy(gameObject);
	}
}
