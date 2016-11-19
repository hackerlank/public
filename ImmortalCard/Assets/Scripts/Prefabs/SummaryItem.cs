using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class SummaryItem : MonoBehaviour {
	public PlayerIcon	player;
	public Text			Wins,Points,HiScore,HiAchvs;
	public Text			Continuous,Winby,Title;

	public play_t Value{
		set{
			player.Value=value;
			Points.text=value.Point.ToString();
			HiAchvs.text=value.Achvs.Count.ToString();
		}
	}
}
