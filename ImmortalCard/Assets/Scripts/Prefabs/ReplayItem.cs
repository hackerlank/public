using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class ReplayItem : MonoBehaviour {

	public Text		Id;
	public Text		Name;
	public Text		MaxRound;
	public Text		Round;
	public Text		Date;

	public Text[]	Scores;

	replay_item _item;

	[HideInInspector]
	public int _round;

	ReplayView view;
	void Awake(){
		var parent=transform.parent.parent;
		view=parent.gameObject.GetComponent<ReplayView>();
	}

	public replay_item Item{
		set{
			_item=value;
			//Id,GameName,MaxRound,Scores,Date
			Id.text		="ID: "+value.GameId;
			Name.text	=RuleIcon.rule2name(value.GameCategory);
			Date.text	=Utils.timestamp_date(value.Timestamp).ToString("yyyy-MM-dd hh:mm");
			MaxRound.text=value.Rounds.ToString();

			Scores[3].gameObject.SetActive(false);
			for(int i=0;i<value.Total.Count;++i){
				key_value kv=value.Total[i];
				Scores[i].text=kv.Key+" : "+kv.Ivalue.ToString();
				Scores[i].gameObject.SetActive(true);
			}

			//hide
			Round.gameObject.SetActive(false);
		}
		get{
			return _item;
		}
	}

	public void SetData(replay_item item,int round){
		_item=item;
		_round=round;

		//Round,Scores
		Round.text	=round.ToString();

		Scores[3].gameObject.SetActive(false);
		var start=round * item.Total.Count;
		for(int i=start;i<item.Total.Count;++i){
			var score=item.RoundScores[i];
			key_value kv=item.Total[i-start];
			Scores[i].text=kv.Key+" : "+score.ToString();
			Scores[i].gameObject.SetActive(true);
		}

		//hide
		Id.gameObject.SetActive(false);
		Name.gameObject.SetActive(false);
		MaxRound.gameObject.SetActive(false);
		Date.gameObject.SetActive(false);
	}

	public void OnTap() {
		if(Id.gameObject.activeSelf){
			view.OnSelect(this);
		}else{
			view.OnReplay(this);
		}
	}
}
