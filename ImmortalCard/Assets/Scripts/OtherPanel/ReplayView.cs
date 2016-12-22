using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ReplayView: MonoBehaviour {
	public Transform ReplaysList,ReplayList;

	public void OnSelect(ReplayItem item) {
		var _item=item.Item;
		for(int i=0;i<_item.Rounds;++i){
			Hashtable param=new Hashtable();
			param["round"]=i;
			/*
				StartCoroutine(Main.Instance.updater.Load<ReplayItem>(
					"Prefabs/ReplayItem",DetailRoot,delegate(Object obj,Hashtable arg){
					var round=(int)arg["round"];
					var item=obj as ReplayItem;
					item.SetData(_item,round);
				},param));
				*/
		}
	}
	
	public void OnReplay(ReplayItem item) {
		Destroy(gameObject);

		ReplayView.Create("ReplayPanel");
	}
	
	public void OnBack() {
		
	}

	public static void Create(string path,System.Action<Component> handler=null){
		Main.Instance.StartCoroutine(Main.Instance.updater.Load<ReplayPanel>(
			"Prefabs/"+path,Main.Instance.RootPanel,delegate(Object obj,Hashtable arg){
			if(handler!=null)handler.Invoke(obj as Component);
		}));
	}
}
