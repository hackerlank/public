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
			Score.text=value.Score.ToString();
			Point.text=value.Point.ToString();
			string achvs="Achvs";
			Achvs.text=achvs;
		}
	}
}
