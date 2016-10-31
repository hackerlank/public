using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class ZipaiBunch : MonoBehaviour {
	public Text		Type;
	public Text		Score;

	public bool ShowType=false;

	public bunch_t Value{
		set{
			//type and score
			if(ShowType){
				Type.text="K";
				Score.text="9";
				Type.gameObject.SetActive(true);
				Score.gameObject.SetActive(true);
			}
			//cards
			var ctrl=Main.Instance.gameController as GamePanel;
			foreach(var card in value.Pawns)
				Card.Create(ctrl.CardPrefab,card,transform);
		}
	}
}
