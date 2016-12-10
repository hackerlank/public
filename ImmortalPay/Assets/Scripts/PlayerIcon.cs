using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class PlayerIcon : MonoBehaviour {
	public Image	Icon;
	public Image	Win;
	public Text		Name;
	public Text		Id;
	public Text		Score;
	public Text		Total;

	public play_t Value{
		set{
			Name.text="Player "+value.Seat;
			Score.text=value.Score.ToString();
			Total.text=value.Total.ToString();
			Win.gameObject.SetActive(value.Win>0);
		}
	}
	
}
