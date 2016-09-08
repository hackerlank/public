using UnityEngine;
using System.Collections;

public class LoginPanel : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	public void OnLogin(){
		Debug.Log("----OnLogin");
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}
}
