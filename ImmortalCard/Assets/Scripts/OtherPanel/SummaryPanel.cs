using UnityEngine;
using System.Collections;

public class SummaryPanel : MonoBehaviour {

	void Start () {
	
	}
	
	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController!=null)Main.Instance.gameController.OnExit();
	}

	public void OnShare(){
		Main.Instance.share.Share("Share","I'm fun!",cn.sharesdk.unity3d.ContentType.Image);
	}
}
