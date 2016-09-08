using UnityEngine;
using System.Collections;

public class CreatePanel : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}
	
	public void OnCreate(){
		Utils.Load<GamePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}

	public void OnJoin(){
		Utils.Load<GameKeyPopup>(gameObject.transform.parent,delegate(Component obj) {
			GameKeyPopup pop=obj as GameKeyPopup;
			if(pop)
				pop.goParent=gameObject;
		});
	}
}
