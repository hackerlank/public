using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public abstract class SettleItem : MonoBehaviour {
	public PlayerIcon	player;

	virtual public play_t Value{
		set{
			player.Value=value;
		}
	}
}
