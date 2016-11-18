using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public abstract class SettleItem : MonoBehaviour {
	public PlayerIcon	Players;

	virtual public play_t Value{
		set{
			Players.Score.text=value.Score.ToString();
			Players.Name.text="Player "+value.Seat;
			Players.Win.gameObject.SetActive(value.Win>0);
		}
	}
}
