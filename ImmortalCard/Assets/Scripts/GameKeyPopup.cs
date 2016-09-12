using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class GameKeyPopup : MonoBehaviour {
	public Text textKey;

	public void OnEnter(){
		Debug.Log("----game id="+textKey.text);
		try{
			var key=uint.Parse(textKey.text);
			Utils.Load<GamePanel>(gameObject.transform.parent,delegate(Component obj){
				if(CreatePanel.Instance){
					CreatePanel.Instance.Connect(key);
				}
				Destroy(gameObject);
			});
		}catch(System.Exception){
			Debug.LogError("Invalid key");
		}
	}
}
