using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ReplayView: MonoBehaviour {
	public Transform ReplaysList,ReplayList;
	
	public void OnSelect(ReplaysItem item) {
		
	}
	
	public void OnReplay(ReplayItem item) {
		
	}
	
	public void OnRemove(ReplaysItem item) {
		
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
