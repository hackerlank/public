using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class PlayerIcon : MonoBehaviour {
	public Image	Icon;
	public Text		Name;
	public Text		Id;
	public Text		Level;
	public Text		Amount;

	public play_t Value{
		set{
			Name.text=value.Player.Pid.ToString();
			Id.text=value.Player.Uid.ToString();
			Level.text=value.Player.Level.ToString();
			Amount.text=value.Player.Gold.ToString();
		}
	}
	
}
