using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class SummaryPanel : MonoBehaviour {
	public Transform	Items;

	IEnumerator Start () {
		var ctrl=Main.Instance.gameController as GamePanel;
		if(null==ctrl)yield break;
		while(null==ctrl.Summary)yield return null;

		var bestScore=0;
		SummaryItem bestItem=null;
		foreach(play_t play in ctrl.Summary.Play){
			SummaryItem item=null;
			yield return StartCoroutine(Main.Instance.updater.Load<SummaryItem>(
				"Prefabs/SummaryItem",Items,delegate(Object obj,Hashtable arg) {
				item=obj as SummaryItem;
			}));

			item.Value=play;

			if(play.Total>bestScore){
				bestScore=play.Total;
				bestItem=item;
			}
		}
		if(null!=bestItem)
			bestItem.player.Win.gameObject.SetActive(true);
	}
	
	public void OnClose(){
		Destroy(gameObject);
		var panel=Main.Instance.gameController as GamePanel;
		if(panel!=null)panel.Dismiss();
	}

	public void OnShare(){
		Main.Instance.share.Share("Share","I'm fun!",cn.sharesdk.unity3d.ContentType.Image);
	}
}
