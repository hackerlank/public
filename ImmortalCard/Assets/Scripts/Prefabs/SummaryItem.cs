using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class SummaryItem : MonoBehaviour {
	public PlayerIcon	Player;
	public Text			Wins,Points,HiScore,HiAchvs;
	public Text			Continuous,Winby,Title;

	public play_t Value{
		set{
			Points.text=value.Point.ToString();
			HiAchvs.text=value.Achvs.Count.ToString();
			Player.Score.text=value.Score.ToString();
			Player.Name.text="Player "+value.Seat;
			Player.Win.gameObject.SetActive(value.Win>0);
		}
	}
}
