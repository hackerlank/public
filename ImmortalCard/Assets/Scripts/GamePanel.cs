using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

public class GamePanel : MonoBehaviour {
	[HideInInspector]
	public int			N=3;
	public List<Card>[]	HandCards;
	public Card[]		BottomCards;

	public Transform	HandArea;
	public Transform[]	DiscardAreas;	//MRL
	public PlayerIcon[]	Players;
	public Text[]		nHandCards;
	public Text			Ante,Multiples,Infomation;

	public static GamePanel	Instance=null;
	void Awake(){
		Players=new PlayerIcon[N];

		Instance=this;
	}

	void OnDestroy(){
		Instance=null;
	}

	// Update is called once per frame
	void Update () {
	
	}

	public void OnExit(){
		Utils.Load<CreatePanel>(gameObject.transform.parent,delegate(Component obj) {
			Destroy(gameObject);
		});
	}
}
