using UnityEngine;
using System.Collections;

public class GamePanel : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	public void OnExit(){
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}
}
