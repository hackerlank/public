using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Main : MonoBehaviour {

	public static Main		Instance=null;

	public const int		Round=2;
	public StoreGame		storeGame;	//store game for reconnect

	public GameController	gameController=null;
	public ShareAPI			share;

	public Transform		RootPanel;
	public Animator			spinner;

	public enum Mode{
		STANDALONE,
		NODE,
		NORMAL,
	}
	public Mode GameMode{get{return Mode.NODE;}}

	void Awake(){
		Instance=this;
		//Loom
		gameObject.AddComponent<Loom>();
		MainPlayer=new Player();
		MainPlayer.playData=new Proto3.play_t();
		MainPlayer.playData.Player=new Proto3.player_t();
		MainPlayer.playData.Player.Uid=SystemInfo.deviceUniqueIdentifier;
		share=new ShareAPI();
	}

	void Start () {
	}
	
	void Update () {
	}

	public bool Wait{
		get{
			return Main.Instance.spinner.gameObject.activeSelf;
		}
		set{
			Main.Instance.spinner.gameObject.SetActive(value);
		}
	}

	public void StartWait(float t=5){
		if(!waiting){
			waiting=true;
			StartCoroutine(waitCo(t));
		}
	}
	public void StopWait(){
		StopCoroutine(waitCo(0));
		Wait=false;
		waiting=false;
	}

	bool waiting=false;
	IEnumerator waitCo(float t){
		yield return new WaitForSeconds(t);
		Wait=true;
	}

	public Player MainPlayer=new Player();
	public List<Player> robots=new List<Player>();
}
