using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class ReplayView: MonoBehaviour {
	public Transform ReplaysList,ReplayList;

	public static MsgLCReplays msgReplays;
	public static MsgLCReplay msgReplayData;

	bool showDetail=false;

	IEnumerator Start(){
		while(null==msgReplays)
			yield return null;

		if(msgReplays.All==null || msgReplays.All.Count<=0)
			yield break;

		foreach(replay_item item in msgReplays.All){
			var param=new Hashtable();
			param["item"]=item;
			StartCoroutine(Main.Instance.updater.Load<ReplayItem>(
				"Prefabs/ReplayItem",ReplaysList,delegate(Object arg1, Hashtable arg2) {
				var i=arg1 as ReplayItem;
				i.Item=arg2["item"] as replay_item;
			},param));
		}
	}

	public void OnSelect(ReplayItem item) {
		showDetail=true;
		ReplaysList.parent.gameObject.SetActive(false);
		ReplayList.parent.gameObject.SetActive(true);

		var _item=item.Item;
		for(int i=0;i<_item.Rounds;++i){
			Hashtable param=new Hashtable();
			param["round"]=i;
			StartCoroutine(Main.Instance.updater.Load<ReplayItem>(
				"Prefabs/ReplayItem",ReplayList,delegate(Object obj,Hashtable arg){
				var round=(int)arg["round"];
				var j=obj as ReplayItem;
				j.SetData(_item,round);
			},param));
		}
	}
	
	public IEnumerator OnReplay(ReplayItem item) {
		MsgCLReplay msg=new MsgCLReplay();
		msg.Mid=pb_msg.MsgClReplay;
		msg.GameId=item.Item.GameId;
		msg.Session=Main.Instance.MainPlayer.session;
		msg.Round=item._round;
		Main.Instance.MainPlayer.http.Request<MsgCLReplay>(msg.Mid,msg);

		while(null==msgReplayData)
			yield return null;

		Destroy(gameObject);

		var data=msgReplayData.Data;
		var gameRule=item.Item.GameCategory;
		ReplayPanel panel=null;
		GameRule rule=null;
		System.Action<Component> handler=delegate(Component obj) {
			panel=obj as ReplayPanel;
			panel.Rule=rule;
			panel.StartCoroutine(panel.Play(data));
		};
		
		switch(gameRule){
		case pb_enum.GameMjChengdu:
			rule=new MahJongRule();
			ReplayPanel.Create("MahjongPanel",handler);
			break;
		case pb_enum.GameDdz:
			rule=new DoudeZhuRule();
			ReplayPanel.Create("MahjongPanel",handler);
			//PaohuziReplay.Create("DoudeZheReplay");
			break;
		case pb_enum.GamePhz:
		default:
			rule=new PaohuziRule();
			ReplayPanel.Create("PaohuziReplay",handler);
			break;
		}
	}

	public void OnBack() {
		if(showDetail){
			ReplayList.parent.gameObject.SetActive(false);
			ReplaysList.parent.gameObject.SetActive(true);
		}else{
			Destroy(gameObject);
		}
	}
}
