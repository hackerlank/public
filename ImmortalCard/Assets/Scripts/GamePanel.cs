using UnityEngine;
using System.Collections;

public class GamePanel : MonoBehaviour {
	public Transform DeckArea;
	public Transform LDiscardArea,MDiscardArea,RDiscardArea;

	public static GamePanel	Instance=null;
	void Awake(){
		Instance=this;
	}

	void OnDestroy(){
		Instance=null;
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
