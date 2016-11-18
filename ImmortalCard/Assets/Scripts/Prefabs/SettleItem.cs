using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public abstract class SettleItem : MonoBehaviour {
	public PlayerIcon	Players;
	public Transform	Cards;
	public Text			Point,Score,Achvs;

	virtual public play_t Value{
		set{
			if(Score!=null)Score.text=value.Score.ToString();
			if(Point!=null)Point.text=value.Point.ToString();
			if(Achvs!=null)Achvs.text="无";

			Players.Score.text=value.Score.ToString();
			Players.Name.text="Player "+value.Seat;
			Players.Win.gameObject.SetActive(value.Win>0);
		}
	}
}
