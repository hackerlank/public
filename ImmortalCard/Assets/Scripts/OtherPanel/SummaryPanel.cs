using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class SummaryPanel : MonoBehaviour {
	public Text			Title;
	public Transform	Items;

	IEnumerator Start () {
		var ctrl=Main.Instance.gameController as GamePanel;
		if(null==ctrl)yield break;
		while(null==ctrl.Summary)yield return null;

		var bestScore=0;
		SummaryItem bestItem=null;
		foreach(play_t play in ctrl.Summary.Play){
			SummaryItem item=null;
			Utils.Load<SummaryItem>(Items,delegate(Component obj) {
				item=obj as SummaryItem;
			});
			while(null==item)yield return null;
			item.Value=play;

			if(play.Total>bestScore){
				bestScore=play.Total;
				bestItem=item;
			}
		}
		bestItem.player.Win.gameObject.SetActive(true);
	}
	
	public void OnClose(){
		Destroy(gameObject);
		if(Main.Instance.gameController!=null)Main.Instance.gameController.OnExit();
	}

	public void OnShare(){
		Main.Instance.share.Share("Share","I'm fun!",cn.sharesdk.unity3d.ContentType.Image);
	}
}
