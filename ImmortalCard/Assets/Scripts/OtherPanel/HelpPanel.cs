using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class HelpPanel : MonoBehaviour {
	public Transform	GameRoot;
	public Text			Content;

	[HideInInspector]
	public pb_enum	CurrentGame;
	[HideInInspector]
	public RuleIcon	GameCategory;

	IEnumerator Start(){
		if(!Config.games.ContainsKey(CurrentGame))
			yield break;

		while(null==RuleSprites.Instance)yield return null;

		var categories=Config.games[CurrentGame];
		foreach(game_t game in categories){
			var param=new Hashtable();
			param["rule"]=game.Rule%100;
			StartCoroutine(Main.Instance.updater.Load<RuleIcon>(
				"Prefabs/RuleIcon",GameRoot,delegate(Object obj,Hashtable arg){
				var icon=obj as RuleIcon;
				icon.Value=(pb_enum)arg["rule"];

				//default selection
				if(GameCategory==null){
					GameCategory=icon;
					GameCategory.OnGame();
				}
			},param));
			yield return null;
		}
	}

	public void OnClose(){
		Destroy(gameObject);
	}
}
