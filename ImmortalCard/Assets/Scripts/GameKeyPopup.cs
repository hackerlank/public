using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GameKeyPopup : MonoBehaviour {
	public Text textKey;
	[HideInInspector]
	public GameObject goParent;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	public void OnEnter(){
		Debug.Log("----game id="+textKey.text);
		Utils.Load<GamePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(goParent);
			Destroy(gameObject);
		});
	}
}
